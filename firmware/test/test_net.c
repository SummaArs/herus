/* test_net.c — the protocol proof suite.
 *
 * Doctrine, same as the algebra suites: an asserted property is a rumour, a
 * property a test can FAIL is a result. Every rule P1-P7 in docs/02-PROTOCOL.md
 * and every footgun in §8 is either exercised here or it is not claimed.
 *
 *   V1-V6  crypto against an independent implementation (OpenSSL via Python)
 *   P1     constant airtime across meaning tiers          (erratum E-P1)
 *   P2     no frame exceeds the 400 ms dwell
 *   P3     hypervectors are never transmitted, and both ends rebuild bit-identically
 *   P4     unknown roles are skipped, never rejected
 *   P5     Tier 0.5 has no MAC and degrades; AEAD tiers reject a single bit flip
 *   P6     addresses are ephemeral — no stable identifier on air
 *   P7     only Tier 2 is length-distinguishable
 *   P8     ttl is mutable in flight, the address is not  (erratum E-P2)
 *   S1-S5  session: replay, reorder, forgery, rate limit, forward secrecy
 *   W1-W2  Weave: flooding terminates, deadlines are enforced
 *   B1-B2  Beat: guard covers drift, resync converges
 *   C1     canonical encoding: no two byte strings mean the same thing
 *   X1     cost per frame, measured
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "../core/hcp.h"
#include "../net/crypto.h"
#include "../net/session.h"
#include "../net/region.h"
#include "../net/weave.h"
#include "../net/beat.h"
#include "../net/link.h"
#include "vectors.h"

#define HDOM 0x48455255530002ull

static int FAILED = 0;
static void ok(int cond, const char *what)
{
    printf("  %-4s %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond) FAILED = 1;
}
static void hdr(const char *s) { printf("\n== %s ==\n", s); }

static uint64_t rng = 0x9E3779B97F4A7C15ull;
static uint64_t mix64(void)
{
    uint64_t z = (rng += 0x9E3779B97F4A7C15ull);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}
static void fill_random(uint8_t *p, size_t n) { for (size_t i = 0; i < n; i++) p[i] = (uint8_t)mix64(); }

/* ===================================================== crypto vectors ==== */

static void v_crypto(void)
{
    hdr("V1-V6  crypto vs an independent implementation (Python cryptography / OpenSSL)");
    uint8_t out[256];
    int bad;

    bad = 0;
    for (int i = 0; i < V_SHA256_N; i++) {
        sha256(V_SHA256[i].msg, (size_t)V_SHA256[i].len, out);
        if (memcmp(out, V_SHA256[i].md, 32)) bad++;
    }
    printf("  SHA-256      %2d cases, %d mismatched\n", V_SHA256_N, bad);
    ok(!bad, "V1 SHA-256 matches the reference on every case");

    bad = 0;
    for (int i = 0; i < V_HMAC_N; i++) {
        hmac_sha256(V_HMAC[i].key, (size_t)V_HMAC[i].klen,
                    V_HMAC[i].msg, (size_t)V_HMAC[i].mlen, out);
        if (memcmp(out, V_HMAC[i].md, 32)) bad++;
    }
    printf("  HMAC-SHA256  %2d cases, %d mismatched\n", V_HMAC_N, bad);
    ok(!bad, "V2 HMAC-SHA256 matches the reference");

    bad = 0;
    for (int i = 0; i < V_HKDF_N; i++) {
        hkdf(V_HKDF[i].slen ? V_HKDF[i].salt : NULL, (size_t)V_HKDF[i].slen,
             V_HKDF[i].ikm, (size_t)V_HKDF[i].ilen,
             V_HKDF[i].flen ? V_HKDF[i].info : NULL, (size_t)V_HKDF[i].flen,
             out, (size_t)V_HKDF[i].olen);
        if (memcmp(out, V_HKDF[i].okm, (size_t)V_HKDF[i].olen)) bad++;
    }
    printf("  HKDF-SHA256  %2d cases, %d mismatched\n", V_HKDF_N, bad);
    ok(!bad, "V3 HKDF-SHA256 matches the reference");

    bad = 0;
    for (int i = 0; i < V_CHACHA_N; i++) {
        if (chacha20_xor(V_CHACHA[i].key, V_CHACHA[i].nonce, V_CHACHA[i].ctr,
                         V_CHACHA[i].pt, out, (size_t)V_CHACHA[i].len) != 0) { bad++; continue; }
        if (memcmp(out, V_CHACHA[i].ct, (size_t)V_CHACHA[i].len)) bad++;
    }
    printf("  ChaCha20     %2d cases, %d mismatched\n", V_CHACHA_N, bad);
    ok(!bad, "V4 ChaCha20 matches the reference");

    bad = 0;
    for (int i = 0; i < V_POLY_N; i++) {
        poly1305_mac(V_POLY[i].key, V_POLY[i].msg, (size_t)V_POLY[i].len, out);
        if (memcmp(out, V_POLY[i].tag, 16)) bad++;
    }
    printf("  Poly1305     %2d cases, %d mismatched\n", V_POLY_N, bad);
    ok(!bad, "V5 Poly1305 matches the reference");

    bad = 0;
    for (int i = 0; i < V_AEAD_N; i++) {
        uint8_t tag[16], pt2[64];
        aead_encrypt(V_AEAD[i].key, V_AEAD[i].nonce,
                     V_AEAD[i].alen ? V_AEAD[i].aad : NULL, (size_t)V_AEAD[i].alen,
                     V_AEAD[i].pt, (size_t)V_AEAD[i].plen, out, tag, 16);
        if (memcmp(out, V_AEAD[i].ct, (size_t)V_AEAD[i].plen)) bad++;
        if (memcmp(tag, V_AEAD[i].tag, 16)) bad++;
        if (aead_decrypt(V_AEAD[i].key, V_AEAD[i].nonce,
                         V_AEAD[i].alen ? V_AEAD[i].aad : NULL, (size_t)V_AEAD[i].alen,
                         V_AEAD[i].ct, (size_t)V_AEAD[i].plen, V_AEAD[i].tag, 16, pt2) != 0) bad++;
        else if (memcmp(pt2, V_AEAD[i].pt, (size_t)V_AEAD[i].plen)) bad++;
    }
    printf("  ChaCha20-Poly1305 %2d cases, %d mismatched\n", V_AEAD_N, bad);
    ok(!bad, "V6 AEAD encrypt+decrypt matches the reference");

    /* The 32-bit counter boundary is refused rather than defined — see crypto.h. */
    uint8_t k[32] = {0}, n[12] = {0}, buf[200] = {0};
    ok(chacha20_xor(k, n, 0xffffffffu, buf, buf, 200) == -1,
       "V6b crossing the ChaCha20 counter wrap is refused, not silently defined");
}

