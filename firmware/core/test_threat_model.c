/* test_threat_model.c — executable threat-evidence invariants. */
#include "threat_model.h"
#include <stdio.h>
#include <string.h>

static int FAILED = 0;

static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

static threat_model_snapshot_t host_controls(void)
{
    threat_model_snapshot_t s;
    memset(&s, 1, sizeof(s));
    return s;
}

static void expect_host(threat_model_threat_t threat, const char *what)
{
    threat_model_snapshot_t s = host_controls();
    threat_model_decision_t d;
    ok(threat_model_assess(threat, &s, &d) == THREAT_MODEL_OK &&
       d.evidence == THREAT_MODEL_MITIGATED_HOST && d.host_mitigated == 1u &&
       d.failures == THREAT_MODEL_FAIL_NONE, what);
}

int main(void)
{
    threat_model_snapshot_t s;
    threat_model_decision_t d;

    printf("\n== T9 executable threat model keeps evidence, scope and residual risk explicit ==\n");
    expect_host(THREAT_MODEL_RADIO_ACTIVE,
                "T9 authenticated radio, replay, rate and flood controls can be host-mitigated together");
    expect_host(THREAT_MODEL_COMPANION_TRUST,
                "T9 pairing, authentication, freshness and revocation are separate required trust evidence");
    expect_host(THREAT_MODEL_MEMORY_RETENTION,
                "T9 selective capture, policy, human authority, collection composition, physical-session binding and conflict controls compose for retention");
    expect_host(THREAT_MODEL_MEMORY_RECOVERY,
                "T9 access gating, ambiguity and one-shot presentation compose for recovery");
    expect_host(THREAT_MODEL_MODEL_AGENCY,
                "T9 model display-only, no-memory and no-send boundaries are independently required");
    expect_host(THREAT_MODEL_TELEMETRY_PRIVACY,
                "T9 numeric-only telemetry and forbidden-data absence are both required for host privacy evidence");

    s = host_controls();
    s.radio_replay_refused = 0u;
    ok(threat_model_assess(THREAT_MODEL_RADIO_ACTIVE, &s, &d) == THREAT_MODEL_E_BLOCKED &&
       d.evidence == THREAT_MODEL_PENDING_TARGET && d.host_mitigated == 0u &&
       (d.failures & THREAT_MODEL_FAIL_RADIO_REPLAY),
       "T9 loss of replay refusal cannot be hidden by encryption, rate limits or flood bounds");

    s = host_controls();
    s.memory_sensitive_reviewed = 0u;
    s.memory_conflict_blocks = 0u;
    ok(threat_model_assess(THREAT_MODEL_MEMORY_RETENTION, &s, &d) == THREAT_MODEL_E_BLOCKED &&
       (d.failures & THREAT_MODEL_FAIL_MEMORY_SENSITIVE) &&
       (d.failures & THREAT_MODEL_FAIL_MEMORY_CONFLICT) && !d.host_mitigated,
       "T9 sensitive review and conflict blocking remain mandatory even after human authority exists");

    s = host_controls();
    s.memory_recovery_topology = 0u;
    ok(threat_model_assess(THREAT_MODEL_MEMORY_RETENTION, &s, &d) == THREAT_MODEL_E_BLOCKED &&
       (d.failures & THREAT_MODEL_FAIL_MEMORY_RECOVERY) && !d.host_mitigated,
       "T9 retention loses host mitigation when crash recovery lacks an explicit safe topology");

    s = host_controls();
    s.memory_collection_composed = 0u;
    ok(threat_model_assess(THREAT_MODEL_MEMORY_RETENTION, &s, &d) == THREAT_MODEL_E_BLOCKED &&
       (d.failures & THREAT_MODEL_FAIL_COLLECTION_FINALE) && !d.host_mitigated,
       "T9 retention loses host mitigation when collection composition can bypass human authority, abstention or no-fallback evidence");

    s = host_controls();
    s.memory_physical_session_bound = 0u;
    ok(threat_model_assess(THREAT_MODEL_MEMORY_RETENTION, &s, &d) == THREAT_MODEL_E_BLOCKED &&
       (d.failures & THREAT_MODEL_FAIL_PHYSICAL_SESSION) && !d.host_mitigated,
       "T9 retention loses host mitigation when collection access can fall back to an unbound physical assertion");

    s = host_controls();
    s.memory_ambiguity_preserved = 0u;
    s.memory_presentation_one_shot = 0u;
    ok(threat_model_assess(THREAT_MODEL_MEMORY_RECOVERY, &s, &d) == THREAT_MODEL_E_BLOCKED &&
       (d.failures & THREAT_MODEL_FAIL_RETRIEVAL_AMBIG) &&
       (d.failures & THREAT_MODEL_FAIL_PRESENTATION),
       "T9 recovery cannot claim host mitigation if it hides uncertainty or repeats local status");

    s = host_controls();
    s.model_no_memory_authority = 0u;
    s.model_no_send_authority = 0u;
    ok(threat_model_assess(THREAT_MODEL_MODEL_AGENCY, &s, &d) == THREAT_MODEL_E_BLOCKED &&
       (d.failures & THREAT_MODEL_FAIL_MODEL_MEMORY) &&
       (d.failures & THREAT_MODEL_FAIL_MODEL_SEND),
       "T9 a future model loses mitigation classification if it gains memory or radio authority");

    s = host_controls();
    s.telemetry_forbidden_absent = 0u;
    ok(threat_model_assess(THREAT_MODEL_TELEMETRY_PRIVACY, &s, &d) == THREAT_MODEL_E_BLOCKED &&
       (d.failures & THREAT_MODEL_FAIL_TELEMETRY_PRIVACY),
       "T9 telemetry privacy fails closed when forbidden product data could enter diagnostics");

    s = host_controls();
    ok(threat_model_assess(THREAT_MODEL_RADIO_METADATA, &s, &d) == THREAT_MODEL_E_BLOCKED &&
       d.evidence == THREAT_MODEL_OUT_OF_SCOPE &&
       d.failures == THREAT_MODEL_FAIL_SCOPE_UNSUPPORTED && !d.host_mitigated,
       "T9 constant airtime does not become a false claim of hiding transmission presence or direction");
    ok(threat_model_assess(THREAT_MODEL_PHYSICAL_PLATFORM, &s, &d) == THREAT_MODEL_E_BLOCKED &&
       d.evidence == THREAT_MODEL_PENDING_TARGET &&
       d.failures == THREAT_MODEL_FAIL_TARGET_PENDING && !d.host_mitigated,
       "T9 portable evidence never upgrades secure boot, flash, JTAG, NVS or power-loss to host mitigation");
    ok(threat_model_assess(THREAT_MODEL_SUPPLY_CHAIN, &s, &d) == THREAT_MODEL_E_BLOCKED &&
       d.evidence == THREAT_MODEL_PENDING_TARGET && !d.host_mitigated &&
       d.failures == THREAT_MODEL_FAIL_TARGET_PENDING,
       "T9 a valid unsigned local digest remains pending until supply-chain provenance is authenticated");
    s = host_controls();
    s.supply_chain_local_integrity = 0u;
    ok(threat_model_assess(THREAT_MODEL_SUPPLY_CHAIN, &s, &d) == THREAT_MODEL_E_BLOCKED &&
       d.evidence == THREAT_MODEL_PENDING_TARGET &&
       (d.failures & THREAT_MODEL_FAIL_SUPPLY_INTEGRITY) &&
       (d.failures & THREAT_MODEL_FAIL_TARGET_PENDING) && !d.host_mitigated,
       "T9 supply-chain evidence fails closed when the local input digest control is absent");

    s = host_controls();
    s.companion_link_fresh = 2u;
    ok(threat_model_assess(THREAT_MODEL_COMPANION_TRUST, &s, &d) == THREAT_MODEL_E_BLOCKED &&
       d.evidence == THREAT_MODEL_PENDING_TARGET &&
       d.failures == THREAT_MODEL_FAIL_FORMAT && !d.host_mitigated,
       "T9 noncanonical evidence fails before a threat can be classified as protected");
    s = host_controls();
    ok(threat_model_assess((threat_model_threat_t)THREAT_MODEL_COUNT, &s, &d) == THREAT_MODEL_E_BLOCKED &&
       d.failures == THREAT_MODEL_FAIL_FORMAT,
       "T9 an unknown threat identifier cannot inherit a permissive classification");
    ok(threat_model_assess(THREAT_MODEL_RADIO_ACTIVE, 0, &d) == THREAT_MODEL_E_ARG &&
       threat_model_assess(THREAT_MODEL_RADIO_ACTIVE, &s, 0) == THREAT_MODEL_E_ARG,
       "T9 missing audit inputs never create an evidence decision");

    if (FAILED) {
        printf("THREAT MODEL TESTS FAILED\n");
        return 1;
    }
    printf("THREAT MODEL INVARIANTS HOLD — mitigation is evidence-scoped, host-only and fail-closed.\n");
    return 0;
}
