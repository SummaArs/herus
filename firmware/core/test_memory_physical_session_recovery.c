/* test_memory_physical_session_recovery.c — durable reservation recovery invariants. */
#include "memory_physical_session_recovery.h"

#include <stdio.h>
#include <string.h>

static int FAILED = 0;

static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

static memory_physical_session_recovery_snapshot_t committed(uint32_t id,
                                                              memory_physical_purpose_t purpose,
                                                              uint8_t uses)
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

static memory_physical_session_recovery_snapshot_t prepared(uint32_t base,
                                                             uint32_t id,
                                                             memory_physical_purpose_t purpose,
                                                             uint8_t uses,
                                                             uint32_t floor)
{
    memory_physical_session_recovery_snapshot_t s;
    memset(&s, 0, sizeof(s));
    s.prepared_present = 1u;
    s.prepared_authenticated = 1u;
    s.prepared_reservation_id = id;
    s.prepared_base_reservation_id = base;
    s.prepared_purpose = purpose;
    s.prepared_uses = uses;
    s.durable_reservation_floor = floor;
    return s;
}

static int assess(memory_physical_session_recovery_snapshot_t *s,
                  memory_physical_session_recovery_action_t expected)
{
    memory_physical_session_recovery_action_t action = MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED;
    return memory_physical_session_recovery_assess(s, &action) ==
               MEMORY_PHYSICAL_SESSION_RECOVERY_OK &&
           action == expected;
}

static int blocks(memory_physical_session_recovery_snapshot_t *s)
{
    memory_physical_session_recovery_action_t action = MEMORY_PHYSICAL_SESSION_RECOVERY_EMPTY;
    return memory_physical_session_recovery_assess(s, &action) ==
               MEMORY_PHYSICAL_SESSION_RECOVERY_E_INVALID &&
           action == MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED;
}

int main(void)
{
    memory_physical_session_recovery_snapshot_t s;
    memory_physical_session_recovery_action_t action;

    printf("\n== T15 physical-session reservation recovery never revives pre-reboot authority ==\n");

    memset(&s, 0, sizeof(s));
    ok(assess(&s, MEMORY_PHYSICAL_SESSION_RECOVERY_EMPTY),
       "T15 empty durable reservation state is the only allowed initial topology");

    s = committed(7u, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY, 3u);
    ok(assess(&s, MEMORY_PHYSICAL_SESSION_RECOVERY_USE_COMMITTED),
       "T15 authenticated committed reservation restores only its replay floor, never an active capability");

    s = prepared(0u, 1u, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 1u, 0u);
    ok(assess(&s, MEMORY_PHYSICAL_SESSION_RECOVERY_DISCARD_PREPARED),
       "T15 first prepared reservation before durable floor commit is discarded");

    s = prepared(0u, 1u, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 1u, 1u);
    ok(assess(&s, MEMORY_PHYSICAL_SESSION_RECOVERY_PROMOTE_PREPARED),
       "T15 first prepared reservation anchored by the new floor can be promoted only as burned state");

    s = committed(8u, MEMORY_PHYSICAL_PURPOSE_COLLECTION_OPEN, 1u);
    s.prepared_present = 1u;
    s.prepared_authenticated = 1u;
    s.prepared_reservation_id = 9u;
    s.prepared_base_reservation_id = 8u;
    s.prepared_purpose = MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY;
    s.prepared_uses = 2u;
    s.durable_reservation_floor = 8u;
    ok(assess(&s, MEMORY_PHYSICAL_SESSION_RECOVERY_DISCARD_PREPARED),
       "T15 successor prepared before its durable floor commit is discarded without consuming new authority");

    s.durable_reservation_floor = 9u;
    ok(assess(&s, MEMORY_PHYSICAL_SESSION_RECOVERY_PROMOTE_PREPARED),
       "T15 immediate authenticated successor bound to floor is promoted as a consumed reservation");

    s = committed(9u, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY, 2u);
    s.prepared_present = 1u;
    s.prepared_authenticated = 1u;
    s.prepared_matches_committed = 1u;
    s.prepared_reservation_id = 9u;
    s.prepared_base_reservation_id = 8u;
    s.prepared_purpose = MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY;
    s.prepared_uses = 2u;
    ok(assess(&s, MEMORY_PHYSICAL_SESSION_RECOVERY_FINALIZE_PREPARED),
       "T15 matching prepared/committed pair finalizes cleanup without restoring a session");

    s.prepared_matches_committed = 0u;
    ok(blocks(&s),
       "T15 same reservation ID without authenticated full equality blocks instead of guessing cleanup");

    s = prepared(0u, 2u, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 1u, 0u);
    ok(blocks(&s),
       "T15 an initial reservation cannot skip the first durable ID");

    s = prepared(7u, 9u, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 1u, 8u);
    ok(blocks(&s),
       "T15 a prepared reservation must be the immediate successor of its base");

    s = committed(5u, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 1u);
    s.committed_authenticated = 0u;
    ok(blocks(&s),
       "T15 unauthenticated committed reservation never establishes a replay floor");

    s = committed(5u, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY, 0u);
    ok(blocks(&s),
       "T15 query reservation with zero uses is noncanonical and blocks");

    s = committed(5u, MEMORY_PHYSICAL_PURPOSE_COLLECTION_REMOVE, 2u);
    ok(blocks(&s),
       "T15 mutation reservation cannot carry more than one use");

    s = committed(5u, MEMORY_PHYSICAL_PURPOSE_NONE, 1u);
    ok(blocks(&s),
       "T15 unknown or none purpose never survives reboot as a reservation");

    memset(&s, 0, sizeof(s));
    s.committed_present = 2u;
    ok(blocks(&s),
       "T15 noncanonical boolean evidence blocks before any recovery action");

    memset(&s, 0, sizeof(s));
    s.committed_reservation_id = 1u;
    ok(blocks(&s),
       "T15 absent record fields cannot smuggle a reservation into recovery");

    ok(memory_physical_session_recovery_assess(0, &action) ==
           MEMORY_PHYSICAL_SESSION_RECOVERY_E_ARG &&
           memory_physical_session_recovery_assess(&s, 0) ==
           MEMORY_PHYSICAL_SESSION_RECOVERY_E_ARG,
       "T15 missing recovery inputs never create a permissive decision");

    if (FAILED) {
        printf("PHYSICAL SESSION RECOVERY TESTS FAILED\n");
        return 1;
    }
    printf("PHYSICAL SESSION RECOVERY INVARIANTS HOLD — post-reboot reservation state never revives authority and every contradiction blocks.\n");
    return 0;
}
