#include "generative_core.h"

#include <stdio.h>
#include <string.h>

#define S_ALICE 1u
#define S_BOB 2u
#define S_CARA 3u
#define S_DAVI 4u
#define S_ERIN 5u
#define P_PARENT 10u
#define P_GRAND 11u
#define P_ANCESTOR 12u

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static sr_pattern_t pattern(sr_symbol_t subject, sr_symbol_t predicate,
                            sr_symbol_t object, uint8_t negated)
{
    sr_pattern_t result;
    result.subject = SR_CONST(subject);
    result.predicate = SR_CONST(predicate);
    result.object = SR_CONST(object);
    result.negated = negated;
    return result;
}

static sr_pattern_t terms(sr_term_t subject, sr_term_t predicate,
                          sr_term_t object)
{
    sr_pattern_t result;
    result.subject = subject;
    result.predicate = predicate;
    result.object = object;
    result.negated = 0u;
    return result;
}

static sr_rule_t binary_rule(uint8_t id, uint16_t premise_predicate,
                             uint16_t conclusion_predicate)
{
    sr_rule_t rule;
    memset(&rule, 0, sizeof(rule));
    rule.id = id;
    rule.premise_count = 2u;
    rule.premise[0] = terms(SR_VAR(0u), SR_CONST(premise_predicate),
                            SR_VAR(1u));
    rule.premise[1] = terms(SR_VAR(1u), SR_CONST(premise_predicate),
                            SR_VAR(2u));
    rule.conclusion = terms(SR_VAR(0u), SR_CONST(conclusion_predicate),
                            SR_VAR(2u));
    rule.cost = 1u;
    return rule;
}

static sr_rule_t ancestor_extend_rule(void)
{
    sr_rule_t rule;
    memset(&rule, 0, sizeof(rule));
    rule.id = 4u;
    rule.premise_count = 2u;
    rule.premise[0] = terms(SR_VAR(0u), SR_CONST(P_ANCESTOR),
                            SR_VAR(1u));
    rule.premise[1] = terms(SR_VAR(1u), SR_CONST(P_PARENT),
                            SR_VAR(2u));
    rule.conclusion = terms(SR_VAR(0u), SR_CONST(P_ANCESTOR),
                            SR_VAR(2u));
    rule.cost = 1u;
    return rule;
}

static sr_rule_t unary_rule(uint8_t id, uint16_t premise_predicate,
                            uint16_t conclusion_predicate)
{
    sr_rule_t rule;
    memset(&rule, 0, sizeof(rule));
    rule.id = id;
    rule.premise_count = 1u;
    rule.premise[0] = terms(SR_VAR(0u), SR_CONST(premise_predicate),
                            SR_VAR(1u));
    rule.conclusion = terms(SR_VAR(0u), SR_CONST(conclusion_predicate),
                            SR_VAR(1u));
    rule.cost = 1u;
    return rule;
}

static void build_reasoner(sr_reasoner_t *reasoner)
{
    sr_rule_t grand = binary_rule(1u, P_PARENT, P_GRAND);
    sr_rule_t ancestor_from_grand = unary_rule(2u, P_GRAND, P_ANCESTOR);
    sr_rule_t ancestor_from_parent = binary_rule(3u, P_PARENT, P_ANCESTOR);
    sr_rule_t ancestor_extend = ancestor_extend_rule();
    sr_init(reasoner);
    (void)sr_add_fact(reasoner, (sr_fact_t){ S_ALICE, P_PARENT, S_BOB, 0u });
    (void)sr_add_fact(reasoner, (sr_fact_t){ S_BOB, P_PARENT, S_CARA, 0u });
    (void)sr_add_fact(reasoner, (sr_fact_t){ S_CARA, P_PARENT, S_DAVI, 0u });
    (void)sr_add_fact(reasoner, (sr_fact_t){ S_DAVI, P_PARENT, S_ERIN, 0u });
    (void)sr_add_rule(reasoner, &grand);
    (void)sr_add_rule(reasoner, &ancestor_from_grand);
    (void)sr_add_rule(reasoner, &ancestor_from_parent);
    (void)sr_add_rule(reasoner, &ancestor_extend);
}

static int expect_reasoner(score_t *score, sr_reasoner_t *reasoner,
                           const char *label, sr_pattern_t query,
                           int expected_status, sr_answer_kind_t expected_kind,
                           uint8_t depth_min)
{
    sr_answer_t answer;
    int status = sr_query(reasoner, &query, &answer);
    int ok = status == expected_status && answer.kind == expected_kind &&
             answer.depth >= depth_min;
    check(score, ok, label);
    return ok;
}

