/* hcp.h — Herus Composition Protocol, revision 0.2 (normative: docs/02-PROTOCOL.md).
 *
 * The unit of communication is a MEANING, not a signal. A message is an intent
 * plus role/filler bindings, carried as 24 bytes of symbol ids and rendered into
 * the receiver's own language and voice at the far end.
 *
 * Two consequences fall out for free and are the reason for the design:
 *   - Translation. The sender transmits meaning, so the receiver renders it in
 *     whatever language it is configured for. No translation engine.
 *   - Forward compatibility (P4). Slots carry a role id; a receiver iterates the
 *     roles IT knows and ignores the rest, so old firmware skips new fields
 *     instead of rejecting the frame. No version negotiation, ever.
 *
 * WHAT CHANGED FROM REVISION 0.1, AND WHY
 * ---------------------------------------
 * 0.1 used a 16-byte plaintext with 3 bytes per slot (role:8, filler:16) and a
 * maximum of 4 slots. That frame was 26 B on air while Tier 0.5 was 34 B, which
 * silently broke P1 — the constant-length invariant the confidentiality claim
 * rests on. Unifying upward to 34 B gives 24 B of plaintext, and repacking a
 * slot as `role:5 | filler:11` spends those bytes on capacity instead of
 * padding: 4 slots become 9, across 32 roles and a 2048-symbol filler space.
 *
 * COST DISCIPLINE
 * ---------------
 * Nothing here allocates, nothing here is static-mutable, and every search takes
 * its candidate range from the caller. `hcp_query_role` over a 2048-symbol
 * filler space costs 2048 code derivations (tens of ms on an ESP32-S3) — which
 * is why the normal receive path does NOT use it. The wire carries ids;
 * unbinding is for cognition (matching a heard phrase against memory), not for
 * decoding. Confusing the two is how a 250 ms frame becomes a 3 s round trip.
 */
#ifndef HERUS_HCP_H
#define HERUS_HCP_H

#include "lexicon.h"

#define HCP_VERSION        1
#define HCP_PLAINTEXT_LEN  24      /* P1: constant for every meaning tier */
#define HCP_HDR_LEN         2
#define HCP_MAX_SLOT        9

/* ---------------- TWO PLAINTEXT LENGTHS, ONE ENCODING ----------------
 *
 * The wire layout is 6 fixed bytes (ver/tier/flags, ttl/prio, intent, seq)
 * followed by 2 bytes per slot. Nothing about it depends on the total length, so
 * a shorter frame is the SAME format with fewer slots — not a second encoding,
 * not a second decoder, not a second set of footguns.
 *
 * That matters because of where the shorter frame comes from. Range here is
 * bounded by the 400 ms dwell rule, not by the radio, and the dwell budget buys
 * spreading factor with bytes:
 *
 *     SF9    up to 59 B      -129.5 dBm      (34 B used: 25 B of headroom spent
 *                                             on expressiveness)
 *     SF10   up to 24 B      -132.0 dBm      +2.5 dB
 *     SF11   up to  0 B      -134.5 dBm      the preamble alone leaves no room
 *     SF12   impossible      the preamble alone is 663 ms
 *
 * So 24 bytes at SF10 is not a tuning choice, it is the ceiling: one more byte
 * is illegal, and the next spreading factor cannot carry a message at all.
 * region.h asserts both at compile time. A 24-byte frame is 2 address + 8 tag +
 * 14 plaintext, and 14 plaintext is 6 fixed + 4 slots.
 *
 * The trade is stated rather than sold: +2.5 dB of link budget, which is +15.5%
 * of range in the d^4 regime a wrist device lives in, paid for with 5 of the 9
 * slots and 50% more airtime per frame (370.7 ms against 246.8 ms). Reach is
 * further, not cheaper.
 *
 * P1 still holds, and holds EXACTLY, because the choice is per group rather than
 * per message: every frame a group transmits has identical airtime. A group that
 * mixed the two would be publishing which of its messages were urgent. */
#define HCP_FIXED_LEN      6u
#define HCP_PT_RICH       24u      /* SF9,  34 B on air, 9 slots */
#define HCP_PT_REACH      14u      /* SF10, 24 B on air, 4 slots */
#define HCP_SLOTS_IN(pt)  (((pt) - HCP_FIXED_LEN) / 2u)

/* Wire tiers. 3 bits, so 8 are available and 5 are used. */
enum { HCP_TIER_GLYPH = 0, HCP_TIER_COMPOSED = 1, HCP_TIER_VOICE = 2,
       HCP_TIER_SKETCH = 3, HCP_TIER_SOS = 4 };

/* Slot packing (docs/02-PROTOCOL.md §3.3). A slot of 0x0000 means ABSENT — it is
 * not "role 0, filler 0", and hcp_encode refuses to emit that combination rather
 * than letting it become a message that round-trips wrong (footgun #5). */
#define HCP_ROLE_BITS      5
#define HCP_FILLER_BITS   11
#define HCP_ROLE_N        (1u << HCP_ROLE_BITS)     /* 32   */
#define HCP_FILLER_N      (1u << HCP_FILLER_BITS)   /* 2048 */
#define HCP_INTENT_N      2048u

