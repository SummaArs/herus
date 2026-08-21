#include "adaptive_change.h"
#include <string.h>

static int valid_source(at_source_t source)
{
    return source == AT_SOURCE_LOCAL_OBSERVATION ||
           source == AT_SOURCE_CORE_KNOWLEDGE;
}

static int fact_equal(const sr_fact_t *a, const sr_fact_t *b)
{
    return a && b && a->subject == b->subject &&
           a->predicate == b->predicate && a->object == b->object &&
           a->negated == b->negated;
}

static int same_key(const sr_fact_t *a, const sr_fact_t *b)
{
    return a && b && a->subject == b->subject &&
           a->predicate == b->predicate && a->negated == b->negated;
}

static int term_matches(sr_term_t term, sr_symbol_t value)
{
    return term.kind == SR_TERM_VARIABLE || term.value == value;
}

static int pattern_matches(const sr_pattern_t *pattern, const sr_fact_t *fact)
{
    return pattern && fact && pattern->negated == fact->negated &&
           term_matches(pattern->subject, fact->subject) &&
           term_matches(pattern->predicate, fact->predicate) &&
           term_matches(pattern->object, fact->object);
}

static int find_card(const ac_index_t *index, uint32_t card_id)
{
    uint16_t i;
    if (!index || card_id == 0u) return -1;
    for (i = 0u; i < index->entry_count; i++)
        if (index->entries[i].card_id == card_id) return (int)i;
    return -1;
}

static int is_revoked(const ac_index_t *index, uint32_t card_id)
{
    int position = find_card(index, card_id);
    return position >= 0 && index->entries[position].status == AC_ENTRY_REVOKED;
}

static int valid_observation(const ac_index_t *index,
                             const ac_observation_t *observation,
                             uint32_t generation)
{
    if (!index || !observation || !valid_source(observation->source) ||
        observation->card_id == 0u || observation->review_receipt_id == 0u ||
        observation->fact.subject == 0u || observation->fact.predicate == 0u ||
        observation->fact.object == 0u || observation->observed_generation == 0u ||
        observation->epoch != index->epoch ||
        observation->observed_generation > generation ||
        observation->observed_generation <= index->generation_floor ||
        observation->confidence < 1u || observation->confidence > 3u ||
        observation->physical_confirmation != 1u ||
        observation->explicit_change_confirmation != 1u ||
        (observation->valid_until_generation != 0u &&
         observation->valid_until_generation < observation->observed_generation))
        return 0;
    if (observation->derived_from_card_id != 0u &&
        is_revoked(index, observation->derived_from_card_id))
        return 0;
    return 1;
}

void ac_init(ac_index_t *index)
{
    if (index) {
        memset(index, 0, sizeof(*index));
        index->epoch = 1u;
    }
}

int ac_apply_change(ac_index_t *index, const ac_observation_t *observation,
                    uint32_t generation, uint32_t *out_card_id)
{
    uint16_t i;
    int existing;
    ac_entry_t *entry;

    if (out_card_id) *out_card_id = 0u;
    if (!index || !observation) return AC_E_ARG;
    if (observation->confidence < AC_MIN_CONFIDENCE) {
        index->rejected++;
        return AC_E_CONFIDENCE;
    }
    if (!valid_observation(index, observation, generation)) {
        index->rejected++;
        return AC_E_AUTH;
    }
    existing = find_card(index, observation->card_id);
    if (existing >= 0) {
        if (fact_equal(&index->entries[existing].fact, &observation->fact) &&
            index->entries[existing].status == AC_ENTRY_ACTIVE)
            return AC_NO_CHANGE;
        index->rejected++;
        return AC_E_FORMAT;
    }
    if (index->entry_count >= AC_MAX_ENTRIES) {
        index->rejected++;
        return AC_E_FULL;
    }
    if (observation->valid_until_generation != 0u &&
        generation > observation->valid_until_generation) {
        index->rejected++;
        return AC_E_EXPIRED;
    }

    /* A confirmed change supersedes all current alternatives for the same
     * subject/predicate. Their history remains available as lineage, but none
     * can answer as current identity. */
    for (i = 0u; i < index->entry_count; i++) {
        entry = &index->entries[i];
        if (entry->status == AC_ENTRY_ACTIVE &&
            same_key(&entry->fact, &observation->fact)) {
            entry->status = AC_ENTRY_SUPERSEDED;
            entry->superseded_by_card_id = observation->card_id;
            index->supersessions++;
        }
    }

    entry = &index->entries[index->entry_count++];
    memset(entry, 0, sizeof(*entry));
    entry->fact = observation->fact;
    entry->card_id = observation->card_id;
    entry->review_receipt_id = observation->review_receipt_id;
    entry->observed_generation = observation->observed_generation;
    entry->valid_until_generation = observation->valid_until_generation;
    entry->derived_from_card_id = observation->derived_from_card_id;
    entry->source = observation->source;
    entry->status = AC_ENTRY_ACTIVE;
    entry->confidence = observation->confidence;
    index->additions++;
    if (out_card_id) *out_card_id = entry->card_id;
    return AC_OK;
}

