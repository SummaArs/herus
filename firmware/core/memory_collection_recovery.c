/* memory_collection_recovery.c — side-effect-free crash-recovery oracle. */
#include "memory_collection_recovery.h"

static int canonical(uint8_t value)
{
    return value == 0u || value == 1u;
}

static int snapshot_valid(const memory_collection_recovery_snapshot_t *s)
{
    if (!s || !canonical(s->committed_present) || !canonical(s->prepared_present) ||
        !canonical(s->committed_authenticated) ||
        !canonical(s->prepared_authenticated) ||
        !canonical(s->prepared_matches_committed))
        return 0;
    if (!s->committed_present &&
        (s->committed_authenticated != 0u || s->committed_generation != 0u))
        return 0;
    if (!s->prepared_present &&
        (s->prepared_authenticated != 0u || s->prepared_matches_committed != 0u ||
         s->prepared_generation != 0u || s->prepared_base_generation != 0u))
        return 0;
    if (s->committed_present &&
        (s->committed_authenticated != 1u || s->committed_generation == 0u))
        return 0;
    if (s->prepared_present &&
        (s->prepared_authenticated != 1u || s->prepared_generation == 0u ||
         s->prepared_base_generation >= s->prepared_generation))
        return 0;
    if (s->prepared_matches_committed &&
        (!s->committed_present || !s->prepared_present ||
         s->prepared_generation != s->committed_generation ||
         s->prepared_base_generation + 1u != s->prepared_generation))
        return 0;
    return 1;
}

int memory_collection_recovery_assess(
    const memory_collection_recovery_snapshot_t *snapshot,
    memory_collection_recovery_action_t *out_action)
{
    if (!out_action) return MEMORY_COLLECTION_RECOVERY_E_ARG;
    *out_action = MEMORY_COLLECTION_RECOVERY_BLOCKED;
    if (!snapshot) return MEMORY_COLLECTION_RECOVERY_E_ARG;
    if (!snapshot_valid(snapshot)) return MEMORY_COLLECTION_RECOVERY_E_INVALID;

    if (!snapshot->committed_present && !snapshot->prepared_present) {
        if (snapshot->durable_generation_floor != 0u)
            return MEMORY_COLLECTION_RECOVERY_E_INVALID;
        *out_action = MEMORY_COLLECTION_RECOVERY_EMPTY;
        return MEMORY_COLLECTION_RECOVERY_OK;
    }

    if (snapshot->prepared_present) {
        if (snapshot->committed_present && snapshot->prepared_matches_committed) {
            if (snapshot->durable_generation_floor != snapshot->prepared_generation)
                return MEMORY_COLLECTION_RECOVERY_E_INVALID;
            *out_action = MEMORY_COLLECTION_RECOVERY_FINALIZE_PREPARED;
            return MEMORY_COLLECTION_RECOVERY_OK;
        }
        if (!snapshot->committed_present) {
            if (snapshot->prepared_base_generation != 0u)
                return MEMORY_COLLECTION_RECOVERY_E_INVALID;
            if (snapshot->durable_generation_floor == snapshot->prepared_generation) {
                *out_action = MEMORY_COLLECTION_RECOVERY_PROMOTE_PREPARED;
                return MEMORY_COLLECTION_RECOVERY_OK;
            }
            if (snapshot->durable_generation_floor == 0u &&
                snapshot->prepared_generation == 1u) {
                *out_action = MEMORY_COLLECTION_RECOVERY_DISCARD_PREPARED;
                return MEMORY_COLLECTION_RECOVERY_OK;
            }
            return MEMORY_COLLECTION_RECOVERY_E_INVALID;
        }
        if (snapshot->prepared_base_generation != snapshot->committed_generation ||
            snapshot->prepared_generation != snapshot->committed_generation + 1u)
            return MEMORY_COLLECTION_RECOVERY_E_INVALID;
        if (snapshot->durable_generation_floor == snapshot->prepared_generation) {
            *out_action = MEMORY_COLLECTION_RECOVERY_PROMOTE_PREPARED;
            return MEMORY_COLLECTION_RECOVERY_OK;
        }
        if (snapshot->durable_generation_floor == snapshot->committed_generation) {
            *out_action = MEMORY_COLLECTION_RECOVERY_DISCARD_PREPARED;
            return MEMORY_COLLECTION_RECOVERY_OK;
        }
        return MEMORY_COLLECTION_RECOVERY_E_INVALID;
    }

    if (snapshot->committed_generation != snapshot->durable_generation_floor)
        return MEMORY_COLLECTION_RECOVERY_E_INVALID;
    *out_action = MEMORY_COLLECTION_RECOVERY_USE_COMMITTED;
    return MEMORY_COLLECTION_RECOVERY_OK;
}
