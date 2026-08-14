/* test_memory_physical_session_recovery_stress.c — F1 deterministic hostile recovery campaign.
 *
 * This host-only campaign feeds synthetic reservation snapshots to the recovery
 * oracle and immediately composes every result with the post-reboot bootstrap.
 * It has no I/O, key, card, query, model, radio, clock, person, event or product
 * telemetry. Its fixed seed is a reproducibility handle, not a security nonce.
 */
#include "memory_physical_session_bootstrap.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define F1_SEED 0xF1A0C0DEu
#define F1_STRUCTURED_ROUNDS 100000u
#define F1_RAW_ROUNDS 20000u
#define F1_TEMPLATE_COUNT 8u

static int FAILED = 0;
static uint32_t RNG = F1_SEED;
static uint32_t ACCEPTED = 0u;
static uint32_t REJECTED = 0u;
static uint32_t ACTIONS[MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED + 1u];

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

static uint32_t pick_id(void)
{
    static const uint32_t ids[] = {0u, 1u, 2u, 7u, 8u, 31u, UINT32_MAX - 1u, UINT32_MAX};
    return ids[next_u32() % (sizeof(ids) / sizeof(ids[0]))];
}

static uint8_t pick_boolish(void)
{
    static const uint8_t values[] = {0u, 1u, 2u, 0x7fu, 0xffu};
    return values[next_u32() % (sizeof(values) / sizeof(values[0]))];
}

static memory_physical_purpose_t pick_purpose(void)
{
    static const uint8_t values[] = {0u, 1u, 2u, 3u, 4u, 5u, 6u, 0xffu};
    return (memory_physical_purpose_t)values[next_u32() % (sizeof(values) / sizeof(values[0]))];
}

static uint8_t pick_uses(void)
{
    static const uint8_t values[] = {0u, 1u, 2u, 3u, 8u, 9u, 0xffu};
    return values[next_u32() % (sizeof(values) / sizeof(values[0]))];
}

static void structured_snapshot(memory_physical_session_recovery_snapshot_t *s)
{
    memset(s, 0, sizeof(*s));
    s->committed_present = pick_boolish();
    s->prepared_present = pick_boolish();
    s->committed_authenticated = pick_boolish();
    s->prepared_authenticated = pick_boolish();
    s->prepared_matches_committed = pick_boolish();
    s->committed_reservation_id = pick_id();
    s->prepared_reservation_id = pick_id();
    s->prepared_base_reservation_id = pick_id();
    s->durable_reservation_floor = pick_id();
    s->committed_purpose = pick_purpose();
    s->prepared_purpose = pick_purpose();
    s->committed_uses = pick_uses();
    s->prepared_uses = pick_uses();
}

static void raw_snapshot(memory_physical_session_recovery_snapshot_t *s)
{
    size_t i;
    uint8_t *bytes = (uint8_t *)s;
    for (i = 0u; i < sizeof(*s); ++i)
        bytes[i] = (uint8_t)next_u32();
}

static memory_physical_session_recovery_snapshot_t committed(uint32_t id)
{
    memory_physical_session_recovery_snapshot_t s;
    memset(&s, 0, sizeof(s));
    s.committed_present = 1u;
    s.committed_authenticated = 1u;
    s.committed_reservation_id = id;
    s.durable_reservation_floor = id;
    s.committed_purpose = MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY;
    s.committed_uses = 2u;
    return s;
}

static memory_physical_session_recovery_snapshot_t prepared(uint32_t base, uint32_t id,
                                                             uint32_t floor)
{
    memory_physical_session_recovery_snapshot_t s;
    memset(&s, 0, sizeof(s));
    s.prepared_present = 1u;
    s.prepared_authenticated = 1u;
    s.prepared_reservation_id = id;
    s.prepared_base_reservation_id = base;
    s.durable_reservation_floor = floor;
    s.prepared_purpose = MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY;
    s.prepared_uses = 2u;
    return s;
}

static memory_physical_session_recovery_snapshot_t template_at(uint32_t index)
{
    memory_physical_session_recovery_snapshot_t s;
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
        s.prepared_reservation_id = 8u;
        s.prepared_base_reservation_id = 7u;
        s.prepared_purpose = MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY;
        s.prepared_uses = 2u;
        return s;
    case 6u:
        s = template_at(5u);
        s.durable_reservation_floor = 8u;
        return s;
    default:
        s = committed(9u);
        s.prepared_present = 1u;
        s.prepared_authenticated = 1u;
        s.prepared_matches_committed = 1u;
        s.prepared_reservation_id = 9u;
        s.prepared_base_reservation_id = 8u;
        s.prepared_purpose = MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY;
        s.prepared_uses = 2u;
        return s;
    }
}

