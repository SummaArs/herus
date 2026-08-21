/*
 * poisoning_guard.h — bounded defense for compositional and dormant poisoning.
 *
 * A bundle is not a new authority domain. It preserves source provenance ids,
 * intersects authority, binds an epoch and can emit only a local offer. A fresh
 * physical confirmation remains necessary for any action.
 */
#ifndef HERUS_POISONING_GUARD_H
#define HERUS_POISONING_GUARD_H

#include "authority_transition.h"

#define PG_MAX_ITEMS 4u

typedef enum {
    PG_OK = 0,
    PG_NO_CHANGE = 1,
    PG_E_ARG = -1,
    PG_E_STAGE = -2,
    PG_E_EPOCH = -3,
    PG_E_EXPIRED = -4,
    PG_E_CONFLICT = -5,
    PG_E_FULL = -6,
    PG_E_CONTEXT = -7,
    PG_E_AUTH = -8
} pg_status_t;

typedef struct {
    uint8_t item_count;
    uint32_t provenance_id[PG_MAX_ITEMS];
    uint32_t epoch;
    uint32_t generation;
    uint32_t valid_until_generation;
    uint32_t authority_intersection;
    at_source_t source;
    uint8_t conflict;
} pg_bundle_t;

void pg_init(pg_bundle_t *bundle, uint32_t epoch);

int pg_add_memory(pg_bundle_t *bundle, const at_capsule_t *memory,
                  uint32_t current_epoch, uint32_t generation);

int pg_trigger_context(const pg_bundle_t *bundle, uint32_t context_token,
                       uint32_t expected_context_token, uint32_t current_epoch,
                       uint32_t generation, at_capsule_t *out_offer);

int pg_grant_action(const pg_bundle_t *bundle, uint32_t current_epoch,
                    uint32_t local_scope, uint8_t physical_confirmation,
                    uint32_t generation, at_capsule_t *out_action);

#endif /* HERUS_POISONING_GUARD_H */
