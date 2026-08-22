/* personal_adapter.h — bounded local preference adaptation.
 *
 * The adapter stores typed numeric preference votes only. It never accepts text,
 * audio, identity, embeddings or network data, and it cannot grant authority.
 */
#ifndef HERUS_PERSONAL_ADAPTER_H
#define HERUS_PERSONAL_ADAPTER_H

#include <stddef.h>
#include <stdint.h>

#define PA_MAX_FEATURES 16u
#define PA_STYLE_MAX     4u
#define PA_MIN_VOTES     2u
#define PA_MIN_MARGIN    1u

typedef enum {
    PA_STYLE_NEUTRAL = 0,
    PA_STYLE_CONCISE,
    PA_STYLE_DETAILED,
    PA_STYLE_TECHNICAL
} pa_style_t;

typedef enum {
    PA_OK = 0,
    PA_ABSTAIN = 1,
    PA_E_ARG = -1,
    PA_E_AUTH = -2,
    PA_E_FULL = -3,
    PA_E_REVOKED = -4,
    PA_E_FORMAT = -5
} pa_status_t;

typedef struct {
    uint32_t feature_id;
    pa_style_t preferred_style;
    uint8_t local_origin;
    uint8_t confidence_pct;
} pa_sample_t;

typedef struct {
    uint32_t feature_id;
    uint16_t votes[PA_STYLE_MAX];
    uint32_t version;
    uint8_t active;
    uint8_t tombstone;
} pa_entry_t;

typedef struct {
    pa_entry_t entries[PA_MAX_FEATURES];
    uint8_t entry_count;
    uint32_t version;
    uint32_t epoch;
    uint32_t accepted_updates;
    uint32_t rejected_updates;
    uint32_t revoked_entries;
} pa_profile_t;

typedef struct {
    pa_status_t status;
    pa_style_t style;
    uint8_t confidence_pct;
    uint8_t abstain;
    uint8_t margin_votes;
    uint16_t total_votes;
    uint32_t feature_id;
    uint32_t version;
} pa_prediction_t;

void pa_init(pa_profile_t *profile);

/* Adds one typed preference vote. Both local_origin and explicit consent must be 1. */
pa_status_t pa_update(pa_profile_t *profile, const pa_sample_t *sample,
                     uint8_t explicit_consent);

/* Tombstones one feature. A stale update cannot silently reintroduce it. */
pa_status_t pa_forget(pa_profile_t *profile, uint32_t feature_id);

/* Returns a style only when votes and margin meet the bounded thresholds. */
pa_status_t pa_predict(const pa_profile_t *profile, uint32_t feature_id,
                      pa_prediction_t *out);

/* Clears active learned state while preserving a monotonic epoch boundary. */
pa_status_t pa_reboot_quarantine(pa_profile_t *profile);

size_t pa_active_entries(const pa_profile_t *profile);

#endif /* HERUS_PERSONAL_ADAPTER_H */