static int action_valid(memory_physical_session_recovery_action_t action)
{
    return action >= MEMORY_PHYSICAL_SESSION_RECOVERY_EMPTY &&
           action <= MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED;
}

static uint32_t expected_floor(const memory_physical_session_recovery_snapshot_t *s,
                               memory_physical_session_recovery_action_t action)
{
    if (action == MEMORY_PHYSICAL_SESSION_RECOVERY_EMPTY) return 0u;
    if (action == MEMORY_PHYSICAL_SESSION_RECOVERY_USE_COMMITTED ||
        action == MEMORY_PHYSICAL_SESSION_RECOVERY_FINALIZE_PREPARED)
        return s->committed_reservation_id;
    if (action == MEMORY_PHYSICAL_SESSION_RECOVERY_PROMOTE_PREPARED)
        return s->prepared_reservation_id;
    return s->durable_reservation_floor; /* DISCARD_PREPARED only on accepted input. */
}

static int gate_is_idle_and_scrubbed(const memory_physical_session_t *gate, uint32_t floor)
{
    return gate && gate->state == MEMORY_PHYSICAL_SESSION_IDLE &&
           gate->session_floor == floor && gate->active_session_id == 0u &&
           gate->active_event_nonce == 0u && gate->active_purpose == MEMORY_PHYSICAL_PURPOSE_NONE &&
           gate->started_at_ms == 0u && gate->expires_at_ms == 0u &&
           gate->uses_remaining == 0u && gate->metrics.begun == 0u &&
           gate->metrics.consumed == 0u && gate->metrics.cancelled == 0u &&
           gate->metrics.expired == 0u && gate->metrics.rejected_format == 0u &&
           gate->metrics.rejected_state == 0u && gate->metrics.rejected_purpose == 0u &&
           gate->metrics.rejected_assertion == 0u && gate->metrics.rejected_time == 0u;
}

static int gate_is_blocked_and_scrubbed(const memory_physical_session_t *gate)
{
    return gate && gate->state == MEMORY_PHYSICAL_SESSION_BLOCKED &&
           gate->session_floor == 0u && gate->active_session_id == 0u &&
           gate->active_event_nonce == 0u && gate->active_purpose == MEMORY_PHYSICAL_PURPOSE_NONE &&
           gate->started_at_ms == 0u && gate->expires_at_ms == 0u &&
           gate->uses_remaining == 0u && gate->metrics.begun == 0u &&
           gate->metrics.consumed == 0u && gate->metrics.cancelled == 0u &&
           gate->metrics.expired == 0u && gate->metrics.rejected_format == 0u &&
           gate->metrics.rejected_state == 0u && gate->metrics.rejected_purpose == 0u &&
           gate->metrics.rejected_assertion == 0u && gate->metrics.rejected_time == 0u;
}

static void exercise_snapshot(const memory_physical_session_recovery_snapshot_t *source)
{
    memory_physical_session_recovery_snapshot_t s = *source;
    memory_physical_session_recovery_snapshot_t before = s;
    memory_physical_session_recovery_action_t action = MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED;
    memory_physical_session_config_t cfg;
    memory_physical_session_bootstrap_result_t result;
    memory_physical_session_t gate;
    uint32_t floor;
    int recovery_rc;
    int bootstrap_rc;

    recovery_rc = memory_physical_session_recovery_assess(&s, &action);
    if (memcmp(&s, &before, sizeof(s)) != 0) {
        fail_once("F1 recovery oracle mutated its caller-supplied snapshot");
        return;
    }

    memory_physical_session_config_default(&cfg);
    memset(&gate, 0xa5, sizeof(gate));
    memset(&result, 0xa5, sizeof(result));
    bootstrap_rc = memory_physical_session_bootstrap(&gate, &cfg, &s, &result);

    if (recovery_rc == MEMORY_PHYSICAL_SESSION_RECOVERY_OK) {
        floor = expected_floor(&s, action);
        ACCEPTED++;
        if (!action_valid(action) || action == MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED ||
            action > MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED) {
            fail_once("F1 recovery emitted an invalid or blocked success action");
            return;
        }
        ACTIONS[action]++;
        if (floor == UINT32_MAX) {
            fail_once("F1 recovery accepted terminal UINT32_MAX floor with no possible successor session");
            return;
        }
        if (bootstrap_rc != MEMORY_PHYSICAL_SESSION_BOOTSTRAP_OK ||
            result.recovery_action != action || result.recovered_session_floor != floor ||
            result.active_evidence_scrubbed != 1u || !gate_is_idle_and_scrubbed(&gate, floor)) {
            fail_once("F1 accepted recovery did not bootstrap to a scrubbed idle gate");
            return;
        }
        if (floor != 0u &&
            memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                                          floor, 1u, 1u, 1u, 10u) !=
                MEMORY_PHYSICAL_SESSION_E_FORMAT) {
            fail_once("F1 recovered floor was reused as an active session ID");
            return;
        }
        if (memory_physical_session_begin(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY,
                                          floor + 1u, 1u, 1u, 1u, 10u) !=
                MEMORY_PHYSICAL_SESSION_OK) {
            fail_once("F1 fresh successor session could not begin after idle bootstrap");
            return;
        }
        if (memory_physical_session_cancel(&gate) != MEMORY_PHYSICAL_SESSION_OK) {
            fail_once("F1 fresh successor session could not be explicitly cancelled");
            return;
        }
    } else if (recovery_rc == MEMORY_PHYSICAL_SESSION_RECOVERY_E_INVALID) {
        REJECTED++;
        if (action != MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED ||
            bootstrap_rc != MEMORY_PHYSICAL_SESSION_BOOTSTRAP_E_RECOVERY ||
            result.recovery_action != MEMORY_PHYSICAL_SESSION_RECOVERY_BLOCKED ||
            result.recovered_session_floor != 0u || result.active_evidence_scrubbed != 0u ||
            !gate_is_blocked_and_scrubbed(&gate)) {
            fail_once("F1 rejected recovery did not block and scrub bootstrap state");
            return;
        }
    } else {
        fail_once("F1 recovery returned an unsupported status for non-null inputs");
    }
}

