/*
 * virtual_lab.c — deterministic pre-hardware virtual lab.
 *
 * This is a composition harness, not a second firmware implementation. The LoRa
 * leg uses the shipping firmware/net objects through sim_world. The Watch, Paper-
 * Core, route and telemetry legs use the shipping host-only C11 contracts. Sensor
 * and battery values are scripted virtual inputs with explicit scenario units; they
 * are not measurements of a real board.
 */
#include "sim.h"
#include "transport_selector.h"
#include "personal_telemetry.h"
#include <stdio.h>
#include <string.h>

#define VIRTUAL_BATTERY_CAPACITY_UNITS 1000u
#define VIRTUAL_COST_SENSOR_UNITS      5u
#define VIRTUAL_COST_LORA_UNITS       80u
#define VIRTUAL_COST_ESPNOW_UNITS     35u
#define VIRTUAL_COST_BLE_UNITS        12u
#define VIRTUAL_COST_WIFI_UNITS       55u

typedef struct {
    uint32_t capacity_units;
    uint32_t remaining_units;
    uint32_t consumed_units;
    uint32_t rejected_spends;
} virtual_battery_t;

static void v_header(const char *title)
{
    printf("\n%s\n", title);
    for (size_t i = 0; i < strlen(title); i++) putchar('-');
    putchar('\n');
}

static void battery_init(virtual_battery_t *battery)
{
    if (!battery) return;
    battery->capacity_units = VIRTUAL_BATTERY_CAPACITY_UNITS;
    battery->remaining_units = VIRTUAL_BATTERY_CAPACITY_UNITS;
    battery->consumed_units = 0u;
    battery->rejected_spends = 0u;
}

static int battery_spend(virtual_battery_t *battery, uint32_t units)
{
    if (!battery || units == 0u || units > battery->remaining_units) {
        if (battery) battery->rejected_spends++;
        return 0;
    }
    battery->remaining_units -= units;
    battery->consumed_units += units;
    return 1;
}

static transport_selector_input_t route_input(transport_environment_t environment,
                                              transport_payload_class_t payload,
                                              uint16_t payload_bytes)
{
    transport_selector_input_t input;
    memset(&input, 0, sizeof(input));
    input.environment = environment;
    input.payload_class = payload;
    input.payload_bytes = payload_bytes;
    input.lora_max_bytes = 64u;
    input.espnow_max_bytes = 250u;
    input.ble_max_bytes = 128u;
    input.wifi_max_bytes = 1400u;
    input.lora_available = 1u;
    input.espnow_available = 1u;
    input.ble_available = 1u;
    input.wifi_available = 1u;
    input.peer_authenticated = 1u;
    input.physical_authorized = 1u;
    input.share_confirmed = 0u;
    return input;
}

static uint32_t route_cost(transport_kind_t kind)
{
    switch (kind) {
    case TRANSPORT_KIND_LORA:   return VIRTUAL_COST_LORA_UNITS;
    case TRANSPORT_KIND_ESPNOW: return VIRTUAL_COST_ESPNOW_UNITS;
    case TRANSPORT_KIND_BLE:    return VIRTUAL_COST_BLE_UNITS;
    case TRANSPORT_KIND_WIFI:   return VIRTUAL_COST_WIFI_UNITS;
    default:                    return 0u;
    }
}

static int virtual_dispatch(virtual_battery_t *battery,
                            const transport_route_t *route)
{
    if (!route || route->kind == TRANSPORT_KIND_NONE) return 0;
    return battery_spend(battery, route_cost(route->kind));
}

static personal_telemetry_sample_t steps_sample(uint32_t session,
                                                telemetry_quality_t quality)
{
    personal_telemetry_sample_t sample;
    memset(&sample, 0, sizeof(sample));
    sample.capture_session_id = session;
    sample.capture_authorized = 1u;
    sample.collect_consent = 1u;
    sample.muted = 0u;
    sample.kind = TELEMETRY_KIND_STEPS;
    sample.source = TELEMETRY_SOURCE_ACCELEROMETER;
    sample.quality = quality;
    sample.value = 3200;
    sample.unit_scale = 1;
    sample.window_start_ms = 100u;
    sample.window_end_ms = 200u;
    sample.now_ms = 500u;
    return sample;
}