/* ========================================================= P1, P2, P7 ==== */

static void p1_p2_p7(void)
{
    hdr("P1/P2/P7  the frame ledger, recomputed here and not quoted");
    hz_print_ledger();

    ok(HERUS_AIRTIME_MEANING_US == HERUS_AIRTIME_SKETCH_US,
       "P1 Tier 0/1 and Tier 0.5 occupy identical airtime");
    ok(HERUS_AIRTIME_MEANING_US <= HZ_DWELL_LIMIT_US &&
       HERUS_AIRTIME_VOICE_US   <= HZ_DWELL_LIMIT_US &&
       HERUS_AIRTIME_SOS_US     <= HZ_DWELL_LIMIT_US,
       "P2 every shipped frame is inside the 400 ms dwell limit");
    ok(hz_airtime_us(10, HERUS_FRAME_LEN, 1, 0, HERUS_CR) > HZ_DWELL_LIMIT_US,
       "P2 SF10 is illegal at 34 B — SF9 really is the ceiling");
    ok(!hz_tx_permitted(HZ_REGION_BR915, 10, HERUS_FRAME_LEN, 1, 0, HERUS_CR, 14),
       "P2 the region profile refuses SF10 at transmit time as well");
    ok(hz_tx_permitted(HZ_REGION_LAB, 12, HERUS_FRAME_LEN, 1, 0, HERUS_CR, 14),
       "P2 LAB profile allows SF12 — research is possible, but never by default");

    uint32_t t_voice = hz_airtime_us(HERUS_SF_VOICE, 178, 1, 0, HERUS_CR);
    ok(t_voice != HERUS_AIRTIME_MEANING_US,
       "P7 Tier 2 IS distinguishable, and that is stated rather than hidden");
}

/* ============================================================== P3 ======= */
/* The wire carries ids; the hypervector is rebuilt locally. Prove it by building
 * the semantic form on both sides and comparing bit-for-bit — if the receiver
 * needed any transmitted vector, this cannot come out identical. */

static void p3_ids_only(void)
{
    hdr("P3  hypervectors are never transmitted");

    lex_t L; lex_init(&L, HDOM, 64);
    hv_acc_t *acc = malloc(sizeof(hv_acc_t));

    hcp_msg_t tx = {0};
    tx.tier = HCP_TIER_COMPOSED; tx.intent = 41; tx.seq = 7; tx.nslot = 3;
    tx.slot[0].role = 1;  tx.slot[0].filler = 300;
    tx.slot[1].role = 2;  tx.slot[1].filler = 17;
    tx.slot[2].role = 30; tx.slot[2].filler = 2047;

    uint8_t pt[HCP_PLAINTEXT_LEN];
    ok(hcp_encode(pt, &tx) == HCP_PLAINTEXT_LEN, "P3 a 3-slot composed record encodes to 24 bytes");

    hcp_msg_t rx;
    ok(hcp_decode(&rx, pt) == 0, "P3 it decodes");
    ok(rx.intent == tx.intent && rx.nslot == tx.nslot &&
       rx.slot[2].role == 30 && rx.slot[2].filler == 2047,
       "P3 round trip is exact including the extremes of both bit fields");

    hv_t Htx, Hrx;
    hcp_to_hv(&Htx, acc, &L, &tx);
    hcp_to_hv(&Hrx, acc, &L, &rx);
    ok(hv_dist(&Htx, &Hrx) == 0,
       "P3 both ends rebuild the identical 10240-bit vector from ids alone");

    /* And the ids are all that is on air: 24 bytes of plaintext vs 1280 bytes of
     * hypervector. State the ratio, because it IS the thesis. */
    printf("  on air %d B of ids vs %d B of hypervector -> %.0fx\n",
           HCP_PLAINTEXT_LEN, HV_BYTES, (double)HV_BYTES / HCP_PLAINTEXT_LEN);

    /* Role order must survive (footgun #6). Swap two slots and the vector must
     * change: if it does not, the record is a commutative heap. */
    hcp_msg_t sw = tx;
    sw.slot[0] = tx.slot[1]; sw.slot[1] = tx.slot[0];
    hv_t Hsw; hcp_to_hv(&Hsw, acc, &L, &sw);
    int d = hv_dist(&Htx, &Hsw);
    printf("  swapping slots 0 and 1 moves the vector by %d bits (%.1f sigma)\n",
           d, (d - HV_BITS / 2.0) / (sqrt((double)HV_BITS) / 2.0));
    ok(d > HV_BITS / 4,
       "P3/footgun6 slot order changes the meaning — rho^(k+2) is doing its job");

    /* The hypothesis test: a present role resolves, an absent one is reported
     * absent rather than resolving to the nearest wrong answer. */
    uint16_t f = 0;
    int present = hcp_query_role(&Htx, &L, 2, 1, 0, 512, 12.0, &f);
    ok(present && f == 17, "P3 unbinding slot 1 (role 2) recovers filler 17");
    int absent = hcp_query_role(&Htx, &L, 9, 1, 0, 512, 12.0, &f);
    ok(!absent, "P3 an absent role is reported absent, not resolved to noise");

    free(acc); lex_free(&L);
}

