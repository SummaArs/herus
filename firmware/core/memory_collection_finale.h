/* memory_collection_finale.h — pure composed audit for the multi-card memory path.
 *
 * This is a deny-by-default diagnostic over evidence already observed at module
 * boundaries. It does not receive a card, card identifier, query, key, session,
 * collection, index, vault, text, audio, transcript, embedding, identity,
 * location, model output or callback. Its result cannot authorize insertion,
 * opening, presentation, deletion, transmission or an action.
 */
#ifndef HERUS_MEMORY_COLLECTION_FINALE_H
#define HERUS_MEMORY_COLLECTION_FINALE_H

#include <stdint.h>
#include "memory_collection.h"
#include "memory_policy.h"
#include "memory_retrieval.h"

/* This snapshot deliberately records only canonical, aggregate boundary evidence.
 * It distinguishes the legacy unit-vault path from the collection path: any fallback
 * to the former, or automatic card opening by the latter, is an audit failure. */
typedef struct {
    uint8_t capture_physical_validated;
    uint8_t extraction_typed;
    memory_disposition_t policy_disposition;
    uint8_t human_review_confirmed;
    uint8_t write_authorization_bound;

    uint8_t collection_inserted;
    memory_collection_state_t collection_state;
    uint8_t collection_recovery_consistent;
    uint8_t collection_record_authenticated;

    uint8_t index_physical_access;
    uint8_t index_typed_query;
    uint8_t index_budget_respected;
    memory_retrieval_status_t index_status;
    uint8_t query_result_card_auto_opened;
    uint8_t unit_vault_fallback_used;

    uint8_t presentation_physical_access;
    uint8_t presentation_one_shot_enforced;
    uint8_t presentation_contract_valid;
    uint8_t model_in_memory_path;
} memory_collection_finale_snapshot_t;

typedef enum {
    MEMORY_COLLECTION_FINALE_FAIL_NONE                   = 0u,
    MEMORY_COLLECTION_FINALE_FAIL_CAPTURE                = 1u << 0,
    MEMORY_COLLECTION_FINALE_FAIL_EXTRACTION             = 1u << 1,
    MEMORY_COLLECTION_FINALE_FAIL_POLICY                 = 1u << 2,
    MEMORY_COLLECTION_FINALE_FAIL_HUMAN_REVIEW           = 1u << 3,
    MEMORY_COLLECTION_FINALE_FAIL_WRITE_AUTHORIZATION    = 1u << 4,
    MEMORY_COLLECTION_FINALE_FAIL_COLLECTION_INSERT      = 1u << 5,
    MEMORY_COLLECTION_FINALE_FAIL_COLLECTION_STATE       = 1u << 6,
    MEMORY_COLLECTION_FINALE_FAIL_COLLECTION_RECOVERY    = 1u << 7,
    MEMORY_COLLECTION_FINALE_FAIL_COLLECTION_AUTH        = 1u << 8,
    MEMORY_COLLECTION_FINALE_FAIL_INDEX_ACCESS           = 1u << 9,
    MEMORY_COLLECTION_FINALE_FAIL_INDEX_QUERY            = 1u << 10,
    MEMORY_COLLECTION_FINALE_FAIL_INDEX_BUDGET           = 1u << 11,
    MEMORY_COLLECTION_FINALE_FAIL_INDEX_STATUS           = 1u << 12,
    MEMORY_COLLECTION_FINALE_FAIL_AUTO_OPEN              = 1u << 13,
    MEMORY_COLLECTION_FINALE_FAIL_LEGACY_FALLBACK        = 1u << 14,
    MEMORY_COLLECTION_FINALE_FAIL_PRESENTATION_ACCESS    = 1u << 15,
    MEMORY_COLLECTION_FINALE_FAIL_PRESENTATION_ONESHOT   = 1u << 16,
    MEMORY_COLLECTION_FINALE_FAIL_PRESENTATION_CONTRACT  = 1u << 17,
    MEMORY_COLLECTION_FINALE_FAIL_MODEL_AGENCY           = 1u << 18
} memory_collection_finale_failure_t;

/* `chain_consistent` diagnoses the particular evidence snapshot. It creates no
 * authority: collection insert/open/index/presentation remain separate operations
 * with their own physical confirmation and failure paths. */
typedef struct {
    uint32_t failures; /* OR of memory_collection_finale_failure_t */
    uint8_t  chain_consistent;
} memory_collection_finale_decision_t;

enum {
    MEMORY_COLLECTION_FINALE_OK = 0,
    MEMORY_COLLECTION_FINALE_E_ARG = -1,
    MEMORY_COLLECTION_FINALE_E_BLOCKED = -2
};

int memory_collection_finale_audit(const memory_collection_finale_snapshot_t *snapshot,
                                   memory_collection_finale_decision_t *out);

#endif /* HERUS_MEMORY_COLLECTION_FINALE_H */
