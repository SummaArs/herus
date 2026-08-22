#include "ambient_presence.h"
#include <string.h>

static int canonical_bool(uint8_t value)
{
    return value == 0u || value == 1u;
}

static int valid_class(ap_opportunity_class_t value)
{
    return value >= AP_CLASS_RECALL && value <= AP_CLASS_CONNECTION;
}

static int valid_privacy(ap_privacy_class_t value)
{
    return value >= AP_PRIVACY_ORDINARY && value <= AP_PRIVACY_THIRD_PARTY;
}

static void clear_candidate(ap_presence_t *presence)
{
    presence->candidate_generation = 0u;
    presence->expires_generation = 0u;
    presence->candidate_valid = 0u;
    presence->attention_ready = 0u;
    presence->consent_granted = 0u;
    presence->opportunity_class = 0;
    presence->privacy_class = 0;
}

static int reject_observation(ap_presence_t *presence, uint32_t reason,
                              ap_status_t status)
{
    clear_candidate(presence);
    presence->status = status;
    presence->reason = reason;
    presence->offer_budget = 0u;
    presence->offered = 0u;
    return AP_NO_OFFER;
}

void ap_init(ap_presence_t *presence)
{
    if (!presence) return;
    memset(presence, 0, sizeof(*presence));
    presence->status = AP_QUIET;
    presence->offer_budget = AP_MAX_OFFER_BUDGET;
}

int ap_observe(ap_presence_t *presence, const ap_observation_t *observation)
{
    uint32_t reason = AP_REASON_NONE;

    if (!presence || !observation) return AP_E_ARG;
    if (!valid_class(observation->opportunity_class) ||
        !valid_privacy(observation->privacy_class) ||
        !canonical_bool(observation->attention_available) ||
        !canonical_bool(observation->proactive_consent) ||
        observation->confidence_pct > 100u ||
        observation->relevance_pct > 100u ||
        observation->novelty_pct > 100u ||
        observation->risk_pct > 100u ||
        observation->now_generation == 0u ||
        observation->valid_until_generation < observation->now_generation ||
        observation->cooldown_generations == 0u ||
        observation->now_generation > UINT32_MAX -
            observation->cooldown_generations)
        return reject_observation(presence, AP_REASON_BAD_FORMAT, AP_ABSTAIN);

    if (presence->candidate_valid) {
        presence->status = presence->offered ? AP_OFFER : AP_HOLD;
        presence->reason = presence->offered ? AP_REASON_BUDGET : AP_REASON_NONE;
        return AP_NO_OFFER;
    }

    if (observation->now_generation < presence->cooldown_until_generation)
        return reject_observation(presence, AP_REASON_COOLDOWN, AP_QUIET);

    if (observation->privacy_class == AP_PRIVACY_SENSITIVE)
        return reject_observation(presence, AP_REASON_SENSITIVE, AP_ABSTAIN);
    if (observation->privacy_class == AP_PRIVACY_THIRD_PARTY)
        return reject_observation(presence, AP_REASON_THIRD_PARTY, AP_ABSTAIN);

    if (observation->attention_available != 1u)
        reason |= AP_REASON_NO_ATTENTION;
    if (observation->proactive_consent != 1u)
        reason |= AP_REASON_NO_CONSENT;
    if (observation->confidence_pct < AP_MIN_CONFIDENCE_PCT)
        reason |= AP_REASON_LOW_CONFIDENCE;
    if (observation->relevance_pct < AP_MIN_RELEVANCE_PCT)
        reason |= AP_REASON_LOW_RELEVANCE;
    if (observation->novelty_pct < AP_MIN_NOVELTY_PCT)
        reason |= AP_REASON_LOW_NOVELTY;
    if (observation->risk_pct > AP_MAX_RISK_PCT)
        reason |= AP_REASON_HIGH_RISK;

    if (reason != AP_REASON_NONE)
        return reject_observation(presence, reason,
                                  (reason & (AP_REASON_NO_ATTENTION |
                                             AP_REASON_NO_CONSENT)) != 0u
                                      ? AP_QUIET : AP_ABSTAIN);

    memset(&presence->opportunity_class, 0,
           sizeof(presence->opportunity_class));
    presence->candidate_generation = observation->now_generation;
    presence->expires_generation = observation->valid_until_generation;
    presence->cooldown_generations = observation->cooldown_generations;
    presence->offer_budget = AP_MAX_OFFER_BUDGET;
    presence->offered = 0u;
    presence->candidate_valid = 1u;
    presence->attention_ready = observation->attention_available;
    presence->consent_granted = observation->proactive_consent;
    presence->opportunity_class = observation->opportunity_class;
    presence->privacy_class = observation->privacy_class;
    presence->status = AP_HOLD;
    presence->reason = AP_REASON_NONE;
    return AP_OK;
}

