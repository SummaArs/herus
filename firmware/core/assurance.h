/* assurance.h — Grand Finale composition gate for HERUS safety invariants.
 *
 * This is a pure auditor of already-observed state. It does not parse speech,
 * own a key, build HCP, call link_send, call a model, log content or access I/O.
 * Its purpose is to make cross-module safety preconditions fail-closed and
 * independently testable before an application takes a confirmed handoff.
 */
#ifndef HERUS_ASSURANCE_H
#define HERUS_ASSURANCE_H

#include <stdint.h>

typedef enum {
    ASSURANCE_SOURCE_CORE = 0,
    ASSURANCE_SOURCE_NUCLEUS = 1
} assurance_source_t;

/* A caller obtains these booleans from existing typed state machines. They are
 * deliberately booleans rather than references to secrets, audio, text, HCP,
 * identity, address or location. A field other than exactly 1 is treated as
 * unsafe, so incomplete adapters cannot accidentally authorize a handoff. */
typedef struct {
    assurance_source_t source;
    uint8_t physical_session_current;
    uint8_t intent_accepted;
    uint8_t physical_confirmation;
    uint8_t handoff_unused;
    uint8_t trust_active;              /* required for a Nucleus-origin result */
    uint8_t control_link_authenticated;/* required for a Nucleus-origin result */
    uint8_t control_link_fresh;        /* required for a Nucleus-origin result */
    uint8_t trust_revoked;              /* always dominates all preceding state */
    uint8_t local_model_enabled;
    uint8_t local_model_accepted;      /* A9 measured-profile decision */
    uint8_t model_reply_display_only;  /* model text is never promoted to intent */
} assurance_snapshot_t;

typedef enum {
    ASSURANCE_FAIL_NONE        = 0u,
    ASSURANCE_FAIL_SOURCE      = 1u << 0,
    ASSURANCE_FAIL_PHYSICAL    = 1u << 1,
    ASSURANCE_FAIL_INTENT      = 1u << 2,
    ASSURANCE_FAIL_CONFIRM     = 1u << 3,
    ASSURANCE_FAIL_HANDOFF     = 1u << 4,
    ASSURANCE_FAIL_TRUST       = 1u << 5,
    ASSURANCE_FAIL_LINK_AUTH   = 1u << 6,
    ASSURANCE_FAIL_LINK_FRESH  = 1u << 7,
    ASSURANCE_FAIL_REVOKED     = 1u << 8,
    ASSURANCE_FAIL_MODEL       = 1u << 9,
    ASSURANCE_FAIL_AGENCY      = 1u << 10
} assurance_failure_t;

typedef struct {
    uint32_t failures; /* OR of assurance_failure_t */
    uint8_t handoff_permitted;
} assurance_decision_t;

enum {
    ASSURANCE_OK = 0,
    ASSURANCE_E_ARG = -1,
    ASSURANCE_E_BLOCKED = -2
};

/* Deny by default. `handoff_permitted` means only that all already-existing
 * conditions agree that the application MAY request its one-time local handoff;
 * it is not a packet, an HCP message, a radio permission or a send operation. */
int assurance_decide(const assurance_snapshot_t *snapshot,
                     assurance_decision_t *out);

#endif /* HERUS_ASSURANCE_H */
