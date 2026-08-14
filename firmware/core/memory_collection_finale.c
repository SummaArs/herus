/* memory_collection_finale.c — pure fail-closed audit of composed collection evidence. */
#include "memory_collection_finale.h"
#include <string.h>

static int exactly_one(uint8_t value)
{
    return value == 1u;
}

static int exactly_zero(uint8_t value)
{
    return value == 0u;
}

static int retrieval_status_valid(memory_retrieval_status_t status)
{
    return status == MEMORY_RETRIEVAL_NO_MATCH ||
           status == MEMORY_RETRIEVAL_MATCH ||
           status == MEMORY_RETRIEVAL_AMBIGUOUS;
}

int memory_collection_finale_audit(const memory_collection_finale_snapshot_t *snapshot,
                                   memory_collection_finale_decision_t *out)
{
    uint32_t failures = MEMORY_COLLECTION_FINALE_FAIL_NONE;

    if (!snapshot || !out) return MEMORY_COLLECTION_FINALE_E_ARG;
    memset(out, 0, sizeof(*out));

    if (!exactly_one(snapshot->capture_physical_validated))
        failures |= MEMORY_COLLECTION_FINALE_FAIL_CAPTURE;
    if (!exactly_one(snapshot->extraction_typed))
        failures |= MEMORY_COLLECTION_FINALE_FAIL_EXTRACTION;
    if (snapshot->policy_disposition != MEMORY_DISPOSITION_AUTO_ELIGIBLE)
        failures |= MEMORY_COLLECTION_FINALE_FAIL_POLICY;
    if (!exactly_one(snapshot->human_review_confirmed))
        failures |= MEMORY_COLLECTION_FINALE_FAIL_HUMAN_REVIEW;
    if (!exactly_one(snapshot->write_authorization_bound))
        failures |= MEMORY_COLLECTION_FINALE_FAIL_WRITE_AUTHORIZATION;

    if (!exactly_one(snapshot->collection_inserted))
        failures |= MEMORY_COLLECTION_FINALE_FAIL_COLLECTION_INSERT;
    if (snapshot->collection_state != MEMORY_COLLECTION_READY)
        failures |= MEMORY_COLLECTION_FINALE_FAIL_COLLECTION_STATE;
    if (!exactly_one(snapshot->collection_recovery_consistent))
        failures |= MEMORY_COLLECTION_FINALE_FAIL_COLLECTION_RECOVERY;
    if (!exactly_one(snapshot->collection_record_authenticated))
        failures |= MEMORY_COLLECTION_FINALE_FAIL_COLLECTION_AUTH;

    if (!exactly_one(snapshot->index_physical_access))
        failures |= MEMORY_COLLECTION_FINALE_FAIL_INDEX_ACCESS;
    if (!exactly_one(snapshot->index_typed_query))
        failures |= MEMORY_COLLECTION_FINALE_FAIL_INDEX_QUERY;
    if (!exactly_one(snapshot->index_budget_respected))
        failures |= MEMORY_COLLECTION_FINALE_FAIL_INDEX_BUDGET;
    if (!retrieval_status_valid(snapshot->index_status))
        failures |= MEMORY_COLLECTION_FINALE_FAIL_INDEX_STATUS;
    if (!exactly_zero(snapshot->query_result_card_auto_opened))
        failures |= MEMORY_COLLECTION_FINALE_FAIL_AUTO_OPEN;
    if (!exactly_zero(snapshot->unit_vault_fallback_used))
        failures |= MEMORY_COLLECTION_FINALE_FAIL_LEGACY_FALLBACK;

    if (!exactly_one(snapshot->presentation_physical_access))
        failures |= MEMORY_COLLECTION_FINALE_FAIL_PRESENTATION_ACCESS;
    if (!exactly_one(snapshot->presentation_one_shot_enforced))
        failures |= MEMORY_COLLECTION_FINALE_FAIL_PRESENTATION_ONESHOT;
    if (!exactly_one(snapshot->presentation_contract_valid))
        failures |= MEMORY_COLLECTION_FINALE_FAIL_PRESENTATION_CONTRACT;
    if (!exactly_zero(snapshot->model_in_memory_path))
        failures |= MEMORY_COLLECTION_FINALE_FAIL_MODEL_AGENCY;

    out->failures = failures;
    out->chain_consistent = failures == MEMORY_COLLECTION_FINALE_FAIL_NONE ? 1u : 0u;
    return failures == MEMORY_COLLECTION_FINALE_FAIL_NONE ?
           MEMORY_COLLECTION_FINALE_OK : MEMORY_COLLECTION_FINALE_E_BLOCKED;
}
