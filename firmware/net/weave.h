/* weave.h — the mesh layer: duplicate suppression, hop limiting, deadlines.
 *
 * Weave is deliberately not a routing protocol. It is flooding with duplicate
 * suppression and an energy-aware decision about who bothers to listen. Routing
 * tables need topology knowledge that a wrist device sleeping 99% of the time
 * cannot maintain, and docs/03-BUILD-GUIDE.md Phase 1 is explicit that
 * reinventing mesh routing is a six-month detour that teaches nothing about
 * Herus. Benchmark against Meshtastic instead.
 *
 * THE DEDUP KEY, CORRECTED (erratum E-P2, see session.h)
 * -----------------------------------------------------
 * docs/02-PROTOCOL.md §5.2 says dedup on (ephemeral_addr, seq). A relay cannot
 * do that: `seq` lives in the plaintext, inside the end-to-end AEAD. What a relay
 * CAN see is the 14-bit address, the ciphertext and the tag — and the tag is a
 * 64-bit function of the whole message under a key the relay does not have,
 * which makes it a far better duplicate discriminator than a 16-bit sequence
 * number ever was.
 *
 * So the key is FNV-1a over (address with ttl masked off) || ciphertext || tag.
 * Masking the ttl is essential: the same frame arrives at ttl 3 and ttl 2 and
 * must be recognised as ONE frame, or the flood never terminates.
 *
 * WHY FLOODING TERMINATES
 * -----------------------
 * Two independent limits, and either alone is sufficient:
 *   1. ttl, decremented per hop, dropped at zero.
 *   2. dedup — every node forwards a given frame at most once, ever, so the
 *      total transmissions are bounded by the node count regardless of ttl.
 * Limit 2 is what makes E-P2's malleable ttl safe: an attacker who raises ttl
 * cannot make a node forward twice.
 */
#ifndef HERUS_WEAVE_H
#define HERUS_WEAVE_H

#include <stdint.h>
#include <stddef.h>

/* ---------------- WHY A RECIPIENT ALSO RELAYS (leak L1) ----------------
 *
 * Revision 1 of this file returned early on `mine`, with the comment "ours:
 * consume, do not relay". It is the obvious optimisation and it is a
 * deanonymiser. Every node that CANNOT read a frame retransmits it; the one that
 * CAN does not. An observer with a single receiver and no key at all writes down
 * who stayed silent and has the recipient.
 *
 * The bench measured it: 8 units, 40 messages, recipient identified with 100%
 * accuracy (sim scenario S3). P6 keeps every stable identifier off the air, and
 * says nothing about what a node DOES with a frame — but doing nothing is an
 * observable, and an anonymity set of one is not an anonymity set.
 *
 * So `mine` no longer suppresses forwarding. The recipient relays under exactly
 * the same rules as everyone else, and the silence carries no information.
 *
 * The cost is one extra transmission per frame received with ttl > 0, and it is
 * charged only to traffic that asked for the mesh. A direct link that does not
 * need relaying sends with ttl = 0, in which case nobody relays, the recipient
 * included, and there is nothing to observe. That is the rule to follow: ttl is
 * not a "how far might this go" hint, it is a request for mesh service, and
 * asking for it costs one frame.
 */
#define WEAVE_DEDUP_N     256      /* ring of recent frame fingerprints */
#define WEAVE_QUEUE_N       8      /* store-and-forward depth */
#define WEAVE_FRAME_MAX    38      /* the largest frame Weave relays */

/* Energy-aware roles (docs/02-PROTOCOL.md §5.1). The role is advertised in the
 * beacon so routing follows energy rather than a configuration flag: a Band with
 * 20 cm2 of cell makes a node relay-capable, a leather Band makes it a leaf. The
 * user's strap choice is a routing decision. */
typedef enum {
    WEAVE_LEAF = 0,        /*  20 ms RX every 2 s     — 1.78 mAh/day */
    WEAVE_RESPONSIVE,      /*  20 ms RX every 0.5 s   — 5.60 mAh/day */
    WEAVE_RELAY            /*  continuous RX          — 127.68 mAh/day */
} weave_role_t;

