#include "memory_physical_session.h"
#include <stdio.h>
#include <string.h>

static int FAILED = 0;

static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

int main(void)
{
    memory_physical_session_config_t cfg;
    memory_physical_session_t gate;
    memory_physical_session_t bad_gate;
    const memory_physical_session_metrics_t *metrics;

    printf("\n== T14 physical-session gate remains purpose-bound, bounded and replay-resistant in RAM ==\n");
    memory_physical_session_config_default(&cfg);
    cfg.window_ms = 100u;
    cfg.max_query_uses = 3u;
    ok(memory_physical_session_init(&gate, &cfg) == MEMORY_PHYSICAL_SESSION_OK &&
       gate.state == MEMORY_PHYSICAL_SESSION_IDLE && gate.session_floor == 0u,
       "T14 portable gate starts empty without a gesture, identity, key or persisted session");

    ok(memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT,
                                     10u, 101u, 1u, 1u, 1000u) ==
           MEMORY_PHYSICAL_SESSION_OK && gate.state == MEMORY_PHYSICAL_SESSION_ACTIVE &&
       gate.active_purpose == MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT &&
       gate.uses_remaining == 1u,
       "T14 canonical adapter evidence starts one bounded insert-purpose session");
    ok(memory_physical_session_consume(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT,
                                       10u, 1001u) == MEMORY_PHYSICAL_SESSION_OK &&
       gate.state == MEMORY_PHYSICAL_SESSION_CONSUMED && gate.active_session_id == 0u &&
       gate.active_event_nonce == 0u,
       "T14 a successful insert use is consumed and transient ID/nonce evidence is scrubbed");
    ok(memory_physical_session_consume(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT,
                                       10u, 1002u) == MEMORY_PHYSICAL_SESSION_E_STATE,
       "T14 replay after a consumed single-use authorization cannot regain authority");
    ok(memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT,
                                     10u, 102u, 1u, 1u, 1003u) ==
           MEMORY_PHYSICAL_SESSION_E_FORMAT,
       "T14 a previously accepted session ID cannot be begun again before reboot");

    ok(memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_OPEN,
                                     11u, 111u, 1u, 1u, 1100u) == MEMORY_PHYSICAL_SESSION_OK &&
       memory_physical_session_validate(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_OPEN,
                                        11u, 1101u) == MEMORY_PHYSICAL_SESSION_OK &&
       gate.uses_remaining == 1u &&
       memory_physical_session_consume(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_REMOVE,
                                       11u, 1101u) == MEMORY_PHYSICAL_SESSION_E_PURPOSE &&
       gate.state == MEMORY_PHYSICAL_SESSION_ACTIVE &&
       memory_physical_session_consume(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_OPEN,
                                       11u, 1102u) == MEMORY_PHYSICAL_SESSION_OK,
       "T14 a wrong collection purpose cannot consume or redirect an active authorization");

    ok(memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                                     12u, 121u, 1u, 3u, 1200u) == MEMORY_PHYSICAL_SESSION_OK &&
       memory_physical_session_consume(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                                       12u, 1201u) == MEMORY_PHYSICAL_SESSION_OK &&
       gate.uses_remaining == 2u &&
       memory_physical_session_consume(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                                       12u, 1202u) == MEMORY_PHYSICAL_SESSION_OK &&
       memory_physical_session_consume(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                                       12u, 1203u) == MEMORY_PHYSICAL_SESSION_OK &&
       gate.state == MEMORY_PHYSICAL_SESSION_CONSUMED &&
       memory_physical_session_consume(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                                       12u, 1204u) == MEMORY_PHYSICAL_SESSION_E_STATE,
       "T14 only typed query purpose may have a bounded multi-use authorization and its final use consumes it");
    ok(memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                                     13u, 131u, 1u, 4u, 1300u) ==
           MEMORY_PHYSICAL_SESSION_E_ASSERTION &&
       memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_REMOVE,
                                     13u, 131u, 1u, 2u, 1300u) ==
           MEMORY_PHYSICAL_SESSION_E_ASSERTION,
       "T14 no purpose can exceed the local query bound or turn mutation into multi-use authority");

    ok(memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_COMPACT,
                                     14u, 141u, 1u, 1u, 2000u) == MEMORY_PHYSICAL_SESSION_OK &&
       memory_physical_session_consume(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_COMPACT,
                                       14u, 1999u) == MEMORY_PHYSICAL_SESSION_E_TIME &&
       gate.state == MEMORY_PHYSICAL_SESSION_ACTIVE &&
       memory_physical_session_consume(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_COMPACT,
                                       14u, 2101u) == MEMORY_PHYSICAL_SESSION_E_TIME &&
       gate.state == MEMORY_PHYSICAL_SESSION_EXPIRED && gate.active_event_nonce == 0u,
       "T14 time rollback is rejected and expiry scrubs evidence instead of accepting a late operation");

    ok(memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_REMOVE,
                                     15u, 151u, 1u, 1u, 2200u) == MEMORY_PHYSICAL_SESSION_OK &&
       memory_physical_session_cancel(&gate) == MEMORY_PHYSICAL_SESSION_OK &&
       gate.state == MEMORY_PHYSICAL_SESSION_CANCELLED &&
       memory_physical_session_consume(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_REMOVE,
                                       15u, 2201u) == MEMORY_PHYSICAL_SESSION_E_STATE,
       "T14 cancel revokes a pending authorization and never becomes a usable fallback");

    ok(memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT,
                                     16u, 0u, 1u, 1u, 2300u) == MEMORY_PHYSICAL_SESSION_E_FORMAT &&
       memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT,
                                     16u, 161u, 2u, 1u, 2300u) == MEMORY_PHYSICAL_SESSION_E_FORMAT &&
       memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_NONE,
                                     16u, 161u, 1u, 1u, 2300u) == MEMORY_PHYSICAL_SESSION_E_PURPOSE,
       "T14 empty nonce, noncanonical event evidence and unknown purpose fail before a session starts");

    ok(memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT,
                                     16u, 161u, 1u, 1u, UINT32_MAX - 50u) ==
           MEMORY_PHYSICAL_SESSION_E_TIME,
       "T14 overflowing expiry arithmetic cannot create an unbounded authorization");

    memset(&cfg, 0, sizeof(cfg));
    ok(memory_physical_session_init(&bad_gate, &cfg) == MEMORY_PHYSICAL_SESSION_E_CONFIG &&
       bad_gate.state == MEMORY_PHYSICAL_SESSION_BLOCKED,
       "T14 invalid configuration blocks rather than selecting an implicit session window or budget");
    ok(memory_physical_session_init(0, &cfg) == MEMORY_PHYSICAL_SESSION_E_ARG &&
       memory_physical_session_init(&bad_gate, 0) == MEMORY_PHYSICAL_SESSION_E_ARG,
       "T14 missing gate inputs never create a physical-session decision");

    metrics = memory_physical_session_metrics(&gate);
    ok(metrics && metrics->begun == 5u && metrics->consumed == 5u &&
       metrics->cancelled == 1u && metrics->expired == 1u &&
       memory_physical_session_metrics(0) == 0,
       "T14 metrics remain aggregate-only and expose neither session nor event evidence");

    if (FAILED) {
        printf("PHYSICAL SESSION INVARIANTS FAILED\n");
        return 1;
    }
    printf("PHYSICAL SESSION INVARIANTS HOLD — purpose, freshness and bounded use fail closed without claiming a physical gesture.\n");
    return 0;
}
