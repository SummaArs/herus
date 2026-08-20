#include "resonator_bridge.h"
#include <string.h>

rb_status_t rb_propose_relation(const hv_t *product,
                                const rv_problem_t *problem,
                                uint8_t negated,
                                uint16_t min_margin_q8,
                                uint16_t max_residual,
                                rb_proposal_t *out)
{
    rv_status_t solved;
    if (!product || !problem || !out || negated > 1u) return RB_E_ARG;
    memset(out, 0, sizeof(*out));
    solved = rv_solve(product, problem, &out->provenance);
    if (solved == RV_E_LIMIT) {
        out->status = RB_E_LIMIT;
        return RB_E_LIMIT;
    }
    if (solved != RV_CONVERGED || out->provenance.ambiguous_mask != 0u ||
        out->provenance.residual_distance > max_residual ||
        out->provenance.margin_q8[0] < (int16_t)min_margin_q8 ||
        out->provenance.margin_q8[1] < (int16_t)min_margin_q8 ||
        out->provenance.margin_q8[2] < (int16_t)min_margin_q8) {
        out->status = RB_ABSTAIN;
        return RB_ABSTAIN;
    }
    out->fact.subject = out->provenance.symbol_id[0];
    out->fact.predicate = out->provenance.symbol_id[1];
    out->fact.object = out->provenance.symbol_id[2];
    out->fact.negated = negated;
    out->status = RB_PROPOSED;
    /* This bit is intentionally zero. Only an outer, human-gated layer may
     * accept a proposal into a reasoner or memory. */
    out->explicitly_accepted = 0u;
    return RB_PROPOSED;
}