/* ---------------- THE RELAY GOVERNOR (attack D1) ----------------
 *
 * A relay forwards frames it cannot read, from senders it cannot identify. That
 * is not a weakness to be fixed — it IS the mesh, and any check strong enough to
 * refuse a stranger's frame would also refuse a friend's, because telling them
 * apart requires a key the relay does not have.
 *
 * What can be bounded is the BILL. Forwarding one frame costs 45 mA for 246.8 ms,
 * or 3.09 uAh. A leaf's entire daily allowance is 1780 uAh. Left ungoverned, a
 * single stranger transmitting continuously drove a leaf to 21.7 mAh/day in the
 * bench (sim scenario S2) — twelve times its own budget, and above the 19.46
 * mAh/day a Band harvests in good sun. A device that a passer-by can push into
 * deficit is not off-grid.
 *
 * So relaying gets a token bucket, and P1 is what makes it exact: every
 * meaning-carrying frame has IDENTICAL airtime, so a budget counted in frames is
 * a budget counted in milliamp-hours, with no conversion and no floating point.
 *
 *   leaf/responsive   50 frames/hour = 1200/day = 3.71 mAh/day
 *   relay            600 frames/hour                 44.5 mAh/day
 *
 * The leaf figure is chosen so that a leaf under sustained attack draws
 * 1.78 + 3.71 = 5.49 mAh/day and stays comfortably inside its 19.46 mAh/day
 * harvest. The relay figure is larger because a relay is, by definition, a node
 * whose strap pays for it. Neither number is a preference: each is the largest
 * value that keeps the role inside its own energy budget, which is the only
 * defensible way to pick one. */
#define WEAVE_RELAY_PER_HOUR_LEAF   50u
#define WEAVE_RELAY_PER_HOUR_RELAY 600u

typedef struct {
    uint32_t key[WEAVE_DEDUP_N];
    uint16_t head;

    struct {
        uint8_t  frame[WEAVE_FRAME_MAX];
        uint8_t  len;
        uint64_t deadline_ms;
        uint8_t  used;
    } q[WEAVE_QUEUE_N];

    weave_role_t role;
    uint64_t     last_traffic_ms;

    /* Relay budget, in thousandths of a frame so the refill stays in integers. */
    uint32_t tok_milli;
    uint64_t tok_refill_ms;

    uint32_t stat_relayed, stat_dup, stat_expired, stat_ttl_exhausted;
    uint32_t stat_decoy;        /* relays performed for our own frames (leak L1) */
    uint32_t stat_governed;     /* relays refused because the budget ran out */
} weave_t;

void weave_init(weave_t *w, weave_role_t role);

/* Returns 1 if this fingerprint is new (and records it), 0 if seen before. */
int  weave_seen(weave_t *w, const uint8_t *frame, size_t len);

/* Decide and act. Returns:
 *    1  queued for forwarding (ttl already decremented in the queued copy)
 *    0  not forwarded, and that is correct (duplicate, ttl 0, or out of budget)
 *   -1  queue full — a real condition worth a counter, not a silent drop
 * `mine` is the caller's answer to "is this frame addressed to me?". It no longer
 * suppresses forwarding — see leak L1 above — and is now used only to count how
 * much of our relaying is decoy traffic, which is a number worth having. */
int  weave_offer(weave_t *w, const uint8_t *frame, size_t len, int mine,
                 uint64_t now_ms, uint32_t deadline_ms);

/* Pop the next frame due for transmission, or return 0. Expired frames are
 * dropped here and counted: a store-and-forward queue that retries forever is a
 * battery attack on its own owner. */
int  weave_next_tx(weave_t *w, uint64_t now_ms, uint8_t *out, size_t *len);

/* Role transitions follow observed traffic and available energy, per §5.1. */
weave_role_t weave_update_role(weave_t *w, uint64_t now_ms, int on_charger,
                               int soc_pct, int harvest_mw);

#endif /* HERUS_WEAVE_H */
