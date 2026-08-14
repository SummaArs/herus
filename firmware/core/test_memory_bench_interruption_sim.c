/* test_memory_bench_interruption_sim.c — host-only interruption plan simulator. */
#include "memory_collection_recovery.h"
#include "memory_physical_session_bootstrap.h"
#include "memory_physical_session_recovery.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int FAILED = 0;
static uint32_t CASES = 0u;
static uint32_t PASSED = 0u;

static void expect(int condition, const char *label)
{
    CASES++;
    if (condition) {
        PASSED++;
        printf("  PASS %s\n", label);
    } else {
        FAILED = 1;
        printf("  FAIL %s\n", label);
    }
}

static memory_physical_session_recovery_snapshot_t physical_base(void)
{
    memory_physical_session_recovery_snapshot_t s;
    memset(&s, 0, sizeof(s));
    return s;
}

static int gate_is_quarantined(const memory_physical_session_t *gate)
{
    return gate->state == MEMORY_PHYSICAL_SESSION_IDLE &&
           gate->active_session_id == 0u &&
           gate->active_event_nonce == 0u &&
           gate->active_purpose == MEMORY_PHYSICAL_PURPOSE_NONE &&
           gate->started_at_ms == 0u && gate->expires_at_ms == 0u &&
           gate->uses_remaining == 0u;
}

static uint32_t expected_floor(const memory_physical_session_recovery_snapshot_t *s,
                               memory_physical_session_recovery_action_t action)
{
    switch (action) {
    case MEMORY_PHYSICAL_SESSION_RECOVERY_USE_COMMITTED:
    case MEMORY_PHYSICAL_SESSION_RECOVERY_FINALIZE_PREPARED:
        return s->committed_reservation_id;
    case MEMORY_PHYSICAL_SESSION_RECOVERY_PROMOTE_PREPARED:
        return s->prepared_reservation_id;
    case MEMORY_PHYSICAL_SESSION_RECOVERY_DISCARD_PREPARED:
        return s->durable_reservation_floor;
    case MEMORY_PHYSICAL_SESSION_RECOVERY_EMPTY:
    case MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED:
    default:
        return 0u;
    }
}

static void expect_physical(const char *label,
                            const memory_physical_session_recovery_snapshot_t *snapshot,
                            int expected_rc,
                            memory_physical_session_recovery_action_t expected_action)
{
    memory_physical_session_config_t cfg;
    memory_physical_session_t gate;
    memory_physical_session_bootstrap_result_t result;
    int rc;

    memory_physical_session_config_default(&cfg);
    rc = memory_physical_session_bootstrap(&gate, &cfg, snapshot, &result);
    if (expected_rc != MEMORY_PHYSICAL_SESSION_BOOTSTRAP_OK) {
        expect(rc == expected_rc && gate.state == MEMORY_PHYSICAL_SESSION_BLOCKED &&
                   result.active_evidence_scrubbed == 0u &&
                   result.recovery_action == MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED,
               label);
        return;
    }

    expect(rc == MEMORY_PHYSICAL_SESSION_BOOTSTRAP_OK &&
               result.recovery_action == expected_action &&
               result.recovered_session_floor == expected_floor(snapshot, expected_action) &&
               result.active_evidence_scrubbed == 1u && gate_is_quarantined(&gate) &&
               gate.session_floor == result.recovered_session_floor,
           label);
}

static void expect_collection(const char *label,
                              const memory_collection_recovery_snapshot_t *snapshot,
                              int expected_rc,
                              memory_collection_recovery_action_t expected_action)
{
    memory_collection_recovery_action_t action = MEMORY_COLLECTION_RECOVERY_BLOCKED;
    int rc = memory_collection_recovery_assess(snapshot, &action);
    expect(rc == expected_rc && action == expected_action, label);
}

static void collection_cases(void)
{
    memory_collection_recovery_snapshot_t s;

    memset(&s, 0, sizeof(s));
    expect_collection("collection interruption before PREPARED -> EMPTY", &s,
                      MEMORY_COLLECTION_RECOVERY_OK, MEMORY_COLLECTION_RECOVERY_EMPTY);

    memset(&s, 0, sizeof(s));
    s.committed_present = 1u;
    s.committed_authenticated = 1u;
    s.committed_generation = 7u;
    s.durable_generation_floor = 7u;
    expect_collection("collection interruption after COMMITTED -> USE_COMMITTED", &s,
                      MEMORY_COLLECTION_RECOVERY_OK, MEMORY_COLLECTION_RECOVERY_USE_COMMITTED);

    memset(&s, 0, sizeof(s));
    s.prepared_present = 1u;
    s.prepared_authenticated = 1u;
    s.prepared_generation = 1u;
    s.durable_generation_floor = 0u;
    expect_collection("collection interruption with orphan first PREPARED -> DISCARD", &s,
                      MEMORY_COLLECTION_RECOVERY_OK, MEMORY_COLLECTION_RECOVERY_DISCARD_PREPARED);

    memset(&s, 0, sizeof(s));
    s.committed_present = 1u;
    s.committed_authenticated = 1u;
    s.committed_generation = 7u;
    s.prepared_present = 1u;
    s.prepared_authenticated = 1u;
    s.prepared_generation = 8u;
    s.prepared_base_generation = 7u;
    s.durable_generation_floor = 7u;
    expect_collection("collection interruption before promotion -> DISCARD successor", &s,
                      MEMORY_COLLECTION_RECOVERY_OK, MEMORY_COLLECTION_RECOVERY_DISCARD_PREPARED);
    s.durable_generation_floor = 8u;
    expect_collection("collection interruption after floor burn -> PROMOTE successor", &s,
                      MEMORY_COLLECTION_RECOVERY_OK, MEMORY_COLLECTION_RECOVERY_PROMOTE_PREPARED);

    s.prepared_generation = 7u;
    s.prepared_base_generation = 6u;
    s.prepared_matches_committed = 1u;
    s.durable_generation_floor = 7u;
    expect_collection("collection cleanup interrupted with equal records -> FINALIZE", &s,
                      MEMORY_COLLECTION_RECOVERY_OK, MEMORY_COLLECTION_RECOVERY_FINALIZE_PREPARED);

    s.committed_authenticated = 0u;
    expect_collection("collection unauthenticated interruption -> BLOCKED", &s,
                      MEMORY_COLLECTION_RECOVERY_E_INVALID, MEMORY_COLLECTION_RECOVERY_BLOCKED);
}

