/* node.c — a virtual Herus, and the rules by which frames survive the air.
 *
 * Everything a node does with a frame is done by the shipping firmware:
 * link_send seals it, session_open opens it, weave_offer decides whether to
 * relay it, hz_airtime_us says how long it occupies the band. This file decides
 * only what physics decides — who could hear it, whether two of them landed on
 * top of each other, and what it cost the battery.
 *
 * THE RECEIVE RULE, IN ORDER, AND WHY THAT ORDER
 * ----------------------------------------------
 *   1. Is the receiver's radio open at the instant the PREAMBLE arrives? A
 *      duty-cycled receiver that wakes mid-frame has missed it — SX1262 duty
 *      cycling detects preambles, not payloads. This is where latency comes from
 *      and it is checked first because it is free.
 *   2. Is the signal above sensitivity? Free.
 *   3. Did something else overlap it on the same channel? Costs a scan of recent
 *      transmissions, and applies LoRa's ~6 dB capture threshold rather than an
 *      "any overlap kills both" rule, which would libel the modulation.
 *   4. Only now is any crypto attempted, and only on the sessions whose address
 *      window matches. That ordering is the firmware's, not the simulator's.
 */
#include "sim.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* The four roles every unit ships knowing. A newer sender's fifth role is
 * dropped by link_recv and the message still arrives (P4). */
static const uint8_t KNOWN_ROLES[] = { 1, 2, 3, 4 };

/* ------------------------------------------------------------- provenance */
uint32_t sim_fingerprint(const uint8_t *frame, size_t len)
{
    /* Byte-for-byte weave.c's fingerprint(): FNV-1a with the ttl bits of byte 1
     * masked off, so a frame at ttl 3 and the same frame at ttl 2 are one key. */
    uint32_t h = 2166136261u;
    for (size_t i = 0; i < len; i++) {
        uint8_t b = frame[i];
        if (i == 1) b = (uint8_t)(b & 0x3f);
        h = (h ^ b) * 16777619u;
    }
    return h ? h : 1u;
}

static sim_prov *prov_slot(sim_world *w, uint32_t fp, int create)
{
    uint32_t h = (fp * 2654435761u) % SIM_PROV_N;
    for (int i = 0; i < SIM_PROV_N; i++) {
        sim_prov *p = &w->prov[(h + (uint32_t)i) % SIM_PROV_N];
        if (p->used && p->fp == fp) return p;
        if (!p->used) {
            if (!create) return NULL;
            p->used = 1; p->fp = fp;
            return p;
        }
    }
    return NULL;
}

/* ------------------------------------------------------------- delivery set
 * A message may arrive twice — directly and via a relay. Only the first arrival
 * is a delivery; the rest are the mesh doing its job. */
#define DSET_N 16384
static uint32_t dset[DSET_N];
static uint8_t  dused[DSET_N];

static void dset_reset(void) { memset(dused, 0, sizeof dused); }

static int dset_first(uint32_t key)
{
    uint32_t h = (key * 2654435761u) % DSET_N;
    for (int i = 0; i < DSET_N; i++) {
        uint32_t k = (h + (uint32_t)i) % DSET_N;
        if (!dused[k]) { dused[k] = 1; dset[k] = key; return 1; }
        if (dset[k] == key) return 0;
    }
    return 0;   /* table full: refuse to count rather than count twice */
}

static uint32_t dkey(int orig, int dst, int seq)
{
    return (uint32_t)orig * 1000000u + (uint32_t)dst * 10000u
         + (uint32_t)(seq & 0x1fff);
}

/* ------------------------------------------------------------ pending sends */
typedef struct {
    int      src, dst;
    uint16_t intent;
    int      nslot;
    uint8_t  roles[HCP_MAX_SLOT];
    uint16_t fillers[HCP_MAX_SLOT];
    uint8_t  ttl;
    uint64_t born_us;
    int      waited;
    /* A retry is a NEW sealed frame carrying the SAME application seq — see the
     * long note in link.h. The seq is therefore assigned once, not per copy. */
    uint16_t seq;
    int      seq_set, copies_left;
    int      defers;                 /* how many slots CAD has already cost us */
} pend_t;

