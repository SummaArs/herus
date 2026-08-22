#include "haptic_semantic_bridge.h"

#include <stdio.h>
#include <string.h>

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static sc_unit_t unit_fixture(sc_unit_kind_t kind)
{
    sc_unit_t unit;
    memset(&unit, 0, sizeof(unit));
    unit.status = SC_OK;
    unit.kind = kind;
    unit.exact_parse = 1u;
    unit.requires_confirmation = kind == SC_UNIT_GOAL ||
                                 kind == SC_UNIT_FACT ||
                                 kind == SC_UNIT_RULE;
    return unit;
}

int main(void)
{
    score_t score = { 0, 0 };
    hs_signal_t signal;
    sc_bridge_result_t bridge;
    sc_unit_t unit;
    int result;

    unit = unit_fixture(SC_UNIT_FACT);
    memset(&bridge, 0, sizeof(bridge));
    bridge.status = SC_BRIDGE_E_AUTH;
    result = hs_from_compiler(&unit, &bridge, &signal);
    check(&score, result == HL_OK && signal.event.scope == HL_SCOPE_MEM &&
                    signal.event.state == HL_STATE_PENDING &&
                    signal.confirmation_required == 1u && signal.actionable == 0u,
          "unconfirmed fact becomes pending memory presentation only");

    bridge.status = SC_BRIDGE_OK;
    bridge.state_changed = 1u;
    result = hs_from_compiler(&unit, &bridge, &signal);
    check(&score, result == HL_OK && signal.event.class_code == HL_CLASS_ACK &&
                    signal.event.state == HL_STATE_CONFIRMED &&
                    signal.confirmation_required == 0u && signal.actionable == 0u,
          "confirmed fact becomes ACK without granting execution");

    unit = unit_fixture(SC_UNIT_QUERY);
    memset(&bridge, 0, sizeof(bridge));
    bridge.status = SC_BRIDGE_OK;
    result = hs_from_compiler(&unit, &bridge, &signal);
    check(&score, result == HL_OK && signal.event.class_code == HL_CLASS_QUERY &&
                    signal.event.state == HL_STATE_CONFIRMED &&
                    signal.actionable == 0u,
          "read-only query is presented without mutation authority");

    bridge.status = SC_BRIDGE_E_LIMIT;
    result = hs_from_compiler(&unit, &bridge, &signal);
    check(&score, result == HL_OK && signal.event.scope == HL_SCOPE_MEM &&
                    signal.event.state == HL_STATE_UNKNOWN && signal.abstained == 1u,
          "query limit becomes explicit unknown/abstention");

    unit = unit_fixture(SC_UNIT_GOAL);
    memset(&bridge, 0, sizeof(bridge));
    bridge.status = SC_BRIDGE_OK;
    bridge.plan.plan_length = 1u;
    result = hs_from_compiler(&unit, &bridge, &signal);
    check(&score, result == HL_OK && signal.event.scope == HL_SCOPE_PLAN &&
                    signal.event.state == HL_STATE_PENDING &&
                    signal.confirmation_required == 1u && signal.actionable == 0u,
          "plan proposal is pending and never encoded as execution");

    bridge.status = SC_BRIDGE_E_NO_PLAN;
    bridge.plan.plan_length = 0u;
    result = hs_from_compiler(&unit, &bridge, &signal);
    check(&score, result == HL_OK && signal.event.state == HL_STATE_UNKNOWN &&
                    signal.abstained == 1u && signal.actionable == 0u,
          "no-plan result is explicit unknown rather than false success");

    unit = unit_fixture(SC_UNIT_REJECT);
    result = hs_from_compiler(&unit, NULL, &signal);
    check(&score, result == HL_OK && signal.event.scope == HL_SCOPE_MEM &&
                    signal.event.class_code == HL_CLASS_PRIVACY &&
                    signal.event.state == HL_STATE_DENIED && signal.actionable == 0u,
          "discard/reject is presented as privacy denial without persistence");

    unit = unit_fixture(SC_UNIT_FACT);
    unit.status = SC_E_SENSITIVE;
    unit.exact_parse = 0u;
    result = hs_from_compiler(&unit, NULL, &signal);
    check(&score, result == HL_OK && signal.event.class_code == HL_CLASS_PRIVACY &&
                    signal.event.state == HL_STATE_DENIED && signal.abstained == 1u,
          "sensitive compiler rejection becomes privacy abstention");

    unit = unit_fixture(SC_UNIT_FACT);
    unit.status = SC_E_COLLISION;
    unit.exact_parse = 0u;
    result = hs_from_compiler(&unit, NULL, &signal);
    check(&score, result == HL_OK && signal.event.class_code == HL_CLASS_ERROR &&
                    signal.event.state == HL_STATE_UNKNOWN && signal.abstained == 1u,
          "symbol collision becomes unknown, never a guessed presentation");

    printf("HAPTIC SEMANTIC BRIDGE: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
