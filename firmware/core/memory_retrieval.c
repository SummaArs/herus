/* memory_retrieval.c — pure typed matching over controlled memory-card inputs. */
#include "memory_retrieval.h"
#include <string.h>

static int access_valid(const memory_consolidation_access_t *access)
{
    return access && access->physical_session_id != 0u &&
           access->physical_confirmed == 1u;
}

static int query_valid(const memory_retrieval_query_t *query)
{
    if (!query || query->preferred_kind >= MEMORY_KIND_COUNT ||
        query->preferred_origin >= MEMORY_EXTRACT_ORIGIN_COUNT ||
        (query->require_explicit != 0u && query->require_explicit != 1u) ||
        query->minimum_confidence_pct > 100u)
        return 0;
    return query->preferred_kind != MEMORY_KIND_NONE ||
           query->preferred_origin != MEMORY_EXTRACT_ORIGIN_NONE ||
           query->require_explicit == 1u || query->minimum_confidence_pct != 0u;
}

static int card_valid(const memory_vault_card_t *card)
{
    memory_assessment_t assessment;
    if (!card || card->card_id == 0u || card->review_receipt_id == 0u ||
        card->signal.session_authorized != 1u ||
        (card->signal.explicit_remember != 0u &&
         card->signal.explicit_remember != 1u) ||
        card->signal.kind <= MEMORY_KIND_NONE || card->signal.kind >= MEMORY_KIND_COUNT ||
        card->signal.scope != MEMORY_SCOPE_SELF ||
        card->signal.sensitivity != MEMORY_SENSITIVITY_ORDINARY ||
        card->signal.confidence_pct > 100u || card->signal.novelty_pct > 100u ||
        card->signal.future_value_pct > 100u ||
        card->signal.consequence_pct > 100u ||
        card->origin <= MEMORY_EXTRACT_ORIGIN_NONE ||
        card->origin >= MEMORY_EXTRACT_ORIGIN_COUNT || card->extract_reasons == 0u)
        return 0;
    if (memory_policy_assess(&card->signal, &assessment) != MEMORY_POLICY_OK)
        return 0;
    return assessment.disposition == MEMORY_DISPOSITION_AUTO_ELIGIBLE;
}

static int cards_valid(const memory_vault_card_t *cards, size_t card_count)
{
    size_t i;
    size_t j;
    if (!cards || card_count == 0u || card_count > MEMORY_RETRIEVAL_MAX_CARDS)
        return 0;
    for (i = 0u; i < card_count; ++i) {
        if (!card_valid(&cards[i])) return 0;
        for (j = 0u; j < i; ++j)
            if (cards[i].card_id == cards[j].card_id) return 0;
    }
    return 1;
}

/* Returns 1 when the card satisfies all explicit filters; never performs fuzzy
 * interpretation. `reasons` explains only typed dimensions and quality signals. */
static int score_card(const memory_retrieval_query_t *query,
                      const memory_vault_card_t *card,
                      uint8_t *score_out, uint32_t *reasons_out)
{
    uint32_t score = 20u;
    uint32_t reasons = MEMORY_RETRIEVAL_REASON_NONE;

    if (query->preferred_kind != MEMORY_KIND_NONE) {
        if (card->signal.kind != query->preferred_kind) return 0;
        score += 35u;
        reasons |= MEMORY_RETRIEVAL_REASON_KIND;
    } else {
        score += 5u;
    }
    if (query->preferred_origin != MEMORY_EXTRACT_ORIGIN_NONE) {
        if (card->origin != query->preferred_origin) return 0;
        score += 20u;
        reasons |= MEMORY_RETRIEVAL_REASON_ORIGIN;
    } else {
        score += 5u;
    }
    if (query->require_explicit == 1u) {
        if (card->signal.explicit_remember != 1u) return 0;
        score += 15u;
        reasons |= MEMORY_RETRIEVAL_REASON_EXPLICIT;
    } else if (card->signal.explicit_remember == 1u) {
        score += 5u;
    }
    if (query->minimum_confidence_pct != 0u) {
        if (card->signal.confidence_pct < query->minimum_confidence_pct) return 0;
        score += 10u;
        reasons |= MEMORY_RETRIEVAL_REASON_CONFIDENCE;
    } else {
        score += (uint32_t)card->signal.confidence_pct / 20u;
    }
    score += (uint32_t)card->signal.novelty_pct / 20u;
    score += (uint32_t)card->signal.future_value_pct / 20u;
    score += (uint32_t)card->signal.consequence_pct / 20u;
    if (card->signal.novelty_pct != 0u) reasons |= MEMORY_RETRIEVAL_REASON_NOVELTY;
    if (card->signal.future_value_pct != 0u) reasons |= MEMORY_RETRIEVAL_REASON_FUTURE;
    if (card->signal.consequence_pct != 0u) reasons |= MEMORY_RETRIEVAL_REASON_CONSEQUENCE;
    *score_out = (uint8_t)(score > 100u ? 100u : score);
    *reasons_out = reasons;
    return 1;
}