static pend_t *pend;
static int npend, cpend;

void sim_pending_reset(void) { npend = 0; }

/* ------------------------------------------------------------------ pairing */
void sim_pair(sim_world *w, int a, int b, uint64_t seed)
{
    uint8_t root[32];
    /* Mix, do not XOR. The first version of this line was
     *     s = seed ^ (lo << 32) ^ hi
     * and scenario S4 caught it: with seed = base + i, (base + i) ^ i collides
     * often for small i, so several "different" pairs shared a root and the
     * measured address-collision rate came out 3x below the birthday model.
     * The bug was the bench's, not the firmware's — which is exactly why the
     * bench checks its own numbers against closed form before reporting them. */
    int lo = a < b ? a : b, hi = a < b ? b : a;
    uint64_t s = seed;
    for (int r = 0; r < 2; r++) {
        s ^= 0x9E3779B97F4A7C15ull * (uint64_t)(lo + 1);
        s = (s ^ (s >> 30)) * 0xBF58476D1CE4E5B9ull;
        s ^= 0xC2B2AE3D27D4EB4Full * (uint64_t)(hi + 1);
        s = (s ^ (s >> 27)) * 0x94D049BB133111EBull;
        s ^= s >> 31;
    }
    for (int i = 0; i < 32; i += 8) {
        uint64_t z = (s += 0x9E3779B97F4A7C15ull);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        z ^= z >> 31;
        memcpy(root + i, &z, 8);
    }
    /* One ratchet per pair, mirrored: A's send chain must be B's receive chain,
     * so exactly one end is the initiator. Getting this wrong on both ends fails
     * loudly (nothing ever opens) rather than quietly. */
    session_init(&w->n[a].sess[b], root, a < b, 0);
    session_init(&w->n[b].sess[a], root, b < a, 0);
    w->n[a].paired[b] = 1;
    w->n[b].paired[a] = 1;
}

static int hop_channel(const sim_world *w, const sim_node *n);

/* Listen before talk. Models the SX1262's SetCad: is there LoRa energy on our
 * channel right now, loud enough that transmitting would collide with it?
 *
 * CAD needs a few dB more than demodulation does — it is deciding from a couple
 * of symbols, not a whole frame — so the threshold here is deliberately 3 dB
 * above sensitivity. Modelling CAD as perfect would have manufactured a benefit
 * the radio cannot deliver. */
int sim_medium_busy(sim_world *w, int id)
{
    const sim_node *n = &w->n[id];
    int chan = hop_channel(w, n);
    for (int i = 0; i < SIM_MAX_INFLIGHT; i++) {
        const sim_tx *t = &w->air[i];
        if (!t->active || t->src == id) continue;
        if (t->t0_us > w->now_us || t->t1_us <= w->now_us) continue;   /* not on air now */
        if (t->chan != chan) continue;
        if (sim_rssi_dbm(w, t->src, id, 0.0) >= sim_sens_dbm(t->sf) + SIM_MARGIN_DB + 3.0)
            return 1;
    }
    return 0;
}

/* --------------------------------------------------------------- transmit */
static int air_slot(sim_world *w)
{
    for (int i = 0; i < SIM_MAX_INFLIGHT; i++) {
        sim_tx *t = &w->air[i];
        /* Finished transmissions are kept for a second so a later frame can
         * still discover that it overlapped them. */
        if (!t->active || t->t1_us + 1000000u < w->now_us) return i;
    }
    return -1;
}

static int hop_channel(const sim_world *w, const sim_node *n)
{
    if (w->ch.nchan <= 1) return 0;
    uint64_t slot = sim_local_us(n, w->now_us) / ((uint64_t)n->period_ms * 1000u);
    /* A group hops together, so this separates GROUPS and does nothing at all
     * for two members of one group transmitting at once. Modelling it any other
     * way would invent collision relief the design does not have. */
    return (int)((slot + (uint64_t)n->group * 7u) % (uint64_t)w->ch.nchan);
}