static void scenario_route_matrix(sim_score *score,
                                  virtual_battery_t *battery)
{
    transport_route_t route;
    transport_selector_metrics_t metrics;
    transport_selector_input_t input;
    memset(&metrics, 0, sizeof(metrics));

    input = route_input(TRANSPORT_ENV_REMOTE, TRANSPORT_PAYLOAD_STATE, 24u);
    sim_ok(score, transport_selector_choose(&input, &route, &metrics) ==
                    TRANSPORT_SELECTOR_OK && route.kind == TRANSPORT_KIND_LORA,
           "remote Watch state selects LoRa");
    sim_ok(score, virtual_dispatch(battery, &route),
           "remote route consumes only bounded virtual energy");

    input = route_input(TRANSPORT_ENV_URBAN, TRANSPORT_PAYLOAD_CARD, 180u);
    sim_ok(score, transport_selector_choose(&input, &route, &metrics) ==
                    TRANSPORT_SELECTOR_OK && route.kind == TRANSPORT_KIND_ESPNOW,
           "urban Paper-Core card selects ESP-NOW");
    sim_ok(score, virtual_dispatch(battery, &route),
           "urban route consumes only bounded virtual energy");

    input = route_input(TRANSPORT_ENV_LOCAL, TRANSPORT_PAYLOAD_STATE, 20u);
    input.espnow_available = 0u;
    input.wifi_available = 0u;
    sim_ok(score, transport_selector_choose(&input, &route, &metrics) ==
                    TRANSPORT_SELECTOR_OK && route.kind == TRANSPORT_KIND_BLE,
           "local control falls back to BLE");
    sim_ok(score, virtual_dispatch(battery, &route),
           "local route consumes only bounded virtual energy");

    input = route_input(TRANSPORT_ENV_REMOTE, TRANSPORT_PAYLOAD_STATE, 24u);
    input.lora_available = 0u;
    sim_ok(score, transport_selector_choose(&input, &route, &metrics) ==
                    TRANSPORT_SELECTOR_NO_ROUTE,
           "remote failure abstains when LoRa is unavailable");
    sim_ok(score, route.kind == TRANSPORT_KIND_NONE,
           "route failure does not silently fall through to urban transport");
}

static void scenario_telemetry_gate(sim_score *score,
                                    virtual_battery_t *battery)
{
    personal_telemetry_t telemetry;
    personal_telemetry_sample_t sample;
    personal_telemetry_record_t record;
    transport_route_t route;
    transport_selector_metrics_t route_metrics;
    transport_selector_input_t input;

    personal_telemetry_init(&telemetry, NULL);
    sim_ok(score, personal_telemetry_begin(&telemetry, 7u, 1u, 100u) ==
                    PERSONAL_TELEMETRY_OK,
           "Watch opens a consented telemetry session");
    sample = steps_sample(7u, TELEMETRY_QUALITY_USABLE);
    sim_ok(score, personal_telemetry_observe(&telemetry, &sample) ==
                    PERSONAL_TELEMETRY_OK,
           "usable derived sensor sample becomes a transient candidate");
    sim_ok(score, telemetry.pending.persist_authorized == 0u,
           "candidate is not persistence-authorized before a gesture");
    sim_ok(score, personal_telemetry_confirm(&telemetry, 1u, 500u, &record) ==
                    PERSONAL_TELEMETRY_OK && record.persist_authorized == 1u,
           "physical confirmation emits an authorized local telemetry record");
    sim_ok(score, battery_spend(battery, VIRTUAL_COST_SENSOR_UNITS),
           "sensor processing consumes declared virtual energy units");

    input = route_input(TRANSPORT_ENV_URBAN,
                        TRANSPORT_PAYLOAD_PERSONAL_TELEMETRY, 32u);
    memset(&route_metrics, 0, sizeof(route_metrics));
    sim_ok(score, transport_selector_choose(&input, &route, &route_metrics) ==
                    TRANSPORT_SELECTOR_E_PRIVACY,
           "local telemetry is not shared without separate consent");
    input.share_confirmed = 1u;
    sim_ok(score, transport_selector_choose(&input, &route, &route_metrics) ==
                    TRANSPORT_SELECTOR_OK,
           "telemetry route requires a separate sharing confirmation");
    sim_ok(score, virtual_dispatch(battery, &route),
           "confirmed telemetry share uses a bounded urban route");
}