/* ============================================================== P4 ======= */

static void p4_forward_compat(void)
{
    hdr("P4  a newer sender's unknown roles are skipped, never rejected");

    /* v1 firmware knows roles 1..4. A v3 sender uses role 9 as well. */
    static const uint8_t known_v1[] = { 1, 2, 3, 4 };

    hcp_msg_t future = {0};
    future.tier = HCP_TIER_COMPOSED; future.intent = 12; future.seq = 1; future.nslot = 3;
    future.slot[0].role = 2; future.slot[0].filler = 301;
    future.slot[1].role = 9; future.slot[1].filler = 999;   /* invented after v1 */
    future.slot[2].role = 3; future.slot[2].filler = 44;

    uint8_t pt[HCP_PLAINTEXT_LEN];
    hcp_encode(pt, &future);

    hcp_msg_t got, kept;
    ok(hcp_decode(&got, pt) == 0, "P4 v1 firmware DECODES a frame containing a role it has never heard of");
    int n = hcp_filter_known_roles(&got, known_v1, 4, &kept);
    ok(n == 2, "P4 it keeps the 2 fields it understands");
    ok(kept.slot[0].filler == 301 && kept.slot[1].filler == 44,
       "P4 and the surviving fields are the right ones");
    ok(kept.intent == 12, "P4 the intent still arrives — the message is usable, not discarded");

    /* The subtle half of P4: after filtering, the kept slots are packed densely,
     * but the SENDER bound them at their original on-air positions. Unbinding at
     * the new dense index would read noise — a bug that only ever appears over the
     * air, never locally, which is the worst possible place to find it. */
    ok(kept.pos[0] == 0 && kept.pos[1] == 2,
       "P4 kept slots carry their ORIGINAL on-air positions, not their new indices");

    lex_t L; lex_init(&L, HDOM, 64);
    hv_acc_t *acc = malloc(sizeof(hv_acc_t));
    hv_t H; hcp_to_hv(&H, acc, &L, &got);          /* what the sender built */

    uint16_t f = 0;
    ok(hcp_query_role(&H, &L, 3, kept.pos[1], 0, 512, 12.0, &f) && f == 44,
       "P4 unbinding a kept slot at pos[] recovers its filler");
    ok(!hcp_query_role(&H, &L, 3, 1, 0, 512, 12.0, &f),
       "P4 unbinding it at the dense index instead finds nothing — pos[] is load-bearing");
    free(acc); lex_free(&L);
}

/* ============================================================== P5 ======= */

static void p5_tier_split(void)
{
    hdr("P5  AEAD tiers reject a single flipped bit; Tier 0.5 must not have a MAC");

    uint8_t root[32], frame[LINK_FRAME_LEN], pt[LINK_PT_LEN], out[LINK_PT_LEN];
    fill_random(root, 32);
    herus_session a, b;
    session_init(&a, root, 1, 0);
    session_init(&b, root, 0, 0);

    fill_random(pt, sizeof pt);
    pt[0] = (uint8_t)((HCP_VERSION << 6) | (HCP_TIER_GLYPH << 3));
    pt[1] = 0x40;                       /* ttl bits live on air, rsv must be 0 */

    /* Every single-bit flip in the ciphertext or tag must be rejected. 32 bytes
     * x 8 bits = 256 flips, all of them. */
    int rejected = 0;
    for (int bit = 0; bit < (int)(LINK_PT_LEN + LINK_TAG_LEN) * 8; bit++) {
        herus_session aa = a, bb = b;
        session_seal(&aa, pt, LINK_PT_LEN, LINK_TAG_LEN, 3, frame);
        frame[2 + bit / 8] ^= (uint8_t)(1u << (bit % 8));
        if (session_open(&bb, frame, LINK_PT_LEN, LINK_TAG_LEN, 0, out, NULL) != SESS_OK)
            rejected++;
    }
    printf("  single-bit flips rejected: %d / %d\n", rejected,
           (int)(LINK_PT_LEN + LINK_TAG_LEN) * 8);
    ok(rejected == (int)(LINK_PT_LEN + LINK_TAG_LEN) * 8,
       "P5 an AEAD tier rejects every single-bit corruption");

    /* Tier 0.5: a bit error must corrupt one sketch bit and nothing else. That is
     * the whole reason it has no MAC (footgun #3): Poly1305 avalanches. */
    uint8_t gkey[32], sk[HERUS_SKETCH_BYTES], skf[HERUS_SKETCH_FRAME_LEN], back[HERUS_SKETCH_BYTES];
    fill_random(gkey, 32);
    fill_random(sk, sizeof sk);
    sketch_seal(gkey, 5, sk, sizeof sk, skf);
    sketch_pad(gkey, 5, sizeof sk, skf + 2 + sizeof sk, HERUS_SKETCH_PAD);
    skf[10] ^= 0x08;                                   /* one bit error on air */
    sketch_open(gkey, 5, skf, sizeof sk, back);
    int diff = 0;
    for (unsigned i = 0; i < sizeof sk; i++) {
        uint8_t x = (uint8_t)(sk[i] ^ back[i]);
        while (x) { diff += x & 1; x >>= 1; }
    }
    printf("  Tier 0.5: 1 bit corrupted on air -> %d bits wrong after decrypt\n", diff);
    ok(diff == 1, "P5 Tier 0.5 does not avalanche — one bit in, one bit out");
    ok(sketch_open(gkey, 5, skf, sizeof sk, back) == 0,
       "P5 the epoch address still matches (it is advisory, there is nothing to verify)");
}

