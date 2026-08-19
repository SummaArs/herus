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

typedef struct {
    uint16_t lora;
    uint16_t espnow;
    uint16_t ble;
    uint16_t wifi;
} virtual_capacities_t;

static int expected_route_kind(const transport_selector_input_t *input)
{
    if (!input || input->physical_authorized != 1u ||
        input->peer_authenticated != 1u ||
        input->payload_class == TRANSPORT_PAYLOAD_AUDIO ||
        input->payload_class == TRANSPORT_PAYLOAD_LOCATION ||
        (input->payload_class == TRANSPORT_PAYLOAD_PERSONAL_TELEMETRY &&
         input->share_confirmed != 1u)) {
        return TRANSPORT_KIND_NONE;
    }

    if (input->environment == TRANSPORT_ENV_REMOTE) {
        return input->lora_available && input->lora_max_bytes >= input->payload_bytes
                   ? TRANSPORT_KIND_LORA : TRANSPORT_KIND_NONE;
    }
    if (input->environment == TRANSPORT_ENV_URBAN) {
        if (input->espnow_available && input->espnow_max_bytes >= input->payload_bytes)
            return TRANSPORT_KIND_ESPNOW;
        if (input->wifi_available && input->wifi_max_bytes >= input->payload_bytes)
            return TRANSPORT_KIND_WIFI;
        if (input->ble_available && input->ble_max_bytes >= input->payload_bytes)
            return TRANSPORT_KIND_BLE;
        return TRANSPORT_KIND_NONE;
    }
    if (input->environment == TRANSPORT_ENV_LOCAL) {
        if (input->payload_class == TRANSPORT_PAYLOAD_STATE &&
            input->ble_available && input->ble_max_bytes >= input->payload_bytes)
            return TRANSPORT_KIND_BLE;
        if (input->wifi_available && input->wifi_max_bytes >= input->payload_bytes)
            return TRANSPORT_KIND_WIFI;
        if (input->espnow_available && input->espnow_max_bytes >= input->payload_bytes)
            return TRANSPORT_KIND_ESPNOW;
        if (input->payload_class == TRANSPORT_PAYLOAD_STATE &&
            input->lora_available && input->lora_max_bytes >= input->payload_bytes)
            return TRANSPORT_KIND_LORA;
    }
    return TRANSPORT_KIND_NONE;
}

static int expected_source_allowed(telemetry_kind_t kind,
                                   telemetry_source_t source)
{
    switch (kind) {
    case TELEMETRY_KIND_STEPS:
    case TELEMETRY_KIND_ACTIVE_MINUTES:
        return source == TELEMETRY_SOURCE_ACCELEROMETER ||
               source == TELEMETRY_SOURCE_DERIVED ||
               source == TELEMETRY_SOURCE_MANUAL;
    case TELEMETRY_KIND_HEART_RATE:
        return source == TELEMETRY_SOURCE_PPG ||
               source == TELEMETRY_SOURCE_DERIVED ||
               source == TELEMETRY_SOURCE_MANUAL;
    case TELEMETRY_KIND_SLEEP_MINUTES:
        return source == TELEMETRY_SOURCE_DERIVED ||
               source == TELEMETRY_SOURCE_MANUAL;
    case TELEMETRY_KIND_SKIN_TEMPERATURE:
        return source == TELEMETRY_SOURCE_TEMPERATURE ||
               source == TELEMETRY_SOURCE_DERIVED ||
               source == TELEMETRY_SOURCE_MANUAL;
    case TELEMETRY_KIND_DISTANCE:
        return source == TELEMETRY_SOURCE_GNSS ||
               source == TELEMETRY_SOURCE_ACCELEROMETER ||
               source == TELEMETRY_SOURCE_DERIVED ||
               source == TELEMETRY_SOURCE_MANUAL;
    case TELEMETRY_KIND_ENERGY_ESTIMATE:
        return source == TELEMETRY_SOURCE_DERIVED ||
               source == TELEMETRY_SOURCE_MANUAL;
    default:
        return 0;
    }
}

