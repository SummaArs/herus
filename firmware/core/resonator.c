#include "resonator.h"
#include <string.h>

static int valid_problem(const rv_problem_t *problem)
{
    if (!problem || !problem->codebook || problem->factor_count == 0u ||
        problem->factor_count > RV_MAX_FACTORS ||
        problem->max_iterations == 0u || problem->max_iterations > RV_MAX_ITERS)
        return 0;
    if (problem->codebook->count == 0u ||
        problem->codebook->vectors == NULL || problem->codebook->symbol_id == NULL)
        return 0;
    for (unsigned i = 0u; i < problem->factor_count; i++) {
        if (problem->offsets[i] >= HV_BITS || problem->offsets[i] <= -HV_BITS)
            return 0;
    }
    return 1;
}

static unsigned cleanup(const hv_t *target, const rv_codebook_t *codebook,
                        hv_t *out, int *best_dist, int *second_dist)
{
    unsigned best_index = 0u;
    *best_dist = HV_BITS + 1;
    *second_dist = HV_BITS + 1;
    for (unsigned i = 0u; i < codebook->count; i++) {
        int distance = hv_dist(target, &codebook->vectors[i]);
        if (distance < *best_dist) {
            *second_dist = *best_dist;
            *best_dist = distance;
            best_index = i;
        } else if (distance < *second_dist) {
            *second_dist = distance;
        }
    }
    *out = codebook->vectors[best_index];
    return best_index;
}

int rv_compose(hv_t *product, const hv_t *factors,
               const int *offsets, uint8_t factor_count)
{
    hv_t rotated;
    hv_t result;
    if (!product || !factors || !offsets || factor_count == 0u ||
        factor_count > RV_MAX_FACTORS) return RV_E_ARG;
    memset(&result, 0, sizeof(result));
    for (unsigned i = 0u; i < factor_count; i++) {
        if (offsets[i] >= HV_BITS || offsets[i] <= -HV_BITS)
            return RV_E_FORMAT;
        hv_rot(&rotated, &factors[i], offsets[i]);
        hv_bind(&result, &result, &rotated);
    }
    *product = result;
    return RV_CONVERGED;
}

typedef struct {
    const hv_t *product;
    const rv_problem_t *problem;
    uint32_t max_nodes;
    uint32_t nodes;
    uint16_t index[RV_MAX_FACTORS];
    uint16_t best_index[RV_MAX_FACTORS];
    uint16_t best_distance;
    uint32_t best_count;
    uint8_t limited;
} exact_ctx_t;

static void exact_eval(exact_ctx_t *ctx)
{
    hv_t reconstructed;
    hv_t rotated;
    int distance;
    if (ctx->limited) return;
    for (unsigned last = 0u; last < ctx->problem->codebook->count; last++) {
        if (ctx->nodes >= ctx->max_nodes) {
            ctx->limited = 1u;
            return;
        }
        ctx->nodes++;
        memset(&reconstructed, 0, sizeof(reconstructed));
        for (unsigned i = 0u; i + 1u < ctx->problem->factor_count; i++) {
            hv_rot(&rotated,
                   &ctx->problem->codebook->vectors[ctx->index[i]],
                   ctx->problem->offsets[i]);
            hv_bind(&reconstructed, &reconstructed, &rotated);
        }
        hv_rot(&rotated, &ctx->problem->codebook->vectors[last],
               ctx->problem->offsets[ctx->problem->factor_count - 1u]);
        hv_bind(&reconstructed, &reconstructed, &rotated);
        distance = hv_dist(&reconstructed, ctx->product);
        if ((uint16_t)distance < ctx->best_distance) {
            ctx->best_distance = (uint16_t)distance;
            ctx->best_count = 1u;
            ctx->index[ctx->problem->factor_count - 1u] = (uint16_t)last;
            memcpy(ctx->best_index, ctx->index,
                   ctx->problem->factor_count * sizeof(ctx->index[0]));
        } else if ((uint16_t)distance == ctx->best_distance) {
            if (ctx->best_count < UINT32_MAX) ctx->best_count++;
        }
    }
}

static void exact_search(exact_ctx_t *ctx, unsigned level)
{
    if (ctx->limited) return;
    if (level + 1u == ctx->problem->factor_count) {
        exact_eval(ctx);
        return;
    }
    for (unsigned i = 0u; i < ctx->problem->codebook->count; i++) {
        ctx->index[level] = (uint8_t)i;
        exact_search(ctx, level + 1u);
        if (ctx->limited) return;
    }
}

static void set_factor_metrics(const rv_codebook_t *book, unsigned selected,
                               int16_t *similarity, int16_t *margin)
{
    int best = HV_BITS + 1;
    int second = HV_BITS + 1;
    for (unsigned i = 0u; i < book->count; i++) {
        int distance = hv_dist(&book->vectors[selected], &book->vectors[i]);
        if (distance < best) {
            second = best;
            best = distance;
        } else if (distance < second) {
            second = distance;
        }
    }
    *similarity = (int16_t)(256 - (best * 256) / HV_BITS);
    *margin = (int16_t)((second - best) * 256 / HV_BITS);
}