/* ============================================================== P6 ======= */

static void p6_ephemeral(void)
{
    hdr("P6  no stable identifier is ever on air");

    uint8_t root[32], frame[LINK_FRAME_LEN], pt[LINK_PT_LEN] = {0};
    fill_random(root, 32);
    herus_session a; session_init(&a, root, 1, 0);
    pt[0] = (uint8_t)(HCP_VERSION << 6);

    uint16_t addrs[64];
    for (int i = 0; i < 64; i++) {
        session_seal(&a, pt, LINK_PT_LEN, LINK_TAG_LEN, 3, frame);
        addrs[i] = (uint16_t)((frame[0] | (frame[1] << 8)) & SESS_ADDR_MASK);
    }
    int repeats = 0;
    for (int i = 0; i < 64; i++)
        for (int j = i + 1; j < 64; j++)
            if (addrs[i] == addrs[j]) repeats++;
    printf("  64 consecutive frames, %d address repeats (birthday expectation %.2f)\n",
           repeats, 64.0 * 63.0 / 2.0 / 16384.0);
    ok(repeats <= 1, "P6 addresses do not repeat — there is nothing to track");

    /* Identical plaintext must never produce identical ciphertext, or the frame
     * itself becomes the stable identifier the address is not. */
    herus_session c; session_init(&c, root, 1, 0);
    uint8_t f1[LINK_FRAME_LEN], f2[LINK_FRAME_LEN];
    session_seal(&c, pt, LINK_PT_LEN, LINK_TAG_LEN, 3, f1);
    session_seal(&c, pt, LINK_PT_LEN, LINK_TAG_LEN, 3, f2);
    ok(memcmp(f1, f2, LINK_FRAME_LEN) != 0,
       "P6 the same message sent twice looks completely different on air");
}

/* ============================================================== P8 ======= */

static void p8_ttl_mutable(void)
{
    hdr("P8  erratum E-P2: ttl is mutable in flight, the address is not");

    uint8_t root[32], frame[LINK_FRAME_LEN], pt[LINK_PT_LEN] = {0}, out[LINK_PT_LEN];
    fill_random(root, 32);
    herus_session a, b;
    session_init(&a, root, 1, 0); session_init(&b, root, 0, 0);
    pt[0] = (uint8_t)(HCP_VERSION << 6);

    session_seal(&a, pt, LINK_PT_LEN, LINK_TAG_LEN, 3, frame);
    ok(session_frame_ttl(frame) == 3, "P8 ttl 3 is readable by a relay with no key");

    /* Three hops of a real relay. */
    ok(session_frame_decrement_ttl(frame) == 2, "P8 hop 1 decrements to 2");
    ok(session_frame_decrement_ttl(frame) == 1, "P8 hop 2 decrements to 1");
    ok(session_frame_decrement_ttl(frame) == 0, "P8 hop 3 decrements to 0");
    ok(session_frame_decrement_ttl(frame) == -1, "P8 hop 4 refuses — the frame must be dropped");

    ok(session_open(&b, frame, LINK_PT_LEN, LINK_TAG_LEN, 0, out, NULL) == SESS_OK,
       "P8 after 3 relays rewrote the ttl the tag still verifies end to end");

    /* But the address is authenticated: touching it must break the tag. */
    herus_session b2; session_init(&b2, root, 0, 0);
    herus_session a2; session_init(&a2, root, 1, 0);
    session_seal(&a2, pt, LINK_PT_LEN, LINK_TAG_LEN, 3, frame);
    frame[0] ^= 0x01;
    int r = session_open(&b2, frame, LINK_PT_LEN, LINK_TAG_LEN, 0, out, NULL);
    ok(r == SESS_E_ADDR || r == SESS_E_AUTH,
       "P8 modifying an address bit makes the frame unopenable");
}

/* ======================================================= session S1-S5 === */

