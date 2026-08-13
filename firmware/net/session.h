/* session.h — the symmetric ratchet, ephemeral addresses, replay defence.
 *
 * This is the half of docs/02-PROTOCOL.md §4 that runs in MCU RAM. The other
 * half — the P-256 identity key and ECDH — lives in the ATECC608A and is
 * deliberately not here (see crypto.h).
 *
 * WHAT THIS GIVES YOU, PRECISELY
 * ------------------------------
 * A KDF chain per direction. Each message consumes one chain key and the old one
 * is overwritten, so:
 *
 *   FORWARD SECRECY:            yes. Compromising the device now does not reveal
 *                               yesterday's messages — those chain keys are gone.
 *   POST-COMPROMISE SECURITY:   NOT from this file. Recovering from a compromise
 *                               requires new asymmetric material, i.e. a DH
 *                               ratchet step. `session_dh_ratchet()` is the hook
 *                               and it does the symmetric mixing correctly; it
 *                               needs an ECDH shared secret from the ATECC to be
 *                               called. Until Phase 4 wires that up, say
 *                               "forward secrecy", never "Signal-grade".
 *
 * Stating that boundary is the difference between a security property and a
 * marketing sentence, and it is cheap to state.
 *
 * OUT-OF-ORDER AND LOSS
 * ---------------------
 * A lossy radio delivers gaps and reorderings. The receiver keeps:
 *   - a window of SESS_WINDOW future addresses, so foreign traffic is rejected
 *     without ever attempting decryption (docs §4.3, 64 bytes of table);
 *   - a cache of skipped message keys, so a late frame from a counter we already
 *     passed still opens.
 *
 * REPLAY, AND A BITMAP THIS FILE DELIBERATELY DOES NOT HAVE
 * --------------------------------------------------------
 * docs/02-PROTOCOL.md §5.3 lists a receiver-side sequence window as one of three
 * replay defences. For the AEAD tiers it is redundant, and shipping redundant
 * security machinery is worse than shipping none: it never executes, so it is
 * never tested, and it invites the belief that it is doing something.
 *
 * The reason it is redundant: every message key here is SINGLE USE. It is
 * derived, used, and destroyed — either by the chain advancing past it or by
 * dropping its skipped-key cache entry on success. A replayed frame therefore has
 * no key left to open it, and is rejected as an unknown address. Key deletion is
 * strictly stronger than a bitmap (it also gives forward secrecy) and costs
 * nothing extra. Proven in test_net S2.
 *
 * A sequence window IS required for the SOS tier, which is plaintext, signed, and
 * has no ratchet to consume. `herus_replay` below exists for that path and for no
 * other — with the ATECC's monotonic counter as its source of truth.
 *
 * RATE LIMIT (footgun #9)
 * -----------------------
 * The on-air tag is truncated to 64 bits, i.e. 2^-64 per forgery ATTEMPT. A
 * token bucket caps attempts per second per session. Without it, truncation is
 * not a byte saving but a downgrade, and an attacker with a transmitter gets to
 * choose how many attempts they make.
 */
#ifndef HERUS_SESSION_H
#define HERUS_SESSION_H

#include <stdint.h>
#include <stddef.h>
#include "crypto.h"

#define SESS_WINDOW        32      /* precomputed addresses / max skip */
/* Cached out-of-order message keys.
 *
 * This was 32, and at 52 bytes an entry it was 1664 of the 1896 bytes a session
 * costs — 88% of the dominant per-peer allocation, sized by round number rather
 * than by anything. The bench measured the depth actually reached across 396
 * sessions of a twelve-unit crowd with heavy collisions: ONE.
 *
 * One measurement is not a bound, so the value is not set from it. The bound
 * comes from the only thing in the system that can deliver a frame out of order:
 * a relay's store-and-forward queue, which is WEAVE_QUEUE_N deep and expires
 * after its deadline. Nothing else can reorder, so nothing else can need a
 * deeper cache. Sizing it to exactly that is a derivation; 32 was a guess and 8
 * is not a smaller guess.
 *
 * What it costs when exceeded, stated: a frame delayed past WEAVE_QUEUE_N later
 * arrivals from the same peer no longer opens. It was already tens of seconds
 * late on a 2 s beat. What it buys: 1248 bytes per peer, which at 32 peers is
 * 39 KB of an ESP32-S3's 512 KB — the difference between "the session table is
 * the RAM budget" and "the session table is a line item". */