/* Symbol id space, shared by every unit in a domain. Codes are derived from
 * (domain_seed, id) and never stored, so a wide space costs no RAM — only the
 * materialised sketch table costs, and only for the symbols you choose to
 * prefilter (see lexicon.h). */
#define HCP_ID_INTENT_BASE     0u
#define HCP_ID_ROLE_BASE    2048u
#define HCP_ID_FILLER_BASE  2080u
#define HCP_ID_PAD          4128u   /* parity filler, never a real symbol */
#define HCP_ID_MAX          4129u

typedef struct {
    uint8_t  ver;                   /* 2 bits */
    uint8_t  tier;                  /* 3 bits */
    uint8_t  flags;                 /* 3 bits */
    uint8_t  ttl;                   /* 4 bits, Weave hop budget */
    uint8_t  prio;                  /* 2 bits */
    uint16_t intent;                /* symbol id, 11 significant bits */
    uint16_t seq;                   /* application sequence, replay window */
    uint8_t  nslot;
    struct { uint8_t role; uint16_t filler; } slot[HCP_MAX_SLOT];
    /* The ON-AIR position of each slot, which is NOT the same as its index in
     * slot[] once hcp_filter_known_roles has dropped fields a newer sender added.
     * The sender bound slot k under rho^(k+2), so anything that unbinds must use
     * pos[i] and not i — a receiver that renumbered densely would unbind at the
     * wrong rotation and read noise. Local only: never transmitted. */
    uint8_t  pos[HCP_MAX_SLOT];
} hcp_msg_t;

/* Encode always writes exactly HCP_PLAINTEXT_LEN bytes and zero-pads the tail
 * BEFORE the caller encrypts (footgun #1). Returns the length, or -1 if a field
 * does not fit its bit budget — a truncating encoder is a protocol violation
 * that surfaces only as a wrong answer at the far end.
 *
 * Zero-padding the tail is load-bearing, not hygiene: padding after encryption
 * leaves the true length visible in the ciphertext structure. */
/* NOTE: *m is not const. Encoding stamps pos[] with the on-air positions it just
 * used, so the very same struct can then be passed to hcp_to_hv. Without that, a
 * sender that built its message with `= {0}` would have pos[] all zero, bind every
 * slot at rho^2, and produce a vector the receiver cannot reproduce — a bug that
 * would only ever show up as "matching works locally but not over the air". */
int  hcp_encode(uint8_t *out24, hcp_msg_t *m);

/* The same encoder at an explicit plaintext length. hcp_encode is this at
 * HCP_PT_RICH. Returns ptlen, or -1 if a field — including the slot count for
 * this length — does not fit. */
int  hcp_encode_n(uint8_t *out, hcp_msg_t *m, unsigned ptlen);

/* Returns 0 on success, -1 on a version or arity violation. Never reads past
 * HCP_PLAINTEXT_LEN and never writes outside *m. */
int  hcp_decode(hcp_msg_t *m, const uint8_t *in24);
int  hcp_decode_n(hcp_msg_t *m, const uint8_t *in, unsigned ptlen);

/* ---------------- P4: forward compatibility, made executable ----------------
 * Copy into *out only the slots whose role appears in known_roles[]. The return
 * value is how many were kept; the ones dropped are exactly the fields a newer
 * sender added. An implementation that instead rejects the frame has turned every
 * future field addition into a flag day.
 *
 * out->pos[] carries each kept slot's ORIGINAL on-air position, so unbinding still
 * works after filtering. */
int  hcp_filter_known_roles(const hcp_msg_t *m,
                            const uint8_t *known_roles, int nknown,
                            hcp_msg_t *out);

/* ---------------- local semantic form ----------------
 * M = rho^1(phi(intent))  bundled with  rho^(k+2)( phi(role_k) XOR phi(filler_k) )
 *
 * The permutation per slot is mandatory (footgun #6): XOR is commutative, so
 * without rho^(k+2) the record is an unordered heap and slot order is lost — the
 * same defect that made the draft's relation encoding answer
 * capital(Brasilia)=Brazil. Proven in test_net.c P6.
 *
 * `scratch` is a caller-owned accumulator (sizeof(hv_acc_t) = 20 KB at D=10240).
 * It is a parameter and not a static so the RAM appears in the caller's budget,
 * where it can be seen. */
void hcp_to_hv(hv_t *out, hv_acc_t *scratch, const lex_t *L, const hcp_msg_t *m);

/* Recover the filler bound to `role` from a composed hypervector, searching the
 * caller-chosen candidate range [filler_lo, filler_lo + filler_n).
 *
 * Returns 1 and writes *filler if the evidence exceeds `sigma` standard
 * deviations below the random baseline; 0 if the role is absent. Present or
 * absent is a hypothesis test, not a lookup: an absent role unbinds to noise
 * whose nearest neighbour still sits a few sigma from D/2, while a present one
 * lands tens of sigma below it. sigma=12 is the tested default and is derived,
 * not chosen — see test_herus T11 and docs/01-ALGEBRA.md §5.
 *
 * `slot_index` is the slot's ON-AIR position — pass m->pos[i], not i, or the
 * rotation will not match what the sender bound. */
int  hcp_query_role(const hv_t *H, const lex_t *L, uint8_t role, int slot_index,
                    uint16_t filler_lo, uint16_t filler_n, double sigma,
                    uint16_t *filler);

#endif /* HERUS_HCP_H */