int sim_start_tx(sim_world *w, int src, const uint8_t *frame, int len,
                 uint8_t sf, int dst, int seq_app, int hops, uint64_t born_us)
{
    int i = air_slot(w);
    if (i < 0) return -1;
    sim_tx *t = &w->air[i];
    memset(t, 0, sizeof *t);
    t->active = 1;
    t->src   = src;
    t->sf    = sf;
    t->len   = len;
    memcpy(t->frame, frame, (size_t)len);
    t->chan  = hop_channel(w, &w->n[src]);
    t->t0_us = w->now_us;
    t->t1_us = w->now_us + hz_airtime_us(sf, (uint16_t)len, 1, 0, HERUS_CR);
    t->dst = dst; t->seq_app = seq_app; t->hops = hops; t->born_us = born_us;

    sim_prov *p = prov_slot(w, sim_fingerprint(frame, (size_t)len), 1);
    if (p && p->born == 0) {
        p->orig = src; p->dst = dst; p->seq = seq_app;
        p->hops = hops; p->born = born_us ? born_us : 1;
    }

    w->n[src].uah_tx += sim_i_tx_ma(SIM_TX_DBM)
                      * (double)(t->t1_us - t->t0_us) / 3.6e9 * 1e3;
    sim_push(w, t->t1_us, EV_TX_END, i, 0, 0);
    return i;
}

/* --------------------------------------------------------------- app send */
void sim_queue_send(sim_world *w, uint64_t t_us, int src, int dst,
                    uint16_t intent, int nslot, const uint8_t *roles,
                    const uint16_t *fillers, uint8_t ttl)
{
    if (npend == cpend) {
        cpend = cpend ? cpend * 2 : 256;
        pend = realloc(pend, (size_t)cpend * sizeof *pend);
    }
    pend_t *p = &pend[npend];
    memset(p, 0, sizeof *p);
    p->src = src; p->dst = dst; p->intent = intent; p->nslot = nslot;
    p->ttl = ttl; p->born_us = t_us;
    for (int i = 0; i < nslot; i++) { p->roles[i] = roles[i]; p->fillers[i] = fillers[i]; }
    sim_push(w, t_us, EV_APP_SEND, npend, 0, 0);
    npend++;
}