static rv_status_t exact_fallback(const hv_t *product,
                                  const rv_problem_t *problem,
                                  rv_result_t *out)
{
    exact_ctx_t ctx;
    uint32_t budget = problem->max_exact_nodes;
    if (budget == 0u) budget = RV_MAX_EXACT_NODES;
    if (budget > RV_MAX_EXACT_NODES) budget = RV_MAX_EXACT_NODES;
    memset(&ctx, 0, sizeof(ctx));
    ctx.product = product;
    ctx.problem = problem;
    ctx.max_nodes = budget;
    ctx.best_distance = (uint16_t)(HV_BITS + 1);
    exact_search(&ctx, 0u);
    out->used_exact_fallback = 1u;
    out->search_nodes = ctx.nodes;
    out->residual_distance = ctx.best_distance;
    if (ctx.limited) {
        out->status = RV_E_LIMIT;
        return RV_E_LIMIT;
    }
    if (ctx.best_count == 0u) {
        out->status = RV_NO_CONVERGENCE;
        return RV_NO_CONVERGENCE;
    }
    for (unsigned i = 0u; i < problem->factor_count; i++) {
        unsigned selected = ctx.best_index[i];
        out->symbol_id[i] = problem->codebook->symbol_id[selected];
        set_factor_metrics(problem->codebook, selected,
                           &out->similarity_q8[i], &out->margin_q8[i]);
    }
    if (ctx.best_count > 1u) {
        out->ambiguous_mask = (uint8_t)((1u << problem->factor_count) - 1u);
        out->status = RV_AMBIGUOUS;
        return RV_AMBIGUOUS;
    }
    /* This threshold is deliberately conservative and visible in the API's
     * residual. It treats a few bit errors as a denoising case, never certainty. */
    if (ctx.best_distance > 64u) {
        out->status = RV_NO_CONVERGENCE;
        return RV_NO_CONVERGENCE;
    }
    out->status = RV_CONVERGED;
    return RV_CONVERGED;
}

rv_status_t rv_solve(const hv_t *product, const rv_problem_t *problem,
                     rv_result_t *out)
{
    hv_t estimate[RV_MAX_FACTORS];
    hv_t candidate;
    hv_t residue;
    int best_dist[RV_MAX_FACTORS];
    int second_dist[RV_MAX_FACTORS];
    uint8_t stable = 0u;
    uint8_t ambiguous = 0u;
    uint8_t iterations = 0u;
    if (!product || !out || !valid_problem(problem)) return RV_E_ARG;
    memset(out, 0, sizeof(*out));
    out->factor_count = problem->factor_count;
    for (unsigned i = 0u; i < problem->factor_count; i++) {
        estimate[i] = problem->codebook->vectors[0];
        out->similarity_q8[i] = -256;
        out->margin_q8[i] = 0;
    }

    for (iterations = 0u; iterations < problem->max_iterations; iterations++) {
        stable = 1u;
        ambiguous = 0u;
        for (unsigned factor = 0u; factor < problem->factor_count; factor++) {
            memset(&residue, 0, sizeof(residue));
            for (unsigned other = 0u; other < problem->factor_count; other++) {
                hv_t rotated;
                if (other == factor) continue;
                hv_rot(&rotated, &estimate[other], problem->offsets[other]);
                hv_bind(&residue, &residue, &rotated);
            }
            hv_bind(&candidate, product, &residue);
            hv_rot(&candidate, &candidate, -problem->offsets[factor]);
            {
                unsigned selected = cleanup(&candidate, problem->codebook,
                                             &estimate[factor],
                                             &best_dist[factor],
                                             &second_dist[factor]);
                if (best_dist[factor] != 0) stable = 0u;
                if (best_dist[factor] == second_dist[factor])
                    ambiguous |= (uint8_t)(1u << factor);
                out->symbol_id[factor] = problem->codebook->symbol_id[selected];
                out->similarity_q8[factor] = (int16_t)(256 -
                    (best_dist[factor] * 256) / HV_BITS);
                out->margin_q8[factor] = (int16_t)((second_dist[factor] -
                    best_dist[factor]) * 256 / HV_BITS);
            }
        }
        if (stable && ambiguous == 0u) break;
    }
    out->iterations = (uint8_t)(iterations +
        (iterations < problem->max_iterations ? 1u : 0u));
    out->ambiguous_mask = ambiguous;
    {
        hv_t reconstructed;
        rv_compose(&reconstructed, estimate, problem->offsets,
                   problem->factor_count);
        out->residual_distance = (uint16_t)hv_dist(&reconstructed, product);
    }
    if (stable && ambiguous == 0u) {
        out->status = RV_CONVERGED;
        return RV_CONVERGED;
    }
    if (problem->factor_count <= 3u && problem->max_exact_nodes != 0u)
        return exact_fallback(product, problem, out);
    if (ambiguous != 0u) {
        out->status = RV_AMBIGUOUS;
        return RV_AMBIGUOUS;
    }
    out->status = iterations >= problem->max_iterations ?
                  RV_E_LIMIT : RV_NO_CONVERGENCE;
    return out->status;
}