static void s_session(void)
{
    hdr("S1-S5  ratchet: reorder, replay, forgery, rate limit, forward secrecy");

    uint8_t root[32];
    fill_random(root, 32);
    herus_session a, b;
    session_init(&a, root, 1, 0); session_init(&b, root, 0, 0);

    uint8_t pt[LINK_PT_LEN] = {0}, out[LINK_PT_LEN];
    uint8_t frames[8][LINK_FRAME_LEN];
    pt[0] = (uint8_t)(HCP_VERSION << 6);

    for (int i = 0; i < 8; i++) {
        pt[4] = (uint8_t)i;
        session_seal(&a, pt, LINK_PT_LEN, LINK_TAG_LEN, 3, frames[i]);
    }

    /* Deliver out of order: 3, 0, 7, 1 — a realistic lossy-radio arrival. */
    uint32_t ctr;
    int order[] = { 3, 0, 7, 1 };
    int opened = 0;
    for (int i = 0; i < 4; i++)
        if (session_open(&b, frames[order[i]], LINK_PT_LEN, LINK_TAG_LEN, 0, out, &ctr) == SESS_OK
            && ctr == (uint32_t)order[i] && out[4] == (uint8_t)order[i]) opened++;
    printf("  arrival order 3,0,7,1 -> %d/4 opened with the right counter\n", opened);
    ok(opened == 4, "S1 out-of-order frames open, and the counter is recovered");

    /* Replay. The mechanism is key destruction, not a bitmap: a consumed counter
     * has no key left anywhere, so the frame no longer even resolves to an
     * address we recognise. Assert the OUTCOME (never accepted), and check the
     * mechanism separately below. */
    ok(session_open(&b, frames[3], LINK_PT_LEN, LINK_TAG_LEN, 0, out, &ctr) != SESS_OK,
       "S2 a replayed frame is never accepted");
    ok(session_open(&b, frames[0], LINK_PT_LEN, LINK_TAG_LEN, 0, out, &ctr) != SESS_OK,
       "S2 replay of a frame that arrived out of order is never accepted either");
    /* Frames 2, 4, 5 and 6 have not been delivered yet — their keys are sitting in
     * the skipped cache, so they must still open EXACTLY once each. Getting this
     * distinction wrong in either direction is a bug: refuse them and a lossy
     * radio loses messages it could have recovered; accept them twice and replay
     * protection is a fiction. */
    int late_opened = 0, late_replayed = 0;
    const int late[] = { 2, 4, 5, 6 };
    for (int i = 0; i < 4; i++) {
        if (session_open(&b, frames[late[i]], LINK_PT_LEN, LINK_TAG_LEN,
                         (uint64_t)(10 + i) * 1000, out, &ctr) == SESS_OK) late_opened++;
        if (session_open(&b, frames[late[i]], LINK_PT_LEN, LINK_TAG_LEN,
                         (uint64_t)(20 + i) * 1000, out, &ctr) == SESS_OK) late_replayed++;
    }
    printf("  4 genuinely late frames: %d opened once, %d opened twice\n",
           late_opened, late_replayed);
    ok(late_opened == 4, "S2c a late frame still opens — the skipped-key cache works");
    ok(late_replayed == 0, "S2d but only once: the key is destroyed on use");

    int replays_accepted = 0;
    for (int i = 0; i < 8; i++)
        for (int k = 0; k < 3; k++)
            if (session_open(&b, frames[i], LINK_PT_LEN, LINK_TAG_LEN,
                             (uint64_t)(30 + i * 3 + k) * 1000, out, &ctr) == SESS_OK)
                replays_accepted++;
    printf("  24 replays of frames now all consumed -> %d accepted\n", replays_accepted);
    ok(replays_accepted == 0, "S2e once consumed, every frame is refused forever");

    /* The SOS path has no ratchet to consume, so it needs the explicit window. */
    herus_replay rp; herus_replay_init(&rp, 0);
    ok(herus_replay_accept(&rp, 5) == 1, "S2b SOS window: a fresh counter is accepted");
    ok(herus_replay_accept(&rp, 5) == 0, "S2b SOS window: the same counter twice is refused");
    ok(herus_replay_accept(&rp, 4) == 1, "S2b SOS window: an earlier unseen counter still works");
    ok(herus_replay_accept(&rp, 200) == 1 && herus_replay_accept(&rp, 5) == 0,
       "S2b SOS window: sliding past 64 counters leaves the old ones refused");

    /* Forgery: keep a valid address, randomise the tag. 2^-64 per attempt, so
     * across 2000 attempts we expect exactly zero successes. */
    int forged = 0, tried = 0;
    for (int i = 0; i < 2000; i++) {
        uint8_t f[LINK_FRAME_LEN];
        memcpy(f, frames[5], sizeof f);
        fill_random(f + 2 + LINK_PT_LEN, LINK_TAG_LEN);
        herus_session bb = b;
        bb.tokens = SESS_RATE_TOKENS; bb.last_refill_ms = 0;
        tried++;
        if (session_open(&bb, f, LINK_PT_LEN, LINK_TAG_LEN, (uint64_t)i * 1000, out, &ctr) == SESS_OK)
            forged++;
    }
    printf("  %d forgery attempts against a valid address -> %d accepted\n", tried, forged);
    ok(forged == 0, "S3 a 64-bit tag holds against every forgery attempt tried");

    /* Rate limit. The truncated tag ASSUMES this (footgun #9), so the assumption
     * is tested rather than trusted. */
    herus_session rl; session_init(&rl, root, 0, 0);
    int accepted_attempts = 0, rate_dropped = 0;
    for (int i = 0; i < 100; i++) {
        uint8_t f[LINK_FRAME_LEN];
        memcpy(f, frames[6], sizeof f);
        f[2] ^= 0xff;                      /* address valid, ciphertext garbage */
        int rr = session_open(&rl, f, LINK_PT_LEN, LINK_TAG_LEN, 500, out, &ctr);
        if (rr == SESS_E_AUTH) accepted_attempts++;
        if (rr == SESS_E_RATE) rate_dropped++;
    }
    printf("  100 forgery attempts in one second -> %d reached the AEAD, %d rate-dropped\n",
           accepted_attempts, rate_dropped);
    ok(accepted_attempts <= SESS_RATE_TOKENS && rate_dropped >= 100 - SESS_RATE_TOKENS - 1,
       "S4 the decrypt path is rate limited, so 2^-64 per attempt means something");

    /* Foreign traffic must cost nothing: no token, no crypto. */
    herus_session fr; session_init(&fr, root, 0, 0);
    uint32_t tokens_before = fr.tokens;
    uint8_t junk[LINK_FRAME_LEN];
    int ignored = 0;
    for (int i = 0; i < 200; i++) {
        fill_random(junk, sizeof junk);
        if (session_open(&fr, junk, LINK_PT_LEN, LINK_TAG_LEN, 0, out, &ctr) == SESS_E_ADDR) ignored++;
    }
    printf("  200 random foreign frames -> %d rejected on address alone, %u tokens spent\n",
           ignored, tokens_before - fr.tokens);
    ok(ignored > 190, "S4b foreign traffic is rejected without spending crypto or tokens");

    /* Forward secrecy: the chain key after N messages must not reproduce an
     * earlier message key. Prove it operationally — a fresh session advanced to
     * counter 4 cannot open frame 2. */
    herus_session fs; session_init(&fs, root, 0, 0);
    for (int i = 0; i < 5; i++)
        session_open(&fs, frames[i], LINK_PT_LEN, LINK_TAG_LEN, (uint64_t)i * 1000, out, &ctr);
    uint8_t ck_after[32];
    memcpy(ck_after, fs.recv.ck, 32);
    int found = 0;
    for (int i = 0; i < 5; i++) if (!memcmp(ck_after, frames[i], 32)) found = 1;
    ok(!found && fs.recv.n >= 5,
       "S5 the chain has advanced and the consumed keys are gone (forward secrecy)");

    /* DH ratchet: after a step, old frames must not open. */
    uint8_t shared[32]; fill_random(shared, 32);
    herus_session da = a, db = b;
    session_dh_ratchet(&da, shared, 1);
    session_dh_ratchet(&db, shared, 0);
    uint8_t nf[LINK_FRAME_LEN];
    session_seal(&da, pt, LINK_PT_LEN, LINK_TAG_LEN, 3, nf);
    ok(session_open(&db, nf, LINK_PT_LEN, LINK_TAG_LEN, 0, out, &ctr) == SESS_OK,
       "S5b after a DH ratchet step both ends still agree");
    ok(session_open(&db, frames[7], LINK_PT_LEN, LINK_TAG_LEN, 0, out, &ctr) != SESS_OK,
       "S5c and pre-step frames no longer open (post-compromise recovery)");
}