static void scenario_adversarial_edges(sim_score *score)
{
    personal_telemetry_t telemetry;
    personal_telemetry_sample_t sample;
    personal_telemetry_record_t record;
    transport_selector_input_t input;
    transport_route_t route;
    transport_selector_metrics_t metrics;
    virtual_battery_t low_battery;

    personal_telemetry_init(&telemetry, NULL);
    sim_ok(score, personal_telemetry_begin(&telemetry, 8u, 1u, 100u) ==
                    PERSONAL_TELEMETRY_OK,
           "adversarial session starts from a clean state");
    sample = steps_sample(8u, TELEMETRY_QUALITY_LOW);
    sim_ok(score, personal_telemetry_observe(&telemetry, &sample) ==
                    PERSONAL_TELEMETRY_E_QUALITY,
           "low-quality sensor data is rejected rather than invented");

    personal_telemetry_init(&telemetry, NULL);
    sim_ok(score, personal_telemetry_begin(&telemetry, 9u, 1u, 100u) ==
                    PERSONAL_TELEMETRY_OK,
           "expiry case starts from a clean state");
    sample = steps_sample(9u, TELEMETRY_QUALITY_USABLE);
    sim_ok(score, personal_telemetry_observe(&telemetry, &sample) ==
                    PERSONAL_TELEMETRY_OK,
           "expiry case creates a candidate");
    sim_ok(score, personal_telemetry_tick(&telemetry, 8200u) ==
                    PERSONAL_TELEMETRY_E_EXPIRED,
           "unconfirmed telemetry expires in the virtual clock");
    sim_ok(score, personal_telemetry_confirm(&telemetry, 1u, 8201u, &record) ==
                    PERSONAL_TELEMETRY_E_STATE,
           "expired telemetry cannot be resurrected");

    input = route_input(TRANSPORT_ENV_REMOTE, TRANSPORT_PAYLOAD_STATE, 24u);
    input.physical_authorized = 0u;
    memset(&metrics, 0, sizeof(metrics));
    sim_ok(score, transport_selector_choose(&input, &route, &metrics) ==
                    TRANSPORT_SELECTOR_E_AUTHORITY,
           "physical revocation blocks every virtual transport");

    battery_init(&low_battery);
    low_battery.remaining_units = 50u;
    input = route_input(TRANSPORT_ENV_REMOTE, TRANSPORT_PAYLOAD_STATE, 24u);
    sim_ok(score, transport_selector_choose(&input, &route, &metrics) ==
                    TRANSPORT_SELECTOR_OK,
           "route policy can still recommend LoRa before energy admission");
    sim_ok(score, !virtual_dispatch(&low_battery, &route) &&
                    low_battery.rejected_spends == 1u,
           "virtual battery admission blocks an unaffordable send");
}

static void scenario_real_lora_leg(sim_score *score,
                                   virtual_battery_t *battery)
{
    transport_selector_input_t input;
    transport_route_t route;
    transport_selector_metrics_t metrics;
    sim_world world;
    uint8_t roles[1] = { 1u };
    uint16_t fillers[1] = { 1u };

    input = route_input(TRANSPORT_ENV_REMOTE, TRANSPORT_PAYLOAD_STATE, 24u);
    memset(&metrics, 0, sizeof(metrics));
    sim_ok(score, transport_selector_choose(&input, &route, &metrics) ==
                    TRANSPORT_SELECTOR_OK && route.kind == TRANSPORT_KIND_LORA,
           "virtual remote intent reaches the real LoRa simulator leg");
    sim_ok(score, virtual_dispatch(battery, &route),
           "LoRa leg passes virtual energy admission");

    sim_world_init(&world, 2, 0x4845525553ull);
    world.n[0].x = 0.0;
    world.n[0].y = 0.0;
    world.n[1].x = 30.0;
    world.n[1].y = 0.0;
    world.n[0].band = 0;
    world.n[1].band = 0;
    weave_init(&world.n[0].weave, WEAVE_LEAF);
    weave_init(&world.n[1].weave, WEAVE_LEAF);
    sim_pair(&world, 0, 1, 0x900Du);
    sim_queue_send(&world, 0u, 0, 1, 1u, 1, roles, fillers, 2u);
    world.end_us = 6000000ull;
    sim_run(&world);

    sim_ok(score, world.g_delivered == 1u,
           "a virtual remote message is delivered by the real firmware/net path");
    sim_ok(score, world.g_false_deliveries == 0u,
           "the real LoRa leg has no false delivery");
    sim_ok(score, world.n[1].n_opened == 1u,
           "the authenticated peer opens the delivered frame");
    sim_world_free(&world);
}

void scenario_virtual(sim_score *score, int argc, char **argv)
{
    (void)argc;
    (void)argv;
    v_header("virtual. pre-hardware lab — Watch, Paper-Core, sensors, battery and LoRa");

    virtual_battery_t battery;
    battery_init(&battery);
    printf("  battery model: %u virtual units; no physical current claim\n",
           battery.capacity_units);

    scenario_route_matrix(score, &battery);
    scenario_telemetry_gate(score, &battery);
    scenario_adversarial_edges(score);
    scenario_real_lora_leg(score, &battery);

    printf("  virtual energy consumed %u / %u units; rejected spends %u\n",
           battery.consumed_units, battery.capacity_units,
           battery.rejected_spends);
    sim_ok(score, battery.consumed_units < battery.capacity_units,
           "the virtual battery retains headroom under the scripted day slice");
    sim_ok(score, battery.rejected_spends == 0u,
           "the normal scripted path does not overspend the virtual battery");
}
