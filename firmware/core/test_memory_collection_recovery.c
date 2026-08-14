#include "memory_collection_recovery.h"
#include <stdio.h>
#include <string.h>

static int FAILED = 0;

static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

static memory_collection_recovery_snapshot_t empty_snapshot(void)
{
    memory_collection_recovery_snapshot_t s;
    memset(&s, 0, sizeof(s));
    return s;
}

static void expect_action(const memory_collection_recovery_snapshot_t *s,
                          memory_collection_recovery_action_t expected,
                          const char *what)
{
    memory_collection_recovery_action_t actual = MEMORY_COLLECTION_RECOVERY_BLOCKED;
    ok(memory_collection_recovery_assess(s, &actual) == MEMORY_COLLECTION_RECOVERY_OK &&
       actual == expected, what);
}

static void expect_blocked(const memory_collection_recovery_snapshot_t *s,
                           const char *what)
{
    memory_collection_recovery_action_t actual = MEMORY_COLLECTION_RECOVERY_EMPTY;
    ok(memory_collection_recovery_assess(s, &actual) == MEMORY_COLLECTION_RECOVERY_E_INVALID &&
       actual == MEMORY_COLLECTION_RECOVERY_BLOCKED, what);
}

int main(void)
{
    memory_collection_recovery_snapshot_t s;

    printf("\n== T12 recovery oracle distinguishes safe crash states from contradictions ==\n");

    s = empty_snapshot();
    expect_action(&s, MEMORY_COLLECTION_RECOVERY_EMPTY,
                  "T12 only zero floor plus no records is a new empty collection");

    s = empty_snapshot();
    s.committed_present = 1u;
    s.committed_authenticated = 1u;
    s.committed_generation = 4u;
    s.durable_generation_floor = 4u;
    expect_action(&s, MEMORY_COLLECTION_RECOVERY_USE_COMMITTED,
                  "T12 authenticated committed record exactly bound to floor remains active");

    s = empty_snapshot();
    s.prepared_present = 1u;
    s.prepared_authenticated = 1u;
    s.prepared_generation = 1u;
    expect_action(&s, MEMORY_COLLECTION_RECOVERY_DISCARD_PREPARED,
                  "T12 first prepared write before floor commit is discarded, not promoted");

    s = empty_snapshot();
    s.prepared_present = 1u;
    s.prepared_authenticated = 1u;
    s.prepared_generation = 1u;
    s.durable_generation_floor = 1u;
    expect_action(&s, MEMORY_COLLECTION_RECOVERY_PROMOTE_PREPARED,
                  "T12 first prepared write after floor commit is promoted deterministically");

    s = empty_snapshot();
    s.committed_present = 1u;
    s.committed_authenticated = 1u;
    s.committed_generation = 4u;
    s.prepared_present = 1u;
    s.prepared_authenticated = 1u;
    s.prepared_generation = 5u;
    s.prepared_base_generation = 4u;
    s.durable_generation_floor = 4u;
    expect_action(&s, MEMORY_COLLECTION_RECOVERY_DISCARD_PREPARED,
                  "T12 old floor retains committed generation and discards incomplete successor");

    s.durable_generation_floor = 5u;
    expect_action(&s, MEMORY_COLLECTION_RECOVERY_PROMOTE_PREPARED,
                  "T12 new floor permits only its authenticated immediate successor");

    s.prepared_generation = 5u;
    s.prepared_base_generation = 4u;
    s.committed_generation = 5u;
    s.prepared_matches_committed = 1u;
    expect_action(&s, MEMORY_COLLECTION_RECOVERY_FINALIZE_PREPARED,
                  "T12 duplicate prepared state after committed write only needs cleanup");

    s = empty_snapshot();
    s.committed_present = 1u;
    s.committed_authenticated = 1u;
    s.committed_generation = 4u;
    s.durable_generation_floor = 3u;
    expect_blocked(&s,
                   "T12 stale floor never silently reactivates a newer committed record");

    s = empty_snapshot();
    s.committed_present = 1u;
    s.committed_authenticated = 1u;
    s.committed_generation = 4u;
    s.prepared_present = 1u;
    s.prepared_authenticated = 1u;
    s.prepared_generation = 6u;
    s.prepared_base_generation = 4u;
    s.durable_generation_floor = 6u;
    expect_blocked(&s,
                   "T12 skipped generation is not a recoverable transaction topology");

    s = empty_snapshot();
    s.prepared_present = 1u;
    s.prepared_authenticated = 1u;
    s.prepared_generation = 2u;
    s.durable_generation_floor = 0u;
    expect_blocked(&s,
                   "T12 orphan prepared record beyond first generation is not discarded blindly");

    s = empty_snapshot();
    s.committed_present = 1u;
    s.committed_authenticated = 0u;
    s.committed_generation = 4u;
    s.durable_generation_floor = 4u;
    expect_blocked(&s,
                   "T12 present but unauthenticated committed state cannot enter recovery");

    s = empty_snapshot();
    s.committed_present = 1u;
    s.committed_authenticated = 1u;
    s.committed_generation = 5u;
    s.prepared_present = 1u;
    s.prepared_authenticated = 1u;
    s.prepared_generation = 5u;
    s.prepared_base_generation = 4u;
    s.durable_generation_floor = 5u;
    expect_blocked(&s,
                   "T12 same-generation records without authenticated equality are contradictory");

    s = empty_snapshot();
    s.committed_present = 2u;
    expect_blocked(&s,
                   "T12 noncanonical presence flag cannot be interpreted as evidence");

    if (FAILED) {
        printf("COLLECTION RECOVERY INVARIANTS FAILED\n");
        return 1;
    }
    printf("COLLECTION RECOVERY INVARIANTS HOLD — only floor-bound authenticated states recover.\n");
    return 0;
}