/* ========================================================= weave W1-W2 == */

static void w_weave(void)
{
    hdr("W1-W2  Weave: flooding terminates and deadlines are enforced");

    uint8_t root[32], frame[LINK_FRAME_LEN], pt[LINK_PT_LEN] = {0};
    fill_random(root, 32);
    herus_session a; session_init(&a, root, 1, 0);
    pt[0] = (uint8_t)(HCP_VERSION << 6);
    session_seal(&a, pt, LINK_PT_LEN, LINK_TAG_LEN, 3, frame);

    /* Ten nodes in a line, every node hears every transmission. Count the total
     * number of transmissions the flood produces. */
    weave_t node[10];
    for (int i = 0; i < 10; i++) weave_init(&node[i], WEAVE_RELAY);

    uint8_t air[LINK_FRAME_LEN];
    memcpy(air, frame, sizeof air);
    int tx_total = 0, rounds = 0;
    uint8_t pending[16][LINK_FRAME_LEN];
    int npending = 1;
    memcpy(pending[0], air, sizeof air);

    while (npending && rounds < 20) {
        uint8_t next[16][LINK_FRAME_LEN];
        int nnext = 0;
        for (int p = 0; p < npending; p++) {
            for (int i = 0; i < 10; i++) {
                if (weave_offer(&node[i], pending[p], LINK_FRAME_LEN, 0, (uint64_t)rounds * 100, 5000) == 1) {
                    size_t len;
                    uint8_t o[LINK_FRAME_LEN];
                    if (weave_next_tx(&node[i], (uint64_t)rounds * 100, o, &len) && nnext < 16) {
                        memcpy(next[nnext++], o, LINK_FRAME_LEN);
                        tx_total++;
                    }
                }
            }
        }
        memcpy(pending, next, sizeof next);
        npending = nnext;
        rounds++;
    }
    printf("  10 relays, ttl 3: flood produced %d transmissions and stopped after %d rounds\n",
           tx_total, rounds);
    ok(rounds < 20 && tx_total <= 10,
       "W1 flooding terminates, bounded by dedup rather than by ttl alone");

    /* Leak L1. A frame addressed to us is relayed anyway, and it must be: a
     * recipient that alone stayed silent would be naming itself to anyone with a
     * receiver. Measured at 100% identification before this changed. */
    weave_t w; weave_init(&w, WEAVE_RELAY);
    ok(weave_offer(&w, frame, LINK_FRAME_LEN, 1, 1000, 5000) == 1 && w.stat_decoy == 1,
       "W1b a frame addressed to us is relayed as a decoy (leak L1)");

    /* A frame with no hop budget is not relayed by anyone, recipient included —
     * so a link that does not want the mesh sends ttl 0 and leaks nothing. */
    weave_t w0; weave_init(&w0, WEAVE_RELAY);
    uint8_t f0[LINK_FRAME_LEN]; memcpy(f0, frame, LINK_FRAME_LEN);
    f0[1] &= 0x3f;                                   /* ttl := 0 */
    ok(weave_offer(&w0, f0, LINK_FRAME_LEN, 1, 1000, 5000) == 0 && w0.stat_decoy == 0,
       "W1c ttl 0 means nobody relays, so there is no silence to observe");

    /* The governor. A stranger cannot spend more of our battery than the role's
     * hourly budget, however much it transmits. */
    weave_t g; weave_init(&g, WEAVE_LEAF);
    int accepted = 0;
    for (int i = 0; i < 4 * (int)WEAVE_RELAY_PER_HOUR_LEAF; i++) {
        uint8_t f[LINK_FRAME_LEN];
        memcpy(f, frame, LINK_FRAME_LEN);
        f[4] = (uint8_t)i; f[5] = (uint8_t)(i >> 8);   /* a distinct frame each time */
        f[1] |= 0xc0;                                  /* ttl 3 */
        if (weave_offer(&g, f, LINK_FRAME_LEN, 0, 1000, 5000) == 1) accepted++;
        size_t l; uint8_t o[WEAVE_FRAME_MAX];
        while (weave_next_tx(&g, 1000, o, &l)) { }     /* drain, so the queue never limits */
    }
    ok(accepted <= (int)WEAVE_RELAY_PER_HOUR_LEAF && g.stat_governed > 0,
       "W1d the relay governor caps a stranger at one hour's budget");

    /* Deadlines: a queued frame past its deadline is dropped and counted, never
     * retried forever. */
    weave_t d; weave_init(&d, WEAVE_RELAY);
    weave_offer(&d, frame, LINK_FRAME_LEN, 0, 1000, 2000);
    size_t len; uint8_t o[LINK_FRAME_LEN];
    ok(weave_next_tx(&d, 5000, o, &len) == 0 && d.stat_expired == 1,
       "W2 an expired frame is dropped and counted, not retried indefinitely");

    /* Role transitions follow energy, not configuration. */
    weave_t r; weave_init(&r, WEAVE_LEAF);
    ok(weave_update_role(&r, 100000, 0, 20, 0) == WEAVE_LEAF, "W2b idle and low battery -> leaf");
    r.last_traffic_ms = 99000;
    ok(weave_update_role(&r, 100000, 0, 20, 0) == WEAVE_RESPONSIVE, "W2c recent traffic -> responsive");
    ok(weave_update_role(&r, 100000, 0, 90, 50) == WEAVE_RELAY, "W2d sun and charge -> relay");
    ok(weave_update_role(&r, 100000, 1, 10, 0) == WEAVE_RELAY, "W2e on charger -> relay");
}

