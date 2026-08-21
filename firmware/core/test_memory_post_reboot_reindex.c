#include "memory_semantic_evidence.h"
#include <stdio.h>
#include <string.h>

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static memory_vault_card_t card(uint32_t id, uint32_t receipt)
{
    memory_vault_card_t out;
    memset(&out, 0, sizeof(out));
    out.card_id = id;
    out.review_receipt_id = receipt;
    return out;
}

static int functional(sr_symbol_t predicate, void *user)
{
    (void)user;
    return predicate == SR_SYMBOL_LEGACY(20u);
}

int main(void)
{
    score_t score = { 0, 0 };
    mse_index_t index;
    mse_query_result_t result;
    memory_vault_card_t card_a = card(801u, 1801u);
    memory_vault_card_t card_b = card(802u, 1802u);
    memory_vault_card_t card_c = card(803u, 1803u);
    sr_symbol_t subject = 0x74000001u;
    sr_symbol_t functional_predicate = SR_SYMBOL_LEGACY(20u);
    sr_symbol_t open_predicate = SR_SYMBOL_LEGACY(21u);
    sr_symbol_t object_a = 0x74000011u;
    sr_symbol_t object_b = 0x74000012u;
    sr_symbol_t object_c = 0x74000013u;
    sr_pattern_t query;

    mse_init(&index, functional, NULL);
    check(&score, mse_set_generation_floor(&index, 10u) == MSE_OK &&
                    index.generation_floor == 10u && index.evidence_count == 0u,
          "post-reboot reindex starts from the imported semantic floor");
    check(&score, mse_add(&index, &card_a,
                          &(sr_fact_t){subject, functional_predicate, object_a, 0u},
                          10u, 0u) == MSE_E_ROLLBACK && index.evidence_count == 0u,
          "equal-generation reindex is rejected at the semantic floor");
    check(&score, mse_add(&index, &card_a,
                          &(sr_fact_t){subject, functional_predicate, object_a, 0u},
                          11u, 14u) == MSE_OK && index.evidence_count == 1u,
          "strictly newer reindex is admitted after reboot");

    check(&score, mse_add(&index, &card_b,
                          &(sr_fact_t){subject, functional_predicate, object_a, 0u},
                          12u, 16u) == MSE_OK &&
                    index.evidence_count == 2u &&
                    index.evidence[0].status == MSE_EVIDENCE_SUPERSEDED,
          "newer exact evidence supersedes the pre-existing reindexed fact");
    query = (sr_pattern_t){SR_CONST(subject), SR_CONST(functional_predicate),
                           SR_VAR(0u), 0u};
    check(&score, mse_query(&index, &query, 12u, &result) == MSE_OK &&
                    result.status == MSE_QUERY_MATCH &&
                    result.selected_card_id == card_b.card_id &&
                    result.selected_generation == 12u,
          "supersession selects only the newest active exact evidence");
    check(&score, mse_expire(&index, 17u) == 1u,
          "generation expiry retires the newest reindexed evidence");
    check(&score, mse_query(&index, &query, 17u, &result) == MSE_OK &&
                    result.status == MSE_QUERY_NO_MATCH,
          "expired reindexed evidence is not presented as current knowledge");

    mse_init(&index, functional, NULL);
    check(&score, mse_set_generation_floor(&index, 20u) == MSE_OK &&
                    mse_add(&index, &card_a,
                            &(sr_fact_t){subject, functional_predicate, object_a, 0u},
                            21u, 0u) == MSE_OK &&
                    mse_add(&index, &card_c,
                            &(sr_fact_t){subject, functional_predicate, object_b, 0u},
                            22u, 0u) == MSE_OK && index.conflicts == 1u,
          "post-reboot functional disagreement becomes explicit conflict");
    check(&score, mse_query(&index, &query, 22u, &result) == MSE_OK &&
                    result.status == MSE_QUERY_CONTRADICTED &&
                    result.active_matches == 0u && result.conflict_matches >= 2u,
          "functional conflict abstains instead of selecting a newer fact");

    mse_init(&index, NULL, NULL);
    check(&score, mse_set_generation_floor(&index, 30u) == MSE_OK &&
                    mse_add(&index, &card_a,
                            &(sr_fact_t){subject, open_predicate, object_a, 0u},
                            31u, 0u) == MSE_OK &&
                    mse_add(&index, &card_b,
                            &(sr_fact_t){subject, open_predicate, object_c, 0u},
                            32u, 0u) == MSE_OK,
          "non-functional reindex alternatives remain independently represented");
    query = (sr_pattern_t){SR_CONST(subject), SR_CONST(open_predicate),
                           SR_VAR(0u), 0u};
    check(&score, mse_query(&index, &query, 32u, &result) == MSE_OK &&
                    result.status == MSE_QUERY_AMBIGUOUS &&
                    result.active_matches == 2u && result.selected_card_id == 0u,
          "compatible alternatives remain ambiguous after reindexing");

    printf("MEMORY POST-REBOOT REINDEX: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
