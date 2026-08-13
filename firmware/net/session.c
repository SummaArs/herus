/* session.c — see session.h. */
#include "session.h"
#include <string.h>

/* Label-separated derivations from one chain key. Distinct HMAC messages under
 * the same key are independent PRF outputs, so four labels give four independent
 * secrets with one primitive and no extra state. The counter is folded into the
 * label as well — it is already implicit in the chain key, but making it explicit
 * means an accidental chain-key reuse degrades to "wrong key", not "reused
 * nonce", and a reused ChaCha20 nonce is a total confidentiality loss. */
static void derive(const uint8_t ck[32], const char *label, uint32_t n,
                   uint8_t *out, size_t outlen)
{
    uint8_t msg[24], mac[32];
    size_t l = strlen(label);
    if (l > 16) l = 16;
    memcpy(msg, label, l);
    msg[l + 0] = (uint8_t)(n);
    msg[l + 1] = (uint8_t)(n >> 8);
    msg[l + 2] = (uint8_t)(n >> 16);
    msg[l + 3] = (uint8_t)(n >> 24);
    hmac_sha256(ck, 32, msg, l + 4, mac);
    memcpy(out, mac, outlen > 32 ? 32 : outlen);
    secure_zero(mac, sizeof mac);
}

/* 14 bits: the top two are the public, mutable hop counter (erratum E-P2). */
static uint16_t chain_addr(const herus_chain *c)
{
    uint8_t a[32];
    derive(c->ck, "herus-addr", c->n, a, 32);
    uint16_t v = (uint16_t)((a[0] | (a[1] << 8)) & SESS_ADDR_MASK);
    secure_zero(a, sizeof a);
    return v;
}

/* The AAD is the address with the ttl bits cleared: authenticate what must not
 * change, leave mutable what every hop must change. Both ends compute it the
 * same way from the received bytes, so no state is needed to agree on it. */
static void aad_from_addr(uint16_t addr14, uint8_t aad[2])
{
    aad[0] = (uint8_t)(addr14 & 0xff);
    aad[1] = (uint8_t)((addr14 >> 8) & (SESS_ADDR_MASK >> 8));
}

uint8_t session_frame_ttl(const uint8_t *frame)
{
    return (uint8_t)(frame[1] >> 6);
}

int session_frame_decrement_ttl(uint8_t *frame)
{
    uint8_t ttl = (uint8_t)(frame[1] >> 6);
    if (ttl == 0) return -1;
    ttl--;
    frame[1] = (uint8_t)((frame[1] & 0x3f) | (ttl << 6));
    return (int)ttl;
}

static void chain_keys(const herus_chain *c, uint8_t key[32], uint8_t nonce[12])
{
    uint8_t t[32];
    derive(c->ck, "herus-key", c->n, key, 32);
    derive(c->ck, "herus-nonce", c->n, t, 32);
    memcpy(nonce, t, 12);
    secure_zero(t, sizeof t);
}

/* One irreversible step. The old chain key is overwritten in place: that
 * overwrite IS the forward secrecy, so it must not be "optimised" into a copy. */
static void chain_step(herus_chain *c)
{
    uint8_t next[32];
    derive(c->ck, "herus-step", c->n, next, 32);
    memcpy(c->ck, next, 32);
    secure_zero(next, sizeof next);
    c->n++;
}

/* Full build. Used once at init and once per DH ratchet step — never on the hot
 * path, which is why the frontier chain exists. */
static void rebuild_window(herus_session *s)
{
    herus_chain t = s->recv;                  /* scratch walk, real chain intact */
    for (int j = 0; j < SESS_WINDOW; j++) {
        s->addr_win[j] = chain_addr(&t);
        chain_step(&t);
    }
    s->win = t;                               /* frontier: recv.n + SESS_WINDOW */
    secure_zero(&t, sizeof t);
}

/* Slide by m counters: shift what is still valid, derive only the m new entries
 * from the frontier. 2 HMACs per counter instead of 64 per accepted frame. */
static void slide_window(herus_session *s, uint32_t m)
{
    if (m == 0) return;
    if (m >= SESS_WINDOW) { rebuild_window(s); return; }
    for (uint32_t j = 0; j < SESS_WINDOW - m; j++)
        s->addr_win[j] = s->addr_win[j + m];
    for (uint32_t j = SESS_WINDOW - m; j < SESS_WINDOW; j++) {
        s->addr_win[j] = chain_addr(&s->win);
        chain_step(&s->win);
    }
}