static void do_app_send(sim_world *w, int idx)
{
    pend_t *p = &pend[idx];
    sim_node *s = &w->n[p->src];

    /* Wait for the sender's own Beat boundary, plus a small random offset so two
     * senders in a group do not deterministically stack. The offset must stay
     * inside BEAT_RX_MS or the receiver's window has already closed — which is
     * precisely why a slotted design has so little room for backoff. */
    if (!p->waited) {
        p->waited = 1;
        uint64_t t = sim_next_slot_us(s, w->now_us);
        /* The offset is protocol, not a simulator convenience: beat.h defines it
         * so that both ends can agree on its mean. See finding B1. */
        t += (uint64_t)(sim_rand01(w) * (double)BEAT_TX_JITTER_MS * 1000.0);
        sim_push(w, t, EV_APP_SEND, idx, 0, 0);
        return;
    }

    if (!p->seq_set) {
        p->seq = s->app_seq[p->dst]++;
        p->seq_set = 1;
        p->copies_left = w->retries < 1 ? 1 : w->retries;
    }

    /* Listen before talk. A busy channel means the frame would be lost anyway,
     * so the slot skip is not a cost — it is the collision, priced correctly. */
    if (w->cad && p->defers < (int)BEAT_CAD_MAX_DEFER && sim_medium_busy(w, p->src)) {
        p->defers++;
        w->g_cad_defer++;
        uint64_t t = sim_next_slot_us(s, w->now_us);
        uint32_t skip = (uint32_t)(sim_rand(w) % BEAT_CAD_SPREAD);
        t += (uint64_t)skip * (uint64_t)s->period_ms * 1000u;
        t += (uint64_t)(sim_rand01(w) * (double)BEAT_TX_JITTER_MS * 1000.0);
        sim_push(w, t, EV_APP_SEND, idx, 0, 0);
        return;
    }

    hcp_msg_t m = {0};
    m.tier   = p->nslot ? HCP_TIER_COMPOSED : HCP_TIER_GLYPH;
    m.intent = p->intent;
    m.nslot  = (uint8_t)p->nslot;
    m.seq    = p->seq;
    for (int i = 0; i < p->nslot; i++) {
        m.slot[i].role   = p->roles[i];
        m.slot[i].filler = p->fillers[i];
    }

    herus_link l = { &s->sess[p->dst], HZ_REGION_BR915, (uint8_t)SIM_TX_DBM, KNOWN_ROLES, 4,
                     NULL, s->profile };
    const hz_link_profile_t *P = link_profile(&l);
    uint8_t frame[SIM_FRAME_MAX];
    if (link_send(&l, &m, p->ttl, frame) != LINK_OK) return;

    s->uah_mcu += SIM_I_MCU_ACTIVE * SIM_MCU_MS_SEND / 3.6e3;
    s->n_sent++;
    if (p->copies_left == (w->retries < 1 ? 1 : w->retries)) w->g_offered++;
    sim_start_tx(w, p->src, frame, P->frame_len, P->sf,
                 p->dst, m.seq, 0, p->born_us);

    /* Another copy, at the next slot. A different sealed frame, the same message. */
    if (--p->copies_left > 0) {
        p->waited = 0;
        sim_push(w, w->now_us + 1000, EV_APP_SEND, idx, 0, 0);
    }
}

/* ---------------------------------------------------------------- receive */
static int collided(sim_world *w, int self_slot, int rx, double rssi_self)
{
    const sim_tx *me = &w->air[self_slot];
    for (int i = 0; i < SIM_MAX_INFLIGHT; i++) {
        if (i == self_slot) continue;
        const sim_tx *o = &w->air[i];
        if (!o->active || o->src == rx) continue;
        if (o->chan != me->chan) continue;
        if (o->t1_us <= me->t0_us || o->t0_us >= me->t1_us) continue;   /* no overlap */
        double r = sim_rssi_dbm(w, o->src, rx, 0.0);
        if (r < sim_sens_dbm(o->sf) + SIM_MARGIN_DB) continue;          /* too weak to matter */
        if (rssi_self - r < SIM_CAPTURE_DB) return 1;                   /* no capture */
    }
    return 0;
}

