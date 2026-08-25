/*
 * HERUS drift — reach as a function of time.
 *
 * Every off-grid communicator answers "how far?" with a number in metres, and
 * that number is bounded by regulation long before it is bounded by physics
 * (docs/00-HERUS-MASTER.md sec.6: SF9 is the ceiling, not SF12). Drift answers a
 * different question. A meaning is 34 bytes; 34 bytes are cheap enough to hand
 * to a stranger's device, which walks away, and hands them on. Reach stops being
 * a function of transmit power and becomes a function of elapsed time.
 *
 * Custody is not readership. A device that carries a bundle holds ciphertext, a
 * rotating two-byte address it cannot attribute, and an eight-byte identifier it
 * cannot invert. It cannot read the meaning, cannot alter it without destroying
 * the tag, cannot learn who sent it, and cannot link two bundles from the same
 * sender. Carrying grants no authority of any kind.
 *
 * Three bounds keep an epidemic from becoming a flood: hop count, wall-clock
 * expiry, and a per-custody fanout. A relay that runs out of any of them stops.
 */
#ifndef HERUS_DRIFT_H
#define HERUS_DRIFT_H

#include <stdint.h>
#include "hir.h"

#define DRIFT_ID_BYTES       8u
#define DRIFT_ADDR_BYTES     2u
#define DRIFT_TAG_BYTES      8u
#define DRIFT_ONAIR_BYTES    HIR_ONAIR_BYTES          /* 2 + 24 + 8 = 34      */
#define DRIFT_MAX_CUSTODY    8u
#define DRIFT_MAX_SEEN      16u
#define DRIFT_MAX_HOPS      10u
#define DRIFT_MAX_FANOUT     3u
#define DRIFT_OPEN_WINDOW   32u                       /* counter lookahead     */

typedef enum {
    DRIFT_OK             = 0,
    DRIFT_E_ARG          = 1,
    DRIFT_E_FULL         = 2,
    DRIFT_E_EXPIRED      = 3,
    DRIFT_E_DUPLICATE    = 4,
    DRIFT_E_EXHAUSTED    = 5,   /* no hops or no fanout left                  */
    DRIFT_E_AUTH         = 6,   /* tag did not verify, or not addressed to me */
    DRIFT_E_CLASS        = 7,   /* drift carries essential meanings only      */
    DRIFT_UPGRADED       = 8    /* already held, but this replica has more legs */
} drift_status_t;

typedef struct {
    uint8_t  id[DRIFT_ID_BYTES];
    uint8_t  onair[DRIFT_ONAIR_BYTES];
    uint8_t  hops_left;
    uint8_t  fanout_left;
    uint8_t  payload_class;      /* ldr_class_t; must be LDR_CLASS_ESSENTIAL  */
    uint32_t expires_at;         /* monotonic seconds                          */
} drift_bundle_t;

typedef struct {
    uint8_t         count;
    drift_bundle_t  slot[DRIFT_MAX_CUSTODY];
    uint8_t         seen_count;
    /* Consumed identifiers only. Holding a bundle is not remembering it: a
     * carrier that already holds a replica must still be allowed to take a
     * fresher one with more hops left, or the first exhausted copy to arrive
     * poisons that carrier for the rest of the bundle's life. */
    uint8_t         seen[DRIFT_MAX_SEEN][DRIFT_ID_BYTES];
    uint32_t        transmissions;   /* what this node put on air, for bounds  */
} drift_store_t;

void drift_store_init(drift_store_t *s);

/* Sender side. `key` is the pair key; `counter` is a strictly increasing send
 * counter owned by the caller. Nothing about the sender appears in the bundle. */
drift_status_t drift_seal(const uint8_t key[32], uint32_t counter,
                          const uint8_t wire[HIR_WIRE_BYTES],
                          uint8_t payload_class, uint32_t now, uint32_t ttl_s,
                          drift_bundle_t *out);

/* Recipient side. Searches a bounded counter window. On success writes the
 * 24-byte HIR wire form; on any failure writes nothing readable. */
drift_status_t drift_open(const uint8_t key[32], uint32_t counter_base,
                          const drift_bundle_t *b, uint8_t wire[HIR_WIRE_BYTES]);

/* Relay side. Accepting custody never reads and never authorises. */
drift_status_t drift_accept(drift_store_t *s, const drift_bundle_t *b, uint32_t now);
drift_status_t drift_deliver_once(drift_store_t *s, const uint8_t id[DRIFT_ID_BYTES]);
uint8_t        drift_gc(drift_store_t *s, uint32_t now);

/* One contact opportunity: `from` offers what it holds to `to`. Returns how many
 * bundles changed custody. Decrements hops and fanout; never resurrects one. */
uint8_t        drift_contact(drift_store_t *from, drift_store_t *to, uint32_t now);

int            drift_holds(const drift_store_t *s, const uint8_t id[DRIFT_ID_BYTES]);

#endif /* HERUS_DRIFT_H */