#define SESS_SKIP_MAX       8
#define SESS_RATE_TOKENS   20      /* decrypt attempts per second per session */

/* ---------------- RECOVERY AFTER A LOSS BURST ----------------
 *
 * The note further down this file says that losing more than SESS_WINDOW
 * consecutive frames leaves us unable to distinguish our peer from a stranger,
 * and that "the recovery is an application-level timeout followed by a re-key,
 * and that is the honest place for it."
 *
 * That was honest about the mechanism and wrong about the consequence. There was
 * no re-key anywhere in the tree, so the real behaviour was: a 150 s walk through
 * a basement ends the link permanently, and no amount of standing next to each
 * other afterwards brings it back. The bench measured it (sim scenario S1) —
 * 50 frames lost against a 32-frame window, counters 90 apart, and nothing ever
 * opened again.
 *
 * A re-key is not needed and would be the wrong tool: it costs a handshake over a
 * radio that may be one-way at that moment. What is needed is for the RECEIVER to
 * look further ahead, which it can do alone, offline, with the keys it already
 * has. session_recover() walks the receive chain forward from the window frontier
 * for up to SESS_RECOVER_SPAN counters looking for the frame's address.
 *
 * Why this is safe, stated rather than assumed:
 *   - It only ever walks FORWARD. Keys behind us stay destroyed, so forward
 *     secrecy is untouched. Recovery cannot reopen yesterday.
 *   - It advances the chain only on an AEAD SUCCESS. An address match alone
 *     proves nothing and moves nothing, so an attacker who guesses an address
 *     (1 in 16384, and 1024 tries buys 6%) still cannot desynchronise us.
 *   - It is expensive — 2 HMACs per counter — so it is gated twice: only when the
 *     link has been silent for SESS_LOST_MS, and at most once per
 *     SESS_RECOVER_COOLDOWN_MS. Worst case is 2048 HMACs per 30 s per peer, which
 *     is under 0.2% of an ESP32-S3.
 *
 * The span is a deliberate, stated limit: a peer that has sent more than
 * SESS_RECOVER_SPAN frames while we were away is genuinely unreachable and needs
 * a re-pair. At 1024 frames and a 246.8 ms airtime that is over four hours of
 * one peer talking continuously into the void. */
#define SESS_RECOVER_SPAN         1024u
#define SESS_RECOVER_COOLDOWN_MS 30000u
#define SESS_LOST_MS             60000u

/* ---------------- ERRATUM E-P2: where the hop counter lives ----------------
 *
 * docs/02-PROTOCOL.md §3.2 puts `ttl` in the plaintext header and §5.2 has each
 * relay decrement it. Those two sentences cannot both be true: the plaintext is
 * inside the end-to-end AEAD, so a relay can neither read nor modify ttl, and
 * §5.2's dedup key (ephemeral_addr, seq) is likewise unreadable — seq is
 * encrypted too. As specified, Weave cannot be implemented.
 *
 * There is no spare byte (P1 fixes the frame at 34), so the fix is to spend two
 * bits of the on-air address:
 *
 *     on air:  bits 15..14 = ttl        public, mutable, NOT authenticated
 *              bits 13..0  = address    public, authenticated via AAD
 *
 * Consequences, stated rather than discovered later:
 *   - The address space drops from 65536 to 16384, so collisions become 4x more
 *     likely. The protocol already declares collisions acceptable and even
 *     useful for traffic analysis resistance; the cost is one wasted decrypt
 *     attempt, which the rate limiter bounds.
 *   - ttl is malleable. An attacker can raise it to 3 or drop it to 0. Raising
 *     it cannot amplify a flood beyond the network diameter, because dedup makes
 *     every node forward a given frame at most once. Dropping it suppresses
 *     forwarding — which the threat model already concedes ("drop/delay is not
 *     defended") and which jamming achieves anyway.
 *   - Modifying the address bits breaks the tag, because they ARE in the AAD.
 *     Proven in test_net P8.
 *
 * Dedup therefore keys on (address, hash of ciphertext+tag) — see weave.h — which
 * a relay can compute, unlike (addr, seq).
 */
