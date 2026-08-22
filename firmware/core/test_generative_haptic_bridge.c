#include "generative_haptic_bridge.h"

#include <stdio.h>
#include <string.h>

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static hl_profile_t profile(void)
{
    hl_profile_t result;
    uint8_t i;
    memset(&result, 0, sizeof(result));
    result.version = HL_VERSION_1;
    result.actuator = HL_ACTUATOR_LRA;
    result.profile_version = 1u;
    result.effect_sync = 1u;
    result.effect_mark = 2u;
    result.effect_end = 3u;
    for (i = 0u; i < HL_CODEBOOK_SIZE; ++i) result.effect_code[i] = (uint8_t)(10u + i);
    return result;
}

static int round_trip(const gc_result_t *result, const hl_profile_t *profile_value,
                      gh_signal_t *signal)
{
    hl_encoded_t encoded;
    hl_event_t decoded;
    if (gh_from_result(result, signal) != HL_OK || signal->actionable != 0u)
        return 0;
    if (hl_encode(&signal->event, profile_value, &encoded) != HL_OK) return 0;
    if (hl_decode_with_profile(&encoded, profile_value, &decoded) != HL_OK) return 0;
    return memcmp(&signal->event, &decoded, sizeof(decoded)) == 0;
}

int main(void)
{
    score_t score = { 0, 0 };
    hl_profile_t profile_value = profile();
    gh_signal_t signal;
    gc_result_t result;

    memset(&result, 0, sizeof(result));
    result.status = GC_STATUS_OK;
    result.kind = GC_KIND_DIRECT;
    result.authority = GC_AUTH_PRESENTATION_ONLY;
    check(&score, round_trip(&result, &profile_value, &signal) &&
                    signal.event.scope == HL_SCOPE_COM &&
                    signal.event.class_code == HL_CLASS_ACK &&
                    signal.event.state == HL_STATE_CONFIRMED,
          "direct local generation becomes a confirmed communication ACK");

    memset(&result, 0, sizeof(result));
    result.status = GC_STATUS_OK;
    result.kind = GC_KIND_DERIVED;
    result.grounded = 1u;
    result.authority = GC_AUTH_PRESENTATION_ONLY;
    check(&score, round_trip(&result, &profile_value, &signal) &&
                    signal.event.scope == HL_SCOPE_MEM &&
                    signal.event.class_code == HL_CLASS_ACK,
          "grounded generation is visibly marked as memory scope");

    memset(&result, 0, sizeof(result));
    result.status = GC_STATUS_OK;
    result.kind = GC_KIND_PLAN;
    result.requires_confirmation = 1u;
    result.authority = GC_AUTH_CONFIRMATION_REQUIRED;
    check(&score, round_trip(&result, &profile_value, &signal) &&
                    signal.event.scope == HL_SCOPE_PLAN &&
                    signal.event.class_code == HL_CLASS_QUERY &&
                    signal.event.state == HL_STATE_PENDING &&
                    signal.confirmation_required == 1u,
          "a generated plan becomes a pending haptic query requiring confirmation");

    memset(&result, 0, sizeof(result));
    result.status = GC_STATUS_OK;
    result.kind = GC_KIND_COUNTERFACTUAL;
    result.authority = GC_AUTH_PRESENTATION_ONLY;
    check(&score, round_trip(&result, &profile_value, &signal) &&
                    signal.event.class_code == HL_CLASS_QUERY &&
                    signal.event.state == HL_STATE_CONFIRMED,
          "a counterfactual is presented as a query, not a command");

    memset(&result, 0, sizeof(result));
    result.status = GC_STATUS_ABSTAIN;
    result.kind = GC_KIND_UNKNOWN;
    result.abstain_reason = GC_ABSTAIN_NO_EVIDENCE;
    check(&score, round_trip(&result, &profile_value, &signal) &&
                    signal.event.scope == HL_SCOPE_COM &&
                    signal.event.class_code == HL_CLASS_ERROR &&
                    signal.event.state == HL_STATE_UNKNOWN && signal.abstained == 1u,
          "unknown generation becomes an explicit haptic abstention");

    memset(&result, 0, sizeof(result));
    result.status = GC_STATUS_ABSTAIN;
    result.kind = GC_KIND_CONTRADICTED;
    result.abstain_reason = GC_ABSTAIN_CONFLICT;
    check(&score, round_trip(&result, &profile_value, &signal) &&
                    signal.event.class_code == HL_CLASS_ALERT &&
                    signal.event.state == HL_STATE_UNKNOWN && signal.abstained == 1u,
          "conflicting generation becomes an alert without selecting a side");

    memset(&result, 0, sizeof(result));
    result.status = GC_STATUS_ABSTAIN;
    result.kind = GC_KIND_POLICY_BLOCKED;
    result.abstain_reason = GC_ABSTAIN_POLICY;
    check(&score, round_trip(&result, &profile_value, &signal) &&
                    signal.event.scope == HL_SCOPE_SFTY &&
                    signal.event.class_code == HL_CLASS_PRIVACY &&
                    signal.event.state == HL_STATE_DENIED && signal.abstained == 1u,
          "policy blocking becomes a privacy denial and never an actionable signal");

    memset(&result, 0, sizeof(result));
    result.status = GC_STATUS_LIMIT;
    result.kind = GC_KIND_LIMIT;
    result.abstain_reason = GC_ABSTAIN_BUDGET;
    check(&score, round_trip(&result, &profile_value, &signal) &&
                    signal.event.state == HL_STATE_UNKNOWN && signal.abstained == 1u,
          "budget exhaustion remains visible as unknown instead of stale success");

    printf("GEN HAPTIC: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail == 0 ? 0 : 1;
}
