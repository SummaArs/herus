#include "resonator.h"
#include <stdio.h>
#include <string.h>

#define CODEBOOK_N 32u
#define DOMAIN     0x48455255535F5256ull

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

int main(void)
{
    hv_t code[CODEBOOK_N];
    uint16_t ids[CODEBOOK_N];
    rv_codebook_t book;
    rv_problem_t problem;
    rv_result_t result;
    hv_t factors[3];
    hv_t product;
    int offsets[3] = { 0, 17, -29 };
    score_t score = { 0, 0 };

    for (unsigned i = 0u; i < CODEBOOK_N; i++) {
        ids[i] = (uint16_t)(100u + i);
        hv_gen(&code[i], DOMAIN, ids[i]);
    }
    book.vectors = code;
    book.symbol_id = ids;
    book.count = CODEBOOK_N;
    factors[0] = code[3];
    factors[1] = code[17];
    factors[2] = code[29];
    check(&score, rv_compose(&product, factors, offsets, 3u) == RV_CONVERGED,
          "a three-factor rotated product is composed");

    memset(&problem, 0, sizeof(problem));
    problem.factor_count = 3u;
    problem.codebook = &book;
    problem.max_iterations = 32u;
    problem.max_exact_nodes = RV_MAX_EXACT_NODES;
    for (unsigned i = 0u; i < 3u; i++) problem.offsets[i] = offsets[i];
    check(&score, rv_solve(&product, &problem, &result) == RV_CONVERGED,
          "resonator converges on a bounded exact codebook");
    printf("    status=%d ids=%u,%u,%u expected=%u,%u,%u residual=%u fallback=%u nodes=%u\n",
           result.status, result.symbol_id[0], result.symbol_id[1],
           result.symbol_id[2], ids[3], ids[17], ids[29],
           result.residual_distance, result.used_exact_fallback,
           result.search_nodes);
    check(&score, result.symbol_id[0] == ids[3] &&
                    result.symbol_id[1] == ids[17] &&
                    result.symbol_id[2] == ids[29],
          "all factors are recovered with their semantic symbol ids");
    check(&score, result.residual_distance == 0u &&
                    result.ambiguous_mask == 0u,
          "converged factorization reconstructs the product exactly");
    check(&score, result.iterations > 0u && result.iterations <= 32u,
          "iteration count remains bounded and observable");
    check(&score, result.margin_q8[0] > 0 && result.margin_q8[1] > 0 &&
                    result.margin_q8[2] > 0,
          "each recovered factor has a positive codebook margin");

    {
        hv_t noisy = product;
        uint64_t rng = 0x123456789abcdef0ull;
        hv_flip_bits(&noisy, 0.0005, &rng);
        check(&score, rv_solve(&noisy, &problem, &result) == RV_CONVERGED &&
                        result.residual_distance < 200u,
              "low controlled vector noise remains bounded without false certainty");
    }

    {
        hv_t duplicate[2];
        uint16_t duplicate_ids[2] = { 700u, 701u };
        rv_codebook_t duplicate_book;
        rv_problem_t one_factor;
        hv_t one_product;
        rv_result_t duplicate_result;
        duplicate[0] = code[4];
        duplicate[1] = code[4];
        duplicate_book.vectors = duplicate;
        duplicate_book.symbol_id = duplicate_ids;
        duplicate_book.count = 2u;
        one_factor.factor_count = 1u;
        one_factor.codebook = &duplicate_book;
        one_factor.offsets[0] = 0;
        one_factor.max_iterations = 4u;
        one_product = code[4];
        check(&score, rv_solve(&one_product, &one_factor, &duplicate_result) ==
                        RV_AMBIGUOUS && duplicate_result.ambiguous_mask == 1u,
              "identical codebook entries produce explicit ambiguity");
    }

    {
        rv_problem_t limited = problem;
                limited.max_iterations = 1u;
        limited.max_exact_nodes = 1u;
        {
            rv_status_t limited_status = rv_solve(&product, &limited, &result);
            check(&score, limited_status == RV_E_LIMIT ||
                            limited_status == RV_CONVERGED,
                    "iteration budget never becomes an unbounded search");
        }

    }

    printf("RESONATOR: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