void session_init(herus_session *s, const uint8_t root[32], int initiator,
                  uint64_t now_ms)
{
    memset(s, 0, sizeof *s);
    /* Two labels, assigned by role. A's "a2b" is B's "a2b" — the roles pick which
     * one is send and which is receive. */
    uint8_t a2b[32], b2a[32];
    hkdf(NULL, 0, root, 32, "herus/chain/a2b", 15, a2b, 32);
    hkdf(NULL, 0, root, 32, "herus/chain/b2a", 15, b2a, 32);
    memcpy(s->send.ck, initiator ? a2b : b2a, 32);
    memcpy(s->recv.ck, initiator ? b2a : a2b, 32);
    s->send.n = s->recv.n = 0;
    s->tokens = SESS_RATE_TOKENS;
    s->last_refill_ms = now_ms;
    secure_zero(a2b, sizeof a2b); secure_zero(b2a, sizeof b2a);
    rebuild_window(s);
}

void session_dh_ratchet(herus_session *s, const uint8_t shared[32], int initiator)
{
    uint8_t root[32], a2b[32], b2a[32], ikm[64];
    /* New root = HKDF(old chain state, new shared secret). Mixing the old state in
     * means a DH step cannot RESET security if the shared secret is ever weak — it
     * can only add to it.
     *
     * The order must be CANONICAL, not local: A holds (send=a2b, recv=b2a) and B
     * holds the mirror image, so concatenating "my send then my recv" gives the
     * two ends different salts and the session silently dies at the next frame.
     * Ordering by role instead of by local meaning is the fix. Caught by
     * test_net S5b, which is the entire reason that test exists. */
    const uint8_t *a_chain = initiator ? s->send.ck : s->recv.ck;
    const uint8_t *b_chain = initiator ? s->recv.ck : s->send.ck;
    memcpy(ikm, a_chain, 32);
    memcpy(ikm + 32, b_chain, 32);
    hkdf(ikm, 64, shared, 32, "herus/dh-ratchet", 16, root, 32);
    hkdf(NULL, 0, root, 32, "herus/chain/a2b", 15, a2b, 32);
    hkdf(NULL, 0, root, 32, "herus/chain/b2a", 15, b2a, 32);
    memcpy(s->send.ck, initiator ? a2b : b2a, 32);
    memcpy(s->recv.ck, initiator ? b2a : a2b, 32);
    s->send.n = s->recv.n = 0;
    s->nskip = 0;
    memset(s->skip, 0, sizeof s->skip);
    secure_zero(root, sizeof root); secure_zero(a2b, sizeof a2b);
    secure_zero(b2a, sizeof b2a);  secure_zero(ikm, sizeof ikm);
    rebuild_window(s);
}

/* ------------------------------------------------------------------ send --- */

int session_seal(herus_session *s, const uint8_t *pt, size_t ptlen,
                 size_t tag_len, uint8_t ttl, uint8_t *frame_out)
{
    if (!s || !pt || !frame_out) return SESS_E_ARG;
    if (tag_len != 8 && tag_len != 16) return SESS_E_ARG;
    if (ttl > SESS_TTL_MAX) return SESS_E_ARG;

    uint8_t key[32], nonce[12], aad[2];
    uint16_t addr = chain_addr(&s->send);
    chain_keys(&s->send, key, nonce);

    aad_from_addr(addr, aad);
    frame_out[0] = aad[0];
    frame_out[1] = (uint8_t)(aad[1] | (ttl << 6));

    /* The address is on air in the clear, so it must be authenticated or an
     * attacker can retarget a frame between sessions whose addresses collide. It
     * costs zero bytes to put it in the AAD, and forgetting to is a real bug in
     * real protocols. The ttl bits are excluded on purpose — see erratum E-P2. */
    aead_encrypt(key, nonce, aad, 2, pt, ptlen,
                 frame_out + 2, frame_out + 2 + ptlen, tag_len);

    chain_step(&s->send);
    s->stat_sent++;
    secure_zero(key, sizeof key); secure_zero(nonce, sizeof nonce);
    return SESS_OK;
}

/* ---------------------------------------- replay window (SOS path only) ----- */

void herus_replay_init(herus_replay *r, uint32_t base)
{
    r->mask = 0; r->base = base;
}

int herus_replay_accept(herus_replay *r, uint32_t n)
{
    if (n < r->base) return 0;                           /* behind the window */
    uint32_t off = n - r->base;
    if (off < 64) {
        uint64_t bit = 1ull << off;
        if (r->mask & bit) return 0;
        r->mask |= bit;
        return 1;
    }
    /* Slide so n becomes the newest bit. What shifts out is now "behind the
     * window" and will be refused — bounded memory, with the bound stated rather
     * than discovered in the field. */
    uint32_t shift = off - 63;
    r->mask = (shift >= 64) ? 0 : (r->mask >> shift);
    r->base += shift;
    r->mask |= 1ull << (n - r->base);
    return 1;
}

