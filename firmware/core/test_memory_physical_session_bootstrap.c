/* test_memory_physical_session_bootstrap.c — post-reboot session quarantine invariants. */
#include "memory_physical_session_bootstrap.h"

#include <stdio.h>
#include <string.h>

static int FAILED = 0;

static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

static memory_physical_session_recovery_snapshot_t committed(
    uint32_t id, memory_physical_purpose_t purpose, uint8_t uses)
{
    memory_physical_session_recovery_snapshot_t s;
    memset(&s, 0, sizeof(s));
    s.committed_present = 1u;
    s.committed_authenticated = 1u;
    s.committed_reservation_id = id;
    s.committed_purpose = purpose;
    s.committed_uses = uses;
    s.durable_reservation_floor = id;
    return s;
}

static int gate_is_idle_and_scrubbed(const memory_physical_session_t *gate,
                                     uint32_t floor)
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

static int bootstrap(memory_physical_session_t *gate,
                     const memory_physical_session_config_t *cfg,
                     const memory_physical_session_recovery_snapshot_t *snapshot,
                     memory_physical_session_bootstrap_result_t *out,
                     memory_physical_session_recovery_action_t action,
                     uint32_t floor)
{
    return memory_physical_session_bootstrap(gate, cfg, snapshot, out) ==
               MEMORY_PHYSICAL_SESSION_BOOTSTRAP_OK &&
           out->recovery_action == action && out->recovered_session_floor == floor &&
           out->active_evidence_scrubbed == 1u && gate_is_idle_and_scrubbed(gate, floor);
}