#define SESS_ADDR_BITS     14
#define SESS_ADDR_MASK     ((1u << SESS_ADDR_BITS) - 1u)   /* 0x3FFF */
#define SESS_TTL_MAX       3u

/* Return codes. Distinguishable because the caller's response differs: AUTH
 * means someone is transmitting garbage at us, RATE means we are being flooded,
 * REPLAY means an attacker or a duplicated relay, WINDOW means we have lost too
 * many frames and must re-key. */
enum {
    SESS_OK      =  0,
    SESS_E_ADDR  = -1,   /* address not in window — not for us, cheapest reject */
    SESS_E_AUTH  = -2,   /* address matched, tag did not */
    SESS_E_REPLAY= -3,
    SESS_E_RATE  = -4,
    SESS_E_ARG   = -6
};
/* There is deliberately no "window exceeded" code. If more than SESS_WINDOW
 * consecutive frames are lost, the sender's address is no longer in our table and
 * its frames become indistinguishable from a stranger's — we return SESS_E_ADDR
 * and cannot know better, because knowing better would require a stable
 * identifier on air, which is exactly what P6 forbids. The recovery is an
 * application-level timeout followed by a re-key, and that is the honest place
 * for it. Inventing an error code we cannot actually detect would be worse than
 * the silence. */

typedef struct {
    uint8_t  ck[32];
    uint32_t n;
} herus_chain;

typedef struct {
    uint32_t n;
    uint16_t addr;
    uint8_t  key[32];
    uint8_t  nonce[12];
} herus_skipped;

typedef struct {
    herus_chain send, recv;
    /* Frontier: the chain key at recv.n + SESS_WINDOW. Keeping it means sliding
     * the address window after an accepted frame costs 2 HMACs per counter
     * advanced instead of 64 for a full rebuild — a 30x saving on the hot path,
     * and the hot path runs on a battery. */
    herus_chain win;

    uint16_t addr_win[SESS_WINDOW];      /* for recv.n .. recv.n+SESS_WINDOW-1 */

    herus_skipped skip[SESS_SKIP_MAX];
    int      nskip;

    uint32_t tokens;                     /* rate limit */
    uint64_t last_refill_ms;

    /* Recovery bookkeeping. last_open_ms answers "is this link actually lost?",
     * which is the only condition under which the deep walk is worth its HMACs. */
    uint64_t last_open_ms;
    uint64_t last_recover_ms;

    /* counters, for the field log — cheap and they answer "is the link healthy
     * or is someone probing us?", which no single metric does */
    /* No stat_replay: with single-use keys a replay is indistinguishable from an
     * unknown address, so a "replays seen" counter would only ever report zero
     * and would imply a detection we do not have. */
    uint32_t stat_sent, stat_opened, stat_auth_fail, stat_rate_drop;
    uint32_t stat_recovered;             /* deep resyncs that actually landed */
} herus_session;

/* Derive both chains from a shared root key. `initiator` must differ between the
 * two ends: it selects which label makes the send chain and which the receive
 * chain, so A's send chain is B's receive chain. Getting this backwards on both
 * ends produces a session that never opens a single frame — which is the correct
 * failure mode (loud) rather than the wrong one (silent halved key space). */
void session_init(herus_session *s, const uint8_t root[32], int initiator,
                  uint64_t now_ms);

/* Mix new asymmetric material (an ECDH output from the ATECC) into both chains.
 * This is the DH ratchet step that buys post-compromise security. Both ends must
 * perform it at the same point in the message stream. */
void session_dh_ratchet(herus_session *s, const uint8_t shared[32], int initiator);

/* --------- send path ---------
 * Writes a 34-byte frame: hdr(2) || ciphertext(ptlen) || tag(tag_len), where the
 * 2-byte header packs ttl:2 | addr:14 (erratum E-P2 above). Herus uses
 * 2 + 24 + 8 = 34. Returns SESS_OK or SESS_E_ARG. */