static int route_output_is_clean(const transport_route_t *route)
{
    return route && route->kind == TRANSPORT_KIND_NONE &&
           route->max_payload_bytes == 0u &&
           route->requires_application_ack == 0u &&
           route->requires_user_confirmation == 0u;
}

static void scenario_transport_exhaustive(sim_score *score)
{
    static const uint16_t payloads[] = {
        1u, 23u, 24u, 25u, 63u, 64u, 65u, 127u, 128u,
        129u, 249u, 250u, 251u, 1399u, 1400u, 1401u, 65535u
    };
    static const virtual_capacities_t capacities[] = {
        { 1u, 1u, 1u, 1u },
        { 24u, 24u, 24u, 24u },
        { 64u, 64u, 64u, 64u },
        { 128u, 128u, 128u, 128u },
        { 250u, 250u, 250u, 250u },
        { 1400u, 1400u, 1400u, 1400u },
        { 64u, 250u, 128u, 1400u },
        { 1u, 250u, 1u, 1400u },
        { 64u, 1u, 128u, 1400u },
        { 64u, 250u, 1u, 1u },
        { 1400u, 1u, 128u, 250u },
        { 1u, 1400u, 250u, 64u }
    };
    int cases = 0;
    int violations = 0;
    int selected = 0;
    int abstained = 0;

    for (int env = TRANSPORT_ENV_REMOTE; env < TRANSPORT_ENV_COUNT; env++) {
        for (int payload = TRANSPORT_PAYLOAD_STATE;
             payload < TRANSPORT_PAYLOAD_COUNT; payload++) {
            for (int share = 0; share <= 1; share++) {
                for (unsigned mask = 0u; mask < 16u; mask++) {
                    for (size_t c = 0u; c < sizeof capacities / sizeof capacities[0]; c++) {
                        for (size_t p = 0u; p < sizeof payloads / sizeof payloads[0]; p++) {
                            transport_selector_input_t input = route_input(
                                (transport_environment_t)env,
                                (transport_payload_class_t)payload,
                                payloads[p]);
                            transport_route_t route;
                            transport_selector_metrics_t metrics;
                            int expected;
                            int status;

                            input.share_confirmed = (uint8_t)share;
                            input.lora_available = (uint8_t)((mask >> 0) & 1u);
                            input.espnow_available = (uint8_t)((mask >> 1) & 1u);
                            input.ble_available = (uint8_t)((mask >> 2) & 1u);
                            input.wifi_available = (uint8_t)((mask >> 3) & 1u);
                            input.lora_max_bytes = capacities[c].lora;
                            input.espnow_max_bytes = capacities[c].espnow;
                            input.ble_max_bytes = capacities[c].ble;
                            input.wifi_max_bytes = capacities[c].wifi;
                            memset(&metrics, 0, sizeof(metrics));
                            expected = expected_route_kind(&input);
                            status = transport_selector_choose(&input, &route, &metrics);
                            cases++;

                            if (expected == TRANSPORT_KIND_NONE) {
                                if (status != TRANSPORT_SELECTOR_NO_ROUTE &&
                                    status != TRANSPORT_SELECTOR_E_PRIVACY) {
                                    violations++;
                                }
                                if (!route_output_is_clean(&route) &&
                                    route.reasons == TRANSPORT_REASON_NONE) {
                                    violations++;
                                }
                                abstained++;
                            } else {
                                if (status != TRANSPORT_SELECTOR_OK ||
                                    route.kind != (transport_kind_t)expected ||
                                    route.max_payload_bytes < input.payload_bytes ||
                                    route.requires_application_ack != 1u ||
                                    route.requires_user_confirmation != 1u) {
                                    violations++;
                                }
                                selected++;
                            }
                            if (metrics.evaluations != 1u) {
                                violations++;
                            }
                            if (status == TRANSPORT_SELECTOR_OK &&
                                (metrics.selected != 1u || metrics.abstained != 0u)) {
                                violations++;
                            }
                            if (status == TRANSPORT_SELECTOR_NO_ROUTE &&
                                (metrics.selected != 0u || metrics.abstained != 1u)) {
                                violations++;
                            }
                            if ((status == TRANSPORT_SELECTOR_E_AUTHORITY ||
                                 status == TRANSPORT_SELECTOR_E_PRIVACY) &&
                                (metrics.selected != 0u || metrics.abstained != 0u)) {
                                violations++;
                            }
                        }
                    }
                }
            }
        }
    }

    sim_ok(score, violations == 0,
           "exhaustive transport matrix has no route or authority violation");
    printf("  transport matrix: %d cases, %d selected, %d abstained, %d violations\n",
           cases, selected, abstained, violations);
}

