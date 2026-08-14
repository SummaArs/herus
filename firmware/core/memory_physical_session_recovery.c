/* memory_physical_session_recovery.c — deterministic reservation recovery. */
#include "memory_physical_session_recovery.h"

static int canonical_bool(uint8_t value)
{
    return value == 0u || value == 1u;
}

static int valid_purpose(memory_physical_purpose_t purpose)
{
    return purpose == MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT ||
           purpose == MEMORY_PHYSICAL_PURPOSE_COLLECTION_OPEN ||
           purpose == MEMORY_PHYSICAL_PURPOSE_COLLECTION_REMOVE ||
           purpose == MEMORY_PHYSICAL_PURPOSE_COLLECTION_COMPACT ||
           purpose == MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY;
}

static int valid_uses(memory_physical_purpose_t purpose, uint8_t uses)
{
    if (!valid_purpose(purpose)) return 0;
    if (purpose == MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY) {
        return uses >= 1u && uses <= MEMORY_PHYSICAL_SESSION_MAX_QUERY_USES;
    }
    return uses == 1u;
}

static int absent_committed_is_zero(const memory_physical_session_recovery_snapshot_t *s)
{
    return s->committed_authenticated == 0u &&
           s->committed_reservation_id == 0u &&
           s->committed_purpose == MEMORY_PHYSICAL_PURPOSE_NONE &&
           s->committed_uses == 0u;
}

static int absent_prepared_is_zero(const memory_physical_session_recovery_snapshot_t *s)
{
    return s->prepared_authenticated == 0u &&
           s->prepared_reservation_id == 0u &&
           s->prepared_base_reservation_id == 0u &&
           s->prepared_purpose == MEMORY_PHYSICAL_PURPOSE_NONE &&
           s->prepared_uses == 0u;
}

static int snapshot_is_canonical(const memory_physical_session_recovery_snapshot_t *s)
{
    if (!canonical_bool(s->committed_present) ||
        !canonical_bool(s->prepared_present) ||
        !canonical_bool(s->committed_authenticated) ||
        !canonical_bool(s->prepared_authenticated) ||
        !canonical_bool(s->prepared_matches_committed)) {
        return 0;
    }
    if (s->committed_present == 0u && !absent_committed_is_zero(s)) return 0;
    if (s->prepared_present == 0u && !absent_prepared_is_zero(s)) return 0;
    if (s->committed_present != 0u &&
        (s->committed_authenticated != 1u ||
         s->committed_reservation_id == 0u ||
         !valid_uses(s->committed_purpose, s->committed_uses))) {
        return 0;
    }
    if (s->prepared_present != 0u &&
        (s->prepared_authenticated != 1u ||
         s->prepared_reservation_id == 0u ||
         s->prepared_base_reservation_id >= s->prepared_reservation_id ||
         !valid_uses(s->prepared_purpose, s->prepared_uses))) {
        return 0;
    }
    if ((s->committed_present == 0u || s->prepared_present == 0u) &&
        s->prepared_matches_committed != 0u) {
        return 0;
    }
    return 1;
}

int memory_physical_session_recovery_assess(
    const memory_physical_session_recovery_snapshot_t *snapshot,
    memory_physical_session_recovery_action_t *out_action)
{
    if (!snapshot || !out_action) return MEMORY_PHYSICAL_SESSION_RECOVERY_E_ARG;
    *out_action = MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED;
    if (!snapshot_is_canonical(snapshot)) return MEMORY_PHYSICAL_SESSION_RECOVERY_E_INVALID;

    if (snapshot->committed_present == 0u && snapshot->prepared_present == 0u) {
        if (snapshot->durable_reservation_floor != 0u) {
            return MEMORY_PHYSICAL_SESSION_RECOVERY_E_INVALID;
        }
        *out_action = MEMORY_PHYSICAL_SESSION_RECOVERY_EMPTY;
        return MEMORY_PHYSICAL_SESSION_RECOVERY_OK;
    }

    if (snapshot->committed_present != 0u && snapshot->prepared_present == 0u) {
        if (snapshot->committed_reservation_id != snapshot->durable_reservation_floor) {
            return MEMORY_PHYSICAL_SESSION_RECOVERY_E_INVALID;
        }
        *out_action = MEMORY_PHYSICAL_SESSION_RECOVERY_USE_COMMITTED;
        return MEMORY_PHYSICAL_SESSION_RECOVERY_OK;
    }

    if (snapshot->committed_present == 0u) {
        if (snapshot->prepared_base_reservation_id != 0u) {
            return MEMORY_PHYSICAL_SESSION_RECOVERY_E_INVALID;
        }
        if (snapshot->durable_reservation_floor == 0u) {
            if (snapshot->prepared_reservation_id != 1u) {
                return MEMORY_PHYSICAL_SESSION_RECOVERY_E_INVALID;
            }
            *out_action = MEMORY_PHYSICAL_SESSION_RECOVERY_DISCARD_PREPARED;
            return MEMORY_PHYSICAL_SESSION_RECOVERY_OK;
        }
        if (snapshot->prepared_reservation_id != snapshot->durable_reservation_floor) {
            return MEMORY_PHYSICAL_SESSION_RECOVERY_E_INVALID;
        }
        *out_action = MEMORY_PHYSICAL_SESSION_RECOVERY_PROMOTE_PREPARED;
        return MEMORY_PHYSICAL_SESSION_RECOVERY_OK;
    }

    if (snapshot->prepared_reservation_id == snapshot->committed_reservation_id) {
        if (snapshot->prepared_matches_committed != 1u ||
            snapshot->durable_reservation_floor != snapshot->committed_reservation_id) {
            return MEMORY_PHYSICAL_SESSION_RECOVERY_E_INVALID;
        }
        *out_action = MEMORY_PHYSICAL_SESSION_RECOVERY_FINALIZE_PREPARED;
        return MEMORY_PHYSICAL_SESSION_RECOVERY_OK;
    }

    if (snapshot->prepared_matches_committed != 0u ||
        snapshot->prepared_base_reservation_id != snapshot->committed_reservation_id ||
        snapshot->prepared_reservation_id != snapshot->committed_reservation_id + 1u) {
        return MEMORY_PHYSICAL_SESSION_RECOVERY_E_INVALID;
    }
    if (snapshot->durable_reservation_floor == snapshot->committed_reservation_id) {
        *out_action = MEMORY_PHYSICAL_SESSION_RECOVERY_DISCARD_PREPARED;
        return MEMORY_PHYSICAL_SESSION_RECOVERY_OK;
    }
    if (snapshot->durable_reservation_floor == snapshot->prepared_reservation_id) {
        *out_action = MEMORY_PHYSICAL_SESSION_RECOVERY_PROMOTE_PREPARED;
        return MEMORY_PHYSICAL_SESSION_RECOVERY_OK;
    }
    return MEMORY_PHYSICAL_SESSION_RECOVERY_E_INVALID;
}
