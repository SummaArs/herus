#include "semantic_life.h"
#include <string.h>

static int sl_is_functional(sr_symbol_t predicate, void *user)
{
    (void)user;
    return predicate == SL_PRED_GOAL || predicate == SL_PRED_CONTEXT;
}

static void trace_reset(sl_trace_t *trace)
{
    if (trace) memset(trace, 0, sizeof(*trace));
}

static void scrub_and_import_floor(sl_life_t *life, uint32_t floor,
                                   sl_trace_t *trace)
{
    ap_forget(&life->physical.presence);
    life->physical.powered = 0u;
    life->physical.haptic_available = 0u;
    mse_init(&life->semantic_index, sl_is_functional, NULL);
    (void)mse_set_generation_floor(&life->semantic_index, floor);
    life->durable_semantic_floor = floor;
    life->quarantined = 1u;
    if (trace) {
        trace->scrubbed_on_reboot = 1u;
        trace->quarantined = 1u;
        trace->generation_floor = floor;
    }
}

void sl_init(sl_life_t *life, const pps_config_t *config)
{
    if (!life) return;
    memset(life, 0, sizeof(*life));
    pps_init(&life->physical, config);
    mse_init(&life->semantic_index, sl_is_functional, NULL);
}

int sl_step(sl_life_t *life, const sl_event_t *event, sl_trace_t *trace)
{
    int presence_result;
    mse_status_t memory_result;
    uint32_t conflicts_before;

    trace_reset(trace);
    if (!life || !event || !trace) return SL_E_ARG;
    if (event->presence.generation == 0u ||
        (event->reboot != 0u && event->recovered_semantic_floor == 0u))
        return SL_E_FORMAT;
    if (event->reboot != 0u) {
        if (event->recovered_semantic_floor < life->durable_semantic_floor)
            return SL_E_FLOOR;
        if (event->presence.generation <= event->recovered_semantic_floor)
            return SL_E_FLOOR;
        scrub_and_import_floor(life, event->recovered_semantic_floor, trace);
    }

    presence_result = pps_step(&life->physical, &event->presence,
                               &((pps_trace_t){ 0 }));
    trace->presence_result = presence_result;
    trace->presence_status = life->physical.presence.status == AP_OFFER
                                 ? PPS_OFFERED
                                 : life->physical.presence.status == AP_HOLD
                                     ? PPS_HOLD
                                     : life->physical.presence.status == AP_EXPIRED
                                         ? PPS_EXPIRED : PPS_SILENT;
    if (presence_result == PPS_E_TIME) return SL_E_TIME;

    if (event->presence.generation < life->last_generation)
        return SL_E_TIME;
    life->last_generation = event->presence.generation;
    trace->expired_count = mse_expire(&life->semantic_index,
                                      event->presence.generation);

    if (event->has_memory_candidate == 0u) {
        trace->generation_floor = life->semantic_index.generation_floor;
        trace->quarantined = life->quarantined;
        return SL_OK;
    }
    life->memory_candidates++;
    if (event->explicit_memory_confirmation != 1u ||
        event->presence.physical_contact != 1u) {
        life->memory_discarded++;
        trace->memory_disposition = SL_MEMORY_DISCARDED_NO_AUTHORITY;
        trace->memory_reason = AP_REASON_NO_CONTACT;
        trace->generation_floor = life->semantic_index.generation_floor;
        trace->quarantined = life->quarantined;
        return SL_OK;
    }

    conflicts_before = life->semantic_index.conflicts;
    memory_result = mse_add(&life->semantic_index, &event->card, &event->fact,
                            event->presence.generation,
                            event->memory_valid_until_generation);
    if (memory_result == MSE_OK || memory_result == MSE_NO_CHANGE) {
        if (life->semantic_index.conflicts > conflicts_before) {
            life->memory_conflicted++;
            trace->memory_disposition = SL_MEMORY_CONFLICTED;
        } else {
            life->memory_retained++;
            trace->memory_disposition = SL_MEMORY_RETAINED;
            life->quarantined = 0u;
        }
    } else {
        life->memory_rejected++;
        trace->memory_disposition = SL_MEMORY_REJECTED;
        trace->memory_reason = (uint32_t)(-memory_result);
    }
    trace->generation_floor = life->semantic_index.generation_floor;
    trace->quarantined = life->quarantined;
    return SL_OK;
}

int sl_query(const sl_life_t *life, const sr_pattern_t *pattern,
             uint32_t generation, mse_query_result_t *out)
{
    if (!life) return MSE_E_ARG;
    return (int)mse_query(&life->semantic_index, pattern, generation, out);
}
