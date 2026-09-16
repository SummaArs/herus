#include "sim.h"
#include <assert.h>
#include <stdio.h>

static sim_host_budget_t host(uint16_t bytes, uint8_t steps, uint8_t authority_none) {
    sim_host_budget_t h = {bytes, steps, authority_none, 1u, 0u, 0u};
    return h;
}

static void assert_h0_h1_valid_case(void) {
    sim_pattern_t p = {{4, 0, 0, 0}, 1u};
    sim_host_budget_t h = host(4096u, 12u, 1u);
    sim_decision_t d = sim_decide(&h, &p, 4096u, 12u, 700u);
    /* H0 golden vector: ARRIVE, SIM-INT8, confidence 1000, PROPOSE, ABSTAIN. */
    assert(d.label == SIM_LABEL_ARRIVE);
    assert(d.representation == SIM_REP_INT8);
    assert(d.confidence_milli == 1000u);
    assert(d.proposal == 1u);
    assert(d.execution == 0u);
}

static void assert_h0_h1_missing_provenance(void) {
    sim_pattern_t p = {{4, 0, 0, 0}, 0u};
    sim_host_budget_t h = host(4096u, 12u, 1u);
    sim_decision_t d = sim_decide(&h, &p, 4096u, 12u, 700u);
    /* H0 keeps representation/confidence but removes the unproven label. */
    assert(d.label == SIM_LABEL_UNKNOWN);
    assert(d.representation == SIM_REP_INT8);
    assert(d.confidence_milli == 1000u);
    assert(d.proposal == 0u);
    assert(d.execution == 0u);
}

static void assert_h0_h1_budget_exhausted(void) {
    sim_pattern_t p = {{4, 0, 0, 0}, 1u};
    sim_host_budget_t h = host(128u, 1u, 1u);
    sim_decision_t d = sim_decide(&h, &p, 4096u, 12u, 700u);
    assert(d.label == SIM_LABEL_UNKNOWN);
    assert(d.representation == SIM_REP_NONE);
    assert(d.confidence_milli == 0u);
    assert(d.proposal == 0u);
    assert(d.execution == 0u);
}

static void assert_h0_h1_authority_fail_closed(void) {
    sim_pattern_t p = {{4, 0, 0, 0}, 1u};
    sim_host_budget_t h = host(4096u, 12u, 0u);
    sim_decision_t d = sim_decide(&h, &p, 4096u, 12u, 700u);
    assert(d.label == SIM_LABEL_UNKNOWN);
    assert(d.representation == SIM_REP_NONE);
    assert(d.confidence_milli == 0u);
    assert(d.proposal == 0u);
    assert(d.execution == 0u);
}

int main(void) {
    assert_h0_h1_valid_case();
    assert_h0_h1_missing_provenance();
    assert_h0_h1_budget_exhausted();
    assert_h0_h1_authority_fail_closed();
    puts("SIM H0/H1 EQUIVALENCE PASS");
    return 0;
}