int ac_revoke(ac_index_t *index, uint32_t card_id, uint8_t physical_confirmation,
              uint32_t generation)
{
    uint16_t i;
    int position;
    int changed = 0;
    int grew;

    if (!index || card_id == 0u || physical_confirmation != 1u)
        return AC_E_AUTH;
    position = find_card(index, card_id);
    if (position < 0) return AC_E_ARG;
    if (generation < index->entries[position].observed_generation)
        return AC_E_ROLLBACK;

    do {
        grew = 0;
        for (i = 0u; i < index->entry_count; i++) {
            ac_entry_t *entry = &index->entries[i];
            if (entry->status != AC_ENTRY_REVOKED &&
                (entry->card_id == card_id ||
                 is_revoked(index, entry->derived_from_card_id))) {
                entry->status = AC_ENTRY_REVOKED;
                changed++;
                grew = 1;
            }
        }
    } while (grew);
    if (changed == 0) return AC_NO_CHANGE;
    index->revocations += (uint32_t)changed;
    return AC_OK;
}

unsigned ac_expire(ac_index_t *index, uint32_t generation)
{
    uint16_t i;
    unsigned changed = 0u;
    if (!index) return 0u;
    for (i = 0u; i < index->entry_count; i++) {
        ac_entry_t *entry = &index->entries[i];
        if (entry->status == AC_ENTRY_ACTIVE &&
            entry->valid_until_generation != 0u &&
            generation > entry->valid_until_generation) {
            entry->status = AC_ENTRY_EXPIRED;
            changed++;
        }
    }
    index->expirations += changed;
    return changed;
}

int ac_query(const ac_index_t *index, const sr_pattern_t *pattern,
             uint32_t generation, ac_query_result_t *out)
{
    uint16_t i;
    uint16_t active = 0u;
    uint16_t historical = 0u;
    uint16_t revoked = 0u;

    if (!index || !pattern || !out) return AC_E_ARG;
    memset(out, 0, sizeof(*out));
    for (i = 0u; i < index->entry_count; i++) {
        const ac_entry_t *entry = &index->entries[i];
        if (!pattern_matches(pattern, &entry->fact)) continue;
        if (entry->status == AC_ENTRY_REVOKED) {
            revoked++;
        } else if (entry->status == AC_ENTRY_ACTIVE &&
                   (entry->valid_until_generation == 0u ||
                    generation <= entry->valid_until_generation)) {
            active++;
            if (active == 1u) {
                out->fact = entry->fact;
                out->selected_card_id = entry->card_id;
                out->selected_generation = entry->observed_generation;
            }
        } else {
            historical++;
        }
    }
    out->active_matches = active;
    out->historical_matches = historical;
    out->revoked_matches = revoked;
    if (active == 0u) {
        out->status = MSE_QUERY_NO_MATCH;
    } else if (active == 1u) {
        out->status = MSE_QUERY_MATCH;
    } else {
        out->selected_card_id = 0u;
        out->selected_generation = 0u;
        out->status = MSE_QUERY_AMBIGUOUS;
    }
    return AC_OK;
}

int ac_set_generation_floor(ac_index_t *index, uint32_t floor)
{
    if (!index || floor == 0u) return AC_E_ARG;
    if (index->entry_count != 0u) return AC_E_FORMAT;
    if (floor < index->generation_floor) return AC_E_ROLLBACK;
    index->generation_floor = floor;
    return AC_OK;
}

void ac_reboot(ac_index_t *index)
{
    uint16_t i;
    if (!index) return;
    if (++index->epoch == 0u) index->epoch = 1u;
    for (i = 0u; i < index->entry_count; i++)
        if (index->entries[i].status == AC_ENTRY_ACTIVE)
            index->entries[i].status = AC_ENTRY_QUARANTINED;
}
