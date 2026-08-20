#include "memory_semantic_evidence.h"
#include <string.h>

static int canonical_bool(uint8_t value)
{
    return value == 0u || value == 1u;
}

static int valid_fact(const sr_fact_t *fact)
{
    return fact && fact->subject != 0u && fact->predicate != 0u &&
           fact->object != 0u && canonical_bool(fact->negated);
}

static int valid_card(const memory_vault_card_t *card)
{
    return card && card->card_id != 0u && card->review_receipt_id != 0u;
}

static int valid_term(const sr_term_t *term)
{
    if (!term) return 0;
    if (term->kind == SR_TERM_VARIABLE) return term->value < SR_MAX_VARIABLES;
    return term->kind == SR_TERM_CONSTANT && term->value != 0u;
}

static int valid_pattern(const sr_pattern_t *pattern)
{
    return pattern && canonical_bool(pattern->negated) &&
           valid_term(&pattern->subject) && valid_term(&pattern->predicate) &&
           valid_term(&pattern->object);
}

static int same_fact(const sr_fact_t *a, const sr_fact_t *b)
{
    return a->subject == b->subject && a->predicate == b->predicate &&
           a->object == b->object && a->negated == b->negated;
}

static int conflicting_fact(const sr_fact_t *a, const sr_fact_t *b,
                            mse_functional_predicate_fn is_functional,
                            void *user)
{
    if (a->subject != b->subject || a->predicate != b->predicate)
        return 0;
    if (a->object == b->object && a->negated != b->negated) return 1;
    return is_functional && is_functional(a->predicate, user) &&
           a->object != b->object;
}

static int bind_term(const sr_term_t *term, sr_symbol_t value,
                     uint8_t used[SR_MAX_VARIABLES],
                     sr_symbol_t bindings[SR_MAX_VARIABLES])
{
    if (term->kind == SR_TERM_CONSTANT) return term->value == value;
    if (!used[term->value]) {
        used[term->value] = 1u;
        bindings[term->value] = value;
        return 1;
    }
    return bindings[term->value] == value;
}

static int matches(const sr_pattern_t *pattern, const sr_fact_t *fact)
{
    uint8_t used[SR_MAX_VARIABLES] = { 0u };
    sr_symbol_t bindings[SR_MAX_VARIABLES] = { 0u };
    return pattern->negated == fact->negated &&
           bind_term(&pattern->subject, fact->subject, used, bindings) &&
           bind_term(&pattern->predicate, fact->predicate, used, bindings) &&
           bind_term(&pattern->object, fact->object, used, bindings);
}

void mse_init(mse_index_t *index,
              mse_functional_predicate_fn is_functional,
              void *policy_user)
{
    if (!index) return;
    memset(index, 0, sizeof(*index));
    index->is_functional = is_functional;
    index->policy_user = policy_user;
}

mse_status_t mse_add(mse_index_t *index,
                     const memory_vault_card_t *card,
                     const sr_fact_t *fact,
                     uint32_t observed_generation,
                     uint32_t valid_until_generation)
{
    int inserted;
    if (!index || !valid_card(card) || !valid_fact(fact) ||
        observed_generation == 0u ||
        (valid_until_generation != 0u &&
         valid_until_generation < observed_generation)) {
        if (index) index->rejected++;
        return MSE_E_ARG;
    }
    for (uint16_t i = 0u; i < index->evidence_count; i++) {
        mse_evidence_t *item = &index->evidence[i];
        if (item->status == MSE_EVIDENCE_ACTIVE && same_fact(&item->fact, fact)) {
            if (observed_generation < item->observed_generation) {
                index->rejected++;
                return MSE_E_ROLLBACK;
            }
            if (observed_generation == item->observed_generation)
                return MSE_NO_CHANGE;
            item->status = MSE_EVIDENCE_SUPERSEDED;
            index->superseded++;
            break;
        }
    }
    if (index->evidence_count >= MSE_MAX_EVIDENCE) {
        index->rejected++;
        return MSE_E_FULL;
    }
    inserted = (int)index->evidence_count++;
    index->evidence[inserted].fact = *fact;
    index->evidence[inserted].card_id = card->card_id;
    index->evidence[inserted].review_receipt_id = card->review_receipt_id;
    index->evidence[inserted].observed_generation = observed_generation;
    index->evidence[inserted].valid_until_generation = valid_until_generation;
    index->evidence[inserted].status = MSE_EVIDENCE_ACTIVE;
    index->additions++;

    for (uint16_t i = 0u; i < index->evidence_count; i++) {
        mse_evidence_t *item = &index->evidence[i];
        if (i == (uint16_t)inserted || item->status != MSE_EVIDENCE_ACTIVE)
            continue;
        if (conflicting_fact(&item->fact, fact, index->is_functional,
                             index->policy_user)) {
            item->status = MSE_EVIDENCE_CONFLICTED;
            index->evidence[inserted].status = MSE_EVIDENCE_CONFLICTED;
            index->conflicts++;
        }
    }
    return MSE_OK;
}

unsigned mse_expire(mse_index_t *index, uint32_t current_generation)
{
    unsigned expired = 0u;
    if (!index || current_generation == 0u) return 0u;
    for (uint16_t i = 0u; i < index->evidence_count; i++) {
        mse_evidence_t *item = &index->evidence[i];
        if (item->status == MSE_EVIDENCE_ACTIVE &&
            item->valid_until_generation != 0u &&
            current_generation > item->valid_until_generation) {
            item->status = MSE_EVIDENCE_EXPIRED;
            index->expired++;
            expired++;
        }
    }
    return expired;
}

mse_status_t mse_query(const mse_index_t *index,
                       const sr_pattern_t *pattern,
                       uint32_t current_generation,
                       mse_query_result_t *out)
{
    uint16_t active = 0u;
    uint16_t conflicted = 0u;
    uint16_t selected = 0u;
    if (out) memset(out, 0, sizeof(*out));
    if (!index || !valid_pattern(pattern) || current_generation == 0u || !out)
        return MSE_E_ARG;
    if (pattern->subject.kind == SR_TERM_VARIABLE &&
        pattern->predicate.kind == SR_TERM_VARIABLE &&
        pattern->object.kind == SR_TERM_VARIABLE)
        return MSE_E_ARG;
    for (uint16_t i = 0u; i < index->evidence_count; i++) {
        const mse_evidence_t *item = &index->evidence[i];
        if (item->status != MSE_EVIDENCE_ACTIVE &&
            item->status != MSE_EVIDENCE_CONFLICTED)
            continue;
        if (item->valid_until_generation != 0u &&
            current_generation > item->valid_until_generation)
            continue;
        if (!matches(pattern, &item->fact)) continue;
        if (item->status == MSE_EVIDENCE_CONFLICTED) conflicted++;
        else {
            active++;
            selected = i;
        }
    }
    out->active_matches = active;
    out->conflict_matches = conflicted;
    if (conflicted != 0u) {
        out->status = MSE_QUERY_CONTRADICTED;
        return MSE_OK;
    }
    if (active == 0u) {
        out->status = MSE_QUERY_NO_MATCH;
        return MSE_OK;
    }
    if (active > 1u) {
        out->status = MSE_QUERY_AMBIGUOUS;
        return MSE_OK;
    }
    out->status = MSE_QUERY_MATCH;
    out->selected_card_id = index->evidence[selected].card_id;
    out->selected_review_receipt_id = index->evidence[selected].review_receipt_id;
    out->selected_generation = index->evidence[selected].observed_generation;
    return MSE_OK;
}
