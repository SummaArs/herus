#include "symbolic_reasoner.h"
#include <stdio.h>
#include <string.h>

#define S_ALICE  1u
#define S_BOB    2u
#define S_CARA   3u
#define P_PARENT 10u
#define P_GRAND  11u
#define P_ANCEST 12u
#define P_STAGE0 20u
#define P_STAGE1 21u
#define P_STAGE2 22u
#define TEST_HANDLE(ns, version, slot) \
    ((((sr_symbol_t)(ns)) << 24) | (((sr_symbol_t)(version)) << 16) | \
     ((sr_symbol_t)(slot)))

typedef struct { int pass; int fail; } test_score_t;

static void check(test_score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static sr_pattern_t pattern(sr_symbol_t subject, sr_symbol_t predicate,
                            sr_symbol_t object, uint8_t negated)
{
    sr_pattern_t p;
    p.subject = SR_CONST(subject);
    p.predicate = SR_CONST(predicate);
    p.object = SR_CONST(object);
    p.negated = negated;
    return p;
}

static sr_pattern_t pattern_terms(sr_term_t subject, sr_term_t predicate,
                                  sr_term_t object, uint8_t negated)
{
    sr_pattern_t p;
    p.subject = subject;
    p.predicate = predicate;
    p.object = object;
    p.negated = negated;
    return p;
}

static sr_rule_t parent_rule(void)
{
    sr_rule_t rule;
    memset(&rule, 0, sizeof(rule));
    rule.id = 1u;
    rule.premise_count = 2u;
    rule.premise[0] = pattern_terms(SR_VAR(0), SR_CONST(P_PARENT),
                                    SR_VAR(1), 0u);
    rule.premise[1] = pattern_terms(SR_VAR(1), SR_CONST(P_PARENT),
                                    SR_VAR(2), 0u);
    rule.conclusion = pattern_terms(SR_VAR(0), SR_CONST(P_GRAND),
                                    SR_VAR(2), 0u);
    rule.cost = 3u;
    return rule;
}

static sr_rule_t ancestor_rule(void)
{
    sr_rule_t rule;
    memset(&rule, 0, sizeof(rule));
    rule.id = 2u;
    rule.premise_count = 1u;
    rule.premise[0] = pattern_terms(SR_VAR(0), SR_CONST(P_GRAND),
                                    SR_VAR(1), 0u);
    rule.conclusion = pattern_terms(SR_VAR(0), SR_CONST(P_ANCEST),
                                    SR_VAR(1), 0u);
    rule.cost = 2u;
    return rule;
}

static sr_rule_t stage_rule(uint8_t id, uint16_t from, uint16_t to)
{
    sr_rule_t rule;
    memset(&rule, 0, sizeof(rule));
    rule.id = id;
    rule.premise_count = 1u;
    rule.premise[0] = pattern_terms(SR_CONST(S_ALICE), SR_CONST(from),
                                    SR_CONST(S_BOB), 0u);
    rule.conclusion = pattern_terms(SR_CONST(S_ALICE), SR_CONST(to),
                                    SR_CONST(S_BOB), 0u);
    rule.cost = 1u;
    return rule;
}

int main(void)
{
    sr_reasoner_t reasoner;
    sr_answer_t answer;
    sr_fact_t direct = { S_ALICE, P_PARENT, S_BOB, 0u };
    sr_fact_t second = { S_BOB, P_PARENT, S_CARA, 0u };
    sr_fact_t contradiction = { S_ALICE, P_PARENT, S_BOB, 1u };
    sr_rule_t invalid;
    sr_rule_t parent = parent_rule();
    sr_rule_t ancestor = ancestor_rule();
    sr_rule_t stage0 = stage_rule(10u, P_STAGE0, P_STAGE1);
    sr_rule_t stage1 = stage_rule(11u, P_STAGE1, P_STAGE2);
    sr_pattern_t query;
    test_score_t score = { 0, 0 };

    sr_init(&reasoner);
    check(&score, sr_add_fact(&reasoner, direct) == SR_OK,
          "a ground fact is accepted");
    check(&score, sr_add_fact(&reasoner, direct) == SR_NO_CHANGE,
          "duplicate facts do not inflate the knowledge base");
    check(&score, sr_add_fact(&reasoner, second) == SR_OK,
          "a second fact is accepted");
    check(&score, sr_add_rule(&reasoner, &parent) == SR_OK,
          "a bounded two-premise generative rule is accepted");
    check(&score, sr_add_rule(&reasoner, &ancestor) == SR_OK,
          "a second rule composes over generated facts");
    check(&score, sr_saturate(&reasoner, 64u) == SR_OK,
          "saturation reaches a fixed point");
    check(&score, sr_fact_count(&reasoner) == 4u,
          "two rules generated two new ground facts");

    query = pattern(S_ALICE, P_PARENT, S_BOB, 0u);
    check(&score, sr_query(&reasoner, &query, &answer) == SR_OK &&
                    answer.kind == SR_ANSWER_DIRECT && answer.depth == 0u,
          "direct evidence is returned as direct");
    query = pattern(S_ALICE, P_ANCEST, S_CARA, 0u);
    check(&score, sr_query(&reasoner, &query, &answer) == SR_OK &&
                    answer.kind == SR_ANSWER_DERIVED && answer.depth >= 1u &&
                    answer.evidence_count > 0u,
          "a novel conclusion is returned with a derivation proof");
    query = pattern(S_CARA, P_ANCEST, S_ALICE, 0u);
    check(&score, sr_query(&reasoner, &query, &answer) == SR_E_NO_EVIDENCE &&
                    answer.kind == SR_ANSWER_ABSENT,
          "lack of evidence is not hallucinated into an answer");
    query = pattern_terms(SR_VAR(0), SR_CONST(P_PARENT),
                          SR_VAR(1), 0u);
    check(&score, sr_query(&reasoner, &query, &answer) ==
                    SR_E_AMBIGUOUS && answer.kind == SR_ANSWER_AMBIGUOUS &&
                    answer.hit_count == 2u,
          "a variable query remains ambiguous instead of choosing a fact");

    query = pattern(S_ALICE, P_PARENT, S_BOB, 0u);
    check(&score, sr_add_fact(&reasoner, contradiction) == SR_OK &&
                    sr_contradiction_count(&reasoner) == 1u,
          "opposite evidence is recorded as a contradiction");
    check(&score, sr_query(&reasoner, &query, &answer) == SR_E_CONTRADICTION &&
                    answer.kind == SR_ANSWER_CONTRADICTED,
          "contradictory evidence blocks a confident answer");

    {
        sr_reasoner_t abductive;
        sr_abduction_t proposal;
        sr_pattern_t goal = pattern(S_ALICE, P_GRAND, S_CARA, 0u);
        unsigned facts_before;
        sr_init(&abductive);
        sr_add_fact(&abductive, direct);
        sr_add_rule(&abductive, &parent);
        facts_before = sr_fact_count(&abductive);
        check(&score, sr_abduce(&abductive, &goal, 8u, &proposal) ==
                        SR_ABDUCTION_FOUND &&
                        proposal.missing_fact.subject == S_BOB &&
                        proposal.missing_fact.predicate == P_PARENT &&
                        proposal.missing_fact.object == S_CARA &&
                        proposal.rule_id == parent.id &&
                        proposal.missing_premise == 1u &&
                        proposal.supporting_count == 1u,
              "abduction proposes the single missing fact with rule and support metadata");
        check(&score, sr_fact_count(&abductive) == facts_before &&
                        sr_rule_count(&abductive) == 1u,
              "abduction is a read-only hypothesis and never mutates local knowledge");
        check(&score, sr_abduce(&abductive, &goal, 0u, &proposal) ==
                        SR_ABDUCTION_LIMIT,
              "abduction exposes a zero-candidate budget instead of searching unboundedly");
        goal = pattern_terms(SR_VAR(0u), SR_CONST(P_GRAND),
                             SR_CONST(S_CARA), 0u);
        check(&score, sr_abduce(&abductive, &goal, 8u, &proposal) ==
                        SR_ABDUCTION_E_ARG,
              "abduction requires a ground goal and does not guess an entity binding");
    }

    {
        sr_reasoner_t ambiguous;
        sr_abduction_t proposal;
        sr_rule_t alt0 = stage_rule(12u, P_STAGE0, P_STAGE1);
        sr_rule_t alt1 = stage_rule(13u, P_STAGE2, P_STAGE1);
        sr_pattern_t goal = pattern(S_ALICE, P_STAGE1, S_BOB, 0u);
        sr_init(&ambiguous);
        sr_add_rule(&ambiguous, &alt0);
        sr_add_rule(&ambiguous, &alt1);
        check(&score, sr_abduce(&ambiguous, &goal, 8u, &proposal) ==
                        SR_ABDUCTION_AMBIGUOUS &&
                        proposal.missing_fact.subject == 0u,
              "multiple valid missing facts produce explicit abduction ambiguity");
        check(&score, sr_abduce(&ambiguous, &goal, 1u, &proposal) ==
                        SR_ABDUCTION_LIMIT,
              "abduction candidate budget stops before selecting an arbitrary explanation");
    }

    memset(&invalid, 0, sizeof(invalid));
    invalid.id = 99u;
    invalid.premise_count = 1u;
    invalid.premise[0] = pattern(1u, P_PARENT, 2u, 0u);
    invalid.conclusion = pattern_terms(SR_VAR(7), SR_CONST(P_GRAND),
                                       SR_CONST(2u), 0u);
    check(&score, sr_add_rule(&reasoner, &invalid) == SR_E_FORMAT,
          "a rule cannot generate an unbound variable");

    sr_init(&reasoner);
    check(&score, sr_add_fact(&reasoner,
                              (sr_fact_t){ S_ALICE, P_STAGE0, S_BOB, 0u }) == SR_OK,
          "a fresh staged knowledge base starts cleanly");
    check(&score, sr_add_rule(&reasoner, &stage0) == SR_OK &&
                    sr_add_rule(&reasoner, &stage1) == SR_OK,
          "a staged generative chain is installed");
    check(&score, sr_saturate(&reasoner, 1u) == SR_E_LIMIT &&
                    reasoner.saturation_truncated == 1u,
          "a bounded search budget produces an explicit limit, not a false result");

    {
        sr_reasoner_t handles;
        sr_answer_t handle_answer;
        sr_symbol_t high_subject = TEST_HANDLE(SRREG_NAMESPACE_PERSONAL,
                                                  7u, 0x1234u);
        sr_symbol_t high_predicate = TEST_HANDLE(SRREG_NAMESPACE_FACTORY,
                                                    7u, 0x1234u);
        sr_symbol_t high_object = TEST_HANDLE(SRREG_NAMESPACE_PERSONAL,
                                                  7u, 0x1235u);
        sr_fact_t high_fact = { high_subject, high_predicate, high_object, 0u };
        sr_pattern_t high_query = pattern(high_subject, high_predicate,
                                          high_object, 0u);
        sr_init(&handles);
        check(&score, high_subject > UINT16_MAX && high_predicate > UINT16_MAX &&
                        high_subject != high_predicate && high_object != high_subject &&
                        sr_add_fact(&handles, high_fact) == SR_OK &&
                        sr_query(&handles, &high_query, &handle_answer) == SR_OK &&
                        handle_answer.kind == SR_ANSWER_DIRECT &&
                        handle_answer.fact.subject == high_subject &&
                        handle_answer.fact.predicate == high_predicate &&
                        handle_answer.fact.object == high_object,
              "reasoner preserves collision-aware 32-bit handles without legacy truncation");
    }

    {
        sr_reasoner_t full;
        sr_rule_t capacity;
        int capacity_facts_ok = 1;
        sr_init(&full);
        for (uint16_t subject = 1u; subject <= SR_MAX_FACTS; subject++) {
            if (sr_add_fact(&full,
                            (sr_fact_t){ subject, P_STAGE0, S_BOB, 0u }) != SR_OK)
                capacity_facts_ok = 0;
        }
        check(&score, capacity_facts_ok && sr_fact_count(&full) == SR_MAX_FACTS,
              "capacity fixture accepts the full bounded fact set");
        memset(&capacity, 0, sizeof(capacity));
        capacity.id = 88u;
        capacity.premise_count = 1u;
        capacity.premise[0] = pattern_terms(SR_VAR(0u), SR_CONST(P_STAGE0),
                                             SR_CONST(S_BOB), 0u);
        capacity.conclusion = pattern_terms(SR_VAR(0u), SR_CONST(P_STAGE1),
                                             SR_CONST(S_BOB), 0u);
        capacity.cost = 1u;
        check(&score, sr_add_rule(&full, &capacity) == SR_OK,
              "capacity fixture installs a valid generative rule");
        check(&score, sr_saturate(&full, 64u) == SR_E_FULL &&
                        full.saturation_truncated == 1u &&
                        sr_fact_count(&full) == SR_MAX_FACTS,
              "full knowledge base reports capacity truncation instead of false fixed point");
    }

    printf("SYMBOLIC REASONER: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