static void scenario_transport_malformed(sim_score *score)
{
    transport_selector_input_t base = route_input(TRANSPORT_ENV_URBAN,
                                                  TRANSPORT_PAYLOAD_STATE, 24u);
    transport_route_t route;
    transport_selector_metrics_t metrics;
    int violations = 0;
    int cases = 0;

    for (unsigned value = 2u; value != 0u; value = value == 2u ? 255u : 0u) {
        transport_selector_input_t input = base;
        input.lora_available = (uint8_t)value;
        memset(&metrics, 0, sizeof(metrics));
        if (transport_selector_choose(&input, &route, &metrics) !=
                TRANSPORT_SELECTOR_E_FORMAT || !route_output_is_clean(&route) ||
            metrics.malformed != 1u) violations++;
        cases++;
        if (value == 255u) break;
    }

    {
        transport_selector_input_t input = base;
        input.environment = TRANSPORT_ENV_UNKNOWN;
        memset(&metrics, 0, sizeof(metrics));
        if (transport_selector_choose(&input, &route, &metrics) !=
                TRANSPORT_SELECTOR_E_FORMAT || !route_output_is_clean(&route))
            violations++;
        cases++;
    }
    {
        transport_selector_input_t input = base;
        input.payload_class = TRANSPORT_PAYLOAD_COUNT;
        memset(&metrics, 0, sizeof(metrics));
        if (transport_selector_choose(&input, &route, &metrics) !=
                TRANSPORT_SELECTOR_E_FORMAT || !route_output_is_clean(&route))
            violations++;
        cases++;
    }
    {
        transport_selector_input_t input = base;
        input.payload_bytes = 0u;
        memset(&metrics, 0, sizeof(metrics));
        if (transport_selector_choose(&input, &route, &metrics) !=
                TRANSPORT_SELECTOR_E_FORMAT || !route_output_is_clean(&route))
            violations++;
        cases++;
    }
    {
        transport_selector_input_t input = base;
        input.espnow_available = 1u;
        input.espnow_max_bytes = 0u;
        memset(&metrics, 0, sizeof(metrics));
        if (transport_selector_choose(&input, &route, &metrics) !=
                TRANSPORT_SELECTOR_E_FORMAT || !route_output_is_clean(&route))
            violations++;
        cases++;
    }
    if (transport_selector_choose(NULL, &route, &metrics) !=
            TRANSPORT_SELECTOR_E_ARG || !route_output_is_clean(&route)) violations++;
    cases++;
    if (transport_selector_choose(&base, NULL, &metrics) != TRANSPORT_SELECTOR_E_ARG)
        violations++;
    cases++;

    sim_ok(score, violations == 0,
           "malformed transport inputs fail closed and scrub route output");
    printf("  malformed transport probes: %d cases, %d violations\n", cases, violations);
}

