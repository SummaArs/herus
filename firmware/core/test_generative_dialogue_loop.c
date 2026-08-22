#include "generative_dialogue_loop.h"

#include <stdio.h>
#include <string.h>

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static int zeroed(const void *value, size_t length)
{
    const unsigned char *bytes = value;
    unsigned char any = 0u;
    size_t i;
    for (i = 0u; i < length; ++i) any |= bytes[i];
    return any == 0u;
}

static gc_result_t candidate(gc_status_t status, gc_kind_t kind,
                             uint8_t requires_confirmation)
{
    gc_result_t result;
    memset(&result, 0, sizeof(result));
    result.status = status;
    result.kind = kind;
    result.authority = requires_confirmation != 0u ?
                       GC_AUTH_CONFIRMATION_REQUIRED : GC_AUTH_PRESENTATION_ONLY;
    result.requires_confirmation = requires_confirmation;
    result.response_length = 5u;
    memcpy(result.response, "local", 5u);
    return result;
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

int main(void)
{
    score_t score = { 0, 0 };
    gdl_t loop;
    gdl_config_t config = { 100u, 40u };
    gc_result_t direct = candidate(GC_STATUS_OK, GC_KIND_DIRECT, 0u);
    gc_result_t plan = candidate(GC_STATUS_OK, GC_KIND_PLAN, 1u);
    gc_result_t unknown = candidate(GC_STATUS_ABSTAIN, GC_KIND_UNKNOWN, 0u);
    gc_result_t invalid = candidate(GC_E_OUTPUT, GC_KIND_NONE, 0u);
    hl_profile_t haptic_profile = profile();
    hl_encoded_t encoded;
    hl_event_t decoded;

    gdl_init(&loop, &config);
    check(&score, loop.state == GDL_IDLE && gdl_metrics(&loop)->turns_started == 0u,
          "loop starts idle with zeroed metrics");
    check(&score, gdl_begin(&loop, 0u, 0u) == GDL_E_ARG && loop.state == GDL_IDLE,
          "zero physical session cannot begin a turn");
    check(&score, gdl_begin(&loop, 7u, 10u) == GDL_OK &&
                    loop.state == GDL_GENERATING && loop.physical_session_id == 7u,
          "a validated nonzero physical session starts generation");
    check(&score, gdl_present(&loop, &direct, 20u) == GDL_OK &&
                    loop.state == GDL_PRESENTED && loop.signal.actionable == 0u,
          "a direct candidate is presented without execution authority");
    check(&score, hl_encode(&loop.signal.event, &haptic_profile, &encoded) == HL_OK &&
                    hl_decode_with_profile(&encoded, &haptic_profile, &decoded) == HL_OK &&
                    memcmp(&loop.signal.event, &decoded, sizeof(decoded)) == 0,
          "presented candidate survives the HAP-SEM encode/decode round-trip");
    check(&score, gdl_confirm(&loop, 7u, 21u) == GDL_E_STATE,
          "a non-pending answer cannot be confirmed as an action");
    check(&score, gdl_abort(&loop) == GDL_OK && loop.state == GDL_ABORTED &&
                    zeroed(&loop.candidate, sizeof(loop.candidate)) &&
                    zeroed(&loop.signal, sizeof(loop.signal)) &&
                    loop.physical_session_id == 0u,
          "interruption clears candidate, signal and physical session");

    check(&score, gdl_begin(&loop, 8u, 30u) == GDL_OK &&
                    gdl_present(&loop, &plan, 35u) == GDL_OK &&
                    loop.state == GDL_CONFIRMATION_PENDING &&
                    loop.signal.confirmation_required == 1u,
          "a plan enters a pending confirmation state");
    check(&score, gdl_confirm(&loop, 99u, 36u) == GDL_E_PHYSICAL &&
                    loop.state == GDL_CONFIRMATION_PENDING,
          "a different physical session cannot confirm the plan");
    check(&score, gdl_confirm(&loop, 8u, 37u) == GDL_OK &&
                    loop.state == GDL_CONFIRMED && loop.physical_session_id == 0u &&
                    gdl_metrics(&loop)->confirmations == 1u,
          "the matching physical session confirms exactly once");
    check(&score, gdl_confirm(&loop, 8u, 38u) == GDL_E_STATE,
          "confirmation replay after one-shot completion is rejected");
    check(&score, gdl_forget(&loop) == GDL_OK && loop.state == GDL_CLEARED &&
                    zeroed(&loop.candidate, sizeof(loop.candidate)) &&
                    zeroed(&loop.signal, sizeof(loop.signal)),
          "privacy forget clears a confirmed candidate and haptic signal");

    check(&score, gdl_begin(&loop, 9u, 50u) == GDL_OK &&
                    gdl_tick(&loop, 149u) == GDL_OK && loop.state == GDL_GENERATING,
          "a generating turn remains live below its timeout");
    check(&score, gdl_tick(&loop, 150u) == GDL_E_TIMEOUT &&
                    loop.state == GDL_TIMED_OUT &&
                    zeroed(&loop.candidate, sizeof(loop.candidate)) &&
                    gdl_metrics(&loop)->timed_out == 1u,
          "generation timeout clears all pending state");
    check(&score, gdl_begin(&loop, 10u, 200u) == GDL_OK &&
                    gdl_present(&loop, &plan, 210u) == GDL_OK &&
                    gdl_tick(&loop, 249u) == GDL_OK &&
                    loop.state == GDL_CONFIRMATION_PENDING,
          "confirmation timer is independent from generation timer");
    check(&score, gdl_tick(&loop, 250u) == GDL_E_TIMEOUT &&
                    loop.state == GDL_TIMED_OUT && loop.physical_session_id == 0u,
          "confirmation timeout cannot leave a plan armed");

    check(&score, gdl_begin(&loop, 11u, 300u) == GDL_OK &&
                    gdl_present(&loop, &plan, 301u) == GDL_OK &&
                    gdl_deny(&loop, 11u, 302u) == GDL_OK &&
                    loop.state == GDL_ABORTED &&
                    gdl_metrics(&loop)->confirmation_denied == 1u,
          "explicit denial aborts a pending plan and records only a counter");
    check(&score, gdl_begin(&loop, 12u, 400u) == GDL_OK &&
                    gdl_present(&loop, &unknown, 401u) == GDL_OK &&
                    loop.state == GDL_PRESENTED && loop.signal.abstained == 1u &&
                    loop.signal.actionable == 0u,
          "an abstaining candidate remains presentable but never actionable");
    check(&score, gdl_forget(&loop) == GDL_OK && gdl_metrics(&loop)->privacy_clears == 2u,
          "forget remains idempotent at the lifecycle boundary");
    check(&score, gdl_begin(&loop, 13u, 500u) == GDL_OK &&
                    gdl_present(&loop, &invalid, 501u) == GDL_E_GENERATION &&
                    loop.state == GDL_ABORTED &&
                    zeroed(&loop.candidate, sizeof(loop.candidate)),
          "invalid generation output aborts without retaining a candidate");
    check(&score, gdl_begin(&loop, 14u, 600u) == GDL_OK &&
                    gdl_present(&loop, &direct, 601u) == GDL_OK &&
                    gdl_forget(&loop) == GDL_OK && loop.state == GDL_CLEARED &&
                    gdl_metrics(&loop)->interrupted == 1u,
          "a later turn can restart only after the prior payload is cleared");
    check(&score, gdl_confirm(&loop, 14u, 602u) == GDL_E_STATE,
          "cleared turns cannot be confirmed through a stale physical session");

    printf("GEN DIALOGUE LOOP: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail == 0 ? 0 : 1;
}
