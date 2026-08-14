/* test_memory_collection_recovery_stress.c — F2 deterministic hostile collection recovery campaign. */
#include "memory_collection_recovery.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define F2_SEED 0xF2C011ECu
#define F2_STRUCTURED_ROUNDS 100000u
#define F2_RAW_ROUNDS 20000u
#define F2_TEMPLATE_COUNT 7u

static uint32_t RNG = F2_SEED;
static uint32_t ACCEPTED = 0u;
static uint32_t REJECTED = 0u;
static uint32_t ACTIONS[MEMORY_COLLECTION_RECOVERY_BLOCKED + 1u];
static int FAILED = 0;

static void fail_once(const char *what)
{
    if (!FAILED) printf("  FAIL %s\n", what);
    FAILED = 1;
}

static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

static uint32_t next_u32(void)
{
    uint32_t x = RNG;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    RNG = x;
    return x;
}

static uint32_t pick_generation(void)
{
    static const uint32_t values[] = {0u, 1u, 2u, 7u, 8u, 31u, UINT32_MAX - 1u, UINT32_MAX};
    return values[next_u32() % (sizeof(values) / sizeof(values[0]))];
}

static uint8_t pick_boolish(void)
{
    static const uint8_t values[] = {0u, 1u, 2u, 0x7fu, 0xffu};
    return values[next_u32() % (sizeof(values) / sizeof(values[0]))];
}

static void structured_snapshot(memory_collection_recovery_snapshot_t *s)
{
    memset(s, 0, sizeof(*s));
    s->committed_present = pick_boolish();
    s->prepared_present = pick_boolish();
    s->committed_authenticated = pick_boolish();
    s->prepared_authenticated = pick_boolish();
    s->prepared_matches_committed = pick_boolish();
    s->committed_generation = pick_generation();
    s->prepared_generation = pick_generation();
    s->prepared_base_generation = pick_generation();
    s->durable_generation_floor = pick_generation();
}

static void raw_snapshot(memory_collection_recovery_snapshot_t *s)
{
    size_t i;
    uint8_t *bytes = (uint8_t *)s;
    for (i = 0u; i < sizeof(*s); ++i)
        bytes[i] = (uint8_t)next_u32();
}

static memory_collection_recovery_snapshot_t committed(uint32_t generation)
{
    memory_collection_recovery_snapshot_t s;
    memset(&s, 0, sizeof(s));
    s.committed_present = 1u;
    s.committed_authenticated = 1u;
    s.committed_generation = generation;
    s.durable_generation_floor = generation;
    return s;
}

static memory_collection_recovery_snapshot_t prepared(uint32_t base, uint32_t generation,
                                                       uint32_t floor)
{
    memory_collection_recovery_snapshot_t s;
    memset(&s, 0, sizeof(s));
    s.prepared_present = 1u;
    s.prepared_authenticated = 1u;
    s.prepared_generation = generation;
    s.prepared_base_generation = base;
    s.durable_generation_floor = floor;
    return s;
}

static memory_collection_recovery_snapshot_t template_at(uint32_t index)
{
    memory_collection_recovery_snapshot_t s;
    switch (index) {
    case 0u:
        memset(&s, 0, sizeof(s));
        return s;
    case 1u:
        return committed(1u);
    case 2u:
        return committed(UINT32_MAX);
    case 3u:
        return prepared(0u, 1u, 0u);
    case 4u:
        return prepared(0u, 1u, 1u);
    case 5u:
        s = committed(7u);
        s.prepared_present = 1u;
        s.prepared_authenticated = 1u;
        s.prepared_generation = 8u;
        s.prepared_base_generation = 7u;
        return s;
    default:
        s = committed(9u);
        s.prepared_present = 1u;
        s.prepared_authenticated = 1u;
        s.prepared_matches_committed = 1u;
        s.prepared_generation = 9u;
        s.prepared_base_generation = 8u;
        return s;
    }
}

static int action_valid(memory_collection_recovery_action_t action)
{
    return action >= MEMORY_COLLECTION_RECOVERY_EMPTY &&
           action <= MEMORY_COLLECTION_RECOVERY_BLOCKED;
}

static int success_shape_is_safe(const memory_collection_recovery_snapshot_t *s,
                                 memory_collection_recovery_action_t action)
{
    if (action == MEMORY_COLLECTION_RECOVERY_EMPTY)
        return s->committed_present == 0u && s->prepared_present == 0u &&
               s->durable_generation_floor == 0u;
    if (action == MEMORY_COLLECTION_RECOVERY_USE_COMMITTED)
        return s->committed_present == 1u && s->prepared_present == 0u &&
               s->committed_generation == s->durable_generation_floor;
    if (action == MEMORY_COLLECTION_RECOVERY_PROMOTE_PREPARED)
        return s->prepared_present == 1u &&
               s->prepared_generation == s->durable_generation_floor;
    if (action == MEMORY_COLLECTION_RECOVERY_FINALIZE_PREPARED)
        return s->committed_present == 1u && s->prepared_present == 1u &&
               s->prepared_matches_committed == 1u &&
               s->prepared_generation == s->durable_generation_floor;
    if (action == MEMORY_COLLECTION_RECOVERY_DISCARD_PREPARED)
        return s->prepared_present == 1u;
    return 0;
}

