/*
 * HERUS resonator — bounded VSA factorization and relational cleanup.
 *
 * This module reuses the existing dense binary VSA. A product is formed as:
 *     P = R(o0)f0 XOR R(o1)f1 XOR ... XOR R(on)fn
 * The solver alternates residue computation and codebook cleanup. It generates
 * candidate factors but has no memory, I/O, radio or execution authority.
 *
 * A converged result is evidence about a bounded codebook, not a language-model
 * answer and not proof of correctness outside that codebook.
 */
#ifndef HERUS_RESONATOR_H
#define HERUS_RESONATOR_H

#include "hv.h"
#include <stdint.h>

#define RV_MAX_FACTORS 4u
#define RV_MAX_ITERS   32u
#define RV_MAX_EXACT_NODES 65536u

typedef struct {
    const hv_t *vectors;
    const uint16_t *symbol_id;
    uint16_t count;
} rv_codebook_t;

typedef struct {
    uint8_t factor_count;
    const rv_codebook_t *codebook;
    int offsets[RV_MAX_FACTORS];
    unsigned max_iterations;
    uint32_t max_exact_nodes;
} rv_problem_t;

typedef enum {
    RV_CONVERGED = 0,
    RV_AMBIGUOUS = 1,
    RV_NO_CONVERGENCE = 2,
    RV_E_ARG = -1,
    RV_E_LIMIT = -2,
    RV_E_FORMAT = -3
} rv_status_t;

typedef struct {
    rv_status_t status;
    uint8_t factor_count;
    uint8_t iterations;
    uint8_t ambiguous_mask;
    uint16_t symbol_id[RV_MAX_FACTORS];
    int16_t similarity_q8[RV_MAX_FACTORS];
    int16_t margin_q8[RV_MAX_FACTORS];
    uint16_t residual_distance;
    uint8_t used_exact_fallback;
    uint32_t search_nodes;
} rv_result_t;

/* Form a rotated XOR product from known factors. */
int rv_compose(hv_t *product, const hv_t *factors,
               const int *offsets, uint8_t factor_count);

/* Factor a product with alternating residue cleanup over bounded codebooks.
 * If cleanup reaches a local minimum, factor_count <= 3 may use a bounded exact
 * residual scan. The result reports that fallback instead of hiding it. */
rv_status_t rv_solve(const hv_t *product, const rv_problem_t *problem,
                     rv_result_t *out);

#endif /* HERUS_RESONATOR_H */