int ap_offer(ap_presence_t *presence, uint32_t now_generation,
             ap_offer_t *out)
{
    if (out) memset(out, 0, sizeof(*out));
    if (!presence || !out || now_generation == 0u) return AP_E_ARG;
    if (now_generation < presence->candidate_generation) return AP_E_FORMAT;
    if (!presence->candidate_valid) return AP_NO_OFFER;
    if (now_generation > presence->expires_generation) {
        (void)ap_tick(presence, now_generation);
        return AP_NO_OFFER;
    }
    if (presence->offered || presence->offer_budget == 0u) {
        presence->status = AP_OFFER;
        presence->reason = AP_REASON_BUDGET;
        return AP_NO_OFFER;
    }
    if (presence->attention_ready != 1u || presence->consent_granted != 1u) {
        presence->status = AP_QUIET;
        presence->reason = AP_REASON_NO_ATTENTION | AP_REASON_NO_CONSENT;
        return AP_NO_OFFER;
    }
    if (presence->privacy_class == AP_PRIVACY_SENSITIVE ||
        presence->privacy_class == AP_PRIVACY_THIRD_PARTY) {
        return reject_observation(presence,
                                  presence->privacy_class == AP_PRIVACY_SENSITIVE
                                      ? AP_REASON_SENSITIVE
                                      : AP_REASON_THIRD_PARTY,
                                  AP_ABSTAIN);
    }

    out->opportunity_class = presence->opportunity_class;
    out->requires_physical_contact = 1u;
    out->explanation_code = (uint32_t)presence->opportunity_class;
    presence->offered = 1u;
    presence->offer_budget = 0u;
    presence->cooldown_until_generation = now_generation +
                                           presence->cooldown_generations;
    presence->status = AP_OFFER;
    presence->reason = AP_REASON_NONE;
    return AP_OK;
}

int ap_acknowledge(ap_presence_t *presence, uint8_t physical_contact)
{
    if (!presence) return AP_E_ARG;
    if (physical_contact != 1u) {
        presence->reason = AP_REASON_NO_CONTACT;
        return AP_E_CONTACT;
    }
    if (!presence->candidate_valid || !presence->offered ||
        presence->status != AP_OFFER)
        return AP_E_STATE;
    clear_candidate(presence);
    presence->status = AP_ACKNOWLEDGED;
    presence->reason = AP_REASON_NONE;
    return AP_OK;
}

int ap_tick(ap_presence_t *presence, uint32_t now_generation)
{
    if (!presence || now_generation == 0u) return AP_E_ARG;
    if (presence->candidate_valid &&
        now_generation < presence->candidate_generation)
        return AP_E_FORMAT;
    if (presence->candidate_valid &&
        now_generation > presence->expires_generation) {
        clear_candidate(presence);
        presence->status = AP_EXPIRED;
        presence->reason = AP_REASON_EXPIRED;
        presence->offer_budget = 0u;
        presence->offered = 0u;
        return AP_NO_OFFER;
    }
    return AP_OK;
}

void ap_forget(ap_presence_t *presence)
{
    if (!presence) return;
    memset(presence, 0, sizeof(*presence));
    presence->status = AP_QUIET;
}
