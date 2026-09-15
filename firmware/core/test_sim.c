#include "sim.h"
#include <assert.h>
#include <stdio.h>

static sim_host_budget_t host(void) {
    sim_host_budget_t h = {8192u, 12u, 1u, 1u, 1u, 1u};
    return h;
}

int main(void) {
    sim_pattern_t pattern = {{4, 0, 0, 0}, 1u};
    sim_neural_scores_t scores;
    sim_decision_t decision;
    sim_host_budget_t h = host();
    sim_infer(&pattern, &scores);
    assert(scores.label == SIM_LABEL_ARRIVE);
    assert(scores.confidence_milli <= 1000u);
    decision = sim_decide(&h, &pattern, 4096u, 12u, 700u);
    assert(decision.representation == SIM_REP_INT8);
    assert(decision.proposal == 1u);
    assert(decision.execution == 0u);
    h.budget_bytes = 128u;
    decision = sim_decide(&h, &pattern, 4096u, 12u, 700u);
    assert(decision.representation == SIM_REP_NONE);
    assert(decision.proposal == 0u);
    h = host();
    h.authority_none = 0u;
    decision = sim_decide(&h, &pattern, 4096u, 12u, 700u);
    assert(decision.proposal == 0u);
    pattern.provenance_present = 0u;
    h.authority_none = 1u;
    decision = sim_decide(&h, &pattern, 4096u, 12u, 700u);
    assert(decision.proposal == 0u);
    puts("SIM C11 TESTS PASS");
    return 0;
}
