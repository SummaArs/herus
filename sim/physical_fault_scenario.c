#include "sim.h"
#include "personal_sim.h"
#include <string.h>

static pps_event_t event(uint32_t generation, uint8_t haptic,
                         uint8_t contact, uint8_t observation)
{
    pps_event_t out;
    memset(&out, 0, sizeof(out));
    out.generation = generation;
    out.power_on = 1u;
    out.haptic_available = haptic;
    out.physical_contact = contact;
    out.has_observation = observation;
    return out;
}

static ap_observation_t useful_observation(uint32_t generation)
{
    ap_observation_t out;
    memset(&out, 0, sizeof(out));
    out.opportunity_class = AP_CLASS_RECALL;
    out.privacy_class = AP_PRIVACY_PERSONAL;
    out.attention_available = 1u;
    out.proactive_consent = 1u;
    out.confidence_pct = 95u;
    out.relevance_pct = 90u;
    out.novelty_pct = 80u;
    out.risk_pct = 5u;
    out.now_generation = generation;
    out.valid_until_generation = generation + 3u;
    out.cooldown_generations = 2u;
    return out;
}

void scenario_physical_faults(sim_score *score, int argc, char **argv)
{
    pps_config_t config;
    pps_device_t device;
    pps_trace_t trace;
    pps_event_t current;
    (void)argc;
    (void)argv;

    pps_config_default(&config);
    pps_init(&device, &config);

    current = event(1u, 0u, 0u, 1u);
    current.observation = useful_observation(1u);
    sim_ok(score, pps_step(&device, &current, &trace) == PPS_OK &&
                   trace.status == PPS_HOLD,
           "a valid candidate is retained while the haptic adapter is absent");

    current = event(2u, 1u, 0u, 0u);
    current.missing_adapters = PPS_MISSING_HAPTIC;
    sim_ok(score, pps_step(&device, &current, &trace) == PPS_OK &&
                   trace.status == PPS_HOLD && trace.haptic_emitted == 0u,
           "haptic loss cannot turn a latent candidate into an unobserved offer");

    current = event(3u, 1u, 0u, 0u);
    sim_ok(score, pps_step(&device, &current, &trace) == PPS_OK &&
                   trace.status == PPS_OFFERED,
           "restored haptic emits one bounded offer");

    current = event(4u, 1u, 1u, 0u);
    current.missing_adapters = PPS_MISSING_CONTACT;
    sim_ok(score, pps_step(&device, &current, &trace) == PPS_OK &&
                   trace.status == PPS_OFFERED &&
                   trace.physical_contact_consumed == 0u,
           "contact-adapter loss leaves the offer unconfirmed");

    current.missing_adapters = 0u;
    sim_ok(score, pps_step(&device, &current, &trace) == PPS_OK &&
                   trace.status == PPS_ACKNOWLEDGED &&
                   trace.physical_contact_consumed == 1u,
           "contact later restored acknowledges only the still-live offer");

    current = event(5u, 0u, 0u, 1u);
    current.observation = useful_observation(5u);
    sim_ok(score, pps_step(&device, &current, &trace) == PPS_OK &&
                   trace.status == PPS_HOLD,
           "a second candidate can begin after a complete physical lifecycle");

    current = event(6u, 1u, 0u, 0u);
    current.missing_adapters = PPS_MISSING_CLOCK;
    sim_ok(score, pps_step(&device, &current, &trace) == PPS_E_TIME &&
                   trace.status == PPS_ADAPTER_LOST && device.last_generation == 5u,
           "clock loss rejects the event without advancing semantic time");

    current = event(7u, 1u, 0u, 0u);
    current.missing_adapters = PPS_MISSING_POWER;
    sim_ok(score, pps_step(&device, &current, &trace) == PPS_OK &&
                   trace.status == PPS_ADAPTER_LOST &&
                   !device.presence.candidate_valid,
           "power loss scrubs the candidate instead of preserving half-authority");

    current = event(8u, 1u, 0u, 0u);
    sim_ok(score, pps_step(&device, &current, &trace) == PPS_OK &&
                   trace.status == PPS_SILENT && device.offers == 1u,
           "power restoration does not revive a pre-loss offer");

    current = event(9u, 1u, 0u, 1u);
    current.missing_adapters = PPS_MISSING_SEMANTIC;
    current.observation = useful_observation(9u);
    sim_ok(score, pps_step(&device, &current, &trace) == PPS_OK &&
                   trace.status == PPS_SILENT && device.offers == 1u,
           "semantic-adapter loss cannot create a candidate or offer");

    current = event(10u, 0u, 0u, 1u);
    current.observation = useful_observation(10u);
    sim_ok(score, pps_step(&device, &current, &trace) == PPS_OK &&
                   trace.status == PPS_HOLD,
           "a fresh candidate remains safe after an adapter fault sequence");
    current = event(11u, 1u, 0u, 0u);
    sim_ok(score, pps_step(&device, &current, &trace) == PPS_OK &&
                   trace.status == PPS_OFFERED,
           "the new candidate can be offered after haptic recovery");

    device.cfg.battery_uj = 10u;
    current = event(11u, 1u, 1u, 0u);
    sim_ok(score, pps_step(&device, &current, &trace) == PPS_OK &&
                   trace.status == PPS_NO_ENERGY &&
                   device.presence.status == AP_OFFER &&
                   device.acknowledgements == 1u,
           "energy loss during contact does not falsely acknowledge the offer");

    device.cfg.battery_uj = 1000u;
    sim_ok(score, pps_step(&device, &current, &trace) == PPS_OK &&
                   trace.status == PPS_ACKNOWLEDGED &&
                   device.acknowledgements == 2u,
           "restored energy permits the same live contact to complete once");
}