static void deliver_to(sim_world *w, int slot, int rx)
{
    sim_tx   *t = &w->air[slot];
    sim_node *r = &w->n[rx];

    double shadow = 0.0;
    if (w->ch.shadow_db > 0.0) shadow = sim_gauss(w) * w->ch.shadow_db;

    double rssi = sim_rssi_dbm(w, t->src, rx, shadow);
    if (rssi < sim_sens_dbm(t->sf) + SIM_MARGIN_DB) { w->g_below_sens++; return; }
    if (!sim_can_hear(r, t->t0_us, t->sf))          { w->g_asleep++;     return; }
    if (collided(w, slot, rx, rssi))                { w->g_collisions++; return; }

    /* The radio stays on for the whole frame once it has locked a preamble; the
     * duty-cycle scan only paid for BEAT_RX_MS of it. */
    uint64_t air = t->t1_us - t->t0_us;
    if (r->weave.role != WEAVE_RELAY && air > (uint64_t)BEAT_RX_MS * 1000u)
        r->rx_extra_us += air - (uint64_t)BEAT_RX_MS * 1000u;
    r->uah_mcu += SIM_I_MCU_ACTIVE * SIM_MCU_MS_RECV / 3.6e3;

    uint8_t frame[SIM_FRAME_MAX];
    memcpy(frame, t->frame, (size_t)t->len);

    /* Marginal link: inject a bit error. The AEAD must reject it — a corrupted
     * frame that is DELIVERED would be a false delivery, and w->g_false_deliveries
     * is the number that must never move. */
    double margin = rssi - (sim_sens_dbm(t->sf) + SIM_MARGIN_DB);
    if (margin < 2.0 && sim_rand01(w) < (2.0 - margin) * 0.25) {
        int bit = (int)(sim_rand(w) % (uint64_t)(t->len * 8));
        frame[bit / 8] ^= (uint8_t)(1u << (bit % 8));
        r->n_corrupt_seen++;
    }

    /* An adversary in range captures the ciphertext. That is all it gets, and
     * the attack scenario proves that is all it can do with it. */
    if (r->adversary) {
        memcpy(r->cap, frame, (size_t)t->len);
        r->cap_len = t->len;
        return;
    }

    const sim_prov *pv = prov_slot(w, sim_fingerprint(t->frame, (size_t)t->len), 0);
    int  orig = pv ? pv->orig : t->src;
    int  odst = pv ? pv->dst  : t->dst;
    int  oseq = pv ? pv->seq  : t->seq_app;
    uint64_t born = pv && pv->born ? pv->born : t->born_us;

    /* Which of my sessions, if any, is this addressed to? The address-window
     * scan is the cheap reject; crypto is only attempted after it matches. */
#define ACCEPT(OUT) do {                                                       \
        r->n_opened++;                                                         \
        sim_discipline(r, t->t0_us);                                           \
        beat_resync(&r->beat, w->now_us / 1000u, 0);                           \
        if (odst != rx) { w->g_false_deliveries++; }                           \
        else if (dset_first(dkey(orig, rx, oseq))) {                           \
            uint64_t lat = w->now_us - born;                                   \
            w->g_delivered++;  w->lat_sum_us += lat;  w->lat_n++;              \
            if (lat > w->lat_max_us) w->lat_max_us = lat;                      \
            if (w->verbose) {                                                  \
                char line[192];                                                \
                render_msg(line, sizeof line, &(OUT), r->lang);                \
                printf("      t=%7.2fs  %d -> %d  hops=%d  %6.0f ms  [%s] %s\n",\
                       (double)w->now_us / 1e6, orig, rx, t->hops,             \
                       (double)lat / 1e3, lang_name(r->lang), line);           \
            }                                                                  \
        }                                                                      \
    } while (0)
    int mine = 0, opened = 0;
    for (int p = 0; p < w->nn && !opened; p++) {
        if (!r->paired[p]) continue;
        if (!session_addr_in_window(&r->sess[p], frame)) continue;
        mine = 1;
        herus_link l = { &r->sess[p], HZ_REGION_BR915, (uint8_t)SIM_TX_DBM, KNOWN_ROLES, 4,
                         &r->seen[p], r->profile };
        hcp_msg_t out; uint32_t ctr; int se = 0;
        int rc = link_recv(&l, frame, w->now_us / 1000u, &out, &ctr, &se);
        if (rc == LINK_OK) {
            opened = 1;
            ACCEPT(out);
        } else if (rc == LINK_E_DUP) {
            /* Opened, authenticated, decoded — and already delivered once. A
             * retry doing exactly what it exists to do. The user sees nothing.
             *
             * The clock does, though. Discarding the MESSAGE does not make its
             * arrival instant a worse time reference: it is authenticated, it
             * came from our peer, and it landed on a boundary. Throwing that away
             * cost 10 of 480 messages in this bench before it was noticed. */
            opened = 1;
            r->n_dup++;
            w->g_dup_suppressed++;
            sim_discipline(r, t->t0_us);
        } else if (se == SESS_E_AUTH) {
            r->n_auth_fail++;
        }
    }

    /* No session claimed the address. Either it is a stranger's frame — the
     * common case, and free — or it is our own peer speaking from beyond a
     * window we have fallen out of. session_recover's two gates (link silent for
     * SESS_LOST_MS, and one attempt per cooldown) decide which, cheaply enough
     * that asking on every unclaimed frame is affordable. */
    if (!mine) {
        r->n_missed_addr++;
        for (int p = 0; p < w->nn && !opened; p++) {
            if (!r->paired[p]) continue;
            herus_link l = { &r->sess[p], HZ_REGION_BR915, (uint8_t)SIM_TX_DBM, KNOWN_ROLES, 4,
                         &r->seen[p], r->profile };
            hcp_msg_t out; uint32_t ctr; int se = 0;
            if (link_recover(&l, frame, w->now_us / 1000u, &out, &ctr, &se) == LINK_OK) {
                opened = 1;
                r->n_recovered++;
                ACCEPT(out);
            }
        }
    }
