#include "memory_reboot_boundary.h"

#include <string.h>

static void scrub_index(mse_index_t *index)
{
    mse_functional_predicate_fn predicate;
    void *user;
    if (!index) return;
    predicate = index->is_functional;
    user = index->policy_user;
    mse_init(index, predicate, user);
}

static void scrub_failure(memory_physical_session_t *gate,
                          mse_index_t *index,
                          magic_trigger_t *trigger,
                          memory_reboot_boundary_result_t *out)
{
    if (gate) {
        memset(gate, 0, sizeof(*gate));
        gate->state = MEMORY_PHYSICAL_SESSION_BLOCKED;
    }
    scrub_index(index);
    magic_trigger_close(trigger);
    if (out) memset(out, 0, sizeof(*out));
}

int memory_reboot_boundary_bootstrap(
    memory_physical_session_t *gate,
    mse_index_t *index,
    magic_trigger_t *trigger,
    const memory_physical_session_config_t *session_cfg,
    const memory_physical_session_recovery_snapshot_t *snapshot,
    memory_reboot_boundary_result_t *out)
{
    memory_physical_session_bootstrap_result_t bootstrap;
    int rc;

    if (out) memset(out, 0, sizeof(*out));
    if (!gate || !index || !trigger || !session_cfg || !snapshot || !out) {
        scrub_failure(gate, index, trigger, out);
        return MEMORY_REBOOT_BOUNDARY_E_ARG;
    }

    /* Volatile evidence is never allowed to cross the reboot boundary, including
     * when recovery later fails. Preserve only the caller's static predicate hook. */
    scrub_index(index);
    magic_trigger_close(trigger);
    rc = memory_physical_session_bootstrap(gate, session_cfg, snapshot, &bootstrap);
    if (rc != MEMORY_PHYSICAL_SESSION_BOOTSTRAP_OK ||
        bootstrap.active_evidence_scrubbed != 1u) {
        scrub_failure(gate, index, trigger, out);
        return MEMORY_REBOOT_BOUNDARY_E_RECOVERY;
    }
    if (bootstrap.recovered_session_floor != 0u &&
        mse_set_generation_floor(index, bootstrap.recovered_session_floor) < MSE_OK) {
        scrub_failure(gate, index, trigger, out);
        return MEMORY_REBOOT_BOUNDARY_E_RECOVERY;
    }

    out->recovered_session_floor = bootstrap.recovered_session_floor;
    out->semantic_generation_floor = bootstrap.recovered_session_floor;
    out->recovery_action = bootstrap.recovery_action;
    out->active_session_scrubbed = bootstrap.active_evidence_scrubbed;
    out->semantic_index_scrubbed = 1u;
    out->contextual_window_scrubbed = 1u;
    return MEMORY_REBOOT_BOUNDARY_OK;
}
