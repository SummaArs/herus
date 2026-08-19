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

typedef struct { int pass; int fail; } test_score_t;

static void check(test_score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static sr_pattern_t pattern(uint16_t subject, uint16_t predicate,
                            uint16_t object, uint8_t negated)
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

    printf("SYMBOLIC REASONER: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