#undef ACCEPT

    /* Weave: dedup and relay. `mine` is the caller's answer, exactly as weave.h
     * specifies — a frame for us needs no relaying. */
    int before_dup = (int)r->weave.stat_dup;
    int q = weave_offer(&r->weave, frame, (size_t)t->len, mine || opened,
                        w->now_us / 1000u, 5000);
    if ((int)r->weave.stat_dup > before_dup) w->g_dedup_drop++;
    if (q == 1) {
        /* The queued copy already has ttl-1. Record where it came from under the
         * fingerprint the relay will compute after popping it. */
        uint8_t peek[SIM_FRAME_MAX];
        memcpy(peek, frame, (size_t)t->len);
        session_frame_decrement_ttl(peek);
        sim_prov *np = prov_slot(w, sim_fingerprint(peek, (size_t)t->len), 1);
        if (np && np->born == 0) {
            np->orig = orig; np->dst = odst; np->seq = oseq;
            np->hops = t->hops + 1; np->born = born ? born : 1;
        }
        sim_push(w, sim_next_slot_us(r, w->now_us), EV_RELAY_KICK, rx, 0, 0);
    }
}

static void do_tx_end(sim_world *w, int slot)
{
    sim_tx *t = &w->air[slot];
    if (!t->active) return;
    for (int rx = 0; rx < w->nn; rx++) {
        if (rx == t->src) continue;
        deliver_to(w, slot, rx);
    }
}

static void do_relay_kick(sim_world *w, int id)
{
    sim_node *n = &w->n[id];
    const hz_link_profile_t *P = hz_link(n->profile);
    uint8_t out[SIM_FRAME_MAX]; size_t len = 0;
    /* A relay that talks over a live frame destroys two of them. */
    if (w->cad && sim_medium_busy(w, id)) {
        w->g_cad_defer++;
        uint64_t t = sim_next_slot_us(n, w->now_us);
        t += (uint64_t)(sim_rand(w) % BEAT_CAD_SPREAD) * (uint64_t)n->period_ms * 1000u;
        sim_push(w, t, EV_RELAY_KICK, id, 0, 0);
        return;
    }
    while (weave_next_tx(&n->weave, w->now_us / 1000u, out, &len)) {
        n->n_relayed++;
        w->g_relay_tx++;
        sim_prov *pvw = prov_slot(w, sim_fingerprint(out, len), 0);
        if (pvw && id < 64) pvw->relayed_by |= (1ull << id);
        const sim_prov *pv = pvw;
        int  dst  = pv ? pv->dst  : -1;
        int  seq  = pv ? pv->seq  : 0;
        int  hops = pv ? pv->hops : 1;
        uint64_t born = pv && pv->born ? pv->born : w->now_us;
        sim_start_tx(w, id, out, (int)len, P->sf, dst, seq, hops, born);
    }
}

/* -------------------------------------------------------------- adversary
 * Three attacks, and the honest limit on all of them: the adversary transmits
 * only what it has heard or what it has guessed. It never gets a key, because
 * there is no mechanism in the protocol by which it could.
 *
 *   ADV_REPLAY  the captured frame, verbatim. Session keys are single use, so
 *               the key that opened it no longer exists — see session.h.
 *   ADV_FORGE   the captured ADDRESS with a fresh random body. This is the
 *               expensive path on purpose: the cheap address reject does not
 *               save the victim, so the 64-bit tag and the rate limiter have to.
 *   ADV_JAM     noise, timed to land inside the victim's receive window.
 */
