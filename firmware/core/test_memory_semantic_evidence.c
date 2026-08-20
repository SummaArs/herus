#include "memory_semantic_evidence.h"
#include <stdio.h>
#include <string.h>

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static int functional(sr_symbol_t predicate, void *user)
{
    (void)user;
    return predicate == SR_SYMBOL_LEGACY(20u);
}

static memory_vault_card_t card(uint32_t card_id, uint32_t receipt)
{
    memory_vault_card_t out;
    memset(&out, 0, sizeof(out));
    out.card_id = card_id;
    out.review_receipt_id = receipt;
    return out;
}

int main(void)
{
    score_t score = { 0, 0 };
    mse_index_t index;
    mse_query_result_t result;
    memory_vault_card_t card_a = card(100u, 900u);
    memory_vault_card_t card_b = card(101u, 901u);
    sr_symbol_t subject = 0x02070001u;
    sr_symbol_t functional_predicate = SR_SYMBOL_LEGACY(20u);
    sr_symbol_t open_predicate = SR_SYMBOL_LEGACY(21u);
    sr_symbol_t object_a = 0x02070011u;
    sr_symbol_t object_b = 0x02070012u;
    sr_pattern_t query;

    mse_init(&index, functional, NULL);
    check(&score, mse_add(&index, &card_a,
                          &(sr_fact_t){subject, functional_predicate, object_a, 0u},
                          10u, 12u) == MSE_OK && index.evidence_count == 1u,
          "reviewed card enters bounded semantic evidence with generation provenance");
    query = (sr_pattern_t){SR_CONST(subject), SR_CONST(functional_predicate),
                           SR_CONST(object_a), 0u};
    check(&score, mse_query(&index, &query, 10u, &result) == MSE_OK &&
                    result.status == MSE_QUERY_MATCH && result.selected_card_id == 100u &&
                    result.selected_generation == 10u,
          "typed query returns the unique active evidence and its opaque provenance");

    check(&score, mse_add(&index, &card_a,
                          &(sr_fact_t){subject, functional_predicate, object_a, 0u},
                          10u, 12u) == MSE_NO_CHANGE && index.evidence_count == 1u,
          "same-generation duplicate is idempotent");
    check(&score, mse_add(&index, &card_b,
                          &(sr_fact_t){subject, functional_predicate, object_a, 0u},
                          9u, 12u) == MSE_E_ROLLBACK,
          "older exact evidence is rejected as rollback");
    check(&score, mse_add(&index, &card_b,
                          &(sr_fact_t){subject, functional_predicate, object_a, 0u},
                          11u, 14u) == MSE_OK && index.evidence[0].status ==
                        MSE_EVIDENCE_SUPERSEDED && index.evidence_count == 2u,
          "newer exact evidence supersedes old evidence rather than duplicating truth");

    check(&score, mse_add(&index, &card_b,
                          &(sr_fact_t){subject, functional_predicate, object_b, 0u},
                          12u, 0u) == MSE_OK && index.conflicts == 1u,
          "functional predicate with a different object becomes explicit conflict");
    query = (sr_pattern_t){SR_CONST(subject), SR_CONST(functional_predicate),
                           SR_VAR(0u), 0u};
    check(&score, mse_query(&index, &query, 12u, &result) == MSE_OK &&
                    result.status == MSE_QUERY_CONTRADICTED &&
                    result.conflict_matches >= 2u,
          "conflicting evidence abstains instead of selecting the newest card");

    mse_init(&index, NULL, NULL);
    check(&score, mse_add(&index, &card_a,
                          &(sr_fact_t){subject, open_predicate, object_a, 0u},
                          2u, 0u) == MSE_OK &&
                    mse_add(&index, &card_b,
                            &(sr_fact_t){subject, open_predicate, object_b, 0u},
                            3u, 0u) == MSE_OK,
          "non-functional alternatives remain separately represented");
    query = (sr_pattern_t){SR_CONST(subject), SR_CONST(open_predicate),
                           SR_VAR(0u), 0u};
    check(&score, mse_query(&index, &query, 3u, &result) == MSE_OK &&
                    result.status == MSE_QUERY_AMBIGUOUS && result.active_matches == 2u,
          "multiple compatible alternatives remain ambiguous");

    check(&score, mse_expire(&index, 4u) == 0u,
          "non-expiring evidence is retained across generation advancement");
    mse_init(&index, NULL, NULL);
    check(&score, mse_add(&index, &card_a,
                          &(sr_fact_t){subject, open_predicate, object_a, 0u},
                          5u, 5u) == MSE_OK && mse_expire(&index, 6u) == 1u,
          "generation expiry retires evidence without deleting provenance abruptly");
    check(&score, mse_query(&index, &query, 6u, &result) == MSE_OK &&
                    result.status == MSE_QUERY_NO_MATCH,
          "expired evidence is not returned as current knowledge");

    query = (sr_pattern_t){SR_VAR(0u), SR_VAR(1u), SR_VAR(2u), 0u};
    result.selected_card_id = 0xdeadbeefu;
    check(&score, mse_query(&index, &query, 6u, &result) == MSE_E_ARG &&
                    result.selected_card_id == 0u,
          "all-variable query cannot enumerate the private evidence index and clears stale output");

    {
        memory_vault_card_t invalid_card = card(0u, 901u);
        memory_vault_card_t valid_card = card(200u, 902u);
        sr_fact_t invalid_fact = { 0u, open_predicate, object_a, 0u };
        mse_index_t bounded;
        mse_init(&bounded, NULL, NULL);
        check(&score, mse_add(&bounded, &invalid_card, &invalid_fact, 1u, 0u) ==
                        MSE_E_ARG && bounded.evidence_count == 0u,
              "zero card, receipt and symbol inputs are rejected without mutation");
        check(&score, mse_add(&bounded, &valid_card,
                              &(sr_fact_t){subject, open_predicate, object_a, 0u},
                              8u, 7u) == MSE_E_ARG && bounded.evidence_count == 0u,
              "validity windows that move backward are rejected before insertion");
        for (uint32_t i = 0u; i < MSE_MAX_EVIDENCE; i++) {
            sr_fact_t item = {subject, open_predicate, (sr_symbol_t)(0x03070001u + i), 0u};
            memory_vault_card_t item_card = card(300u + i, 400u + i);
            if (mse_add(&bounded, &item_card, &item, 20u + i, 0u) != MSE_OK)
                score.fail++;
        }
        check(&score, bounded.evidence_count == MSE_MAX_EVIDENCE &&
                        mse_add(&bounded, &valid_card,
                                &(sr_fact_t){subject, open_predicate, 0x0307ffffu, 0u},
                                99u, 0u) == MSE_E_FULL &&
                        bounded.evidence_count == MSE_MAX_EVIDENCE,
              "full bounded evidence refuses the next card without partial insertion");
    }

    printf("MEMORY SEMANTIC EVIDENCE: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
