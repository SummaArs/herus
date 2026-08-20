#include "memory_reasoning_bridge.h"
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

static sr_rule_t local_rule(sr_symbol_t premise_predicate,
                            sr_symbol_t premise_object,
                            sr_symbol_t conclusion_predicate,
                            sr_symbol_t conclusion_object)
{
    sr_rule_t rule;
    memset(&rule, 0, sizeof(rule));
    rule.id = 7u;
    rule.premise_count = 1u;
    rule.premise[0] = (sr_pattern_t){ SR_VAR(0u), SR_CONST(premise_predicate),
                                     SR_CONST(premise_object), 0u };
    rule.conclusion = (sr_pattern_t){ SR_VAR(0u), SR_CONST(conclusion_predicate),
                                      SR_CONST(conclusion_object), 0u };
    rule.cost = 1u;
    return rule;
}

int main(void)
{
    score_t score = { 0, 0 };
    const sr_symbol_t subject = 0x04070001u;
    const sr_symbol_t has = SR_SYMBOL_LEGACY(10u);
    const sr_symbol_t book = SR_SYMBOL_LEGACY(11u);
    const sr_symbol_t ready = SR_SYMBOL_LEGACY(12u);
    const sr_symbol_t today = SR_SYMBOL_LEGACY(13u);
    const sr_symbol_t open = SR_SYMBOL_LEGACY(21u);
    sr_reasoner_t base;
    sr_reasoner_t scratch;
    sr_reasoner_t before;
    mse_index_t memory;
    mrb_meta_t meta;
    sr_answer_t answer;
    sr_pattern_t query;
    sr_rule_t rule = local_rule(has, book, ready, today);
    memory_vault_card_t reviewed = card(77u, 707u);

    sr_init(&base);
    check(&score, sr_add_rule(&base, &rule) == SR_OK,
          "local rule is accepted before memory composition");
    mse_init(&memory, NULL, NULL);
    check(&score, mse_add(&memory, &reviewed,
                          &(sr_fact_t){subject, has, book, 0u}, 4u, 0u) == MSE_OK,
          "reviewed memory fact is available to the offline bridge");
    query = (sr_pattern_t){SR_CONST(subject), SR_CONST(ready), SR_CONST(today), 0u};
    before = base;
    check(&score, mrb_query(&base, &memory, 4u, &query, 16u, &scratch,
                            &answer, &meta) == MRB_OK &&
                    answer.kind == SR_ANSWER_DERIVED &&
                    meta.memory_imported == 1u && meta.selected_card_id == 0u &&
                    base.fact_count == before.fact_count &&
                    scratch.fact_count == 2u,
          "memory fact composes with a local rule offline without mutating the base reasoner");
    check(&score, meta.memory_status == MSE_QUERY_NO_MATCH &&
                    meta.reasoner_status == SR_OK && scratch.meta[0].origin == 0u,
          "composition reports the local query path and preserves input/derived metadata");

    query = (sr_pattern_t){SR_VAR(0u), SR_VAR(1u), SR_VAR(2u), 0u};
    answer.fact.subject = 0xdeadbeefu;
    check(&score, mrb_query(&base, &memory, 4u, &query, 16u, &scratch,
                            &answer, &meta) == MRB_E_ARG &&
                    answer.fact.subject == 0u,
          "all-variable bridge queries are rejected and stale answers are cleared");
    check(&score, mrb_query(&base, &memory, 4u, &query, 16u, &base,
                            &answer, &meta) == MRB_E_SCRATCH,
          "scratch aliasing the base reasoner is rejected");

    mse_init(&memory, NULL, NULL);
    check(&score, mse_add(&memory, &reviewed,
                          &(sr_fact_t){subject, open, book, 0u}, 5u, 0u) == MSE_OK &&
                    mse_add(&memory, &reviewed,
                            &(sr_fact_t){subject, open, today, 0u}, 6u, 0u) == MSE_OK,
          "non-functional memory alternatives are retained for composition");
    query = (sr_pattern_t){SR_CONST(subject), SR_CONST(open), SR_VAR(0u), 0u};
    check(&score, mrb_query(&base, &memory, 6u, &query, 16u, &scratch,
                            &answer, &meta) == MRB_AMBIGUOUS &&
                    meta.memory_status == MSE_QUERY_AMBIGUOUS &&
                    answer.kind == SR_ANSWER_NONE,
          "ambiguous memory abstains before the reasoner chooses an alternative");

    mse_init(&memory, functional, NULL);
    check(&score, mse_add(&memory, &reviewed,
                          &(sr_fact_t){subject, SR_SYMBOL_LEGACY(20u), book, 0u},
                          7u, 0u) == MSE_OK &&
                    mse_add(&memory, &reviewed,
                            &(sr_fact_t){subject, SR_SYMBOL_LEGACY(20u), today, 0u},
                            8u, 0u) == MSE_OK,
          "functional memory alternatives are marked as conflict");
    query = (sr_pattern_t){SR_CONST(subject), SR_CONST(SR_SYMBOL_LEGACY(20u)),
                           SR_VAR(0u), 0u};
    check(&score, mrb_query(&base, &memory, 8u, &query, 16u, &scratch,
                            &answer, &meta) == MRB_CONTRADICTED &&
                    meta.memory_status == MSE_QUERY_CONTRADICTED,
          "contradictory memory never enters the reasoner scratch state");

    mse_init(&memory, NULL, NULL);
    check(&score, mse_add(&memory, &reviewed,
                          &(sr_fact_t){subject, has, book, 0u}, 9u, 9u) == MSE_OK,
          "expiring memory fact is accepted with a bounded validity generation");
    check(&score, mrb_query(&base, &memory, 10u, &query, 16u, &scratch,
                            &answer, &meta) == MRB_NO_EVIDENCE &&
                    meta.memory_imported == 0u && meta.memory_skipped == 1u,
          "expired memory is skipped and does not become offline evidence");

    mse_init(&memory, NULL, NULL);
    check(&score, mse_add(&memory, &reviewed,
                          &(sr_fact_t){subject, has, book, 0u}, 11u, 0u) == MSE_OK,
          "bounded memory is restored for saturation-limit proof");
    check(&score, mrb_query(&base, &memory, 11u, &query, 1u, &scratch,
                            &answer, &meta) == MRB_LIMIT,
          "partial saturation is never returned as a successful answer");

    printf("MEMORY REASONING BRIDGE: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