/* ------------------------------------------------------------------ recv --- */

static void skip_store(herus_session *s, uint32_t n, uint16_t addr,
                       const uint8_t key[32], const uint8_t nonce[12])
{
    herus_skipped *e;
    if (s->nskip < SESS_SKIP_MAX) {
        e = &s->skip[s->nskip++];
    } else {
        /* Evict the oldest counter: a frame older than the cache is a frame we
         * have already given up on. */
        int oldest = 0;
        for (int i = 1; i < s->nskip; i++)
            if (s->skip[i].n < s->skip[oldest].n) oldest = i;
        e = &s->skip[oldest];
    }
    e->n = n; e->addr = addr;
    memcpy(e->key, key, 32); memcpy(e->nonce, nonce, 12);
}

static void skip_drop(herus_session *s, int idx)
{
    secure_zero(&s->skip[idx], sizeof s->skip[idx]);
    s->skip[idx] = s->skip[s->nskip - 1];
    secure_zero(&s->skip[s->nskip - 1], sizeof s->skip[s->nskip - 1]);
    s->nskip--;
}

static int take_token(herus_session *s, uint64_t now_ms)
{
    if (now_ms < s->last_refill_ms) s->last_refill_ms = now_ms;   /* clock reset */
    uint64_t elapsed = now_ms - s->last_refill_ms;
    if (elapsed >= 1000) {
        uint64_t add = (elapsed / 1000) * SESS_RATE_TOKENS;
        s->tokens = (uint32_t)((s->tokens + add > SESS_RATE_TOKENS)
                               ? SESS_RATE_TOKENS : s->tokens + add);
        s->last_refill_ms += (elapsed / 1000) * 1000;
    }
    if (s->tokens == 0) return 0;
    s->tokens--;
    return 1;
}

int session_open(herus_session *s, const uint8_t *frame, size_t ptlen,
                 size_t tag_len, uint64_t now_ms,
                 uint8_t *pt_out, uint32_t *counter_out)
{
    if (!s || !frame || !pt_out) return SESS_E_ARG;
    if (tag_len != 8 && tag_len != 16) return SESS_E_ARG;

    const uint16_t addr = (uint16_t)((frame[0] | (frame[1] << 8)) & SESS_ADDR_MASK);
    const uint8_t *ct  = frame + 2;
    const uint8_t *tag = frame + 2 + ptlen;
    uint8_t aad[2];
    aad_from_addr(addr, aad);

    /* Cheap gate first: is this address one we could possibly own? Answering "no"
     * must cost nothing — a mesh where ignoring a stranger's frame costs crypto is
     * a battery attack surface. */
    int in_skip = 0, in_win = -1;
    for (int i = 0; i < s->nskip; i++) if (s->skip[i].addr == addr) { in_skip = 1; break; }
    for (int j = 0; j < SESS_WINDOW; j++) if (s->addr_win[j] == addr) { in_win = j; break; }
    if (!in_skip && in_win < 0) return SESS_E_ADDR;

    /* One token per FRAME, not per attempt: a 14-bit address collides once in
     * 16384, so a single frame can plausibly match a cached key and a window entry
     * at once, and both deserve a try. Charging a token each would let an attacker
     * drain the bucket faster than the rule intends. */
    if (!take_token(s, now_ms)) { s->stat_rate_drop++; return SESS_E_RATE; }

    /* 1. Cached skipped keys first: a late frame is more likely than a forgery, and
     *    this path costs one uint16 compare per cached entry.
     *    On failure we FALL THROUGH to the window rather than giving up: an address
     *    collision between a stale cached key and a live one would otherwise drop a
     *    perfectly good frame about once in every 500. */
    for (int i = 0; i < s->nskip; i++) {
        if (s->skip[i].addr != addr) continue;
        if (aead_decrypt(s->skip[i].key, s->skip[i].nonce, aad, 2,
                         ct, ptlen, tag, tag_len, pt_out) == 0) {
            uint32_t n = s->skip[i].n;
            /* Destroying the entry is the replay defence: this counter now has no
             * key anywhere in the system, so a second copy of this frame cannot
             * open. No bitmap needed — see the note in session.h. */
            skip_drop(s, i);
            if (counter_out) *counter_out = n;
            s->stat_opened++;
            s->last_open_ms = now_ms;
            return SESS_OK;
        }
    }

    /* 2. The address window. */
    int hit = in_win;
    if (hit < 0) { s->stat_auth_fail++; return SESS_E_AUTH; }

    /* 3. Walk a scratch chain to the hit counter and try it. */
    herus_chain t = s->recv;
    uint8_t key[32], nonce[12];
    for (int j = 0; j < hit; j++) chain_step(&t);
    chain_keys(&t, key, nonce);

    int ok = (aead_decrypt(key, nonce, aad, 2, ct, ptlen, tag, tag_len, pt_out) == 0);
    if (!ok) {
        /* A 2-byte address collides once in 65536, so an address hit with a bad
         * tag is usually a forgery attempt and occasionally an unlucky neighbour.
         * Either way: charge a token, learn nothing, advance nothing. */
        secure_zero(key, sizeof key); secure_zero(nonce, sizeof nonce);
        secure_zero(&t, sizeof t);
        s->stat_auth_fail++;
        return SESS_E_AUTH;
    }

    uint32_t n = s->recv.n + (uint32_t)hit;

    /* 4. Cache the keys for the frames we jumped over, then advance past n.
     *    Without this cache, one lost frame followed by an out-of-order arrival
     *    would drop a message that is perfectly openable.
     *    One forward walk, not one per gap: quadratic work on a battery-powered
     *    receiver is a bug even when it is correct. */
    {
        herus_chain u = s->recv;
        for (int j = 0; j < hit; j++) {
            uint8_t k2[32], nn2[12];
            chain_keys(&u, k2, nn2);
            skip_store(s, u.n, s->addr_win[j], k2, nn2);
            secure_zero(k2, sizeof k2); secure_zero(nn2, sizeof nn2);
            chain_step(&u);
        }
        secure_zero(&u, sizeof u);
    }
    for (int j = 0; j <= hit; j++) chain_step(&s->recv);
    slide_window(s, (uint32_t)hit + 1u);

    if (counter_out) *counter_out = n;
    s->stat_opened++;
    s->last_open_ms = now_ms;
    secure_zero(key, sizeof key); secure_zero(nonce, sizeof nonce);
    secure_zero(&t, sizeof t);
    return SESS_OK;
}

