/* memory_prehardware_finale.c — final host-only audit of the post-reboot memory chain. */
#include "memory_prehardware_finale.h"

#include <string.h>

static void block_gate(memory_physical_session_t *gate)
{
    if (!gate) return;
    memset(gate, 0, sizeof(*gate));
    gate->state = MEMORY_PHYSICAL_SESSION_BLOCKED;
}

static void reset_out(memory_prehardware_finale_decision_t *out)
{
    if (!out) return;
    memset(out, 0, sizeof(*out));
    out->recovery_action = MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED;
}

static int gate_is_quarantined(const memory_physical_session_t *gate, uint32_t floor)
{
    return gate && gate->state == MEMORY_PHYSICAL_SESSION_IDLE &&
           gate->session_floor == floor && gate->active_session_id == 0u &&
           gate->active_event_nonce == 0u &&
           gate->active_purpose == MEMORY_PHYSICAL_PURPOSE_NONE &&
           gate->started_at_ms == 0u && gate->expires_at_ms == 0u &&
           gate->uses_remaining == 0u && gate->metrics.begun == 0u &&
           gate->metrics.consumed == 0u && gate->metrics.cancelled == 0u &&
           gate->metrics.expired == 0u && gate->metrics.rejected_format == 0u &&
           gate->metrics.rejected_state == 0u && gate->metrics.rejected_purpose == 0u &&
           gate->metrics.rejected_assertion == 0u && gate->metrics.rejected_time == 0u;
}

int memory_prehardware_finale_audit(
    memory_physical_session_t *gate,
    const memory_prehardware_finale_input_t *input,
    memory_prehardware_finale_decision_t *out)
{
    memory_physical_session_bootstrap_result_t bootstrap;
    memory_collection_finale_decision_t collection;
    threat_model_decision_t threat;
    uint32_t failures = MEMORY_PREHARDWARE_FINALE_FAIL_NONE;
    int rc;

    reset_out(out);
    if (!gate || !input || !out || !input->session_config ||
        !input->reservation_snapshot || !input->collection_snapshot ||
        !input->threat_snapshot) {
        block_gate(gate);
        return MEMORY_PREHARDWARE_FINALE_E_ARG;
    }

    rc = memory_physical_session_bootstrap(gate, input->session_config,
                                           input->reservation_snapshot, &bootstrap);
    if (rc != MEMORY_PHYSICAL_SESSION_BOOTSTRAP_OK) {
        failures |= MEMORY_PREHARDWARE_FINALE_FAIL_BOOTSTRAP;
    } else {
        out->recovered_session_floor = bootstrap.recovered_session_floor;
        out->recovery_action = bootstrap.recovery_action;
        if (bootstrap.active_evidence_scrubbed != 1u ||
            !gate_is_quarantined(gate, bootstrap.recovered_session_floor)) {
            failures |= MEMORY_PREHARDWARE_FINALE_FAIL_GATE_QUARANTINE;
        }
    }

    if (memory_collection_finale_audit(input->collection_snapshot, &collection) !=
            MEMORY_COLLECTION_FINALE_OK ||
        collection.chain_consistent != 1u ||
        collection.failures != MEMORY_COLLECTION_FINALE_FAIL_NONE) {
        failures |= MEMORY_PREHARDWARE_FINALE_FAIL_COLLECTION;
    }

    if (threat_model_assess(THREAT_MODEL_MEMORY_RETENTION, input->threat_snapshot, &threat) !=
            THREAT_MODEL_OK ||
        threat.evidence != THREAT_MODEL_MITIGATED_HOST ||
        threat.host_mitigated != 1u || threat.failures != THREAT_MODEL_FAIL_NONE) {
        failures |= MEMORY_PREHARDWARE_FINALE_FAIL_THREAT_MODEL;
    }

    out->failures = failures;
    if (failures != MEMORY_PREHARDWARE_FINALE_FAIL_NONE) {
        block_gate(gate);
        return MEMORY_PREHARDWARE_FINALE_E_BLOCKED;
    }

    out->ready_for_target_validation = 1u;
    return MEMORY_PREHARDWARE_FINALE_OK;
}
