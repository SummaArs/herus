/* threat_model.c — pure evidence classification for threat boundaries. */
#include "threat_model.h"
#include <string.h>

static int canonical(uint8_t value)
{
    return value == 0u || value == 1u;
}

static int snapshot_format_valid(const threat_model_snapshot_t *s)
{
    const uint8_t *const fields[] = {
        &s->radio_aead, &s->radio_replay_refused, &s->radio_rate_limited,
        &s->radio_flood_bounded, &s->radio_constant_airtime,
        &s->companion_pairing_bound, &s->companion_link_authenticated,
        &s->companion_link_fresh, &s->companion_revocation_dominates,
        &s->memory_capture_gated, &s->memory_policy_selective,
        &s->memory_human_authority, &s->memory_vault_authenticated,
        &s->memory_generation_monotonic, &s->memory_sensitive_reviewed,
        &s->memory_conflict_blocks, &s->memory_recovery_topology,
        &s->memory_retrieval_access_gated,
        &s->memory_ambiguity_preserved, &s->memory_presentation_one_shot,
        &s->model_display_only, &s->model_no_memory_authority,
        &s->model_no_send_authority, &s->telemetry_numeric_only,
        &s->telemetry_forbidden_absent, &s->target_secure_boot,
        &s->target_flash_encrypted, &s->target_jtag_disabled,
        &s->target_nvs_protected, &s->target_power_loss_tested
    };
    size_t i;
    for (i = 0u; i < sizeof(fields) / sizeof(fields[0]); ++i)
        if (!canonical(*fields[i])) return 0;
    return 1;
}

static void require_flag(uint8_t value, uint32_t bit, uint32_t *failures)
{
    if (value != 1u) *failures |= bit;
}

static int finish(threat_model_decision_t *out, uint32_t failures)
{
    out->failures = failures;
    if (failures == THREAT_MODEL_FAIL_NONE) {
        out->evidence = THREAT_MODEL_MITIGATED_HOST;
        out->host_mitigated = 1u;
        return THREAT_MODEL_OK;
    }
    if (failures == THREAT_MODEL_FAIL_TARGET_PENDING) {
        out->evidence = THREAT_MODEL_PENDING_TARGET;
    } else if (failures == THREAT_MODEL_FAIL_SCOPE_UNSUPPORTED) {
        out->evidence = THREAT_MODEL_OUT_OF_SCOPE;
    } else {
        out->evidence = THREAT_MODEL_PENDING_TARGET;
    }
    return THREAT_MODEL_E_BLOCKED;
}

int threat_model_assess(threat_model_threat_t threat,
                        const threat_model_snapshot_t *snapshot,
                        threat_model_decision_t *out)
{
    uint32_t failures = THREAT_MODEL_FAIL_NONE;
    if (!snapshot || !out) return THREAT_MODEL_E_ARG;
    memset(out, 0, sizeof(*out));
    if (threat < THREAT_MODEL_RADIO_ACTIVE || threat >= THREAT_MODEL_COUNT ||
        !snapshot_format_valid(snapshot)) {
        out->failures = THREAT_MODEL_FAIL_FORMAT;
        out->evidence = THREAT_MODEL_PENDING_TARGET;
        return THREAT_MODEL_E_BLOCKED;
    }

    switch (threat) {
    case THREAT_MODEL_RADIO_ACTIVE:
        require_flag(snapshot->radio_aead, THREAT_MODEL_FAIL_RADIO_AEAD, &failures);
        require_flag(snapshot->radio_replay_refused, THREAT_MODEL_FAIL_RADIO_REPLAY, &failures);
        require_flag(snapshot->radio_rate_limited, THREAT_MODEL_FAIL_RADIO_RATE, &failures);
        require_flag(snapshot->radio_flood_bounded, THREAT_MODEL_FAIL_RADIO_FLOOD, &failures);
        break;
    case THREAT_MODEL_RADIO_METADATA:
        require_flag(snapshot->radio_constant_airtime, THREAT_MODEL_FAIL_RADIO_AIRTIME, &failures);
        if (failures == THREAT_MODEL_FAIL_NONE)
            failures = THREAT_MODEL_FAIL_SCOPE_UNSUPPORTED;
        break;
    case THREAT_MODEL_COMPANION_TRUST:
        require_flag(snapshot->companion_pairing_bound, THREAT_MODEL_FAIL_TRUST_PAIRING, &failures);
        require_flag(snapshot->companion_link_authenticated, THREAT_MODEL_FAIL_TRUST_AUTH, &failures);
        require_flag(snapshot->companion_link_fresh, THREAT_MODEL_FAIL_TRUST_FRESH, &failures);
        require_flag(snapshot->companion_revocation_dominates, THREAT_MODEL_FAIL_TRUST_REVOKED, &failures);
        break;
    case THREAT_MODEL_MEMORY_RETENTION:
        require_flag(snapshot->memory_capture_gated, THREAT_MODEL_FAIL_MEMORY_CAPTURE, &failures);
        require_flag(snapshot->memory_policy_selective, THREAT_MODEL_FAIL_MEMORY_POLICY, &failures);
        require_flag(snapshot->memory_human_authority, THREAT_MODEL_FAIL_MEMORY_AUTHORITY, &failures);
        require_flag(snapshot->memory_vault_authenticated, THREAT_MODEL_FAIL_MEMORY_VAULT, &failures);
        require_flag(snapshot->memory_generation_monotonic, THREAT_MODEL_FAIL_MEMORY_GENERATION, &failures);
        require_flag(snapshot->memory_sensitive_reviewed, THREAT_MODEL_FAIL_MEMORY_SENSITIVE, &failures);
        require_flag(snapshot->memory_conflict_blocks, THREAT_MODEL_FAIL_MEMORY_CONFLICT, &failures);
        require_flag(snapshot->memory_recovery_topology, THREAT_MODEL_FAIL_MEMORY_RECOVERY, &failures);
        break;
    case THREAT_MODEL_MEMORY_RECOVERY:
        require_flag(snapshot->memory_retrieval_access_gated, THREAT_MODEL_FAIL_RETRIEVAL_ACCESS, &failures);
        require_flag(snapshot->memory_ambiguity_preserved, THREAT_MODEL_FAIL_RETRIEVAL_AMBIG, &failures);
        require_flag(snapshot->memory_presentation_one_shot, THREAT_MODEL_FAIL_PRESENTATION, &failures);
        break;
    case THREAT_MODEL_MODEL_AGENCY:
        require_flag(snapshot->model_display_only, THREAT_MODEL_FAIL_MODEL_DISPLAY, &failures);
        require_flag(snapshot->model_no_memory_authority, THREAT_MODEL_FAIL_MODEL_MEMORY, &failures);
        require_flag(snapshot->model_no_send_authority, THREAT_MODEL_FAIL_MODEL_SEND, &failures);
        break;
    case THREAT_MODEL_TELEMETRY_PRIVACY:
        require_flag(snapshot->telemetry_numeric_only, THREAT_MODEL_FAIL_TELEMETRY_NUMERIC, &failures);
        require_flag(snapshot->telemetry_forbidden_absent, THREAT_MODEL_FAIL_TELEMETRY_PRIVACY, &failures);
        break;
    case THREAT_MODEL_PHYSICAL_PLATFORM:
        failures = THREAT_MODEL_FAIL_TARGET_PENDING;
        break;
    case THREAT_MODEL_SUPPLY_CHAIN:
        failures = THREAT_MODEL_FAIL_SCOPE_UNSUPPORTED;
        break;
    default:
        failures = THREAT_MODEL_FAIL_FORMAT;
        break;
    }
    return finish(out, failures);
}