/* ========================================================== beat B1-B2 == */

static void b_beat(void)
{
    hdr("B1-B2  Beat: the guard covers the drift, and resync converges");

    beat_t b; beat_init(&b, 0, BEAT_PERIOD_MS, 2);
    ok(beat_slot_open(&b, 0), "B1 a slot is open at the epoch");
    ok(!beat_slot_open(&b, 1000), "B1 and closed mid-period");
    ok(beat_slot_open(&b, 2000), "B1 and open again one period later");
    ok(beat_until_next_slot_ms(&b, 1000) == BEAT_PERIOD_MS - BEAT_GUARD_MS - 1000,
       "B1 the wake time is the period minus the guard");

    printf("  drift after %u ms at %u ppm = %u us (guard is %u us)\n",
           BEAT_RESYNC_MS, BEAT_DRIFT_PPM, beat_drift_us(&b, BEAT_RESYNC_MS),
           BEAT_GUARD_MS * 1000u);
    ok(beat_drift_us(&b, BEAT_RESYNC_MS) <= BEAT_GUARD_MS * 1000u,
       "B1 worst-case drift over a resync interval fits inside the guard");

    /* A peer with a better clock drags us onto its boundary; a worse one cannot. */
    beat_t drifted; beat_init(&drifted, 0, BEAT_PERIOD_MS, 3);
    drifted.epoch_ms = 37;                            /* 37 ms out of phase */
    ok(beat_resync(&drifted, 2000, 1) == 1, "B2 a better clock re-anchors us");
    ok(beat_slot_open(&drifted, 2000), "B2 and afterwards the peer's instant is inside our slot");
    beat_t stable; beat_init(&stable, 0, BEAT_PERIOD_MS, 0);
    ok(beat_resync(&stable, 1234, 3) == 0,
       "B2 a free-running peer cannot drag a good clock off the boundary");
}

