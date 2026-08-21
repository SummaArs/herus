#include "sim.h"
#include "semantic_life.h"
#include <string.h>

static sl_event_t memory_event(uint32_t generation, sr_symbol_t predicate,
                               sr_symbol_t object, uint8_t contact,
                               uint8_t confirmation, uint32_t expiry)
{
    sl_event_t event;
    memset(&event, 0, sizeof(event));
    event.presence.generation = generation;
    event.presence.power_on = 1u;
    event.presence.haptic_available = 0u;
    event.presence.physical_contact = contact;
    event.presence.has_observation = 0u;
    event.has_memory_candidate = 1u;
    event.explicit_memory_confirmation = confirmation;
    event.card.card_id = 1000u + generation;
    event.card.review_receipt_id = 2000u + generation;
    event.fact = (sr_fact_t){SL_USER_SUBJECT, predicate, object, 0u};
    event.memory_valid_until_generation = expiry;
    return event;
}

static sl_event_t quiet_event(uint32_t generation)
{
    sl_event_t event;
    memset(&event, 0, sizeof(event));
    event.presence.generation = generation;
    event.presence.power_on = 1u;
    return event;
}

static sr_pattern_t fact_query(sr_symbol_t predicate)
{
    return (sr_pattern_t){SR_CONST(SL_USER_SUBJECT), SR_CONST(predicate),
                          SR_VAR(0u), 0u};
}

void scenario_semantic_life(sim_score *score, int argc, char **argv)
{
    pps_config_t config;
    sl_life_t life;
    sl_trace_t trace;
    mse_query_result_t result;
    sr_pattern_t query;
    sl_event_t event;
    (void)argc;
    (void)argv;

    pps_config_default(&config);
    sl_init(&life, &config);

    event = memory_event(1u, SL_PRED_PREFERENCE, SR_SYMBOL_LEGACY(51u),
                         0u, 1u, 0u);
    sim_ok(score, sl_step(&life, &event, &trace) == SL_OK &&
                   trace.memory_disposition == SL_MEMORY_DISCARDED_NO_AUTHORITY &&
                   life.semantic_index.evidence_count == 0u,
           "a semantic observation without physical authority is discarded");

    event = memory_event(2u, SL_PRED_PREFERENCE, SR_SYMBOL_LEGACY(51u),
                         1u, 1u, 0u);
    sim_ok(score, sl_step(&life, &event, &trace) == SL_OK &&
                   trace.memory_disposition == SL_MEMORY_RETAINED &&
                   life.semantic_index.evidence_count == 1u,
           "an explicitly authorised preference enters the local semantic model");
    query = fact_query(SL_PRED_PREFERENCE);
    sim_ok(score, sl_query(&life, &query, 2u, &result) == MSE_OK &&
                   result.status == MSE_QUERY_MATCH &&
                   result.selected_card_id == 1002u,
           "the continuous model retrieves a relevant preference with provenance");

    event = memory_event(3u, SL_PRED_PREFERENCE, SR_SYMBOL_LEGACY(52u),
                         1u, 1u, 0u);
    sim_ok(score, sl_step(&life, &event, &trace) == SL_OK &&
                   trace.memory_disposition == SL_MEMORY_RETAINED,
           "a second compatible preference is retained rather than overwritten");
    sim_ok(score, sl_query(&life, &query, 3u, &result) == MSE_OK &&
                   result.status == MSE_QUERY_AMBIGUOUS && result.active_matches == 2u,
           "the model exposes preference ambiguity instead of choosing silently");

    event = memory_event(4u, SL_PRED_GOAL, SR_SYMBOL_LEGACY(61u),
                         1u, 1u, 0u);
    sim_ok(score, sl_step(&life, &event, &trace) == SL_OK &&
                   trace.memory_disposition == SL_MEMORY_RETAINED,
           "a reviewed goal enters the same local model");
    event = memory_event(5u, SL_PRED_GOAL, SR_SYMBOL_LEGACY(62u),
                         1u, 1u, 0u);
    sim_ok(score, sl_step(&life, &event, &trace) == SL_OK &&
                   trace.memory_disposition == SL_MEMORY_CONFLICTED,
           "incompatible goals become an explicit conflict");
    query = fact_query(SL_PRED_GOAL);
    sim_ok(score, sl_query(&life, &query, 5u, &result) == MSE_OK &&
                   result.status == MSE_QUERY_CONTRADICTED &&
                   result.selected_card_id == 0u,
           "conflicting goals produce abstention without selecting a winner");

    event = memory_event(6u, SL_PRED_CONTEXT, SR_SYMBOL_LEGACY(71u),
                         1u, 1u, 6u);
    sim_ok(score, sl_step(&life, &event, &trace) == SL_OK &&
                   trace.memory_disposition == SL_MEMORY_RETAINED,
           "a bounded contextual fact enters with an expiry generation");
    event = quiet_event(7u);
    sim_ok(score, sl_step(&life, &event, &trace) == SL_OK &&
                   trace.expired_count == 1u,
           "contextual knowledge expires as the semantic life advances");
    query = fact_query(SL_PRED_CONTEXT);
    sim_ok(score, sl_query(&life, &query, 7u, &result) == MSE_OK &&
                   result.status == MSE_QUERY_NO_MATCH,
           "expired context is not retrieved as current life knowledge");

    event = quiet_event(10u);
    event.reboot = 1u;
    event.recovered_semantic_floor = 7u;
    sim_ok(score, sl_step(&life, &event, &trace) == SL_OK &&
                   trace.scrubbed_on_reboot == 1u && trace.quarantined == 1u &&
                   life.semantic_index.evidence_count == 0u &&
                   life.semantic_index.generation_floor == 7u,
           "reboot scrubs active semantic evidence and imports only a floor");
    query = fact_query(SL_PRED_PREFERENCE);
    sim_ok(score, sl_query(&life, &query, 10u, &result) == MSE_OK &&
                   result.status == MSE_QUERY_NO_MATCH,
           "pre-reboot preferences do not revive without post-reboot reindexing");

    event = memory_event(11u, SL_PRED_PREFERENCE, SR_SYMBOL_LEGACY(53u),
                         1u, 1u, 0u);
    sim_ok(score, sl_step(&life, &event, &trace) == SL_OK &&
                   trace.memory_disposition == SL_MEMORY_RETAINED &&
                   life.semantic_index.generation_floor == 7u,
           "a post-reboot successor can reindex above the recovered floor");

    event = quiet_event(12u);
    event.reboot = 1u;
    event.recovered_semantic_floor = 6u;
    sim_ok(score, sl_step(&life, &event, &trace) == SL_E_FLOOR,
           "a divergent lower reboot floor is rejected before state mutation");

    query = (sr_pattern_t){SR_VAR(0u), SR_VAR(1u), SR_VAR(2u), 0u};
    sim_ok(score, sl_query(&life, &query, 11u, &result) == MSE_E_ARG,
           "the continuous model does not expose an all-variable memory listing");
}