static void exercise_snapshot(const memory_collection_recovery_snapshot_t *source)
{
    memory_collection_recovery_snapshot_t s = *source;
    memory_collection_recovery_snapshot_t before = s;
    memory_collection_recovery_action_t action = MEMORY_COLLECTION_RECOVERY_BLOCKED;
    int rc = memory_collection_recovery_assess(&s, &action);

    if (memcmp(&s, &before, sizeof(s)) != 0) {
        fail_once("F2 collection recovery oracle mutated its caller-supplied snapshot");
        return;
    }
    if (rc == MEMORY_COLLECTION_RECOVERY_OK) {
        ACCEPTED++;
        if (!action_valid(action) || action == MEMORY_COLLECTION_RECOVERY_BLOCKED ||
            !success_shape_is_safe(&s, action)) {
            fail_once("F2 collection recovery emitted an action incompatible with its input topology");
            return;
        }
        ACTIONS[action]++;
        return;
    }
    if (rc == MEMORY_COLLECTION_RECOVERY_E_INVALID &&
        action == MEMORY_COLLECTION_RECOVERY_BLOCKED) {
        REJECTED++;
        return;
    }
    fail_once("F2 collection recovery returned an unsupported status for non-null input");
}

static void mutate_templates(void)
{
    uint32_t template_index;
    for (template_index = 0u; template_index < F2_TEMPLATE_COUNT; ++template_index) {
        memory_collection_recovery_snapshot_t s = template_at(template_index);
        size_t byte_index;
        uint8_t bit;
        exercise_snapshot(&s);
        for (byte_index = 0u; byte_index < sizeof(s); ++byte_index) {
            for (bit = 0u; bit < 8u; ++bit) {
                memory_collection_recovery_snapshot_t mutation = s;
                ((uint8_t *)&mutation)[byte_index] ^= (uint8_t)(1u << bit);
                exercise_snapshot(&mutation);
            }
        }
    }
}

int main(void)
{
    memory_collection_recovery_snapshot_t s;
    memory_collection_recovery_action_t action = MEMORY_COLLECTION_RECOVERY_EMPTY;
    uint32_t i;

    printf("\n== F2 deterministic hostile collection-recovery campaign ==\n");
    ok(memory_collection_recovery_assess(0, &action) == MEMORY_COLLECTION_RECOVERY_E_ARG &&
       memory_collection_recovery_assess(&s, 0) == MEMORY_COLLECTION_RECOVERY_E_ARG,
       "F2 null oracle inputs fail before any permissive collection decision");

    mutate_templates();
    for (i = 0u; i < F2_STRUCTURED_ROUNDS; ++i) {
        structured_snapshot(&s);
        exercise_snapshot(&s);
    }
    for (i = 0u; i < F2_RAW_ROUNDS; ++i) {
        raw_snapshot(&s);
        exercise_snapshot(&s);
    }

    ok(!FAILED && ACCEPTED >= F2_TEMPLATE_COUNT && REJECTED != 0u,
       "F2 generated accepted and rejected collection topologies without unsafe action shape");
    ok(ACTIONS[MEMORY_COLLECTION_RECOVERY_EMPTY] != 0u &&
       ACTIONS[MEMORY_COLLECTION_RECOVERY_USE_COMMITTED] != 0u &&
       ACTIONS[MEMORY_COLLECTION_RECOVERY_PROMOTE_PREPARED] != 0u &&
       ACTIONS[MEMORY_COLLECTION_RECOVERY_FINALIZE_PREPARED] != 0u &&
       ACTIONS[MEMORY_COLLECTION_RECOVERY_DISCARD_PREPARED] != 0u,
       "F2 exercised every permitted collection-recovery action");

    printf("  INFO seed=0x%08x structured=%u raw=%u accepted=%u rejected=%u\n",
           F2_SEED, F2_STRUCTURED_ROUNDS, F2_RAW_ROUNDS, ACCEPTED, REJECTED);
    if (FAILED) {
        printf("COLLECTION RECOVERY STRESS TESTS FAILED\n");
        return 1;
    }
    printf("COLLECTION RECOVERY STRESS INVARIANTS HOLD — hostile collection snapshots yield a bounded action or fail closed.\n");
    return 0;
}
