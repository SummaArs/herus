/* memory_policy.c — deterministic selective-memory relevance policy. */
#include "memory_policy.h"
#include <string.h>

static int canonical_bool(uint8_t value)
{
    return value == 0u || value == 1u;
}

static int valid_signal(const memory_signal_t *s)
{
    if (!s || !canonical_bool(s->session_authorized) ||
        !canonical_bool(s->explicit_remember)) return 0;
    if (s->kind <= MEMORY_KIND_NONE || s->kind >= MEMORY_KIND_COUNT) return 0;
    if (s->scope <= MEMORY_SCOPE_NONE || s->scope >= MEMORY_SCOPE_COUNT) return 0;
    if (s->sensitivity <= MEMORY_SENSITIVITY_NONE ||
        s->sensitivity >= MEMORY_SENSITIVITY_COUNT) return 0;
    return s->confidence_pct <= 100u && s->novelty_pct <= 100u &&
           s->future_value_pct <= 100u && s->consequence_pct <= 100u;
}

static uint8_t relevance_score(const memory_signal_t *s)
{
    /* Future recoverability dominates: this is a memory complement, not a generic
     * engagement score. The weighted calculation is rounded then bounded. */
    unsigned score = ((unsigned)s->novelty_pct * 25u +
                      (unsigned)s->future_value_pct * 45u +
                      (unsigned)s->consequence_pct * 30u + 50u) / 100u;
    if (s->kind == MEMORY_KIND_DECISION || s->kind == MEMORY_KIND_COMMITMENT)
        score += 10u;
    if (score > 100u) score = 100u;
    return (uint8_t)score;
}

int memory_policy_assess(const memory_signal_t *signal, memory_assessment_t *out)
{
    uint8_t score;
    uint32_t reasons = MEMORY_REASON_NONE;
    int protected_scope;

    if (!out) return MEMORY_POLICY_E_ARG;
    memset(out, 0, sizeof(*out));
    out->disposition = MEMORY_DISPOSITION_DISCARD;
    if (!signal) return MEMORY_POLICY_E_ARG;
    if (!valid_signal(signal)) return MEMORY_POLICY_E_FORMAT;

    if (signal->session_authorized != 1u) {
        out->reasons = MEMORY_REASON_NOT_AUTHORIZED;
        return MEMORY_POLICY_E_NOT_AUTHORIZED;
    }
    if (signal->confidence_pct < MEMORY_POLICY_MIN_CONFIDENCE_PCT) {
        out->reasons = MEMORY_REASON_LOW_CONFIDENCE | MEMORY_REASON_AMBIGUOUS;
        return MEMORY_POLICY_OK;
    }

    score = relevance_score(signal);
    if (signal->explicit_remember == 1u) reasons |= MEMORY_REASON_EXPLICIT;
    if (signal->kind == MEMORY_KIND_DECISION || signal->kind == MEMORY_KIND_COMMITMENT)
        reasons |= MEMORY_REASON_DECISIONAL;
    if (signal->future_value_pct >= MEMORY_POLICY_REVIEW_SCORE)
        reasons |= MEMORY_REASON_FUTURE_VALUE;
    if (signal->novelty_pct >= MEMORY_POLICY_REVIEW_SCORE)
        reasons |= MEMORY_REASON_NOVEL;
    if (signal->consequence_pct >= MEMORY_POLICY_REVIEW_SCORE)
        reasons |= MEMORY_REASON_CONSEQUENTIAL;
    if (signal->sensitivity >= MEMORY_SENSITIVITY_PERSONAL)
        reasons |= MEMORY_REASON_SENSITIVE;
    if (signal->scope != MEMORY_SCOPE_SELF)
        reasons |= MEMORY_REASON_THIRD_PARTY;

    out->relevance_score = score;
    protected_scope = signal->sensitivity >= MEMORY_SENSITIVITY_PERSONAL ||
                      signal->scope != MEMORY_SCOPE_SELF;

    /* Explicit recall requests are valuable, but they are not permission to retain
     * sensitive or other-person data without a later human review. */
    if (protected_scope) {
        out->disposition = MEMORY_DISPOSITION_REVIEW;
    } else if (signal->explicit_remember == 1u || score >= MEMORY_POLICY_AUTO_SCORE) {
        out->disposition = MEMORY_DISPOSITION_AUTO_ELIGIBLE;
    } else if (score >= MEMORY_POLICY_REVIEW_SCORE) {
        out->disposition = MEMORY_DISPOSITION_REVIEW;
    } else {
        reasons |= MEMORY_REASON_LOW_RELEVANCE;
        out->disposition = MEMORY_DISPOSITION_DISCARD;
    }

    out->reasons = reasons;
    return MEMORY_POLICY_OK;
}