void memory_retrieval_init(memory_retrieval_t *r)
{
    if (r) memset(r, 0, sizeof(*r));
}

int memory_retrieval_query(memory_retrieval_t *r,
                           const memory_consolidation_access_t *access,
                           const memory_retrieval_query_t *query,
                           const memory_vault_card_t *cards, size_t card_count,
                           memory_retrieval_result_t *out)
{
    size_t i;
    uint8_t top_score = 0u;
    uint8_t second_score = 0u;
    uint32_t top_id = 0u;
    memory_kind_t top_kind = MEMORY_KIND_NONE;
    memory_extract_origin_t top_origin = MEMORY_EXTRACT_ORIGIN_NONE;
    uint32_t top_reasons = MEMORY_RETRIEVAL_REASON_NONE;

    if (!r || !out) return MEMORY_RETRIEVAL_E_ARG;
    memset(out, 0, sizeof(*out));
    r->metrics.queries++;
    if (!access_valid(access)) {
        r->metrics.rejected_access++;
        return MEMORY_RETRIEVAL_E_ACCESS;
    }
    if (!query_valid(query)) {
        r->metrics.rejected_query++;
        return MEMORY_RETRIEVAL_E_QUERY;
    }
    if (!cards_valid(cards, card_count)) {
        r->metrics.rejected_source++;
        return card_count > MEMORY_RETRIEVAL_MAX_CARDS ? MEMORY_RETRIEVAL_E_CAPACITY :
                                                        MEMORY_RETRIEVAL_E_SOURCE;
    }

    for (i = 0u; i < card_count; ++i) {
        uint8_t score;
        uint32_t reasons;
        if (!score_card(query, &cards[i], &score, &reasons)) continue;
        if (score > top_score) {
            second_score = top_score;
            top_score = score;
            top_id = cards[i].card_id;
            top_kind = cards[i].signal.kind;
            top_origin = cards[i].origin;
            top_reasons = reasons;
        } else if (score > second_score) {
            second_score = score;
        }
    }

    out->score_pct = top_score;
    out->runner_up_score_pct = second_score;
    if (top_score < MEMORY_RETRIEVAL_MIN_SCORE) {
        out->status = MEMORY_RETRIEVAL_NO_MATCH;
        r->metrics.no_match++;
        return MEMORY_RETRIEVAL_OK;
    }
    if (second_score != 0u && (uint32_t)top_score - (uint32_t)second_score <
        MEMORY_RETRIEVAL_MIN_MARGIN) {
        out->status = MEMORY_RETRIEVAL_AMBIGUOUS;
        r->metrics.ambiguous++;
        return MEMORY_RETRIEVAL_OK;
    }
    out->status = MEMORY_RETRIEVAL_MATCH;
    out->card_id = top_id;
    out->kind = top_kind;
    out->origin = top_origin;
    out->reasons = top_reasons;
    r->metrics.matches++;
    return MEMORY_RETRIEVAL_OK;
}

const memory_retrieval_metrics_t *memory_retrieval_metrics(const memory_retrieval_t *r)
{
    return r ? &r->metrics : 0;
}