int main(void)
{
    score_t score = { 0, 0 };
    sr_reasoner_t reasoner;
    sr_reasoner_t limited;
    sr_answer_t answer;
    gc_request_t request;
    gc_result_t generated;
    gc_lexicon_t lexicon;
    sr_reasoner_t scratch;
    const gc_lexeme_t entries[] = {
        { S_ALICE, "alice", 5u },
        { S_BOB, "bob", 3u },
        { S_CARA, "cara", 4u },
        { S_DAVI, "davi", 4u },
        { S_ERIN, "erin", 4u },
        { P_PARENT, "pai", 3u },
        { P_GRAND, "avo", 3u },
        { P_ANCESTOR, "ancestral", 9u }
    };

    build_reasoner(&reasoner);
    check(&score, sr_saturate(&reasoner, 128u) == SR_OK,
          "known atoms reach a bounded fixed point");
    expect_reasoner(&score, &reasoner, "in-distribution direct baseline",
                    pattern(S_ALICE, P_PARENT, S_BOB, 0u),
                    SR_OK, SR_ANSWER_DIRECT, 0u);
    expect_reasoner(&score, &reasoner, "in-distribution two-premise baseline",
                    pattern(S_ALICE, P_GRAND, S_CARA, 0u),
                    SR_OK, SR_ANSWER_DERIVED, 1u);
    expect_reasoner(&score, &reasoner, "held-out entity permutation",
                    pattern(S_BOB, P_GRAND, S_DAVI, 0u),
                    SR_OK, SR_ANSWER_DERIVED, 1u);
    expect_reasoner(&score, &reasoner, "held-out rule recomposition",
                    pattern(S_ALICE, P_ANCESTOR, S_DAVI, 0u),
                    SR_OK, SR_ANSWER_DERIVED, 2u);
    expect_reasoner(&score, &reasoner, "held-out deeper composition",
                    pattern(S_ALICE, P_ANCESTOR, S_ERIN, 0u),
                    SR_OK, SR_ANSWER_DERIVED, 2u);
    expect_reasoner(&score, &reasoner, "reversed arguments abstain",
                    pattern(S_ERIN, P_ANCESTOR, S_ALICE, 0u),
                    SR_E_NO_EVIDENCE, SR_ANSWER_ABSENT, 0u);
    expect_reasoner(&score, &reasoner, "missing premise abstains",
                    pattern(S_ERIN, P_GRAND, S_ALICE, 0u),
                    SR_E_NO_EVIDENCE, SR_ANSWER_ABSENT, 0u);

    sr_init(&limited);
    (void)sr_add_fact(&limited, (sr_fact_t){ S_ALICE, P_PARENT, S_BOB, 0u });
    (void)sr_add_fact(&limited, (sr_fact_t){ S_BOB, P_PARENT, S_CARA, 0u });
    (void)sr_add_fact(&limited, (sr_fact_t){ S_CARA, P_PARENT, S_DAVI, 0u });
    (void)sr_add_fact(&limited, (sr_fact_t){ S_DAVI, P_PARENT, S_ERIN, 0u });
    {
        sr_rule_t grand = binary_rule(1u, P_PARENT, P_GRAND);
        sr_rule_t ancestor_from_grand = unary_rule(2u, P_GRAND, P_ANCESTOR);
        check(&score, sr_add_rule(&limited, &grand) == SR_OK &&
                        sr_add_rule(&limited, &ancestor_from_grand) == SR_OK &&
                        sr_saturate(&limited, 1u) == SR_E_LIMIT,
              "bounded OOD search exposes a limit instead of guessing");
    }

    {
        sr_reasoner_t contradicted;
        sr_init(&contradicted);
        (void)sr_add_fact(&contradicted,
                          (sr_fact_t){ S_ALICE, P_PARENT, S_BOB, 0u });
        (void)sr_add_fact(&contradicted,
                          (sr_fact_t){ S_ALICE, P_PARENT, S_BOB, 1u });
        check(&score, sr_query(&contradicted,
                               & (sr_pattern_t) { SR_CONST(S_ALICE),
                                                   SR_CONST(P_PARENT),
                                                   SR_CONST(S_BOB), 0u },
                               &answer) == SR_E_CONTRADICTION &&
                        answer.kind == SR_ANSWER_CONTRADICTED,
              "opposite evidence blocks an OOD confident answer");
    }

    {
        sr_pattern_t variable_query = terms(SR_VAR(0u), SR_CONST(P_PARENT),
                                            SR_VAR(1u));
        check(&score, sr_query(&reasoner, &variable_query, &answer) ==
                        SR_E_AMBIGUOUS && answer.kind == SR_ANSWER_AMBIGUOUS,
              "variable recombination remains explicitly ambiguous");
    }

    lexicon.entries = entries;
    lexicon.count = sizeof(entries) / sizeof(entries[0]);
    memset(&request, 0, sizeof(request));
    request.mode = GC_MODE_ANSWER;
    request.query = pattern(S_ALICE, P_ANCESTOR, S_ERIN, 0u);
    request.derivation_budget = 128u;
    request.current_generation = 1u;
    check(&score, gc_generate(&reasoner, &lexicon, &request, &scratch,
                              &generated) == GC_STATUS_OK &&
                    generated.kind == GC_KIND_DERIVED &&
                    generated.response_length > 0u &&
                    generated.evidence_count > 0u &&
                    generated.lexeme_missing == 0u,
          "held-out symbolic composition reaches bounded textual generation");
    request.query = pattern(S_ERIN, P_ANCESTOR, S_ALICE, 0u);
    check(&score, gc_generate(&reasoner, &lexicon, &request, &scratch,
                              &generated) == GC_STATUS_ABSTAIN &&
                    generated.abstain_reason == GC_ABSTAIN_NO_EVIDENCE,
          "held-out absent composition becomes generative abstention");

    printf("COMPOSITIONAL OOD: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail == 0 ? 0 : 1;
}
