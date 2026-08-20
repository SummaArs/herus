/*
 * VSA-to-symbolic bridge.
 *
 * A resonator result is only a candidate relation. This bridge applies explicit
 * quality gates and emits a proposal; it never mutates a reasoner, memory or
 * transport. The caller may add the proposal as a derived fact only after its
 * own authority policy accepts the provenance.
 */
#ifndef HERUS_RESONATOR_BRIDGE_H
#define HERUS_RESONATOR_BRIDGE_H

#include "resonator.h"
#include "symbolic_reasoner.h"

#define RB_DEFAULT_MIN_MARGIN_Q8  8u
#define RB_DEFAULT_MAX_RESIDUAL  64u

typedef enum {
    RB_PROPOSED = 0,
    RB_ABSTAIN = 1,
    RB_E_ARG = -1,
    RB_E_LIMIT = -2
} rb_status_t;

typedef struct {
    rb_status_t status;
    sr_fact_t fact;
    rv_result_t provenance;
    uint8_t explicitly_accepted;
} rb_proposal_t;

rb_status_t rb_propose_relation(const hv_t *product,
                                const rv_problem_t *problem,
                                uint8_t negated,
                                uint16_t min_margin_q8,
                                uint16_t max_residual,
                                rb_proposal_t *out);

#endif /* HERUS_RESONATOR_BRIDGE_H */