/* ---------------- deep resync: see the long note in session.h ---------------- */
int session_recover(herus_session *s, const uint8_t *frame, size_t ptlen,
                    size_t tag_len, uint64_t now_ms,
                    uint8_t *pt_out, uint32_t *counter_out)
{
    if (!s || !frame || !pt_out) return SESS_E_ARG;
    if (tag_len != 8 && tag_len != 16) return SESS_E_ARG;

    /* Gate 1: is the link actually lost? A link that opened something recently is
     * fine, and paying 2048 HMACs to confirm that would be a self-inflicted
     * battery attack. */
    if (now_ms - s->last_open_ms < SESS_LOST_MS) return SESS_E_ADDR;

    /* Gate 2: at most one deep walk per cooldown, so a stranger transmitting
     * garbage cannot turn our own recovery into the drain. */
    if (s->last_recover_ms && now_ms - s->last_recover_ms < SESS_RECOVER_COOLDOWN_MS)
        return SESS_E_RATE;
    s->last_recover_ms = now_ms ? now_ms : 1u;

    const uint16_t addr = (uint16_t)((frame[0] | (frame[1] << 8)) & SESS_ADDR_MASK);
    const uint8_t *ct  = frame + 2;
    const uint8_t *tag = frame + 2 + ptlen;
    uint8_t aad[2];
    aad_from_addr(addr, aad);

    /* Start at the frontier, which is where the ordinary window stops. Walking
     * from recv.n would redo work session_open has already done. */
    herus_chain t = s->win;

    for (uint32_t k = 0; k < SESS_RECOVER_SPAN; k++) {
        if (chain_addr(&t) == addr) {
            uint8_t key[32], nonce[12];
            chain_keys(&t, key, nonce);
            int ok = (aead_decrypt(key, nonce, aad, 2, ct, ptlen, tag, tag_len,
                                   pt_out) == 0);
            secure_zero(key, sizeof key); secure_zero(nonce, sizeof nonce);
            if (ok) {
                uint32_t n = t.n;
                /* Jump. The cached skipped keys belong to counters we have now
                 * passed; keeping them would leave openable keys for messages we
                 * have declared lost, which is the opposite of what the ratchet
                 * is for. */
                secure_zero(s->skip, sizeof s->skip);
                s->nskip = 0;
                s->recv = t;
                chain_step(&s->recv);
                rebuild_window(s);
                s->stat_recovered++;
                s->last_open_ms = now_ms;
                if (counter_out) *counter_out = n;
                secure_zero(&t, sizeof t);
                return SESS_OK;
            }
            /* Address matched, tag did not: a collision or a forgery. Neither is
             * a reason to move the chain. Keep walking. */
        }
        chain_step(&t);
    }
    secure_zero(&t, sizeof t);
    return SESS_E_ADDR;
}