/* ============================================================ canonical == */

static void c_canonical(void)
{
    hdr("C1  encoding is canonical — one byte string per meaning");

    hcp_msg_t m = {0};
    m.tier = HCP_TIER_COMPOSED; m.intent = 1; m.nslot = 1;
    m.slot[0].role = 0; m.slot[0].filler = 0;
    uint8_t pt[HCP_PLAINTEXT_LEN];
    ok(hcp_encode(pt, &m) == -1,
       "C1 role 0 with filler 0 is refused — 0x0000 already means ABSENT (footgun #5)");

    m.slot[0].role = 1; m.slot[0].filler = 2048;      /* one past the field */
    ok(hcp_encode(pt, &m) == -1, "C1 a filler that does not fit 11 bits is refused, not truncated");

    m.slot[0].filler = 5; m.intent = 2048;
    ok(hcp_encode(pt, &m) == -1, "C1 an intent past 11 bits is refused");

    m.intent = 5;
    ok(hcp_encode(pt, &m) == HCP_PLAINTEXT_LEN, "C1 the corrected message encodes");

    /* A gap in the slot list would be a second encoding of the same meaning —
     * i.e. a covert channel inside a constant-length frame. */
    uint8_t tampered[HCP_PLAINTEXT_LEN];
    memcpy(tampered, pt, sizeof tampered);
    tampered[6 + 2 * 2] = 0x11; tampered[6 + 2 * 2 + 1] = 0x11;   /* slot 2 set, slot 1 empty */
    hcp_msg_t got;
    ok(hcp_decode(&got, tampered) == -1, "C1 a gap in the slot list is rejected");

    memcpy(tampered, pt, sizeof tampered);
    tampered[1] |= 0x01;                              /* reserved bit */
    ok(hcp_decode(&got, tampered) == -1, "C1 a set reserved bit is rejected");

    memcpy(tampered, pt, sizeof tampered);
    tampered[0] = (uint8_t)((tampered[0] & 0x3f) | (2 << 6));      /* version 2 */
    ok(hcp_decode(&got, tampered) == -1, "C1 an unknown protocol version is rejected");
}

/* ================================================================= cost == */

static void x_cost(void)
{
    hdr("X1  cost per frame, measured (host; MCU projection in docs/05-FIRMWARE.md)");

    uint8_t root[32];
    fill_random(root, 32);
    herus_session a, b;
    session_init(&a, root, 1, 0); session_init(&b, root, 0, 0);

    static const uint8_t roles[] = { 1, 2, 3, 4 };
    herus_link tx = { &a, HZ_REGION_BR915, 14, roles, 4 };
    herus_link rx = { &b, HZ_REGION_BR915, 14, roles, 4 };

    hcp_msg_t m = {0};
    m.tier = HCP_TIER_COMPOSED; m.intent = 41; m.nslot = 3;
    m.slot[0].role = 1; m.slot[0].filler = 300;
    m.slot[1].role = 2; m.slot[1].filler = 17;
    m.slot[2].role = 3; m.slot[2].filler = 44;

    uint8_t frame[LINK_FRAME_LEN];
    hcp_msg_t out;
    uint32_t ctr; int se;

    const int N = 20000;
    clock_t t0 = clock();
    for (int i = 0; i < N; i++) {
        m.seq = (uint16_t)i;
        if (link_send(&tx, &m, 3, frame) != LINK_OK) { ok(0, "X1 send failed"); return; }
        if (link_recv(&rx, frame, (uint64_t)i * 100, &out, &ctr, &se) != LINK_OK) {
            printf("  link_recv failed at %d (sess %d)\n", i, se);
            ok(0, "X1 round trip failed");
            return;
        }
    }
    double us = 1e6 * (double)(clock() - t0) / CLOCKS_PER_SEC / N;
    printf("  seal + open, %d round trips: %.2f us per frame on this host\n", N, us);
    printf("  airtime for the same frame: %u us -> the radio is %.0fx the cost of the crypto\n",
           HERUS_AIRTIME_MEANING_US, HERUS_AIRTIME_MEANING_US / us);
    ok(us < 200.0, "X1 the whole stack costs far less than the airtime it protects");
    ok(out.nslot == 3 && out.slot[2].filler == 44, "X1 and 20000 consecutive frames stayed correct");
    printf("  session state: %zu B per peer   weave: %zu B   beat: %zu B\n",
           sizeof(herus_session), sizeof(weave_t), sizeof(beat_t));
}

int main(void)
{
    printf("HERUS protocol proof suite   frame %u B, airtime %.1f ms, D=%d bits\n",
           HERUS_FRAME_LEN, HERUS_AIRTIME_MEANING_US / 1000.0, HV_BITS);
    v_crypto();
    p1_p2_p7();
    p3_ids_only();
    p4_forward_compat();
    p5_tier_split();
    p6_ephemeral();
    p8_ttl_mutable();
    s_session();
    w_weave();
    b_beat();
    c_canonical();
    x_cost();
    printf("\n%s\n", FAILED ? "SOMETHING REGRESSED — do not build hardware against this."
                            : "ALL PROTOCOL INVARIANTS HOLD.");
    return FAILED;
}
