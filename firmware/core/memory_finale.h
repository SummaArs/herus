/* memory_finale.h — Grand Finale audit for selective-memory composition.
 *
 * This is a pure, deny-by-default auditor of states already observed in the
 * memory pipeline. It does not parse input, retain a candidate, operate capture,
 * access a vault, own a key, create a record, query retrieval, present UI, send
 * data, log content or call a model. Its purpose is to make cross-layer evidence
 * and missing conditions independently testable at the memory boundary.
 */
#ifndef HERUS_MEMORY_FINALE_H
#define HERUS_MEMORY_FINALE_H

#include <stdint.h>
#include "memory_retrieval.h"

typedef struct {
    uint8_t capture_physical_validated;
    uint8_t extraction_typed;
    memory_disposition_t policy_disposition;
    uint8_t human_review_confirmed;
    uint8_t consolidation_conflicted;
    uint8_t vault_sealed;
    uint8_t retrieval_physical_access;
    memory_retrieval_status_t retrieval_status;
    uint8_t presentation_physical_access;
    uint8_t presentation_one_shot_enforced;
    uint8_t presentation_contract_valid;
    uint8_t model_in_memory_path;
} memory_finale_snapshot_t;

typedef enum {
    MEMORY_FINALE_FAIL_NONE                 = 0u,
    MEMORY_FINALE_FAIL_CAPTURE              = 1u << 0,
    MEMORY_FINALE_FAIL_EXTRACTION           = 1u << 1,
    MEMORY_FINALE_FAIL_POLICY               = 1u << 2,
    MEMORY_FINALE_FAIL_HUMAN_REVIEW         = 1u << 3,
    MEMORY_FINALE_FAIL_CONFLICT             = 1u << 4,
    MEMORY_FINALE_FAIL_VAULT                = 1u << 5,
    MEMORY_FINALE_FAIL_RETRIEVAL_ACCESS     = 1u << 6,
    MEMORY_FINALE_FAIL_RETRIEVAL_STATE      = 1u << 7,
    MEMORY_FINALE_FAIL_PRESENTATION_ACCESS  = 1u << 8,
    MEMORY_FINALE_FAIL_PRESENTATION_ONESHOT = 1u << 9,
    MEMORY_FINALE_FAIL_PRESENTATION_CONTRACT = 1u << 10,
    MEMORY_FINALE_FAIL_MODEL_AGENCY         = 1u << 11
} memory_finale_failure_t;

/* `chain_consistent` is an audit result only. It does not authorize storage,
 * retrieval, presentation, deletion, transmission or a model action; those remain
 * controlled by their existing modules and separate physical confirmations. */
typedef struct {
    uint32_t failures; /* OR of memory_finale_failure_t */
    uint8_t  chain_consistent;
} memory_finale_decision_t;

enum {
    MEMORY_FINALE_OK = 0,
    MEMORY_FINALE_E_ARG = -1,
    MEMORY_FINALE_E_BLOCKED = -2
};

int memory_finale_audit(const memory_finale_snapshot_t *snapshot,
                        memory_finale_decision_t *out);

#endif /* HERUS_MEMORY_FINALE_H */
