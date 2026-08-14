/* threat_model.h — executable evidence classifier for HERUS threat boundaries.
 *
 * This is a pure risk-evidence auditor. It is not a security monitor, cryptographic
 * primitive, radio client, vault client, telemetry sink, model adapter or policy
 * engine. It receives no key, packet, card, candidate, text, audio, transcript,
 * embedding, identity, location, timestamp, model output or callback. Its result
 * never authorises send, retention, deletion, pairing, presentation or an action.
 */
#ifndef HERUS_THREAT_MODEL_H
#define HERUS_THREAT_MODEL_H

#include <stdint.h>

typedef enum {
    THREAT_MODEL_RADIO_ACTIVE = 0,
    THREAT_MODEL_RADIO_METADATA,
    THREAT_MODEL_COMPANION_TRUST,
    THREAT_MODEL_MEMORY_RETENTION,
    THREAT_MODEL_MEMORY_RECOVERY,
    THREAT_MODEL_MODEL_AGENCY,
    THREAT_MODEL_TELEMETRY_PRIVACY,
    THREAT_MODEL_PHYSICAL_PLATFORM,
    THREAT_MODEL_SUPPLY_CHAIN,
    THREAT_MODEL_COUNT
} threat_model_threat_t;

/* Classification is evidence-scoped, not a probability, impact score or security
 * guarantee. PENDING_TARGET and OUT_OF_SCOPE are first-class safe results. */
typedef enum {
    THREAT_MODEL_MITIGATED_HOST = 0,
    THREAT_MODEL_PENDING_TARGET,
    THREAT_MODEL_OUT_OF_SCOPE
} threat_model_evidence_t;

/* All flags are canonical booleans. Exactly 1 provides only the named evidence
 * to this audit; it never creates a probability, signature or trust boundary. The
 * physical-platform fields and local supply-chain integrity flag are intentionally
 * never enough to emit MITIGATED_HOST from portable C. */
typedef struct {
    uint8_t radio_aead;
    uint8_t radio_replay_refused;
    uint8_t radio_rate_limited;
    uint8_t radio_flood_bounded;
    uint8_t radio_constant_airtime;

    uint8_t companion_pairing_bound;
    uint8_t companion_link_authenticated;
    uint8_t companion_link_fresh;
    uint8_t companion_revocation_dominates;

    uint8_t memory_capture_gated;
    uint8_t memory_policy_selective;
    uint8_t memory_human_authority;
    uint8_t memory_vault_authenticated;
    uint8_t memory_generation_monotonic;
    uint8_t memory_sensitive_reviewed;
    uint8_t memory_conflict_blocks;
    uint8_t memory_recovery_topology;
    uint8_t memory_collection_composed;
    uint8_t memory_physical_session_bound;
    uint8_t memory_retrieval_access_gated;
    uint8_t memory_ambiguity_preserved;
    uint8_t memory_presentation_one_shot;

    uint8_t model_display_only;
    uint8_t model_no_memory_authority;
    uint8_t model_no_send_authority;

    uint8_t telemetry_numeric_only;
    uint8_t telemetry_forbidden_absent;

    uint8_t target_secure_boot;
    uint8_t target_flash_encrypted;
    uint8_t target_jtag_disabled;
    uint8_t target_nvs_protected;
    uint8_t target_power_loss_tested;

    uint8_t supply_chain_local_integrity;
} threat_model_snapshot_t;

typedef enum {
    THREAT_MODEL_FAIL_NONE              = 0u,
    THREAT_MODEL_FAIL_FORMAT            = 1u << 0,
    THREAT_MODEL_FAIL_RADIO_AEAD        = 1u << 1,
    THREAT_MODEL_FAIL_RADIO_REPLAY      = 1u << 2,
    THREAT_MODEL_FAIL_RADIO_RATE        = 1u << 3,
    THREAT_MODEL_FAIL_RADIO_FLOOD       = 1u << 4,
    THREAT_MODEL_FAIL_RADIO_AIRTIME     = 1u << 5,
    THREAT_MODEL_FAIL_TRUST_PAIRING     = 1u << 6,
    THREAT_MODEL_FAIL_TRUST_AUTH        = 1u << 7,
    THREAT_MODEL_FAIL_TRUST_FRESH       = 1u << 8,
    THREAT_MODEL_FAIL_TRUST_REVOKED     = 1u << 9,
    THREAT_MODEL_FAIL_MEMORY_CAPTURE    = 1u << 10,
    THREAT_MODEL_FAIL_MEMORY_POLICY     = 1u << 11,
    THREAT_MODEL_FAIL_MEMORY_AUTHORITY  = 1u << 12,
    THREAT_MODEL_FAIL_MEMORY_VAULT      = 1u << 13,
    THREAT_MODEL_FAIL_MEMORY_GENERATION = 1u << 14,
    THREAT_MODEL_FAIL_MEMORY_SENSITIVE  = 1u << 15,
    THREAT_MODEL_FAIL_MEMORY_CONFLICT   = 1u << 16,
    THREAT_MODEL_FAIL_RETRIEVAL_ACCESS  = 1u << 17,
    THREAT_MODEL_FAIL_RETRIEVAL_AMBIG   = 1u << 18,
    THREAT_MODEL_FAIL_PRESENTATION      = 1u << 19,
    THREAT_MODEL_FAIL_MODEL_DISPLAY     = 1u << 20,
    THREAT_MODEL_FAIL_MODEL_MEMORY      = 1u << 21,
    THREAT_MODEL_FAIL_MODEL_SEND        = 1u << 22,
    THREAT_MODEL_FAIL_TELEMETRY_NUMERIC = 1u << 23,
    THREAT_MODEL_FAIL_TELEMETRY_PRIVACY = 1u << 24,
    THREAT_MODEL_FAIL_TARGET_PENDING    = 1u << 25,
    THREAT_MODEL_FAIL_SCOPE_UNSUPPORTED = 1u << 26,
    THREAT_MODEL_FAIL_MEMORY_RECOVERY   = 1u << 27,
    THREAT_MODEL_FAIL_SUPPLY_INTEGRITY  = 1u << 28,
    THREAT_MODEL_FAIL_COLLECTION_FINALE = 1u << 29,
    THREAT_MODEL_FAIL_PHYSICAL_SESSION  = 1u << 30
} threat_model_failure_t;

typedef struct {
    threat_model_evidence_t evidence;
    uint32_t                failures; /* OR of threat_model_failure_t */
    uint8_t                 host_mitigated; /* exactly 1 only for MITIGATED_HOST */
} threat_model_decision_t;

enum {
    THREAT_MODEL_OK = 0,
    THREAT_MODEL_E_ARG = -1,
    THREAT_MODEL_E_BLOCKED = -2
};

/* Evaluates evidence for exactly one named threat. Missing, noncanonical or
 * contradictory evidence fails closed. This function only classifies evidence; it
 * does not claim likelihood, residual impact or protection beyond the named scope. */
int threat_model_assess(threat_model_threat_t threat,
                        const threat_model_snapshot_t *snapshot,
                        threat_model_decision_t *out);

#endif /* HERUS_THREAT_MODEL_H */
