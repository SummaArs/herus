/* test_model_lab.c — executable contract for Advance 9 model acceptance. */
#include "model_lab.h"
#include <stdio.h>
#include <string.h>

static int FAILED = 0;
static void ok(int cond, const char *what)
{
    printf("  %-4s %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond) FAILED = 1;
}

static int zeroed(const void *p, size_t n)
{
    const uint8_t *b = p;
    uint8_t any = 0;
    for (size_t i = 0; i < n; i++) any |= b[i];
    return any == 0;
}

static void accepted_profile(model_lab_profile_t *p)
{
    memset(p, 0, sizeof(*p));
    p->measured_on_target = 1;
    p->local_only = 1;
    for (unsigned i = 0; i < sizeof(p->model_digest); i++)
        p->model_digest[i] = (uint8_t)(i + 1u);
    p->model_flash_bytes = 900u;
    p->peak_internal_bytes = 90u;
    p->peak_psram_bytes = 190u;
    p->p95_latency_ms = 49u;
    p->energy_per_turn_uj = 99u;
    p->functional_cases = 3u;
    p->functional_passed = 3u;
    p->adversarial_cases = 4u;
    p->adversarial_rejected = 4u;
}

static void accepted_budget(model_lab_budget_t *b)
{
    memset(b, 0, sizeof(*b));
    b->max_model_flash_bytes = 1000u;
    b->max_peak_internal_bytes = 100u;
    b->max_peak_psram_bytes = 200u;
    b->max_p95_latency_ms = 50u;
    b->max_energy_per_turn_uj = 100u;
    b->min_functional_cases = 3u;
    b->min_adversarial_cases = 4u;
}

int main(void)
{
    model_lab_profile_t profile;
    model_lab_budget_t budget;
    model_lab_decision_t decision;
    dialogue_model_reply_t candidate, displayed;

    printf("\n== M1  measured local-model acceptance gate ==\n");
    accepted_profile(&profile);
    accepted_budget(&budget);
    ok(model_lab_decide(&profile, &budget, &decision) == MODEL_LAB_OK &&
       decision.accepted && decision.failures == MODEL_LAB_FAIL_NONE,
       "M1 only a complete target-measured, local-only profile inside every budget is accepted");

    model_lab_budget_default(&budget);
    ok(model_lab_decide(&profile, &budget, &decision) == MODEL_LAB_E_REJECTED &&
       !decision.accepted && (decision.failures & MODEL_LAB_FAIL_MEMORY) &&
       (decision.failures & MODEL_LAB_FAIL_LATENCY) && (decision.failures & MODEL_LAB_FAIL_ENERGY),
       "M1 an unspecified budget denies release instead of silently accepting a model");

    accepted_budget(&budget);
    profile.measured_on_target = 0;
    profile.local_only = 0;
    memset(profile.model_digest, 0, sizeof(profile.model_digest));
    ok(model_lab_decide(&profile, &budget, &decision) == MODEL_LAB_E_REJECTED &&
       (decision.failures & MODEL_LAB_FAIL_TARGET) && (decision.failures & MODEL_LAB_FAIL_LOCAL) &&
       (decision.failures & MODEL_LAB_FAIL_DIGEST),
       "M1 host-only, connected or unidentified weights cannot enter production");

    accepted_profile(&profile);
    profile.model_flash_bytes = 1001u;
    profile.peak_internal_bytes = 101u;
    profile.peak_psram_bytes = 201u;
    profile.p95_latency_ms = 51u;
    profile.energy_per_turn_uj = 101u;
    ok(model_lab_decide(&profile, &budget, &decision) == MODEL_LAB_E_REJECTED &&
       (decision.failures & MODEL_LAB_FAIL_MEMORY) && (decision.failures & MODEL_LAB_FAIL_LATENCY) &&
       (decision.failures & MODEL_LAB_FAIL_ENERGY),
       "M1 one byte, millisecond or microjoule above a declared budget fails acceptance");

    accepted_profile(&profile);
    profile.functional_passed = 2u;
    profile.adversarial_rejected = 3u;
    profile.network_attempts = 1u;
    profile.authority_attempts = 1u;
    ok(model_lab_decide(&profile, &budget, &decision) == MODEL_LAB_E_REJECTED &&
       (decision.failures & MODEL_LAB_FAIL_FUNCTIONAL) &&
       (decision.failures & MODEL_LAB_FAIL_ADVERSARY) &&
       (decision.failures & MODEL_LAB_FAIL_NETWORK) &&
       (decision.failures & MODEL_LAB_FAIL_AUTHORITY),
       "M1 incomplete evaluation, a network attempt or authority escalation fails closed");

    memset(&candidate, 0, sizeof(candidate));
    strcpy(candidate.reply, "enviar ajuda agora");
    candidate.reply_len = (uint8_t)strlen(candidate.reply);
    candidate.topic = DIALOGUE_TOPIC_SAFETY;
    ok(model_lab_display_only(&candidate, &displayed) == MODEL_LAB_OK &&
       !strcmp(displayed.reply, "enviar ajuda agora") && displayed.topic == DIALOGUE_TOPIC_SAFETY,
       "M1 action-looking output remains an explicitly display-only reply");

    candidate.topic = DIALOGUE_TOPIC_NONE;
    memset(&displayed, 0xA5, sizeof(displayed));
    ok(model_lab_display_only(&candidate, &displayed) == MODEL_LAB_E_REPLY &&
       zeroed(&displayed, sizeof(displayed)),
       "M1 malformed reply is rejected and its destination is zeroized");

    if (FAILED) {
        printf("MODEL LAB TESTS FAILED\n");
        return 1;
    }
    printf("MODEL LAB INVARIANTS HOLD — only measured, local, bounded and adversarially clean models may be accepted.\n");
    return 0;
}
