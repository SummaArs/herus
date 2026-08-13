/* weave.c — see weave.h. */
#include "weave.h"
#include "session.h"
#include <string.h>

/* Frames per hour this role may relay. See the note in weave.h: each figure is
 * the largest that keeps the role inside its own energy budget. */
static uint32_t budget_per_hour(weave_role_t r)
{
    return (r == WEAVE_RELAY) ? WEAVE_RELAY_PER_HOUR_RELAY : WEAVE_RELAY_PER_HOUR_LEAF;
}

/* Continuous refill, integer only. Capped at one hour's worth so a node that has
 * been idle for a week cannot bank a week of relaying and spend it in a minute —
 * a burst is exactly what the governor exists to prevent. */
static void refill(weave_t *w, uint64_t now_ms)
{
    uint32_t cap = budget_per_hour(w->role) * 1000u;
    if (now_ms > w->tok_refill_ms) {
        uint64_t d = now_ms - w->tok_refill_ms;
        uint64_t add = d * (uint64_t)budget_per_hour(w->role) / 3600u;
        if (add > cap) add = cap;
        uint64_t t = (uint64_t)w->tok_milli + add;
        w->tok_milli = (uint32_t)(t > cap ? cap : t);
        w->tok_refill_ms = now_ms;
    }
    if (w->tok_milli > cap) w->tok_milli = cap;      /* role may have shrunk */
}

void weave_init(weave_t *w, weave_role_t role)
{
    memset(w, 0, sizeof *w);
    w->role = role;
    /* Boot with a full bucket. A node that woke up owing its neighbours an hour
     * of silence would be a worse failure than the attack the bucket prevents. */
    w->tok_milli = budget_per_hour(role) * 1000u;
    w->tok_refill_ms = 0;
}

/* FNV-1a over the immutable part of the frame. The ttl bits in byte 1 are masked
 * off so that the same frame at two different hop counts is one fingerprint. */
static uint32_t fingerprint(const uint8_t *frame, size_t len)
{
    uint32_t h = 2166136261u;
    for (size_t i = 0; i < len; i++) {
        uint8_t b = frame[i];
        if (i == 1) b = (uint8_t)(b & 0x3f);
        h = (h ^ b) * 16777619u;
    }
    return h ? h : 1u;                 /* 0 is the empty-slot marker */
}

int weave_seen(weave_t *w, const uint8_t *frame, size_t len)
{
    uint32_t k = fingerprint(frame, len);
    for (int i = 0; i < WEAVE_DEDUP_N; i++)
        if (w->key[i] == k) return 0;
    w->key[w->head] = k;
    w->head = (uint16_t)((w->head + 1) % WEAVE_DEDUP_N);
    return 1;
}

int weave_offer(weave_t *w, const uint8_t *frame, size_t len, int mine,
                uint64_t now_ms, uint32_t deadline_ms)
{
    if (!w || !frame || len < 2 || len > WEAVE_FRAME_MAX) return -1;

    w->last_traffic_ms = now_ms;

    if (!weave_seen(w, frame, len)) { w->stat_dup++; return 0; }

    /* `mine` deliberately does NOT return here — see leak L1 in weave.h. A
     * recipient that stayed silent would be naming itself to anyone listening. */
    if (session_frame_ttl(frame) == 0) { w->stat_ttl_exhausted++; return 0; }

    /* The governor. Forwarding is the one thing a stranger can make us spend
     * energy on, so it is the one thing that is rationed. */
    refill(w, now_ms);
    if (w->tok_milli < 1000u) { w->stat_governed++; return 0; }

    for (int i = 0; i < WEAVE_QUEUE_N; i++) {
        if (w->q[i].used) continue;
        memcpy(w->q[i].frame, frame, len);
        w->q[i].len = (uint8_t)len;
        /* Decrement in the queued copy, never in the caller's buffer. The ttl
         * bits are outside the AAD, so this cannot invalidate the tag. */
        session_frame_decrement_ttl(w->q[i].frame);
        w->q[i].deadline_ms = now_ms + deadline_ms;
        w->q[i].used = 1;
        w->tok_milli -= 1000u;
        w->stat_relayed++;
        if (mine) w->stat_decoy++;
        return 1;
    }
    return -1;
}

int weave_next_tx(weave_t *w, uint64_t now_ms, uint8_t *out, size_t *len)
{
    int best = -1;
    for (int i = 0; i < WEAVE_QUEUE_N; i++) {
        if (!w->q[i].used) continue;
        if (now_ms > w->q[i].deadline_ms) {
            w->q[i].used = 0;
            w->stat_expired++;
            continue;
        }
        if (best < 0 || w->q[i].deadline_ms < w->q[best].deadline_ms) best = i;
    }
    if (best < 0) return 0;
    memcpy(out, w->q[best].frame, w->q[best].len);
    *len = w->q[best].len;
    w->q[best].used = 0;
    return 1;
}

weave_role_t weave_update_role(weave_t *w, uint64_t now_ms, int on_charger,
                               int soc_pct, int harvest_mw)
{
    /* Continuous receive costs 127.68 mAh/day against a 19.46 mAh/day capsule
     * harvest, so relay duty must be paid for by something other than the
     * battery. Both conditions below are that payment made explicit. */
    if (on_charger || (soc_pct > 80 && harvest_mw > 10)) {
        w->role = WEAVE_RELAY;
    } else if (now_ms - w->last_traffic_ms < 60000) {
        w->role = WEAVE_RESPONSIVE;
    } else {
        w->role = WEAVE_LEAF;
    }
    return w->role;
}