static void mutate_templates(void)
{
    uint32_t template_index;
    for (template_index = 0u; template_index < F1_TEMPLATE_COUNT; ++template_index) {
        memory_physical_session_recovery_snapshot_t s = template_at(template_index);
        size_t byte_index;
        uint8_t bit;
        exercise_snapshot(&s);
        for (byte_index = 0u; byte_index < sizeof(s); ++byte_index) {
            for (bit = 0u; bit < 8u; ++bit) {
                memory_physical_session_recovery_snapshot_t mutation = s;
                ((uint8_t *)&mutation)[byte_index] ^= (uint8_t)(1u << bit);
                exercise_snapshot(&mutation);
            }
        }
    }
}

int main(void)
{
    memory_physical_session_recovery_snapshot_t s;
    memory_physical_session_recovery_action_t action = MEMORY_PHYSICAL_SESSION_RECOVERY_EMPTY;
    uint32_t i;

    printf("\n== F1 deterministic hostile recovery/bootstrap campaign ==\n");
    ok(memory_physical_session_recovery_assess(0, &action) ==
           MEMORY_PHYSICAL_SESSION_RECOVERY_E_ARG &&
       memory_physical_session_recovery_assess(&s, 0) ==
           MEMORY_PHYSICAL_SESSION_RECOVERY_E_ARG,
       "F1 null oracle inputs fail before any permissive action");

    mutate_templates();
    for (i = 0u; i < F1_STRUCTURED_ROUNDS; ++i) {
        structured_snapshot(&s);
        exercise_snapshot(&s);
    }
    for (i = 0u; i < F1_RAW_ROUNDS; ++i) {
        raw_snapshot(&s);
        exercise_snapshot(&s);
    }

    ok(!FAILED && ACCEPTED >= F1_TEMPLATE_COUNT && REJECTED != 0u,
       "F1 generated accepted and rejected topologies without an unsafe bootstrap transition");
    ok(ACTIONS[MEMORY_PHYSICAL_SESSION_RECOVERY_EMPTY] != 0u &&
       ACTIONS[MEMORY_PHYSICAL_SESSION_RECOVERY_USE_COMMITTED] != 0u &&
       ACTIONS[MEMORY_PHYSICAL_SESSION_RECOVERY_PROMOTE_PREPARED] != 0u &&
       ACTIONS[MEMORY_PHYSICAL_SESSION_RECOVERY_FINALIZE_PREPARED] != 0u &&
       ACTIONS[MEMORY_PHYSICAL_SESSION_RECOVERY_DISCARD_PREPARED] != 0u,
       "F1 exercised every permitted recovery action under bootstrap quarantine");

    printf("  INFO seed=0x%08x structured=%u raw=%u accepted=%u rejected=%u\n",
           F1_SEED, F1_STRUCTURED_ROUNDS, F1_RAW_ROUNDS, ACCEPTED, REJECTED);
    if (FAILED) {
        printf("RECOVERY STRESS TESTS FAILED\n");
        return 1;
    }
    printf("RECOVERY STRESS INVARIANTS HOLD — hostile snapshots either reach idle quarantine or a scrubbed blocked gate.\n");
    return 0;
}