static void do_adversary(sim_world *w, int id, int mode, int victim)
{
    sim_node *j = &w->n[id];
    uint8_t f[LINK_FRAME_LEN];

    if (mode == ADV_REPLAY) {
        if (j->cap_len < (int)LINK_FRAME_LEN) return;
        memcpy(f, j->cap, LINK_FRAME_LEN);
        w->g_replay_tx++;
    } else if (mode == ADV_FORGE) {
        /* An adversary STRONGER than any real one. It is handed the victim's
         * live receive-window address — something no attacker can observe, since
         * P6 keeps any stable identifier off the air — so that the cheap
         * address reject cannot do the work. What is left defending the victim
         * is exactly what session.h claims defends it: a 64-bit tag and a token
         * bucket. Giving the attacker this much is the only way to find out
         * whether those two are enough.
         *
         * Frame header, per session.c: addr = frame[0] | (frame[1] << 8), 14
         * bits, with ttl in bits 7..6 of byte 1. */
        int peer = -1;
        for (int i = 0; i < w->nn; i++) if (w->n[victim].paired[i]) { peer = i; break; }
        if (peer < 0) return;
        uint16_t addr = w->n[victim].sess[peer].addr_win[0];
        for (int i = 0; i < (int)LINK_FRAME_LEN; i++) f[i] = (uint8_t)(sim_rand(w) & 0xff);
        f[0] = (uint8_t)(addr & 0xff);
        f[1] = (uint8_t)((addr >> 8) & 0x3f);
        w->g_forge_tx++;
    } else {
        for (int i = 0; i < (int)LINK_FRAME_LEN; i++) f[i] = (uint8_t)(sim_rand(w) & 0xff);
        w->g_jam_tx++;
    }
    sim_start_tx(w, id, f, LINK_FRAME_LEN, HERUS_SF_MEANING, victim, -1, 0, w->now_us);
}

/* ----------------------------------------------------------------- energy */
void sim_settle_energy(sim_world *w)
{
    uint64_t dur = w->end_us;
    for (int i = 0; i < w->nn; i++) {
        sim_node *n = &w->n[i];
        double on_us;
        if (n->weave.role == WEAVE_RELAY) {
            on_us = (double)dur;                       /* continuous receive */
        } else {
            double duty = (double)BEAT_RX_MS / (double)n->period_ms;
            on_us = (double)dur * duty + (double)n->rx_extra_us;
        }
        n->uah_rx = SIM_I_RX * on_us / 3.6e9 * 1e3;
    }
}

double sim_mah_per_day(const sim_node *n, uint64_t duration_us)
{
    if (duration_us == 0) return 0.0;
    double uah  = n->uah_tx + n->uah_rx + n->uah_mcu;
    double days = (double)duration_us / 86.4e9;
    return uah / 1000.0 / days + SIM_I_BASELINE * 24.0;
}

/* ------------------------------------------------------------------- loop */
void sim_run(sim_world *w)
{
    dset_reset();
    sim_event e;
    while (sim_pop(w, &e)) {
        if (e.t_us > w->end_us) break;
        w->now_us = e.t_us;
        switch (e.type) {
            case EV_APP_SEND:   do_app_send(w, e.a);   break;
            case EV_TX_END:     do_tx_end(w, e.a);     break;
            case EV_RELAY_KICK: do_relay_kick(w, e.a); break;
            case EV_ADVERSARY:  do_adversary(w, e.a, e.b, e.c); break;
            case EV_MOVE:       w->n[e.a].x = (double)e.b;
                                w->n[e.a].y = (double)e.c; break;
            case EV_STOP:       goto done;
            default: break;
        }
    }
done:
    sim_settle_energy(w);
}
