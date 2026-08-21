/*
 * personal_sim.h — deterministic host substitute for the physical HERUS watch.
 *
 * The simulator supplies typed events that future hardware adapters will supply:
 * monotonic generation, power state, haptic availability and physical contact.
 * It never fabricates audio, text, identity or a successful human interpretation.
 * The policy under test is the real firmware/core ambient_presence module.
 */
#ifndef HERUS_PERSONAL_SIM_H
#define HERUS_PERSONAL_SIM_H

#include <stdint.h>
#include "ambient_presence.h"

typedef enum {
    PPS_SILENT = 0u,
    PPS_HOLD = 1u,
    PPS_OFFERED = 2u,
    PPS_ACKNOWLEDGED = 3u,
    PPS_EXPIRED = 4u,
    PPS_POWERED_OFF = 5u,
    PPS_NO_ENERGY = 6u,
    PPS_REJECTED = 7u
} pps_status_t;

typedef struct {
    uint32_t generation;
    uint8_t power_on;
    uint8_t haptic_available;
    uint8_t physical_contact;
    uint8_t has_observation;
    ap_observation_t observation;
} pps_event_t;

typedef struct {
    uint32_t observation_cost_uj;
    uint32_t haptic_cost_uj;
    uint32_t battery_uj;
} pps_config_t;

typedef struct {
    pps_status_t status;
    uint32_t reason;
    uint32_t energy_spent_uj;
    uint32_t latency_generations;
    uint8_t haptic_emitted;
    uint8_t physical_contact_consumed;
} pps_trace_t;

typedef struct {
    pps_config_t cfg;
    ap_presence_t presence;
    uint32_t last_generation;
    uint8_t powered;
    uint8_t haptic_available;
    uint32_t offers;
    uint32_t acknowledgements;
    uint32_t expirations;
    uint32_t power_cycles;
    uint32_t rejected_events;
} pps_device_t;

enum {
    PPS_OK = 0,
    PPS_E_ARG = -1,
    PPS_E_FORMAT = -2,
    PPS_E_TIME = -3
};

void pps_config_default(pps_config_t *out);
void pps_init(pps_device_t *device, const pps_config_t *config);

/* Advance one deterministic physical event. No event creates action authority. */
int pps_step(pps_device_t *device, const pps_event_t *event,
             pps_trace_t *trace);

/* Simulate a power loss: transient presence is scrubbed, counters remain audit data. */
void pps_power_cycle(pps_device_t *device);

#endif /* HERUS_PERSONAL_SIM_H */
