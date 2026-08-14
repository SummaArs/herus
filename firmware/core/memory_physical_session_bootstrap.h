/* memory_physical_session_bootstrap.h — post-reboot session quarantine bridge.
 *
 * This pure C11 composition takes a recovery snapshot already declared by a
 * future durable adapter, obtains its safe reservation-floor action, and creates
 * a new RAM-only physical-session gate. It imports only a recovered floor. It
 * never imports, receives or returns an active session, nonce, event, person,
 * card, query, key, I/O handle, callback, clock, model, radio or network object.
 */
#ifndef HERUS_MEMORY_PHYSICAL_SESSION_BOOTSTRAP_H
#define HERUS_MEMORY_PHYSICAL_SESSION_BOOTSTRAP_H

#include <stdint.h>

#include "memory_physical_session_recovery.h"

typedef struct {
    uint32_t recovered_session_floor;
    memory_physical_session_recovery_action_t recovery_action;
    uint8_t active_evidence_scrubbed; /* exactly 1 only after a successful idle bootstrap */
} memory_physical_session_bootstrap_result_t;

enum {
    MEMORY_PHYSICAL_SESSION_BOOTSTRAP_OK         =  0,
    MEMORY_PHYSICAL_SESSION_BOOTSTRAP_E_ARG      = -1,
    MEMORY_PHYSICAL_SESSION_BOOTSTRAP_E_CONFIG   = -2,
    MEMORY_PHYSICAL_SESSION_BOOTSTRAP_E_RECOVERY = -3
};

/* Reinitializes `gate` from `cfg`, assesses the supplied reservation snapshot,
 * and imports exactly one coherent recovered floor. Successful returns always
 * leave `gate` in IDLE with all active-session fields zeroed. A subsequent
 * operation still requires `memory_physical_session_begin()` with an ID strictly
 * above that floor and a new adapter-supplied physical-event assertion.
 *
 * Invalid configuration, missing input or a blocked recovery result fail closed:
 * if `gate` is supplied, it is scrubbed and left BLOCKED. `out` never carries an
 * active capability; on failure it is zeroed with action BLOCKED.
 */
int memory_physical_session_bootstrap(
    memory_physical_session_t *gate,
    const memory_physical_session_config_t *cfg,
    const memory_physical_session_recovery_snapshot_t *snapshot,
    memory_physical_session_bootstrap_result_t *out);

#endif /* HERUS_MEMORY_PHYSICAL_SESSION_BOOTSTRAP_H */
