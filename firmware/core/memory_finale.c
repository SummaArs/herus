/* memory_finale.c — pure fail-closed audit of memory-pipeline evidence. */
#include "memory_finale.h"
#include <string.h>

static int exactly_one(uint8_t value)
{
    return value == 1u;
}

static int retrieval_status_valid(memory_retrieval_status_t status)
{
    return status == MEMORY_RETRIEVAL_NO_MATCH ||
           status == MEMORY_RETRIEVAL_MATCH ||
           status == MEMORY_RETRIEVAL_AMBIGUOUS;
}

int memory_finale_audit(const memory_finale_snapshot_t *snapshot,
                        memory_finale_decision_t *out)
{
    uint32_t failures = MEMORY_FINALE_FAIL_NONE;
    if (!snapshot || !out) return MEMORY_FINALE_E_ARG;
    memset(out, 0, sizeof(*out));

    if (!exactly_one(snapshot->capture_physical_validated))
        failures |= MEMORY_FINALE_FAIL_CAPTURE;
    if (!exactly_one(snapshot->extraction_typed))
        failures |= MEMORY_FINALE_FAIL_EXTRACTION;
    if (snapshot->policy_disposition != MEMORY_DISPOSITION_AUTO_ELIGIBLE)
        failures |= MEMORY_FINALE_FAIL_POLICY;
    if (!exactly_one(snapshot->human_review_confirmed))
        failures |= MEMORY_FINALE_FAIL_HUMAN_REVIEW;
    if (snapshot->consolidation_conflicted != 0u)
        failures |= MEMORY_FINALE_FAIL_CONFLICT;
    if (!exactly_one(snapshot->vault_sealed))
        failures |= MEMORY_FINALE_FAIL_VAULT;
    if (!exactly_one(snapshot->retrieval_physical_access))
        failures |= MEMORY_FINALE_FAIL_RETRIEVAL_ACCESS;
    if (!retrieval_status_valid(snapshot->retrieval_status))
        failures |= MEMORY_FINALE_FAIL_RETRIEVAL_STATE;
    if (!exactly_one(snapshot->presentation_physical_access))
        failures |= MEMORY_FINALE_FAIL_PRESENTATION_ACCESS;
    if (!exactly_one(snapshot->presentation_one_shot_enforced))
        failures |= MEMORY_FINALE_FAIL_PRESENTATION_ONESHOT;
    if (!exactly_one(snapshot->presentation_contract_valid))
        failures |= MEMORY_FINALE_FAIL_PRESENTATION_CONTRACT;
    if (snapshot->model_in_memory_path != 0u)
        failures |= MEMORY_FINALE_FAIL_MODEL_AGENCY;

    out->failures = failures;
    out->chain_consistent = failures == MEMORY_FINALE_FAIL_NONE ? 1u : 0u;
    return failures == MEMORY_FINALE_FAIL_NONE ? MEMORY_FINALE_OK :
                                                 MEMORY_FINALE_E_BLOCKED;
}
