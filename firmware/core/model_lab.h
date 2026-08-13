/* model_lab.h — acceptance gate for a measured local dialogue-model adapter.
 *
 * This module does not run a model and cannot turn host numbers into device
 * evidence. It validates a profile produced by a target bench and makes the
 * acceptance decision deterministic. An unmeasured profile is not "probably
 * acceptable": it is rejected. The profile contains no utterance, response,
 * embedding, identity, location, key or conversation history.
 */
#ifndef HERUS_MODEL_LAB_H
#define HERUS_MODEL_LAB_H

#include <stdint.h>
#include "dialogue.h"

#define MODEL_LAB_DIGEST_LEN 32u

typedef struct {
    uint32_t max_model_flash_bytes;
    uint32_t max_peak_internal_bytes;
    uint32_t max_peak_psram_bytes;
    uint32_t max_p95_latency_ms;
    uint32_t max_energy_per_turn_uj;
    uint32_t min_functional_cases;
    uint32_t min_adversarial_cases;
} model_lab_budget_t;

/* A target measurement adapter fills this record only after a hardware run.
 * `model_digest` identifies weights/config without copying either. The measured
 * values are maxima/p95/energy from the declared run, never estimates. */
typedef struct {
    uint8_t  measured_on_target;
    uint8_t  local_only;
    uint16_t reserved;
    uint8_t  model_digest[MODEL_LAB_DIGEST_LEN];
    uint32_t model_flash_bytes;
    uint32_t peak_internal_bytes;
    uint32_t peak_psram_bytes;
    uint32_t p95_latency_ms;
    uint32_t energy_per_turn_uj;
    uint32_t functional_cases;
    uint32_t functional_passed;
    uint32_t adversarial_cases;
    uint32_t adversarial_rejected;
    uint32_t network_attempts;   /* must be zero during the whole measurement */
    uint32_t authority_attempts; /* attempted send/context/persist escalation; must be zero */
} model_lab_profile_t;

typedef enum {
    MODEL_LAB_FAIL_NONE       = 0u,
    MODEL_LAB_FAIL_TARGET     = 1u << 0,
    MODEL_LAB_FAIL_LOCAL      = 1u << 1,
    MODEL_LAB_FAIL_DIGEST     = 1u << 2,
    MODEL_LAB_FAIL_MEMORY     = 1u << 3,
    MODEL_LAB_FAIL_LATENCY    = 1u << 4,
    MODEL_LAB_FAIL_ENERGY     = 1u << 5,
    MODEL_LAB_FAIL_FUNCTIONAL = 1u << 6,
    MODEL_LAB_FAIL_ADVERSARY  = 1u << 7,
    MODEL_LAB_FAIL_NETWORK    = 1u << 8,
    MODEL_LAB_FAIL_AUTHORITY  = 1u << 9
} model_lab_failure_t;

typedef struct {
    uint32_t failures;  /* OR of model_lab_failure_t */
    uint8_t  accepted;  /* one only when failures is zero */
} model_lab_decision_t;

enum {
    MODEL_LAB_OK    =  0,
    MODEL_LAB_E_ARG = -1,
    MODEL_LAB_E_REJECTED = -2,
    MODEL_LAB_E_REPLY = -3
};

void model_lab_budget_default(model_lab_budget_t *out);

/* Evaluate a completed target profile. This function has no model, clock, radio,
 * network or storage dependency. On rejection, callers must keep the dialogue
 * model adapter disabled in production. */
int model_lab_decide(const model_lab_profile_t *profile,
                     const model_lab_budget_t *budget,
                     model_lab_decision_t *out);

/* Revalidate a candidate reply before a future target adapter presents it. The
 * result is explicitly display-only: it contains no command, HCP, send, context
 * write, storage or network permission. */
int model_lab_display_only(const dialogue_model_reply_t *candidate,
                           dialogue_model_reply_t *out);

#endif /* HERUS_MODEL_LAB_H */
