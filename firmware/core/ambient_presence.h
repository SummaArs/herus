/*
 * HERUS ambient_presence — local, quiet and bounded presence policy.
 *
 * This layer does not listen, transcribe, store content, transmit, remember or
 * actuate. It receives already-authorized typed observations and decides only
 * whether a transient local micro-offer may be surfaced. Any durable memory,
 * transport or actuation remains behind its existing physical gates.
 */
#ifndef HERUS_AMBIENT_PRESENCE_H
#define HERUS_AMBIENT_PRESENCE_H

#include <stdint.h>

#define AP_MIN_CONFIDENCE_PCT 80u
#define AP_MIN_RELEVANCE_PCT  70u
#define AP_MIN_NOVELTY_PCT    40u
#define AP_MAX_RISK_PCT       20u
#define AP_MAX_OFFER_BUDGET   1u

typedef enum {
    AP_PRIVACY_ORDINARY = 1u,
    AP_PRIVACY_PERSONAL = 2u,
    AP_PRIVACY_SENSITIVE = 3u,
    AP_PRIVACY_THIRD_PARTY = 4u
} ap_privacy_class_t;

typedef enum {
    AP_CLASS_RECALL = 1u,
    AP_CLASS_REMINDER = 2u,
    AP_CLASS_GAP = 3u,
    AP_CLASS_CONNECTION = 4u
} ap_opportunity_class_t;

typedef enum {
    AP_QUIET = 0u,
    AP_HOLD = 1u,
    AP_OFFER = 2u,
    AP_ABSTAIN = 3u,
    AP_EXPIRED = 4u,
    AP_ACKNOWLEDGED = 5u
} ap_status_t;

typedef enum {
    AP_REASON_NONE = 0u,
    AP_REASON_NO_ATTENTION = 1u << 0,
    AP_REASON_NO_CONSENT = 1u << 1,
    AP_REASON_LOW_CONFIDENCE = 1u << 2,
    AP_REASON_LOW_RELEVANCE = 1u << 3,
    AP_REASON_LOW_NOVELTY = 1u << 4,
    AP_REASON_HIGH_RISK = 1u << 5,
    AP_REASON_SENSITIVE = 1u << 6,
    AP_REASON_THIRD_PARTY = 1u << 7,
    AP_REASON_EXPIRED = 1u << 8,
    AP_REASON_COOLDOWN = 1u << 9,
    AP_REASON_BUDGET = 1u << 10,
    AP_REASON_NO_CONTACT = 1u << 11,
    AP_REASON_BAD_FORMAT = 1u << 12
} ap_reason_t;

typedef struct {
    ap_opportunity_class_t opportunity_class;
    ap_privacy_class_t privacy_class;
    uint8_t attention_available;
    uint8_t proactive_consent;
    uint8_t confidence_pct;
    uint8_t relevance_pct;
    uint8_t novelty_pct;
    uint8_t risk_pct;
    uint32_t now_generation;
    uint32_t valid_until_generation;
    uint32_t cooldown_generations;
} ap_observation_t;

typedef struct {
    ap_opportunity_class_t opportunity_class;
    uint8_t requires_physical_contact;
    uint32_t explanation_code;
} ap_offer_t;

typedef struct {
    ap_status_t status;
    uint32_t reason;
    uint32_t candidate_generation;
    uint32_t expires_generation;
    uint32_t cooldown_until_generation;
    uint32_t cooldown_generations;
    uint8_t offer_budget;
    uint8_t offered;
    uint8_t candidate_valid;
    uint8_t attention_ready;
    uint8_t consent_granted;
    ap_opportunity_class_t opportunity_class;
    ap_privacy_class_t privacy_class;
} ap_presence_t;

enum {
    AP_OK = 0,
    AP_NO_OFFER = 1,
    AP_E_ARG = -1,
    AP_E_FORMAT = -2,
    AP_E_STATE = -3,
    AP_E_CONTACT = -4
};

void ap_init(ap_presence_t *presence);

/* Stores only a bounded typed opportunity. It never creates authority. */
int ap_observe(ap_presence_t *presence, const ap_observation_t *observation);

/* Surfaces at most one local offer. The caller must map it to a local haptic/UI. */
int ap_offer(ap_presence_t *presence, uint32_t now_generation,
             ap_offer_t *out);

/* A physical contact acknowledges reception only; it does not authorize action. */
int ap_acknowledge(ap_presence_t *presence, uint8_t physical_contact);

/* Advances the caller-owned generation and scrubs expired or invalid state. */
int ap_tick(ap_presence_t *presence, uint32_t now_generation);

/* Clears all transient presence state. */
void ap_forget(ap_presence_t *presence);

#endif /* HERUS_AMBIENT_PRESENCE_H */
