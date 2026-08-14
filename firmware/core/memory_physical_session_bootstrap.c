/* memory_physical_session_bootstrap.c — post-reboot floor-only session bootstrap. */
#include "memory_physical_session_bootstrap.h"

#include <string.h>

static void block_gate(memory_physical_session_t *gate)
{
    if (!gate) return;
    memset(gate, 0, sizeof(*gate));
    gate->state = MEMORY_PHYSICAL_SESSION_BLOCKED;
}

static void block_result(memory_physical_session_bootstrap_result_t *out)
{
    if (!out) return;
    memset(out, 0, sizeof(*out));
    out->recovery_action = MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED;
}

static int recovered_floor(const memory_physical_session_recovery_snapshot_t *snapshot,
                           memory_physical_session_recovery_action_t action,
                           uint32_t *out_floor)
{
    if (!snapshot || !out_floor) return MEMORY_PHYSICAL_SESSION_BOOTSTRAP_E_ARG;
    switch (action) {
    case MEMORY_PHYSICAL_SESSION_RECOVERY_EMPTY:
        *out_floor = 0u;
        return MEMORY_PHYSICAL_SESSION_BOOTSTRAP_OK;
    case MEMORY_PHYSICAL_SESSION_RECOVERY_USE_COMMITTED:
    case MEMORY_PHYSICAL_SESSION_RECOVERY_FINALIZE_PREPARED:
        *out_floor = snapshot->committed_reservation_id;
        return MEMORY_PHYSICAL_SESSION_BOOTSTRAP_OK;
    case MEMORY_PHYSICAL_SESSION_RECOVERY_PROMOTE_PREPARED:
        *out_floor = snapshot->prepared_reservation_id;
        return MEMORY_PHYSICAL_SESSION_BOOTSTRAP_OK;
    case MEMORY_PHYSICAL_SESSION_RECOVERY_DISCARD_PREPARED:
        *out_floor = snapshot->durable_reservation_floor;
        return MEMORY_PHYSICAL_SESSION_BOOTSTRAP_OK;
    case MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED:
    default:
        return MEMORY_PHYSICAL_SESSION_BOOTSTRAP_E_RECOVERY;
    }
}

int memory_physical_session_bootstrap(
    memory_physical_session_t *gate,
    const memory_physical_session_config_t *cfg,
    const memory_physical_session_recovery_snapshot_t *snapshot,
    memory_physical_session_bootstrap_result_t *out)
{
    memory_physical_session_recovery_action_t action;
    uint32_t floor;
    int rc;

    block_result(out);
    if (!gate || !cfg || !snapshot || !out) {
        block_gate(gate);
        return MEMORY_PHYSICAL_SESSION_BOOTSTRAP_E_ARG;
    }

    rc = memory_physical_session_init(gate, cfg);
    if (rc != MEMORY_PHYSICAL_SESSION_OK) {
        block_result(out);
        return MEMORY_PHYSICAL_SESSION_BOOTSTRAP_E_CONFIG;
    }

    rc = memory_physical_session_recovery_assess(snapshot, &action);
    if (rc != MEMORY_PHYSICAL_SESSION_RECOVERY_OK ||
        recovered_floor(snapshot, action, &floor) != MEMORY_PHYSICAL_SESSION_BOOTSTRAP_OK) {
        block_gate(gate);
        block_result(out);
        return MEMORY_PHYSICAL_SESSION_BOOTSTRAP_E_RECOVERY;
    }

    /* `memory_physical_session_init` has scrubbed all transient evidence. The
     * only post-reboot import is this floor; it is deliberately not a capability. */
    gate->session_floor = floor;
    out->recovered_session_floor = floor;
    out->recovery_action = action;
    out->active_evidence_scrubbed = 1u;
    return MEMORY_PHYSICAL_SESSION_BOOTSTRAP_OK;
}
