#include "memory_reboot_boundary.h"
#include <stdio.h>
#include <string.h>

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static memory_physical_session_recovery_snapshot_t committed_snapshot(
    uint32_t floor)
{
    memory_physical_session_recovery_snapshot_t snapshot;
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.committed_present = 1u;
    snapshot.committed_authenticated = 1u;
    snapshot.committed_reservation_id = floor;
    snapshot.durable_reservation_floor = floor;
    snapshot.committed_purpose = MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY;
    snapshot.committed_uses = 1u;
    return snapshot;
}

static void stale_index(mse_index_t *index)
{
    memory_vault_card_t card;
    sr_fact_t fact;
    memset(&card, 0, sizeof(card));
    card.card_id = 700u;
    card.review_receipt_id = 1700u;
    fact.subject = 0x72000001u;
    fact.predicate = SR_SYMBOL_LEGACY(30u);
    fact.object = SR_SYMBOL_LEGACY(31u);
    fact.negated = 0u;
    mse_init(index, NULL, NULL);
    (void)mse_add(index, &card, &fact, 4u, 0u);
}

int main(void)
{
    score_t score = { 0, 0 };
    memory_physical_session_t gate;
    memory_physical_session_config_t cfg;
    memory_physical_session_recovery_snapshot_t snapshot;
    memory_reboot_boundary_result_t result;
    mse_index_t index;
    memory_vault_card_t semantic_card;
    sr_fact_t semantic_fact = {0x73000001u, SR_SYMBOL_LEGACY(30u),
                               SR_SYMBOL_LEGACY(31u), 0u};

    memset(&semantic_card, 0, sizeof(semantic_card));
    semantic_card.card_id = 701u;
    semantic_card.review_receipt_id = 1701u;
    memory_physical_session_config_default(&cfg);
    snapshot = committed_snapshot(8u);
    stale_index(&index);
    check(&score, mse_set_generation_floor(&index, 8u) == MSE_E_FLOOR &&
                    index.generation_floor == 0u && index.evidence_count == 1u,
          "semantic floor cannot be installed over nonempty volatile evidence");
    memory_physical_session_init(&gate, &cfg);
    check(&score, memory_physical_session_begin(
                        &gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                        8u, 88u, 1u, 2u, 100u) == MEMORY_PHYSICAL_SESSION_OK &&
                    gate.state == MEMORY_PHYSICAL_SESSION_ACTIVE,
          "pre-reboot fixture contains active session evidence and semantic facts");

    check(&score, memory_reboot_boundary_bootstrap(&gate, &index, &cfg,
                                                    &snapshot, &result) ==
                        MEMORY_REBOOT_BOUNDARY_OK &&
                    result.recovered_session_floor == 8u &&
                    result.semantic_generation_floor == 8u &&
                    result.active_session_scrubbed == 1u &&
                    result.semantic_index_scrubbed == 1u &&
                    gate.state == MEMORY_PHYSICAL_SESSION_IDLE &&
                    gate.session_floor == 8u &&
                    gate.active_session_id == 0u &&
                    gate.active_event_nonce == 0u &&
                    gate.active_purpose == MEMORY_PHYSICAL_PURPOSE_NONE &&
                    index.evidence_count == 0u,
          "successful reboot imports only the durable floor and scrubs session plus semantic evidence");
    check(&score, mse_set_generation_floor(&index, 7u) == MSE_E_ROLLBACK &&
                    index.generation_floor == 8u,
          "semantic generation floor cannot decrease after reboot");
    check(&score, mse_add(&index, &semantic_card, &semantic_fact, 8u, 0u) ==
                        MSE_E_ROLLBACK && index.evidence_count == 0u,
          "semantic reindex at the recovered floor is rejected as stale evidence");
    check(&score, mse_add(&index, &semantic_card, &semantic_fact, 9u, 0u) ==
                        MSE_OK && index.evidence_count == 1u,
          "semantic reindex strictly above the recovered floor is the only accepted successor");

    check(&score, memory_physical_session_begin(
                        &gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                        8u, 99u, 1u, 1u, 200u) ==
                        MEMORY_PHYSICAL_SESSION_E_FORMAT &&
                    gate.state == MEMORY_PHYSICAL_SESSION_IDLE,
          "a pre-reboot or equal session id cannot reuse the recovered floor");
    check(&score, memory_physical_session_begin(
                        &gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                        9u, 99u, 1u, 1u, 200u) ==
                        MEMORY_PHYSICAL_SESSION_OK &&
                    gate.state == MEMORY_PHYSICAL_SESSION_ACTIVE,
          "a new post-reboot session requires a fresh physical assertion above the floor");

    stale_index(&index);
    snapshot = committed_snapshot(11u);
    snapshot.committed_authenticated = 0u;
    memset(&result, 0xa5, sizeof(result));
    check(&score, memory_reboot_boundary_bootstrap(&gate, &index, &cfg,
                                                    &snapshot, &result) ==
                        MEMORY_REBOOT_BOUNDARY_E_RECOVERY &&
                    gate.state == MEMORY_PHYSICAL_SESSION_BLOCKED &&
                    gate.active_session_id == 0u &&
                    index.evidence_count == 0u &&
                    result.recovered_session_floor == 0u &&
                    result.active_session_scrubbed == 0u &&
                    result.semantic_index_scrubbed == 0u,
          "an unauthenticated recovery snapshot blocks and scrubs both boundaries");
    check(&score, memory_physical_session_begin(
                        &gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                        12u, 100u, 1u, 1u, 300u) ==
                        MEMORY_PHYSICAL_SESSION_E_STATE,
          "a blocked reboot boundary cannot be bypassed by starting a new session");

    stale_index(&index);
    memset(&result, 0xa5, sizeof(result));
    check(&score, memory_reboot_boundary_bootstrap(&gate, &index, &cfg,
                                                    NULL, &result) ==
                        MEMORY_REBOOT_BOUNDARY_E_ARG &&
                    gate.state == MEMORY_PHYSICAL_SESSION_BLOCKED &&
                    index.evidence_count == 0u &&
                    result.recovered_session_floor == 0u &&
                    result.active_session_scrubbed == 0u &&
                    result.semantic_index_scrubbed == 0u,
          "a missing recovery snapshot is rejected while the semantic index is scrubbed");

    stale_index(&index);
    memset(&result, 0xa5, sizeof(result));
    check(&score, memory_reboot_boundary_bootstrap(&gate, NULL, &cfg,
                                                    &snapshot, &result) ==
                        MEMORY_REBOOT_BOUNDARY_E_ARG &&
                    gate.state == MEMORY_PHYSICAL_SESSION_BLOCKED &&
                    result.recovered_session_floor == 0u &&
                    result.active_session_scrubbed == 0u &&
                    result.semantic_index_scrubbed == 0u,
          "a missing semantic index is rejected instead of yielding a partial recovery");

    printf("MEMORY REBOOT BOUNDARY: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
