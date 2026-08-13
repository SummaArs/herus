/* model_lab.c — deterministic acceptance gate for local dialogue models. */
#include "model_lab.h"
#include "crypto.h"
#include <string.h>

static int nonzero(const uint8_t *p, unsigned n)
{
    uint8_t any = 0;
    if (!p) return 0;
    for (unsigned i = 0; i < n; i++) any |= p[i];
    return any != 0;
}

static int topic_valid(dialogue_topic_t topic)
{
    return topic > DIALOGUE_TOPIC_NONE && topic < DIALOGUE_TOPIC_COUNT;
}

void model_lab_budget_default(model_lab_budget_t *out)
{
    if (!out) return;
    /* Deliberately not a product budget. Zero maxima deny acceptance until the
     * hardware owner pre-commits limits for a particular Nucleus revision. */
    memset(out, 0, sizeof(*out));
}

int model_lab_decide(const model_lab_profile_t *profile,
                     const model_lab_budget_t *budget,
                     model_lab_decision_t *out)
{
    uint32_t failures = MODEL_LAB_FAIL_NONE;
    if (!profile || !budget || !out) return MODEL_LAB_E_ARG;
    if (profile->measured_on_target != 1u) failures |= MODEL_LAB_FAIL_TARGET;
    if (profile->local_only != 1u) failures |= MODEL_LAB_FAIL_LOCAL;
    if (!nonzero(profile->model_digest, sizeof(profile->model_digest)))
        failures |= MODEL_LAB_FAIL_DIGEST;
    if (!budget->max_model_flash_bytes || !budget->max_peak_internal_bytes ||
        !budget->max_peak_psram_bytes ||
        profile->model_flash_bytes > budget->max_model_flash_bytes ||
        profile->peak_internal_bytes > budget->max_peak_internal_bytes ||
        profile->peak_psram_bytes > budget->max_peak_psram_bytes)
        failures |= MODEL_LAB_FAIL_MEMORY;
    if (!budget->max_p95_latency_ms || profile->p95_latency_ms > budget->max_p95_latency_ms)
        failures |= MODEL_LAB_FAIL_LATENCY;
    if (!budget->max_energy_per_turn_uj ||
        profile->energy_per_turn_uj > budget->max_energy_per_turn_uj)
        failures |= MODEL_LAB_FAIL_ENERGY;
    if (!budget->min_functional_cases || profile->functional_cases < budget->min_functional_cases ||
        profile->functional_passed != profile->functional_cases)
        failures |= MODEL_LAB_FAIL_FUNCTIONAL;
    if (!budget->min_adversarial_cases || profile->adversarial_cases < budget->min_adversarial_cases ||
        profile->adversarial_rejected != profile->adversarial_cases)
        failures |= MODEL_LAB_FAIL_ADVERSARY;
    if (profile->network_attempts) failures |= MODEL_LAB_FAIL_NETWORK;
    if (profile->authority_attempts) failures |= MODEL_LAB_FAIL_AUTHORITY;
    out->failures = failures;
    out->accepted = failures == MODEL_LAB_FAIL_NONE;
    return out->accepted ? MODEL_LAB_OK : MODEL_LAB_E_REJECTED;
}

int model_lab_display_only(const dialogue_model_reply_t *candidate,
                           dialogue_model_reply_t *out)
{
    if (!candidate || !out) return MODEL_LAB_E_ARG;
    secure_zero(out, sizeof(*out));
    if (!topic_valid(candidate->topic) || !candidate->reply_len ||
        candidate->reply_len >= DIALOGUE_REPLY_MAX ||
        candidate->reply[candidate->reply_len] != '\0') return MODEL_LAB_E_REPLY;
    for (unsigned i = 0; i < candidate->reply_len; i++) {
        unsigned char c = (unsigned char)candidate->reply[i];
        if (c == 0 || c < 0x20u || c == 0x7fu) return MODEL_LAB_E_REPLY;
    }
    *out = *candidate;
    return MODEL_LAB_OK;
}
