/*
 * adaptive_change.h — AGSC-D: adaptive, non-amplifying semantic continuity.
 *
 * The module stores typed facts and opaque provenance only. It is not a recorder,
 * recognizer, identity model or claim about human preference understanding.
 */
#ifndef HERUS_ADAPTIVE_CHANGE_H
#define HERUS_ADAPTIVE_CHANGE_H

#include <stdint.h>
#include "authority_transition.h"
#include "memory_semantic_evidence.h"

#define AC_MAX_ENTRIES 8u
#define AC_MIN_CONFIDENCE 2u

typedef enum {
    AC_ENTRY_NONE = 0u,
    AC_ENTRY_ACTIVE,
    AC_ENTRY_SUPERSEDED,
    AC_ENTRY_REVOKED,
    AC_ENTRY_EXPIRED,
    AC_ENTRY_QUARANTINED
} ac_entry_status_t;

typedef enum {
    AC_OK = 0,
    AC_NO_CHANGE = 1,
    AC_E_ARG = -1,
    AC_E_FORMAT = -2,
    AC_E_AUTH = -3,
    AC_E_CONFLICT = -4,
    AC_E_EXPIRED = -5,
    AC_E_ROLLBACK = -6,
    AC_E_EPOCH = -7,
    AC_E_REVOKED = -8,
    AC_E_FULL = -9,
    AC_E_CONFIDENCE = -10
} ac_status_t;

typedef struct {
    sr_fact_t fact;
    uint32_t card_id;
    uint32_t review_receipt_id;
    at_source_t source;
    uint32_t observed_generation;
    uint32_t valid_until_generation;
    uint32_t derived_from_card_id;
    uint32_t epoch;
    uint8_t confidence; /* 1..3; does not itself grant authority */
    uint8_t physical_confirmation;
    uint8_t explicit_change_confirmation;
} ac_observation_t;

typedef struct {
    sr_fact_t fact;
    uint32_t card_id;
    uint32_t review_receipt_id;
    uint32_t observed_generation;
    uint32_t valid_until_generation;
    uint32_t superseded_by_card_id;
    uint32_t derived_from_card_id;
    at_source_t source;
    ac_entry_status_t status;
    uint8_t confidence;
} ac_entry_t;

typedef struct {
    uint16_t active_matches;
    uint16_t historical_matches;
    uint16_t revoked_matches;
    sr_fact_t fact;
    uint32_t selected_card_id;
    uint32_t selected_generation;
    mse_query_status_t status;
} ac_query_result_t;

typedef struct {
    uint32_t epoch;
    uint32_t generation_floor;
    uint32_t additions;
    uint32_t supersessions;
    uint32_t revocations;
    uint32_t expirations;
    uint32_t rejected;
    uint16_t entry_count;
    ac_entry_t entries[AC_MAX_ENTRIES];
} ac_index_t;

void ac_init(ac_index_t *index);

int ac_apply_change(ac_index_t *index, const ac_observation_t *observation,
                    uint32_t generation, uint32_t *out_card_id);

int ac_revoke(ac_index_t *index, uint32_t card_id, uint8_t physical_confirmation,
              uint32_t generation);

unsigned ac_expire(ac_index_t *index, uint32_t generation);

int ac_query(const ac_index_t *index, const sr_pattern_t *pattern,
             uint32_t generation, ac_query_result_t *out);

int ac_set_generation_floor(ac_index_t *index, uint32_t floor);
void ac_reboot(ac_index_t *index);

#endif /* HERUS_ADAPTIVE_CHANGE_H */