static int telemetry_bounds(telemetry_kind_t kind, int32_t *min_value,
                            int32_t *max_value)
{
    if (!min_value || !max_value) return 0;
    switch (kind) {
    case TELEMETRY_KIND_STEPS:
        *min_value = 0; *max_value = 100000; return 1;
    case TELEMETRY_KIND_ACTIVE_MINUTES:
    case TELEMETRY_KIND_SLEEP_MINUTES:
        *min_value = 0; *max_value = 1440; return 1;
    case TELEMETRY_KIND_HEART_RATE:
        *min_value = 20; *max_value = 240; return 1;
    case TELEMETRY_KIND_SKIN_TEMPERATURE:
        *min_value = -5000; *max_value = 10000; return 1;
    case TELEMETRY_KIND_DISTANCE:
        *min_value = 0; *max_value = 100000000; return 1;
    case TELEMETRY_KIND_ENERGY_ESTIMATE:
        *min_value = 0; *max_value = 10000000; return 1;
    default:
        return 0;
    }
}

static int telemetry_window_valid(uint32_t start, uint32_t end,
                                  uint32_t now)
{
    uint32_t span;
    if (end < start) return 0;
    span = end - start;
    return span > 0u && span <= PERSONAL_TELEMETRY_MAX_WINDOW_MS && end <= now;
}

static void scenario_telemetry_exhaustive(sim_score *score)
{
    static const telemetry_quality_t qualities[] = {
        TELEMETRY_QUALITY_UNKNOWN, TELEMETRY_QUALITY_LOW,
        TELEMETRY_QUALITY_USABLE
    };
    static const int window_cases[][3] = {
        { 1000, 2000, 2000 },
        { 2000, 1000, 2000 },
        { 1000, 1000, 2000 },
        { 1000, 9000, 2000 },
        { 0, 86400000, 86400000 },
        { 0, 86400001, 86400001 }
    };
    int cases = 0;
    int accepted = 0;
    int rejected = 0;
    int violations = 0;

    for (int kind = TELEMETRY_KIND_STEPS;
         kind < TELEMETRY_KIND_COUNT; kind++) {
        int32_t min_value;
        int32_t max_value;
        int32_t values[5];
        size_t value_count = 0u;
        if (!telemetry_bounds((telemetry_kind_t)kind, &min_value, &max_value)) {
            violations++;
            continue;
        }
        values[value_count++] = min_value;
        values[value_count++] = max_value;
        if (min_value > -2147483647) values[value_count++] = min_value - 1;
        if (max_value < 2147483647) values[value_count++] = max_value + 1;
        values[value_count++] = (min_value + max_value) / 2;

        for (int source = TELEMETRY_SOURCE_ACCELEROMETER;
             source < TELEMETRY_SOURCE_COUNT; source++) {
            for (size_t qi = 0u; qi < sizeof qualities / sizeof qualities[0]; qi++) {
                for (size_t vi = 0u; vi < value_count; vi++) {
                    for (size_t wi = 0u;
                         wi < sizeof window_cases / sizeof window_cases[0]; wi++) {
                        personal_telemetry_t telemetry;
                        personal_telemetry_sample_t sample;
                        personal_telemetry_record_t record;
                        int valid_value = values[vi] >= min_value &&
                                          values[vi] <= max_value;
                        int valid_window = telemetry_window_valid(
                            (uint32_t)window_cases[wi][0],
                            (uint32_t)window_cases[wi][1],
                            (uint32_t)window_cases[wi][2]);
                        int expected = expected_source_allowed(
                            (telemetry_kind_t)kind,
                            (telemetry_source_t)source) &&
                            qualities[qi] == TELEMETRY_QUALITY_USABLE &&
                            valid_value && valid_window;
                        int status;

                        personal_telemetry_init(&telemetry, NULL);
                        if (personal_telemetry_begin(&telemetry, 99u, 1u,
                                                     (uint32_t)window_cases[wi][2]) !=
                                PERSONAL_TELEMETRY_OK) {
                            violations++;
                            continue;
                        }
                        memset(&sample, 0, sizeof(sample));
                        sample.capture_session_id = 99u;
                        sample.capture_authorized = 1u;
                        sample.collect_consent = 1u;
                        sample.muted = 0u;
                        sample.kind = (telemetry_kind_t)kind;
                        sample.source = (telemetry_source_t)source;
                        sample.quality = qualities[qi];
                        sample.value = values[vi];
                        sample.unit_scale = 1;
                        sample.window_start_ms = (uint32_t)window_cases[wi][0];
                        sample.window_end_ms = (uint32_t)window_cases[wi][1];
                        sample.now_ms = (uint32_t)window_cases[wi][2];
                        status = personal_telemetry_observe(&telemetry, &sample);
                        cases++;

                        if (expected) {
                            if (status != PERSONAL_TELEMETRY_OK ||
                                personal_telemetry_state(&telemetry) !=
                                    TELEMETRY_CANDIDATE ||
                                telemetry.pending.persist_authorized != 0u ||
                                personal_telemetry_confirm(&telemetry, 1u,
                                                           (uint32_t)window_cases[wi][2] + 1u,
                                                           &record) != PERSONAL_TELEMETRY_OK ||
                                record.persist_authorized != 1u ||
                                telemetry.pending.kind != TELEMETRY_KIND_NONE ||
                                telemetry.pending.persist_authorized != 0u) {
                                violations++;
                            }
                            accepted++;
                        } else {
                            if (status == PERSONAL_TELEMETRY_OK ||
                                personal_telemetry_state(&telemetry) ==
                                    TELEMETRY_CANDIDATE ||
                                telemetry.pending.persist_authorized != 0u ||
                                telemetry.pending.kind != TELEMETRY_KIND_NONE ||
                                telemetry.pending.value != 0) {
                                violations++;
                            }
                            rejected++;
                        }
                    }
                }
            }
        }
    }

    sim_ok(score, violations == 0,
           "exhaustive telemetry matrix never promotes an invalid sample");
    printf("  telemetry matrix: %d cases, %d accepted, %d rejected, %d violations\n",
           cases, accepted, rejected, violations);
}

