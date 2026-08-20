#include "resonator_bridge.h"
#include <stdio.h>
#include <string.h>

#define MAX_N 24u
#define TRIALS 12u

typedef struct { unsigned pass; unsigned fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    if (condition) score->pass++; else {
        score->fail++;
        printf("  FAIL  %s\n", label);
    }
}

int main(void)
{
    const unsigned sizes[] = { 8u, 16u, 24u };
    const int offsets[3] = { 0, 11, -23 };
    score_t score = { 0u, 0u };
    unsigned converged = 0u;
    unsigned abstained = 0u;
    unsigned total = 0u;

    for (unsigned size_index = 0u; size_index < 3u; size_index++) {
        unsigned n = sizes[size_index];
        hv_t vectors[MAX_N];
        uint16_t ids[MAX_N];
        rv_codebook_t book;
        rv_problem_t problem;
        for (unsigned i = 0u; i < n; i++) {
            ids[i] = (uint16_t)(1000u + size_index * 100u + i);
            hv_gen(&vectors[i], 0x535452455353ull + size_index, ids[i]);
        }
        book.vectors = vectors;
        book.symbol_id = ids;
        book.count = (uint16_t)n;
        memset(&problem, 0, sizeof(problem));
        problem.factor_count = 3u;
        problem.codebook = &book;
        problem.max_iterations = RV_MAX_ITERS;
        problem.max_exact_nodes = RV_MAX_EXACT_NODES;
        for (unsigned j = 0u; j < 3u; j++) problem.offsets[j] = offsets[j];

        for (unsigned trial = 0u; trial < TRIALS; trial++) {
            hv_t factors[3];
            hv_t product;
            rb_proposal_t proposal;
            factors[0] = vectors[(trial * 3u + 1u) % n];
            factors[1] = vectors[(trial * 5u + 2u) % n];
            factors[2] = vectors[(trial * 7u + 3u) % n];
            rv_compose(&product, factors, offsets, 3u);
            total++;
            rb_status_t status = rb_propose_relation(
                &product, &problem, 0u, RB_DEFAULT_MIN_MARGIN_Q8,
                RB_DEFAULT_MAX_RESIDUAL, &proposal);
            if (status == RB_PROPOSED) {
                converged++;
                check(&score, proposal.provenance.residual_distance == 0u,
                      "accepted stress relation reconstructs exactly");
            } else {
                abstained++;
                check(&score, proposal.provenance.residual_distance > 0u ||
                            proposal.provenance.ambiguous_mask != 0u ||
                            proposal.provenance.status != RV_CONVERGED,
                      "rejected stress relation carries a reason to abstain");
            }
        }
    }

    {
        hv_t vectors[32];
        uint16_t ids[32];
        rv_codebook_t book;
        rv_problem_t problem;
        hv_t factors[3];
        hv_t product;
        rb_proposal_t proposal;
        for (unsigned i = 0u; i < 32u; i++) {
            ids[i] = (uint16_t)(2000u + i);
            hv_gen(&vectors[i], 0x4E4F495345ull, ids[i]);
        }
        book.vectors = vectors; book.symbol_id = ids; book.count = 32u;
        memset(&problem, 0, sizeof(problem));
        problem.factor_count = 3u; problem.codebook = &book;
        problem.max_iterations = RV_MAX_ITERS;
        problem.max_exact_nodes = RV_MAX_EXACT_NODES;
        problem.offsets[0] = offsets[0]; problem.offsets[1] = offsets[1];
        problem.offsets[2] = offsets[2];
        factors[0] = vectors[3]; factors[1] = vectors[12]; factors[2] = vectors[27];
        rv_compose(&product, factors, offsets, 3u);
        {
            uint64_t rng = 0x9e3779b97f4a7c15ull;
            hv_flip_bits(&product, 0.02, &rng);
        }
        check(&score, rb_propose_relation(&product, &problem, 0u,
                                           RB_DEFAULT_MIN_MARGIN_Q8,
                                           RB_DEFAULT_MAX_RESIDUAL,
                                           &proposal) != RB_PROPOSED ||
                        proposal.provenance.residual_distance > 0u,
              "2 percent vector corruption never presents as a perfect proof");
    }

    printf("RESONATOR STRESS: %u pass, %u fail; trials=%u converged=%u abstained=%u\n",
           score.pass, score.fail, total, converged, abstained);
    return score.fail ? 1 : 0;
}
