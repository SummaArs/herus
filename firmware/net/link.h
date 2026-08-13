/* link.h — the one call the application makes to send a meaning, and the one it
 * makes to receive one.
 *
 * Everything below this line is testable on a host with no radio: hcp (meaning
 * -> 24 bytes), session (24 bytes -> 34-byte sealed frame), region (is this
 * frame legal here?), weave (should I relay it?), beat (when do I transmit?).
 * Everything above it is one HAL with eight functions (port/hal.h).
 *
 * That split is deliberate and it is the reason Phase 1 can be debugged on a Mac:
 * the only thing the ESP32 adds is a radio and a clock.
 */
#ifndef HERUS_LINK_H
#define HERUS_LINK_H

#include "../core/hcp.h"
#include "session.h"
#include "region.h"
#include "weave.h"

#define LINK_TAG_LEN     8u
#define LINK_PT_LEN      HCP_PLAINTEXT_LEN            /* 24 */
#define LINK_FRAME_LEN   (2u + LINK_PT_LEN + LINK_TAG_LEN)   /* 34 */

_Static_assert(LINK_FRAME_LEN == HERUS_FRAME_LEN,
               "P1: the link frame must be exactly the ledger's frame length");

/* ---------------- RETRY, AND WHY IT COULD NOT BE DONE THE OBVIOUS WAY -------
 *
 * A lost frame is lost. There is no acknowledgement in this protocol and adding
 * one would double the traffic on a band where a frame costs 246.8 ms — but the
 * cheap alternative, "just transmit it again", is not available either, and the
 * reason is worth stating because it is a consequence of a decision made
 * elsewhere for a good reason.
 *
 * Every message key is SINGLE USE (session.h). That is what gives replay defence
 * for free and what makes forward secrecy real. It also means a byte-identical
 * retransmission cannot open at the far end: the key that would have opened it
 * was destroyed when the first copy did or did not arrive. Weave's dedup would
 * suppress the duplicate anyway. So the anti-replay design forbids retry — the
 * two properties are the same mechanism seen from two sides.
 *
 * A retry must therefore be a genuinely NEW sealed frame, consuming a new
 * counter, carrying the SAME application sequence number. hcp already has that
 * field and spends no extra bytes on it. What was missing was the other half:
 * something at the receiver that recognises two different frames as one message.
 *
 * link_dedup is that half. It reuses herus_replay — the sliding 64-counter
 * bitmap already written and tested for the SOS tier — with a 16-bit unwrapper
 * in front of it, because hcp's seq is 16 bits and wraps. Stated limits: it
 * tolerates reordering up to 64 messages and sequence jumps below 32768. Beyond
 * either, a duplicate may be delivered twice, which is a visible annoyance and
 * not a security failure.
 *
 * The cost of the policy is exactly one extra transmission per copy, paid by the
 * sender, chosen per message. Two copies take a 98.1% link to 99.96%. */
typedef struct {
    herus_replay r;
    uint16_t     last;
    uint32_t     ext;
    uint8_t      started;
} link_dedup;

typedef struct {
    herus_session *sess;
    hz_region_t    region;
    uint8_t        tx_dbm;
    /* The roles THIS firmware understands. A slot whose role is not here is
     * dropped on receive and the frame is still accepted — that is P4, and it is
     * why a v1 unit can talk to a v3 unit forever. */
    const uint8_t *known_roles;
    int            nknown;
    /* Optional. NULL means no application-level duplicate suppression, which is
     * the correct setting for a sender that never retries. */
    link_dedup    *seen;
    /* Rich (SF9, 34 B, 9 slots) or Reach (SF10, 24 B, 4 slots, +2.5 dB). Zero is
     * Rich, so every existing initialiser keeps its old behaviour. Per group. */
    hz_link_t      profile;
} herus_link;

/* Everything the caller needs to know about the profile it is running: airtime,
 * frame length, slot budget. Never NULL. */
const hz_link_profile_t *link_profile(const herus_link *l);

enum {
    LINK_OK        =  0,
    LINK_E_ARG     = -1,
    LINK_E_ILLEGAL = -2,   /* the region profile forbids this transmission */
    LINK_E_ENCODE  = -3,
    LINK_E_SESSION = -4,   /* see the SESS_E_* code returned in *sess_err */
    LINK_E_DECODE  = -5,
    LINK_E_DUP     = -6    /* opened, decoded, and already delivered once */
};

/* Meaning in, 34 bytes out. Refuses rather than transmits if the region profile
 * would make the frame illegal (footgun #10 in reverse: the check is compiled in
 * and cannot be talked out of by a runtime setting). */
int link_send(herus_link *l, hcp_msg_t *m, uint8_t ttl, uint8_t *frame34);

/* 34 bytes in, meaning out. `sess_err` receives the session-layer code so the
 * caller can distinguish "not for me" (free, ignore) from "someone is trying
 * forgeries at me" (log it, and the rate limiter is already handling it). */
int link_recv(herus_link *l, const uint8_t *frame34, uint64_t now_ms,
              hcp_msg_t *out, uint32_t *counter_out, int *sess_err);

/* Same contract as link_recv, for the one case link_recv cannot serve: the frame
 * is from our peer but our receive window has fallen too far behind to recognise
 * it. Call it only when link_recv has returned LINK_E_SESSION with SESS_E_ADDR;
 * session_recover's own two gates make it cheap to call and safe to call often.
 *
 * A caller that never calls this still works exactly as before — and will also
 * lose the link permanently the first time its owner walks into a basement. */
int link_recover(herus_link *l, const uint8_t *frame34, uint64_t now_ms,
                 hcp_msg_t *out, uint32_t *counter_out, int *sess_err);

#endif /* HERUS_LINK_H */