int session_addr_in_window(const herus_session *s, const uint8_t *frame)
{
    uint16_t addr = (uint16_t)((frame[0] | (frame[1] << 8)) & SESS_ADDR_MASK);
    for (int j = 0; j < SESS_WINDOW; j++) if (s->addr_win[j] == addr) return 1;
    for (int i = 0; i < s->nskip; i++)    if (s->skip[i].addr == addr) return 1;
    return 0;
}

/* --------------------------------------------------------- Tier 0.5 ------- */

static void sketch_material(const uint8_t gkey[32], uint32_t epoch,
                            uint16_t *addr, uint8_t key[32], uint8_t nonce[12])
{
    uint8_t a[32], t[32];
    derive(gkey, "herus-sk-addr", epoch, a, 32);
    *addr = (uint16_t)((a[0] | (a[1] << 8)) & SESS_ADDR_MASK);
    derive(gkey, "herus-sk-key", epoch, key, 32);
    derive(gkey, "herus-sk-non", epoch, t, 32);
    memcpy(nonce, t, 12);
    secure_zero(a, sizeof a); secure_zero(t, sizeof t);
}

/* Writes 2 + sketch_len + pad_len bytes. Herus calls it with 32 + 4 = 38 total,
 * because that is what makes the airtime identical to a Tier 0/1 frame (erratum
 * E-P1). The pad is keystream, not zeros: a run of zeros at a fixed offset in
 * every beacon is a fingerprint, and it costs nothing to avoid. */
void sketch_seal(const uint8_t gkey[32], uint32_t epoch,
                 const uint8_t *sketch, size_t sketch_len, uint8_t *frame_out)
{
    uint8_t key[32], nonce[12];
    uint16_t addr;
    sketch_material(gkey, epoch, &addr, key, nonce);
    frame_out[0] = (uint8_t)(addr & 0xff);
    frame_out[1] = (uint8_t)((addr >> 8) & (SESS_ADDR_MASK >> 8));
    chacha20_xor(key, nonce, 1, sketch, frame_out + 2, sketch_len);
    secure_zero(key, sizeof key); secure_zero(nonce, sizeof nonce);
}

void sketch_pad(const uint8_t gkey[32], uint32_t epoch, size_t sketch_len,
                uint8_t *pad_out, size_t pad_len)
{
    uint8_t key[32], nonce[12], scratch[64] = {0};
    uint16_t addr;

    /* Bounded by construction: the caller asks for 4 bytes after a 32-byte sketch.
     * Refusing anything that would not fit one keystream block keeps this a
     * fixed-cost function with no allocation and no overflow to reason about. */
    if (sketch_len + pad_len > sizeof scratch) { memset(pad_out, 0, pad_len); return; }

    sketch_material(gkey, epoch, &addr, key, nonce);
    /* Continue the same keystream past the sketch, so the pad is indistinguishable
     * from the sketch to an observer and costs no extra state. */
    chacha20_xor(key, nonce, 1, scratch, scratch, sketch_len + pad_len);
    memcpy(pad_out, scratch + sketch_len, pad_len);
    secure_zero(key, sizeof key); secure_zero(nonce, sizeof nonce);
    secure_zero(scratch, sizeof scratch);
}

int sketch_open(const uint8_t gkey[32], uint32_t epoch,
                const uint8_t *frame, size_t sketch_len, uint8_t *sketch_out)
{
    uint8_t key[32], nonce[12];
    uint16_t addr;
    sketch_material(gkey, epoch, &addr, key, nonce);
    /* The address is advisory here: with no MAC there is nothing to verify, so a
     * mismatch means "probably not our epoch" and the caller may still try the
     * nearest-neighbour decode. Returning -1 lets it decide. */
    uint16_t got = (uint16_t)((frame[0] | (frame[1] << 8)) & SESS_ADDR_MASK);
    chacha20_xor(key, nonce, 1, frame + 2, sketch_out, sketch_len);
    secure_zero(key, sizeof key); secure_zero(nonce, sizeof nonce);
    return (got == addr) ? 0 : -1;
}
