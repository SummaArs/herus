#include "resonator_bridge.h"
#include <stdio.h>
#include <string.h>

#define N 16u
#define DOMAIN 0x52425F4252494447ull

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

int main(void)
{
    hv_t code[N];
    uint16_t id[N];
    rv_codebook_t book;
    rv_problem_t problem;
    hv_t factors[3];
    hv_t product;
    rb_proposal_t proposal;
    sr_reasoner_t reasoner;
    int offsets[3] = { 0, 13, -21 };
    score_t score = { 0, 0 };

    for (unsigned i = 0u; i < N; i++) {
        id[i] = (uint16_t)(500u + i);
        hv_gen(&code[i], DOMAIN, id[i]);
    }
    book.vectors = code;
    book.symbol_id = id;
    book.count = N;
    factors[0] = code[2];
    factors[1] = code[7];
    factors[2] = code[11];
    rv_compose(&product, factors, offsets, 3u);
    memset(&problem, 0, sizeof(problem));
    problem.factor_count = 3u;
    problem.codebook = &book;
    problem.max_iterations = RV_MAX_ITERS;
    problem.max_exact_nodes = RV_MAX_EXACT_NODES;
    for (unsigned i = 0u; i < 3u; i++) problem.offsets[i] = offsets[i];

    check(&score, rb_propose_relation(&product, &problem, 0u,
                                      RB_DEFAULT_MIN_MARGIN_Q8,
                                      RB_DEFAULT_MAX_RESIDUAL,
                                      &proposal) == RB_PROPOSED,
          "resonator bridge emits a typed relation proposal");
    check(&score, proposal.fact.subject == id[2] &&
                    proposal.fact.predicate == id[7] &&
                    proposal.fact.object == id[11] &&
                    proposal.explicitly_accepted == 0u,
          "proposal preserves factor identities but has no authority");

    sr_init(&reasoner);
    check(&score, sr_add_fact(&reasoner, proposal.fact) == SR_OK &&
                    sr_fact_count(&reasoner) == 1u,
          "outer caller can explicitly accept the proposal into the reasoner");

    check(&score, rb_propose_relation(&product, &problem, 1u,
                                      RB_DEFAULT_MIN_MARGIN_Q8,
                                      RB_DEFAULT_MAX_RESIDUAL,
                                      &proposal) == RB_PROPOSED &&
                    proposal.fact.negated == 1u,
          "negated relations remain typed rather than silently discarded");

    {
        rb_proposal_t strict;
        check(&score, rb_propose_relation(&product, &problem, 0u,
                                          32767u, 0u, &strict) == RB_ABSTAIN,
              "strict margin and residual gates force abstention");
    }

    {
        hv_t noisy = product;
        uint64_t rng = 0xfeedface12345678ull;
        hv_flip_bits(&noisy, 0.005, &rng);
        check(&score, rb_propose_relation(&noisy, &problem, 0u,
                                          RB_DEFAULT_MIN_MARGIN_Q8,
                                          RB_DEFAULT_MAX_RESIDUAL,
                                          &proposal) != RB_PROPOSED ||
                        proposal.provenance.residual_distance > 0u,
              "noisy input cannot be mistaken for a perfect proof");
    }

    printf("RESONATOR BRIDGE: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