static void scenario_telemetry_sequences(sim_score *score)
{
    personal_telemetry_t telemetry;
    personal_telemetry_sample_t sample = steps_sample(123u, TELEMETRY_QUALITY_USABLE);
    personal_telemetry_record_t record;
    int violations = 0;

    personal_telemetry_init(&telemetry, NULL);
    if (personal_telemetry_begin(&telemetry, 123u, 1u, 100u) !=
            PERSONAL_TELEMETRY_OK ||
        personal_telemetry_observe(&telemetry, &sample) != PERSONAL_TELEMETRY_OK) {
        violations++;
    }
    if (personal_telemetry_begin(&telemetry, 124u, 1u, 101u) !=
            PERSONAL_TELEMETRY_E_STATE ||
        telemetry.pending.persist_authorized != 0u) {
        violations++;
    }
    if (personal_telemetry_confirm(&telemetry, 2u, 200u, &record) !=
            PERSONAL_TELEMETRY_E_ARG ||
        telemetry.pending.persist_authorized != 0u ||
        personal_telemetry_state(&telemetry) != TELEMETRY_CANDIDATE) {
        violations++;
    }
    if (personal_telemetry_confirm(&telemetry, 0u, 200u, &record) !=
            PERSONAL_TELEMETRY_NO_CANDIDATE ||
        personal_telemetry_state(&telemetry) != TELEMETRY_REJECTED ||
        telemetry.pending.kind != TELEMETRY_KIND_NONE) {
        violations++;
    }
    if (personal_telemetry_confirm(&telemetry, 1u, 201u, &record) !=
            PERSONAL_TELEMETRY_E_STATE || record.persist_authorized != 0u) {
        violations++;
    }

    personal_telemetry_forget(&telemetry);
    if (personal_telemetry_begin(&telemetry, 125u, 1u, 300u) !=
            PERSONAL_TELEMETRY_OK) {
        violations++;
    }
    sample = steps_sample(125u, TELEMETRY_QUALITY_USABLE);
    if (personal_telemetry_observe(&telemetry, &sample) != PERSONAL_TELEMETRY_OK) {
        violations++;
    }
    personal_telemetry_mute(&telemetry);
    if (personal_telemetry_state(&telemetry) != TELEMETRY_MUTED ||
        telemetry.pending.kind != TELEMETRY_KIND_NONE ||
        personal_telemetry_confirm(&telemetry, 1u, 301u, &record) !=
            PERSONAL_TELEMETRY_E_STATE) {
        violations++;
    }

    personal_telemetry_init(&telemetry, NULL);
    if (personal_telemetry_begin(&telemetry, 500u, 1u, 100u) !=
            PERSONAL_TELEMETRY_OK ||
        personal_telemetry_observe(&telemetry,
                                   &(personal_telemetry_sample_t){
                                       .capture_session_id = 501u,
                                       .capture_authorized = 1u,
                                       .collect_consent = 1u,
                                       .muted = 0u,
                                       .kind = TELEMETRY_KIND_STEPS,
                                       .source = TELEMETRY_SOURCE_ACCELEROMETER,
                                       .quality = TELEMETRY_QUALITY_USABLE,
                                       .value = 1,
                                       .unit_scale = 1,
                                       .window_start_ms = 1u,
                                       .window_end_ms = 2u,
                                       .now_ms = 2u }) != PERSONAL_TELEMETRY_E_AUTH ||
        personal_telemetry_state(&telemetry) != TELEMETRY_REJECTED ||
        telemetry.pending.kind != TELEMETRY_KIND_NONE) {
        violations++;
    }

    personal_telemetry_init(&telemetry, NULL);
    sample = steps_sample(126u, TELEMETRY_QUALITY_USABLE);
    sample.window_start_ms = 0xffffe000u;
    sample.window_end_ms = 0xfffff000u;
    sample.now_ms = 0xfffffff0u;
    if (personal_telemetry_begin(&telemetry, 126u, 1u, sample.now_ms) !=
            PERSONAL_TELEMETRY_OK ||
        personal_telemetry_observe(&telemetry, &sample) != PERSONAL_TELEMETRY_OK ||
        personal_telemetry_tick(&telemetry, 0x00000010u) != PERSONAL_TELEMETRY_OK ||
        personal_telemetry_state(&telemetry) != TELEMETRY_CANDIDATE ||
        personal_telemetry_tick(&telemetry, 0x00001f30u) !=
            PERSONAL_TELEMETRY_E_EXPIRED ||
        personal_telemetry_state(&telemetry) != TELEMETRY_EXPIRED) {
        violations++;
    }

    personal_telemetry_init(&telemetry, NULL);
    sample = steps_sample(127u, TELEMETRY_QUALITY_USABLE);
    sample.window_start_ms = 0xfffff000u;
    sample.window_end_ms = 0x00001000u;
    sample.now_ms = 0x00002000u;
    if (personal_telemetry_begin(&telemetry, 127u, 1u, sample.now_ms) !=
            PERSONAL_TELEMETRY_OK ||
        personal_telemetry_observe(&telemetry, &sample) != PERSONAL_TELEMETRY_OK ||
        personal_telemetry_confirm(&telemetry, 1u, 0x00002001u, &record) !=
            PERSONAL_TELEMETRY_OK || record.persist_authorized != 1u) {
        violations++;
    }

    personal_telemetry_init(&telemetry, NULL);
    sample = steps_sample(128u, TELEMETRY_QUALITY_USABLE);
    sample.window_start_ms = 0xfffff000u;
    sample.window_end_ms = 0x00001000u;
    sample.now_ms = 0xfffffff0u;
    if (personal_telemetry_begin(&telemetry, 128u, 1u, sample.now_ms) !=
            PERSONAL_TELEMETRY_OK ||
        personal_telemetry_observe(&telemetry, &sample) !=
            PERSONAL_TELEMETRY_E_FORMAT ||
        personal_telemetry_state(&telemetry) != TELEMETRY_REJECTED ||
        telemetry.pending.kind != TELEMETRY_KIND_NONE) {
        violations++;
    }

    sim_ok(score, violations == 0,
           "telemetry terminal states, malformed gestures and clock wrap fail closed");
}