int main(void)
{
    memory_physical_session_config_t cfg;
    memory_physical_session_recovery_snapshot_t s;
    memory_physical_session_bootstrap_result_t out;
    memory_physical_session_t gate;

    memory_physical_session_config_default(&cfg);
    printf("\n== T16 post-reboot bootstrap imports a floor, never session authority ==\n");

    memset(&s, 0, sizeof(s));
    ok(bootstrap(&gate, &cfg, &s, &out, MEMORY_PHYSICAL_SESSION_RECOVERY_EMPTY, 0u),
       "T16 empty recovery starts an idle, fully scrubbed gate at floor zero");
    ok(memory_physical_session_validate(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT,
                                        1u, 1u) == MEMORY_PHYSICAL_SESSION_E_STATE &&
       memory_physical_session_consume(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT,
                                       1u, 1u) == MEMORY_PHYSICAL_SESSION_E_STATE,
       "T16 empty bootstrap cannot validate or consume a session that was never newly begun");
    ok(memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT,
                                     1u, 11u, 1u, 1u, 1u) == MEMORY_PHYSICAL_SESSION_OK,
       "T16 a first session still requires an explicit new canonical event assertion");

    ok(memory_physical_session_init(&gate, &cfg) == MEMORY_PHYSICAL_SESSION_OK &&
       memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                                     97u, 98u, 1u, 2u, 10u) == MEMORY_PHYSICAL_SESSION_OK,
       "T16 fixture establishes pre-reboot active evidence that bootstrap must erase");
    s = committed(7u, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY, 2u);
    ok(bootstrap(&gate, &cfg, &s, &out,
                 MEMORY_PHYSICAL_SESSION_RECOVERY_USE_COMMITTED, 7u),
       "T16 committed floor replaces an active pre-reboot session with idle quarantine");
    ok(memory_physical_session_validate(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                                        97u, 11u) == MEMORY_PHYSICAL_SESSION_E_STATE &&
       memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                                     7u, 12u, 1u, 2u, 11u) == MEMORY_PHYSICAL_SESSION_E_FORMAT,
       "T16 neither the old active ID nor the recovered floor ID can regain authority");
    ok(memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                                     8u, 12u, 0u, 2u, 11u) == MEMORY_PHYSICAL_SESSION_E_FORMAT &&
       memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                                     8u, 12u, 1u, 2u, 11u) == MEMORY_PHYSICAL_SESSION_OK,
       "T16 only a successor ID with a new canonical event assertion can begin after bootstrap");

    s = committed(7u, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY, 2u);
    s.prepared_present = 1u;
    s.prepared_authenticated = 1u;
    s.prepared_reservation_id = 8u;
    s.prepared_base_reservation_id = 7u;
    s.prepared_purpose = MEMORY_PHYSICAL_PURPOSE_COLLECTION_REMOVE;
    s.prepared_uses = 1u;
    s.durable_reservation_floor = 8u;
    ok(bootstrap(&gate, &cfg, &s, &out,
                 MEMORY_PHYSICAL_SESSION_RECOVERY_PROMOTE_PREPARED, 8u),
       "T16 promoted prepared marker imports only its burned successor floor");
    ok(memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_REMOVE,
                                     8u, 13u, 1u, 1u, 12u) == MEMORY_PHYSICAL_SESSION_E_FORMAT &&
       memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_REMOVE,
                                     9u, 13u, 1u, 1u, 12u) == MEMORY_PHYSICAL_SESSION_OK,
       "T16 promotion forbids reuse of the prepared ID and does not carry its purpose or use");

    s = committed(8u, MEMORY_PHYSICAL_PURPOSE_COLLECTION_REMOVE, 1u);
    s.prepared_present = 1u;
    s.prepared_authenticated = 1u;
    s.prepared_reservation_id = 9u;
    s.prepared_base_reservation_id = 8u;
    s.prepared_purpose = MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT;
    s.prepared_uses = 1u;
    s.durable_reservation_floor = 8u;
    ok(bootstrap(&gate, &cfg, &s, &out,
                 MEMORY_PHYSICAL_SESSION_RECOVERY_DISCARD_PREPARED, 8u),
       "T16 pre-floor prepared marker is discarded while its declared prior floor remains quarantined");
    ok(memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT,
                                     9u, 14u, 0u, 1u, 13u) == MEMORY_PHYSICAL_SESSION_E_FORMAT &&
       memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT,
                                     9u, 14u, 1u, 1u, 13u) == MEMORY_PHYSICAL_SESSION_OK,
       "T16 discard creates no authority; reuse is possible only through a new asserted event beyond retained floor");

    s = committed(9u, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY, 2u);
    s.prepared_present = 1u;
    s.prepared_authenticated = 1u;
    s.prepared_matches_committed = 1u;
    s.prepared_reservation_id = 9u;
    s.prepared_base_reservation_id = 8u;
    s.prepared_purpose = MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY;
    s.prepared_uses = 2u;
    ok(bootstrap(&gate, &cfg, &s, &out,
                 MEMORY_PHYSICAL_SESSION_RECOVERY_FINALIZE_PREPARED, 9u),
       "T16 cleanup finalization leaves a scrubbed idle gate, not the matching session active");

    s = committed(3u, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 1u);
    s.committed_authenticated = 0u;
    ok(memory_physical_session_init(&gate, &cfg) == MEMORY_PHYSICAL_SESSION_OK &&
       memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT,
                                     31u, 32u, 1u, 1u, 1u) == MEMORY_PHYSICAL_SESSION_OK &&
       memory_physical_session_bootstrap(&gate, &cfg, &s, &out) ==
           MEMORY_PHYSICAL_SESSION_BOOTSTRAP_E_RECOVERY &&
       out.recovery_action == MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED &&
       out.recovered_session_floor == 0u && out.active_evidence_scrubbed == 0u &&
       gate.state == MEMORY_PHYSICAL_SESSION_BLOCKED && gate.active_session_id == 0u &&
       gate.active_event_nonce == 0u &&
       memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT,
                                     4u, 5u, 1u, 1u, 2u) == MEMORY_PHYSICAL_SESSION_E_STATE,
       "T16 contradictory recovery blocks and scrubs even an already active in-RAM fixture");

    memset(&s, 0, sizeof(s));
    cfg.window_ms = 0u;
    ok(memory_physical_session_bootstrap(&gate, &cfg, &s, &out) ==
           MEMORY_PHYSICAL_SESSION_BOOTSTRAP_E_CONFIG &&
       out.recovery_action == MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED &&
       gate.state == MEMORY_PHYSICAL_SESSION_BLOCKED && gate.active_session_id == 0u,
       "T16 invalid session configuration blocks rather than retaining stale authority");
    memory_physical_session_config_default(&cfg);

    memset(&s, 0, sizeof(s));
    ok(memory_physical_session_bootstrap(0, &cfg, &s, &out) ==
           MEMORY_PHYSICAL_SESSION_BOOTSTRAP_E_ARG &&
       out.recovery_action == MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED &&
       memory_physical_session_init(&gate, &cfg) == MEMORY_PHYSICAL_SESSION_OK &&
       memory_physical_session_bootstrap(&gate, &cfg, &s, 0) ==
           MEMORY_PHYSICAL_SESSION_BOOTSTRAP_E_ARG &&
       gate.state == MEMORY_PHYSICAL_SESSION_BLOCKED,
       "T16 missing bootstrap inputs never yield a permissive result or preserve gate state");

    if (FAILED) {
        printf("PHYSICAL SESSION BOOTSTRAP TESTS FAILED\n");
        return 1;
    }
    printf("PHYSICAL SESSION BOOTSTRAP INVARIANTS HOLD — recovery imports only a floor and no pre-reboot session can revive.\n");
    return 0;
}