int session_seal(herus_session *s, const uint8_t *pt, size_t ptlen,
                 size_t tag_len, uint8_t ttl, uint8_t *frame_out);

/* Rewrite the ttl bits of a frame in place, for a relay. Cannot invalidate the
 * tag: the ttl bits are excluded from the AAD by construction. Returns the new
 * ttl, or -1 if it was already 0 (in which case the frame must be dropped, not
 * forwarded). */
int  session_frame_decrement_ttl(uint8_t *frame);
uint8_t session_frame_ttl(const uint8_t *frame);

/* --------- receive path ---------
 * `frame` is addr(2) || ct(ptlen) || tag(tag_len). On SESS_OK, `pt_out` holds
 * ptlen plaintext bytes and *counter_out the message counter that opened it.
 *
 * Cost of rejecting foreign traffic: one 32-entry uint16 scan. Cost of rejecting
 * a forgery: one AEAD over 24 bytes. Both are bounded, which is the property
 * that matters when the attacker controls how often you pay them. */
int session_open(herus_session *s, const uint8_t *frame, size_t ptlen,
                 size_t tag_len, uint64_t now_ms,
                 uint8_t *pt_out, uint32_t *counter_out);

/* Deep resync. Call this ONLY after session_open has returned SESS_E_ADDR and the
 * link has been silent long enough to believe it is lost — the caller knows both,
 * and making it a separate entry point keeps the hot path exactly as cheap as it
 * was. Returns SESS_OK and fills pt_out/counter_out on success, SESS_E_RATE if
 * the cooldown has not elapsed, SESS_E_ADDR if the peer is not within the span.
 *
 * On success the receive chain jumps to the recovered counter. Every message in
 * the gap is permanently unreadable, which is correct: those chain keys were
 * destroyed on purpose. Recovery restores the LINK, never the lost messages. */
int session_recover(herus_session *s, const uint8_t *frame, size_t ptlen,
                    size_t tag_len, uint64_t now_ms,
                    uint8_t *pt_out, uint32_t *counter_out);

/* True if the frame's 14-bit address is in our receive window. A relay uses this
 * to answer "is this mine?" without touching the crypto, and the answer is also
 * "should I forward it?" — a frame addressed to us needs no relaying. */
int  session_addr_in_window(const herus_session *s, const uint8_t *frame);

/* --------- replay window for the SOS / plaintext path ---------
 * A 64-counter sliding bitmap. The AEAD tiers do not use it (see the note above);
 * SOS does, because a signed plaintext beacon can be captured and re-transmitted
 * verbatim and nothing about the signature says "again". */
typedef struct { uint64_t mask; uint32_t base; } herus_replay;

void herus_replay_init(herus_replay *r, uint32_t base);
/* 1 if this counter is fresh (and it is now recorded), 0 if it is a replay or has
 * fallen behind the 64-counter window. */
int  herus_replay_accept(herus_replay *r, uint32_t counter);

/* --------- Tier 0.5, broadcast sketch ---------
 * Keystream only: no MAC, by design (P5 / footgun #3). Poly1305 avalanches, so a
 * single bit error would destroy exactly the frame this tier exists to recover.
 * Integrity comes from nearest-neighbour plausibility instead: garbage resolves
 * to no symbol above threshold.
 *
 * Confidentiality here is GROUP-level (the domain key), not pairwise: a broadcast
 * beacon has no session. Say that out loud in the UI — a user who thinks a
 * beacon is as private as a message has been misled by us, not by the radio. */
void sketch_seal(const uint8_t gkey[32], uint32_t epoch,
                 const uint8_t *sketch, size_t sketch_len, uint8_t *frame_out);
int  sketch_open(const uint8_t gkey[32], uint32_t epoch,
                 const uint8_t *frame, size_t sketch_len, uint8_t *sketch_out);
/* Keystream padding that carries the sketch frame from 34 to the 38 bytes that
 * make its airtime identical to Tier 0/1 (erratum E-P1). Keystream, not zeros:
 * a constant tail at a fixed offset in every beacon is a fingerprint. */
void sketch_pad(const uint8_t gkey[32], uint32_t epoch, size_t sketch_len,
                uint8_t *pad_out, size_t pad_len);

#endif /* HERUS_SESSION_H */
