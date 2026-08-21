#include "personal_adapter.h"

#include <string.h>

static pa_entry_t *find_entry(pa_profile_t *profile, uint32_t feature_id)
{
    uint8_t i;
    if (profile == NULL) return NULL;
    for (i = 0u; i < profile->entry_count; ++i) {
        if (profile->entries[i].feature_id == feature_id) {
            return &profile->entries[i];
        }
    }
    return NULL;
}

static const pa_entry_t *find_entry_const(const pa_profile_t *profile,
                                          uint32_t feature_id)
{
    uint8_t i;
    if (profile == NULL) return NULL;
    for (i = 0u; i < profile->entry_count; ++i) {
        if (profile->entries[i].feature_id == feature_id) {
            return &profile->entries[i];
        }
    }
    return NULL;
}

void pa_init(pa_profile_t *profile)
{
    if (profile == NULL) return;
    memset(profile, 0, sizeof(*profile));
    profile->epoch = 1u;
}

pa_status_t pa_update(pa_profile_t *profile, const pa_sample_t *sample,
                     uint8_t explicit_consent)
{
    pa_entry_t *entry;
    if (profile == NULL || sample == NULL || sample->feature_id == 0u ||
        sample->preferred_style >= PA_STYLE_MAX ||
        sample->confidence_pct == 0u) {
        return PA_E_ARG;
    }
    if (explicit_consent != 1u || sample->local_origin != 1u) {
        profile->rejected_updates++;
        return PA_E_AUTH;
    }
    entry = find_entry(profile, sample->feature_id);
    if (entry == NULL) {
        if (profile->entry_count >= PA_MAX_FEATURES) {
            profile->rejected_updates++;
            return PA_E_FULL;
        }
        entry = &profile->entries[profile->entry_count++];
        memset(entry, 0, sizeof(*entry));
        entry->feature_id = sample->feature_id;
        entry->active = 1u;
        entry->version = ++profile->version;
    }
    if (entry->tombstone != 0u) {
        profile->rejected_updates++;
        return PA_E_REVOKED;
    }
    if (entry->votes[sample->preferred_style] != UINT16_MAX) {
        entry->votes[sample->preferred_style]++;
    }
    entry->active = 1u;
    entry->version = ++profile->version;
    profile->accepted_updates++;
    return PA_OK;
}

pa_status_t pa_forget(pa_profile_t *profile, uint32_t feature_id)
{
    pa_entry_t *entry;
    if (profile == NULL || feature_id == 0u) return PA_E_ARG;
    entry = find_entry(profile, feature_id);
    if (entry == NULL) {
        if (profile->entry_count >= PA_MAX_FEATURES) return PA_E_FULL;
        entry = &profile->entries[profile->entry_count++];
        memset(entry, 0, sizeof(*entry));
        entry->feature_id = feature_id;
    }
    entry->active = 0u;
    entry->tombstone = 1u;
    entry->version = ++profile->version;
    profile->revoked_entries++;
    return PA_OK;
}

pa_status_t pa_predict(const pa_profile_t *profile, uint32_t feature_id,
                      pa_prediction_t *out)
{
    const pa_entry_t *entry;
    uint16_t best_votes = 0u;
    uint16_t second_votes = 0u;
    uint8_t best = 0u;
    uint8_t i;
    uint16_t total = 0u;
    if (out == NULL) return PA_E_ARG;
    memset(out, 0, sizeof(*out));
    out->feature_id = feature_id;
    if (profile == NULL || feature_id == 0u) return PA_E_ARG;
    entry = find_entry_const(profile, feature_id);
    if (entry == NULL || entry->active == 0u || entry->tombstone != 0u) {
        out->status = PA_ABSTAIN;
        out->abstain = 1u;
        return PA_ABSTAIN;
    }
    for (i = 0u; i < PA_STYLE_MAX; ++i) {
        total = (uint16_t)(total + entry->votes[i]);
        if (entry->votes[i] > best_votes) {
            second_votes = best_votes;
            best_votes = entry->votes[i];
            best = i;
        } else if (entry->votes[i] > second_votes) {
            second_votes = entry->votes[i];
        }
    }
    out->total_votes = total;
    out->version = entry->version;
    out->style = (pa_style_t)best;
    out->margin_votes = (uint8_t)(best_votes - second_votes);
    if (best_votes < PA_MIN_VOTES || out->margin_votes < PA_MIN_MARGIN) {
        out->status = PA_ABSTAIN;
        out->abstain = 1u;
        return PA_ABSTAIN;
    }
    out->confidence_pct = (uint8_t)((best_votes * 100u) / total);
    out->status = PA_OK;
    return PA_OK;
}

pa_status_t pa_reboot_quarantine(pa_profile_t *profile)
{
    uint8_t i;
    if (profile == NULL) return PA_E_ARG;
    profile->epoch++;
    for (i = 0u; i < profile->entry_count; ++i) {
        profile->entries[i].active = 0u;
    }
    return PA_OK;
}

size_t pa_active_entries(const pa_profile_t *profile)
{
    size_t count = 0u;
    uint8_t i;
    if (profile == NULL) return 0u;
    for (i = 0u; i < profile->entry_count; ++i) {
        if (profile->entries[i].active != 0u &&
            profile->entries[i].tombstone == 0u) count++;
    }
    return count;
}
