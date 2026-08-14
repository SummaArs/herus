/* memory_prehardware_finale.h — final host-only memory-chain composition.
 *
 * This deny-by-default composition rebuilds a transient session gate through the
 * post-reboot bootstrap, then checks the already-observed collection-chain and
 * TM-04 evidence. It produces a diagnostic for target validation only. It never
 * returns an active session, card, card ID, query, result, key, event, person,
 * nonce, clock, model output, storage handle, callback, radio object or authority
 * to insert, open, present, delete, retain or transmit.
 */
#ifndef HERUS_MEMORY_PREHARDWARE_FINALE_H
#define HERUS_MEMORY_PREHARDWARE_FINALE_H

#include <stdint.h>

#include "memory_collection_finale.h"
#include "memory_physical_session_bootstrap.h"
#include "threat_model.h"

typedef struct {
    const memory_physical_session_config_t *session_config;
    const memory_physical_session_recovery_snapshot_t *reservation_snapshot;
    const memory_collection_finale_snapshot_t *collection_snapshot;
    const threat_model_snapshot_t *threat_snapshot;
} memory_prehardware_finale_input_t;

typedef enum {
    MEMORY_PREHARDWARE_FINALE_FAIL_NONE          = 0u,
    MEMORY_PREHARDWARE_FINALE_FAIL_BOOTSTRAP     = 1u << 0,
    MEMORY_PREHARDWARE_FINALE_FAIL_GATE_QUARANTINE = 1u << 1,
    MEMORY_PREHARDWARE_FINALE_FAIL_COLLECTION    = 1u << 2,
    MEMORY_PREHARDWARE_FINALE_FAIL_THREAT_MODEL  = 1u << 3
} memory_prehardware_finale_failure_t;

typedef struct {
    uint32_t failures; /* OR of memory_prehardware_finale_failure_t */
    uint32_t recovered_session_floor;
    memory_physical_session_recovery_action_t recovery_action;
    uint8_t ready_for_target_validation; /* exactly 1 only for a coherent host chain */
} memory_prehardware_finale_decision_t;

enum {
    MEMORY_PREHARDWARE_FINALE_OK        =  0,
    MEMORY_PREHARDWARE_FINALE_E_ARG     = -1,
    MEMORY_PREHARDWARE_FINALE_E_BLOCKED = -2
};

/* Rebuilds `gate` only through the post-reboot bootstrap and then audits all
 * declared boundaries. On any blocked input or inconsistent evidence, the gate
 * is scrubbed and BLOCKED. On success it remains IDLE: a caller still must supply
 * a new adapter event and an ID greater than the recovered floor to begin any
 * later purpose-bound operation. */
int memory_prehardware_finale_audit(
    memory_physical_session_t *gate,
    const memory_prehardware_finale_input_t *input,
    memory_prehardware_finale_decision_t *out);

#endif /* HERUS_MEMORY_PREHARDWARE_FINALE_H */