static void scenario_combined_radio_campaign(sim_score *score)
{
    int violations = 0;
    int exercised = 0;
    uint32_t total_delivered = 0u;
    uint32_t total_false = 0u;

    for (int case_id = 0; case_id < 16; case_id++) {
        sim_world world;
        const uint64_t seed = 0xA11CE000ull + (uint64_t)case_id * 7919ull;
        const uint64_t base = 14000000ull;
        const int distance = 50 + case_id * 7;
        uint8_t roles[1] = { 2u };
        uint16_t fillers[1] = { (uint16_t)(case_id + 1) };
        int honest = 6;

        sim_world_init(&world, 4, seed);
        world.ch.shadow_db = (double)(case_id % 5) * 2.5;
        world.ch.nchan = 1 + (case_id % 3);
        world.cad = case_id % 2;
        world.retries = 1 + (case_id % 3 == 0 ? 1 : 0);
        world.n[0].x = 0.0;
        world.n[1].x = (double)distance;
        world.n[2].x = (double)distance / 2.0;
        world.n[3].x = (double)distance + 5.0;
        world.n[0].band = case_id % 2;
        world.n[1].band = case_id % 2;
        world.n[2].band = 1;
        world.n[3].band = 1;
        weave_init(&world.n[0].weave, WEAVE_LEAF);
        weave_init(&world.n[1].weave, WEAVE_LEAF);
        weave_init(&world.n[2].weave, WEAVE_RELAY);
        weave_init(&world.n[3].weave, WEAVE_RELAY);
        world.n[3].adversary = 1;
        sim_pair(&world, 0, 1, seed ^ 0x55AAu);

        for (int message = 0; message < honest; message++) {
            sim_queue_send(&world, (uint64_t)message * 2000000ull,
                           0, 1, 4u, 1, roles, fillers, 3u);
        }

        for (int replay = 0; replay < 4; replay++) {
            sim_push(&world, base + (uint64_t)replay * 400000ull,
                     EV_ADVERSARY, 3, ADV_REPLAY, 1);
        }
        for (int forge = 0; forge < 6; forge++) {
            sim_push(&world, base + 2000000ull + (uint64_t)forge * 300000ull,
                     EV_ADVERSARY, 3, ADV_FORGE, 1);
        }
        for (int jam = 0; jam < 4; jam++) {
            uint64_t t = base + 5000000ull + (uint64_t)jam * 500000ull;
            sim_push(&world, t, EV_ADVERSARY, 3, ADV_JAM, 1);
            sim_queue_send(&world, t, 0, 1, 4u, 1, roles, fillers, 3u);
        }
        world.end_us = base + 12000000ull;
        sim_run(&world);
        exercised++;
        total_delivered += world.g_delivered;
        total_false += world.g_false_deliveries;

        if (world.g_false_deliveries != 0u ||
            world.g_delivered > (uint32_t)(honest + 4) ||
            world.n[2].n_opened != 0u ||
            world.n[3].n_opened != 0u ||
            world.n[3].cap_len > SIM_FRAME_MAX) {
            violations++;
        }
        sim_world_free(&world);
    }

    sim_ok(score, violations == 0,
           "combined radio campaign keeps ciphertext relays and adversaries from false delivery");
    printf("  combined radio campaign: %d worlds, %u delivered, %u false, %d violations\n",
           exercised, total_delivered, total_false, violations);
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
    scenario_transport_exhaustive(score);
    scenario_transport_malformed(score);
    scenario_telemetry_gate(score, &battery);
    scenario_telemetry_exhaustive(score);
    scenario_telemetry_sequences(score);
    scenario_adversarial_edges(score);
    scenario_combined_radio_campaign(score);
    scenario_real_lora_leg(score, &battery);

    printf("  virtual energy consumed %u / %u units; rejected spends %u\n",
           battery.consumed_units, battery.capacity_units,
           battery.rejected_spends);
    sim_ok(score, battery.consumed_units < battery.capacity_units,
           "the virtual battery retains headroom under the scripted day slice");
    sim_ok(score, battery.rejected_spends == 0u,
           "the normal scripted path does not overspend the virtual battery");
}
