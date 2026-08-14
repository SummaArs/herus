/* memory_physical_session_recovery.h — pure post-reboot reservation classifier.
 *
 * A durable adapter may record that a physical-session ID was reserved/consumed,
 * then recover that marker after interruption. This C11 oracle decides only the
 * safe fate of already-authenticated PREPARED and COMMITTED reservation records
 * against an independently supplied monotonic floor. It never restores an ACTIVE
 * session and never receives an event, person, nonce, card, query, key, I/O,
 * callback, model, radio or network object.
 */
#ifndef HERUS_MEMORY_PHYSICAL_SESSION_RECOVERY_H
#define HERUS_MEMORY_PHYSICAL_SESSION_RECOVERY_H

#include <stdint.h>

#include "memory_physical_session.h"

typedef enum {
    MEMORY_PHYSICAL_SESSION_RECOVERY_EMPTY = 0,
    MEMORY_PHYSICAL_SESSION_RECOVERY_USE_COMMITTED,
    MEMORY_PHYSICAL_SESSION_RECOVERY_PROMOTE_PREPARED,
    MEMORY_PHYSICAL_SESSION_RECOVERY_FINALIZE_PREPARED,
    MEMORY_PHYSICAL_SESSION_RECOVERY_DISCARD_PREPARED,
    MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED
} memory_physical_session_recovery_action_t;

/* A reservation records only a session floor and the operation shape it burned.
 * It is never an active capability: recovery returns a floor that rejects old
 * IDs and requires a new adapter event. `prepared_matches_committed` may be 1
 * only after the caller authenticated both records and established complete
 * equality of reservation ID, purpose and allowed uses. */
typedef struct {
    uint8_t committed_present;
    uint8_t prepared_present;
    uint8_t committed_authenticated;
    uint8_t prepared_authenticated;
    uint8_t prepared_matches_committed;
    uint32_t committed_reservation_id;
    uint32_t prepared_reservation_id;
    uint32_t prepared_base_reservation_id;
    uint32_t durable_reservation_floor;
    memory_physical_purpose_t committed_purpose;
    memory_physical_purpose_t prepared_purpose;
    uint8_t committed_uses;
    uint8_t prepared_uses;
} memory_physical_session_recovery_snapshot_t;

enum {
    MEMORY_PHYSICAL_SESSION_RECOVERY_OK        =  0,
    MEMORY_PHYSICAL_SESSION_RECOVERY_E_ARG     = -1,
    MEMORY_PHYSICAL_SESSION_RECOVERY_E_INVALID = -2
};

/* Returns exactly one action. Any noncanonical boolean, unauthenticated present
 * record, unknown purpose, invalid use budget, impossible reservation ordering or
 * floor contradiction yields E_INVALID and BLOCKED. A caller must fail closed.
 * No action permits a pre-reboot session to become active again. */
int memory_physical_session_recovery_assess(
    const memory_physical_session_recovery_snapshot_t *snapshot,
    memory_physical_session_recovery_action_t *out_action);

#endif /* HERUS_MEMORY_PHYSICAL_SESSION_RECOVERY_H */
