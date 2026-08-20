#include "symbolic_dialogue.h"
#include <stdio.h>
#include <string.h>

#define PERSON  1u
#define PLACE   2u
#define OWNS    10u
#define NEEDS   11u
#define READY   12u

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static sr_rule_t ownership_rule(void)
{
    sr_rule_t rule;
    memset(&rule, 0, sizeof(rule));
    rule.id = 1u;
    rule.premise_count = 1u;
    rule.premise[0] = (sr_pattern_t){ SR_VAR(0), SR_CONST(OWNS),
                                     SR_CONST(PLACE), 0u };
    rule.conclusion = (sr_pattern_t){ SR_VAR(0), SR_CONST(NEEDS),
                                     SR_CONST(READY), 0u };
    rule.cost = 2u;
    return rule;
}

int main(void)
{
    sd_dialogue_t dialogue;
    sd_dialogue_t limited;
    sd_dialogue_t vsa_dialogue;
    sd_reply_t reply;
    sr_pattern_t query;
    sr_rule_t rule = ownership_rule();
    score_t score = { 0, 0 };
    sr_fact_t personal = { PERSON, OWNS, PLACE, 0u };

    sd_init(&dialogue);
    check(&score, sd_add_rule(&dialogue, &rule) == SD_OK,
          "factory rule is installed separately from personal memory");
    check(&score, sd_add_personal_fact(&dialogue, personal, 0u) == SD_E_AUTH &&
                    sr_fact_count(&dialogue.reasoner) == 0u,
          "unconfirmed personal knowledge never enters the reasoner");
    check(&score, sd_add_personal_fact(&dialogue, personal, 1u) == SD_OK,
          "explicitly confirmed personal knowledge enters the reasoner");

    query = (sr_pattern_t){ SR_CONST(PERSON), SR_CONST(NEEDS),
                            SR_CONST(READY), 0u };
    check(&score, sd_ask(&dialogue, &query, SD_MAX_DERIVATION_STEPS, &reply) ==
                    SD_OK && reply.answer.kind == SR_ANSWER_DERIVED &&
                    reply.turn == 1u,
          "dialogue composes a novel answer from a personal fact and a rule");
    check(&score, reply.answer.depth == 1u && reply.answer.evidence_count == 1u,
          "the generated reply exposes a compact proof");

    query = (sr_pattern_t){ SR_CONST(99u), SR_CONST(NEEDS),
                            SR_CONST(READY), 0u };
    check(&score, sd_ask(&dialogue, &query, SD_MAX_DERIVATION_STEPS, &reply) ==
                    SR_E_NO_EVIDENCE && reply.answer.kind == SR_ANSWER_ABSENT,
          "dialogue says absent instead of inventing a personal fact");

    sd_init(&limited);
    check(&score, sd_add_rule(&limited, &rule) == SD_OK &&
                    sd_add_personal_fact(&limited, personal, 1u) == SD_OK,
          "a fresh dialogue isolates a new derivation budget experiment");
    query = (sr_pattern_t){ SR_CONST(PERSON), SR_CONST(NEEDS),
                            SR_CONST(READY), 0u };
    check(&score, sd_ask(&limited, &query, 1u, &reply) == SD_E_LIMIT &&
                    reply.answer.kind == SR_ANSWER_LIMIT,
          "dialogue exposes a derivation budget limit");

    sd_init(&vsa_dialogue);
    {
        hv_t code[8];
        uint16_t ids[8];
        hv_t factors[3];
        hv_t product;
        rv_codebook_t book;
        rv_problem_t problem;
        rb_proposal_t proposal;
        int offsets[3] = { 0, 7, -13 };
        for (unsigned i = 0u; i < 8u; i++) {
            ids[i] = (uint16_t)(900u + i);
            hv_gen(&code[i], 0x4449414Cull, ids[i]);
        }
        book.vectors = code; book.symbol_id = ids; book.count = 8u;
        memset(&problem, 0, sizeof(problem));
        problem.factor_count = 3u; problem.codebook = &book;
        problem.max_iterations = RV_MAX_ITERS;
        problem.max_exact_nodes = RV_MAX_EXACT_NODES;
        for (unsigned i = 0u; i < 3u; i++) problem.offsets[i] = offsets[i];
        factors[0] = code[1]; factors[1] = code[4]; factors[2] = code[6];
        rv_compose(&product, factors, offsets, 3u);
        check(&score, sd_propose_vsa_relation(&vsa_dialogue, &product,
                                              &problem, 0u, &proposal) == SD_OK,
              "dialogue accepts a VSA relation only as a transient proposal");
        check(&score, sd_accept_vsa_proposal(&vsa_dialogue, &proposal, 0u) ==
                        SD_E_AUTH && sr_fact_count(&vsa_dialogue.reasoner) == 0u,
              "VSA proposal without physical confirmation is not persisted");
        check(&score, sd_accept_vsa_proposal(&vsa_dialogue, &proposal, 1u) ==
                        SD_OK && sr_fact_count(&vsa_dialogue.reasoner) == 1u,
              "physical confirmation promotes the VSA relation to a fact");
        query = (sr_pattern_t){ SR_CONST(901u), SR_CONST(904u),
                                SR_CONST(906u), 0u };
        check(&score, sd_ask(&vsa_dialogue, &query, SD_MAX_DERIVATION_STEPS,
                             &reply) == SD_OK &&
                        reply.answer.kind == SR_ANSWER_DIRECT,
              "the accepted VSA fact is queryable as direct local evidence");
    }

    check(&score, sd_add_personal_fact(&dialogue,
                                       (sr_fact_t){ PERSON, OWNS, PLACE, 1u },
                                       1u) == SD_OK,
          "contradictory personal evidence can be recorded for review");

    {
        sd_dialogue_t handles;
        sd_reply_t handle_reply;
        sr_symbol_t high_person = srreg_handle_make(SRREG_NAMESPACE_PERSONAL,
                                                     7u, 0x3234u);
        sr_symbol_t high_predicate = srreg_handle_make(SRREG_NAMESPACE_FACTORY,
                                                        7u, 0x3235u);
        sr_symbol_t high_object = srreg_handle_make(SRREG_NAMESPACE_PERSONAL,
                                                     7u, 0x3236u);
        sr_fact_t high_fact = { high_person, high_predicate, high_object, 0u };
        sr_pattern_t high_query = { SR_CONST(high_person),
                                     SR_CONST(high_predicate),
                                     SR_CONST(high_object), 0u };
        sd_init(&handles);
        check(&score, high_person > UINT16_MAX && high_predicate > UINT16_MAX &&
                        sd_add_personal_fact(&handles, high_fact, 1u) == SD_OK &&
                        sd_ask(&handles, &high_query, SD_MAX_DERIVATION_STEPS,
                               &handle_reply) == SD_OK &&
                        handle_reply.answer.kind == SR_ANSWER_DIRECT &&
                        handle_reply.answer.fact.subject == high_person &&
                        handle_reply.answer.fact.predicate == high_predicate &&
                        handle_reply.answer.fact.object == high_object,
              "dialogue preserves collision-aware 32-bit identity through confirmation and query");
    }

    query = (sr_pattern_t){ SR_CONST(PERSON), SR_CONST(OWNS),
                            SR_CONST(PLACE), 0u };
    check(&score, sd_ask(&dialogue, &query, SD_MAX_DERIVATION_STEPS, &reply) ==
                    SR_E_CONTRADICTION &&
                    reply.answer.kind == SR_ANSWER_CONTRADICTED,
          "contradiction is surfaced rather than collapsed into confidence");

    {
        sd_dialogue_t abductive;
        sr_abduction_t proposal;
        sr_pattern_t goal = (sr_pattern_t){ SR_CONST(PERSON), SR_CONST(NEEDS),
                                           SR_CONST(READY), 0u };
        sr_rule_t alternative;
        sr_rule_t second_alternative;
        sd_init(&abductive);
        check(&score, sd_add_rule(&abductive, &rule) == SD_OK &&
                        sd_abduce(&abductive, &goal, 16u, &proposal) == SD_OK &&
                        proposal.status == SR_ABDUCTION_FOUND &&
                        proposal.missing_fact.subject == PERSON &&
                        proposal.missing_fact.predicate == OWNS &&
                        proposal.missing_fact.object == PLACE &&
                        sr_fact_count(&abductive.reasoner) == 0u,
              "dialogue exposes a missing personal fact as a read-only hypothesis");
        check(&score, sd_abduce(&abductive, &goal, 0u, &proposal) == SD_E_LIMIT,
              "dialogue abduction preserves an explicit derivation budget limit");
        memset(&alternative, 0, sizeof(alternative));
        alternative.id = 7u;
        alternative.premise_count = 1u;
        alternative.premise[0] = (sr_pattern_t){ SR_CONST(PERSON),
                                                SR_CONST(OWNS),
                                                SR_CONST(99u), 0u };
        alternative.conclusion = (sr_pattern_t){ SR_CONST(PERSON),
                                                SR_CONST(NEEDS),
                                                SR_CONST(READY), 0u };
        alternative.cost = 1u;
        second_alternative = alternative;
        second_alternative.id = 8u;
        second_alternative.premise[0].object = SR_CONST(100u);
        sd_init(&abductive);
        check(&score, sd_add_rule(&abductive, &alternative) == SD_OK &&
                        sd_add_rule(&abductive, &second_alternative) == SD_OK &&
                        sd_abduce(&abductive, &goal, 16u, &proposal) ==
                            SD_E_ABSTAIN &&
                        proposal.status == SR_ABDUCTION_AMBIGUOUS,
              "dialogue refuses to choose among multiple missing explanations");
        goal.subject = SR_VAR(1u);
        check(&score, sd_abduce(&abductive, &goal, 16u, &proposal) == SD_E_ARG,
              "dialogue abduction rejects a non-ground goal instead of binding an entity");
    }

    {
        sd_dialogue_t full;
        sr_rule_t capacity;
        sr_pattern_t capacity_query;
        sd_dialogue_t rules_full;
        int capacity_facts_ok = 1;
        int capacity_rules_ok = 1;
        int extra_fact_result;
        int extra_rule_result;
        sd_init(&full);
        for (uint16_t subject = 1u; subject <= SR_MAX_FACTS; subject++) {
            if (sd_add_personal_fact(&full,
                                     (sr_fact_t){subject, OWNS, PLACE, 0u}, 1u) != SD_OK)
                capacity_facts_ok = 0;
        }
        memset(&capacity, 0, sizeof(capacity));
        capacity.id = 91u;
        capacity.premise_count = 1u;
        capacity.premise[0] = (sr_pattern_t){SR_VAR(0u), SR_CONST(OWNS),
                                             SR_CONST(PLACE), 0u};
        capacity.conclusion = (sr_pattern_t){SR_VAR(0u), SR_CONST(NEEDS),
                                             SR_CONST(READY), 0u};
        capacity.cost = 1u;
        capacity_query = (sr_pattern_t){SR_CONST(1u), SR_CONST(NEEDS),
                                        SR_CONST(READY), 0u};
        check(&score, capacity_facts_ok && sr_fact_count(&full.reasoner) == SR_MAX_FACTS &&
                        sd_add_rule(&full, &capacity) == SD_OK &&
                        sd_ask(&full, &capacity_query, SD_MAX_DERIVATION_STEPS,
                               &reply) == SD_E_LIMIT &&
                        reply.answer.kind == SR_ANSWER_LIMIT,
              "dialogue exposes a full-memory derivation as an explicit limit");
        extra_fact_result = sd_add_personal_fact(
            &full, (sr_fact_t){999u, OWNS, PLACE, 0u}, 1u);
        sd_init(&rules_full);
        for (uint16_t rule_id = 0u; rule_id < SR_MAX_RULES; rule_id++) {
            capacity.id = (uint8_t)rule_id;
            if (sd_add_rule(&rules_full, &capacity) != SD_OK)
                capacity_rules_ok = 0;
        }
        capacity.id = 200u;
        extra_rule_result = sd_add_rule(&rules_full, &capacity);
        check(&score, extra_fact_result == SD_E_LIMIT && capacity_rules_ok &&
                        sr_rule_count(&rules_full.reasoner) == SR_MAX_RULES &&
                        extra_rule_result == SD_E_LIMIT,
              "dialogue maps full fact and rule insertion to explicit limits");
    }

    printf("SYMBOLIC DIALOGUE: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
