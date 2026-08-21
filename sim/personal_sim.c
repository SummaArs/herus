#include "personal_sim.h"
#include <string.h>

static int canonical_bool(uint8_t value)
{
    return value == 0u || value == 1u;
}

static void trace_reset(pps_trace_t *trace)
{
    if (trace) memset(trace, 0, sizeof(*trace));
}

static int missing_bits_valid(uint8_t missing)
{
    return (missing & (uint8_t)~(PPS_MISSING_CLOCK | PPS_MISSING_SEMANTIC |
                                  PPS_MISSING_HAPTIC | PPS_MISSING_CONTACT |
                                  PPS_MISSING_POWER)) == 0u;
}

static void trace_from_presence(const pps_device_t *device, pps_trace_t *trace)
{
    if (!device || !trace) return;
    trace->reason = device->presence.reason;
    if (device->presence.status == AP_EXPIRED)
        trace->status = PPS_EXPIRED;
    else if (device->presence.status == AP_ACKNOWLEDGED)
        trace->status = PPS_ACKNOWLEDGED;
    else if (device->presence.status == AP_OFFER)
        trace->status = PPS_OFFERED;
    else if (device->presence.status == AP_HOLD)
        trace->status = PPS_HOLD;
    else
        trace->status = PPS_SILENT;
}

void pps_config_default(pps_config_t *out)
{
    if (!out) return;
    out->observation_cost_uj = 10u;
    out->haptic_cost_uj = 20u;
    out->contact_cost_uj = 15u;
    out->battery_uj = 1000u;
}

void pps_init(pps_device_t *device, const pps_config_t *config)
{
    pps_config_t fallback;
    if (!device) return;
    pps_config_default(&fallback);
    memset(device, 0, sizeof(*device));
    device->cfg = config ? *config : fallback;
    ap_init(&device->presence);
    device->powered = 1u;
}

void pps_power_cycle(pps_device_t *device)
{
    if (!device) return;
    ap_forget(&device->presence);
    device->powered = 0u;
    device->haptic_available = 0u;
    device->power_cycles++;
}

static int spend(pps_device_t *device, uint32_t amount, pps_trace_t *trace)
{
    if (device->cfg.battery_uj < amount) {
        if (trace) trace->status = PPS_NO_ENERGY;
        return 0;
    }
    device->cfg.battery_uj -= amount;
    if (trace) trace->energy_spent_uj += amount;
    return 1;
}

int pps_step(pps_device_t *device, const pps_event_t *event,
             pps_trace_t *trace)
{
    ap_offer_t offer;
    int result;

    trace_reset(trace);
    if (!device || !event || !trace) return PPS_E_ARG;
    if (!canonical_bool(event->power_on) ||
        !canonical_bool(event->haptic_available) ||
        !canonical_bool(event->physical_contact) ||
        !canonical_bool(event->has_observation) ||
        !missing_bits_valid(event->missing_adapters) ||
        event->generation == 0u)
        return PPS_E_FORMAT;
    if ((event->missing_adapters & PPS_MISSING_CLOCK) != 0u) {
        trace->status = PPS_ADAPTER_LOST;
        trace->reason = event->missing_adapters;
        device->rejected_events++;
        return PPS_E_TIME;
    }
    if ((event->missing_adapters & PPS_MISSING_POWER) != 0u) {
        pps_power_cycle(device);
        trace->status = PPS_ADAPTER_LOST;
        trace->reason = event->missing_adapters;
        return PPS_OK;
    }
    if (device->last_generation != 0u &&
        event->generation < device->last_generation) {
        trace->status = PPS_REJECTED;
        trace->reason = AP_REASON_BAD_FORMAT;
        device->rejected_events++;
        return PPS_E_TIME;
    }

    device->last_generation = event->generation;
    device->haptic_available = ((event->missing_adapters & PPS_MISSING_HAPTIC) == 0u) &&
                                event->haptic_available;
    if (event->power_on == 0u) {
        if (device->powered != 0u) pps_power_cycle(device);
        trace->status = PPS_POWERED_OFF;
        return PPS_OK;
    }
    device->powered = 1u;

    if (event->has_observation != 0u &&
        !spend(device, device->cfg.observation_cost_uj, trace)) {
        trace->reason = AP_REASON_BUDGET;
        return PPS_OK;
    }

    result = ap_tick(&device->presence, event->generation);
    if (result == AP_NO_OFFER &&
        device->presence.status == AP_EXPIRED) {
        device->expirations++;
        trace_from_presence(device, trace);
        return PPS_OK;
    }
    if (result == AP_E_FORMAT) {
        trace->status = PPS_REJECTED;
        trace->reason = AP_REASON_BAD_FORMAT;
        device->rejected_events++;
        return PPS_E_TIME;
    }

    if (event->physical_contact != 0u &&
        (event->missing_adapters & PPS_MISSING_CONTACT) == 0u &&
        device->presence.status == AP_OFFER) {
        if (!spend(device, device->cfg.contact_cost_uj, trace)) {
            trace->status = PPS_NO_ENERGY;
            trace->reason = AP_REASON_BUDGET;
            return PPS_OK;
        }
        result = ap_acknowledge(&device->presence, 1u);
        if (result == AP_OK) {
            device->acknowledgements++;
            trace->physical_contact_consumed = 1u;
            trace_from_presence(device, trace);
            return PPS_OK;
        }
    }

    if (event->has_observation != 0u &&
        (event->missing_adapters & PPS_MISSING_SEMANTIC) == 0u) {
        result = ap_observe(&device->presence, &event->observation);
        if (result == AP_E_FORMAT || result == AP_E_ARG) {
            trace->status = PPS_REJECTED;
            trace->reason = AP_REASON_BAD_FORMAT;
            device->rejected_events++;
            return PPS_E_FORMAT;
        }
    }

    if (device->presence.candidate_valid &&
        device->presence.status == AP_HOLD &&
        device->haptic_available != 0u) {
        if (!spend(device, device->cfg.haptic_cost_uj, trace)) {
            trace->status = PPS_NO_ENERGY;
            trace->reason = AP_REASON_BUDGET;
            return PPS_OK;
        }
        result = ap_offer(&device->presence, event->generation, &offer);
        if (result == AP_OK) {
            device->offers++;
            trace->haptic_emitted = 1u;
            trace->latency_generations = event->generation -
                                         device->presence.candidate_generation;
        }
    }

    trace_from_presence(device, trace);
    return PPS_OK;
}
