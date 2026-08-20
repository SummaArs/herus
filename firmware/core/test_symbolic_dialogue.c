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
    query = (sr_pattern_t){ SR_CONST(PERSON), SR_CONST(OWNS),
                            SR_CONST(PLACE), 0u };
    check(&score, sd_ask(&dialogue, &query, SD_MAX_DERIVATION_STEPS, &reply) ==
                    SR_E_CONTRADICTION &&
                    reply.answer.kind == SR_ANSWER_CONTRADICTED,
          "contradiction is surfaced rather than collapsed into confidence");

    printf("SYMBOLIC DIALOGUE: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
