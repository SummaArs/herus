#include "sim.h"
#include "personal_sim.h"
#include <string.h>

static pps_event_t event_base(uint32_t generation, uint8_t haptic,
                              uint8_t contact, uint8_t has_observation)
{
    pps_event_t event;
    memset(&event, 0, sizeof(event));
    event.generation = generation;
    event.power_on = 1u;
    event.haptic_available = haptic;
    event.physical_contact = contact;
    event.has_observation = has_observation;
    return event;
}

static ap_observation_t observation(uint32_t generation, uint32_t expiry)
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
    out.valid_until_generation = expiry;
    out.cooldown_generations = 2u;
    return out;
}

void scenario_personal(sim_score *score, int argc, char **argv)
{
    pps_config_t config;
    pps_device_t device;
    pps_trace_t trace;
    pps_event_t event;
    (void)argc;
    (void)argv;

    pps_config_default(&config);
    pps_init(&device, &config);

    event = event_base(1u, 0u, 0u, 1u);
    event.observation = observation(1u, 4u);
    sim_ok(score, pps_step(&device, &event, &trace) == PPS_OK &&
                   trace.status == PPS_HOLD && device.offers == 0u,
           "personal simulator holds a useful observation when haptic is unavailable");

    event = event_base(2u, 0u, 0u, 0u);
    sim_ok(score, pps_step(&device, &event, &trace) == PPS_OK &&
                   trace.status == PPS_HOLD && trace.haptic_emitted == 0u,
           "lack of haptic output does not create an invisible authority path");

    event = event_base(3u, 1u, 0u, 0u);
    sim_ok(score, pps_step(&device, &event, &trace) == PPS_OK &&
                   trace.status == PPS_OFFERED && trace.haptic_emitted == 1u &&
                   trace.latency_generations == 2u && device.offers == 1u,
           "haptic availability surfaces one bounded offer with measured simulated latency");

    event = event_base(3u, 1u, 0u, 0u);
    sim_ok(score, pps_step(&device, &event, &trace) == PPS_OK &&
                   trace.status == PPS_OFFERED && device.offers == 1u,
           "polling an offer does not repeat it or destroy its contact window");

    event = event_base(4u, 1u, 0u, 0u);
    sim_ok(score, pps_step(&device, &event, &trace) == PPS_OK &&
                   trace.status == PPS_OFFERED && !trace.physical_contact_consumed,
           "no physical contact leaves the offer unconfirmed");

    event = event_base(4u, 1u, 1u, 0u);
    sim_ok(score, pps_step(&device, &event, &trace) == PPS_OK &&
                   trace.status == PPS_ACKNOWLEDGED &&
                   trace.physical_contact_consumed == 1u &&
                   device.acknowledgements == 1u,
           "physical contact acknowledges receipt without authorising an action");

    event = event_base(10u, 0u, 0u, 1u);
    event.observation = observation(10u, 12u);
    sim_ok(score, pps_step(&device, &event, &trace) == PPS_OK &&
                   trace.status == PPS_HOLD,
           "a new opportunity can be observed after the previous bounded lifecycle");

    pps_power_cycle(&device);
    event = event_base(11u, 1u, 0u, 0u);
    sim_ok(score, pps_step(&device, &event, &trace) == PPS_OK &&
                   trace.status == PPS_SILENT && device.offers == 1u,
           "power loss scrubs latent presence and does not revive a candidate");

    event = event_base(20u, 0u, 0u, 1u);
    event.observation = observation(20u, 22u);
    sim_ok(score, pps_step(&device, &event, &trace) == PPS_OK &&
                   trace.status == PPS_HOLD,
           "the simulator accepts a fresh typed opportunity after recovery");
    event = event_base(23u, 1u, 0u, 0u);
    sim_ok(score, pps_step(&device, &event, &trace) == PPS_OK &&
                   trace.status == PPS_EXPIRED && device.expirations == 1u,
           "an unanswered opportunity expires before any late presentation");

    device.cfg.battery_uj = 5u;
    event = event_base(30u, 0u, 0u, 1u);
    event.observation = observation(30u, 32u);
    sim_ok(score, pps_step(&device, &event, &trace) == PPS_OK &&
                   trace.status == PPS_NO_ENERGY && device.offers == 1u,
           "insufficient simulated energy suppresses observation without offering");

    event = event_base(29u, 1u, 0u, 0u);
    sim_ok(score, pps_step(&device, &event, &trace) == PPS_E_TIME &&
                   trace.status == PPS_REJECTED,
           "a regressing monotonic clock is rejected rather than rewriting history");

    event = event_base(31u, 1u, 0u, 0u);
    event.power_on = 0u;
    sim_ok(score, pps_step(&device, &event, &trace) == PPS_OK &&
                   trace.status == PPS_POWERED_OFF,
           "power-off is an explicit physical state and produces no offer");
}