static void physical_cases(void)
{
    memory_physical_session_recovery_snapshot_t s;

    s = physical_base();
    expect_physical("session boot before any reservation -> idle floor zero", &s,
                    MEMORY_PHYSICAL_SESSION_BOOTSTRAP_OK,
                    MEMORY_PHYSICAL_SESSION_RECOVERY_EMPTY);

    s = physical_base();
    s.committed_present = 1u;
    s.committed_authenticated = 1u;
    s.committed_reservation_id = 7u;
    s.committed_purpose = MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY;
    s.committed_uses = 3u;
    s.durable_reservation_floor = 7u;
    expect_physical("session boot after COMMITTED -> idle floor committed", &s,
                    MEMORY_PHYSICAL_SESSION_BOOTSTRAP_OK,
                    MEMORY_PHYSICAL_SESSION_RECOVERY_USE_COMMITTED);

    s = physical_base();
    s.prepared_present = 1u;
    s.prepared_authenticated = 1u;
    s.prepared_reservation_id = 1u;
    s.prepared_purpose = MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY;
    s.prepared_uses = 3u;
    s.durable_reservation_floor = 0u;
    expect_physical("session orphan first PREPARED -> idle floor zero", &s,
                    MEMORY_PHYSICAL_SESSION_BOOTSTRAP_OK,
                    MEMORY_PHYSICAL_SESSION_RECOVERY_DISCARD_PREPARED);

    s = physical_base();
    s.committed_present = 1u;
    s.committed_authenticated = 1u;
    s.committed_reservation_id = 7u;
    s.committed_purpose = MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY;
    s.committed_uses = 3u;
    s.prepared_present = 1u;
    s.prepared_authenticated = 1u;
    s.prepared_reservation_id = 8u;
    s.prepared_base_reservation_id = 7u;
    s.prepared_purpose = MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY;
    s.prepared_uses = 3u;
    s.durable_reservation_floor = 8u;
    expect_physical("session floor burn then boot -> idle promoted floor", &s,
                    MEMORY_PHYSICAL_SESSION_BOOTSTRAP_OK,
                    MEMORY_PHYSICAL_SESSION_RECOVERY_PROMOTE_PREPARED);

    s.prepared_reservation_id = 7u;
    s.prepared_base_reservation_id = 6u;
    s.prepared_purpose = MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY;
    s.prepared_uses = 3u;
    s.prepared_matches_committed = 1u;
    s.durable_reservation_floor = 7u;
    expect_physical("session cleanup interrupted -> idle finalized floor", &s,
                    MEMORY_PHYSICAL_SESSION_BOOTSTRAP_OK,
                    MEMORY_PHYSICAL_SESSION_RECOVERY_FINALIZE_PREPARED);

    s.committed_authenticated = 0u;
    expect_physical("session unauthenticated marker -> blocked scrubbed gate", &s,
                    MEMORY_PHYSICAL_SESSION_BOOTSTRAP_E_RECOVERY,
                    MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED);

    s = physical_base();
    s.committed_present = 1u;
    s.committed_authenticated = 1u;
    s.committed_reservation_id = UINT32_MAX;
    s.committed_purpose = MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY;
    s.committed_uses = 3u;
    s.durable_reservation_floor = UINT32_MAX;
    expect_physical("session terminal floor -> blocked before boot", &s,
                    MEMORY_PHYSICAL_SESSION_BOOTSTRAP_E_RECOVERY,
                    MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED);
}

int main(void)
{
    printf("\n== HOST-ONLY BENCH INTERRUPTION SIMULATOR ==\n");
    printf("This executes portable oracles; it observes no reset, media, GPIO, radio or power event.\n");
    collection_cases();
    physical_cases();
    printf("  INFO cases=%u passed=%u\n", CASES, PASSED);
    if (FAILED) {
        printf("BENCH INTERRUPTION SIMULATION FAILED\n");
        return 1;
    }
    printf("BENCH INTERRUPTION SIMULATION INVARIANTS HOLD — physical gates remain unmeasured.\n");
    return 0;
}
