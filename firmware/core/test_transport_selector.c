#include "transport_selector.h"
#include <stdio.h>
#include <string.h>

static int failures;

static void ok(int condition, const char *message)
{
    if (!condition) {
        printf("FAIL: %s\n", message);
        failures++;
    }
}

static transport_selector_input_t base_input(void)
{
    transport_selector_input_t input;
    memset(&input, 0, sizeof(input));
    input.environment = TRANSPORT_ENV_REMOTE;
    input.payload_class = TRANSPORT_PAYLOAD_STATE;
    input.payload_bytes = 24u;
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

static void test_remote_and_urban(void)
{
    transport_selector_input_t input = base_input();
    transport_route_t route;
    transport_selector_metrics_t metrics;

    memset(&metrics, 0, sizeof(metrics));
    ok(transport_selector_choose(&input, &route, &metrics) ==
           TRANSPORT_SELECTOR_OK,
       "remote state chooses a route");
    ok(route.kind == TRANSPORT_KIND_LORA &&
           route.requires_application_ack == 1u &&
           route.requires_user_confirmation == 1u,
       "remote state prefers LoRa with explicit downstream gates");

    input.environment = TRANSPORT_ENV_URBAN;
    ok(transport_selector_choose(&input, &route, &metrics) ==
           TRANSPORT_SELECTOR_OK,
       "urban state chooses a route");
    ok(route.kind == TRANSPORT_KIND_ESPNOW,
       "urban state prefers authenticated ESP-NOW");

    input.espnow_available = 0u;
    ok(transport_selector_choose(&input, &route, &metrics) ==
           TRANSPORT_SELECTOR_OK,
       "urban state falls back when preferred route is absent");
    ok(route.kind == TRANSPORT_KIND_WIFI &&
           (route.reasons & TRANSPORT_REASON_FALLBACK) != 0u,
       "urban fallback is explicit and uses local Wi-Fi");

    input.wifi_available = 0u;
    ok(transport_selector_choose(&input, &route, &metrics) ==
           TRANSPORT_SELECTOR_OK,
       "urban state can use BLE as a bounded fallback");
    ok(route.kind == TRANSPORT_KIND_BLE,
       "urban BLE fallback is selected only after larger transports fail");
}

static void test_local_and_limits(void)
{
    transport_selector_input_t input = base_input();
    transport_route_t route;
    transport_selector_metrics_t metrics;
    memset(&metrics, 0, sizeof(metrics));

    input.environment = TRANSPORT_ENV_LOCAL;
    input.payload_class = TRANSPORT_PAYLOAD_STATE;
    ok(transport_selector_choose(&input, &route, &metrics) ==
           TRANSPORT_SELECTOR_OK && route.kind == TRANSPORT_KIND_BLE,
       "local state prefers BLE for short control");

    input.payload_class = TRANSPORT_PAYLOAD_CARD;
    ok(transport_selector_choose(&input, &route, &metrics) ==
           TRANSPORT_SELECTOR_OK && route.kind == TRANSPORT_KIND_WIFI,
       "local card prefers Wi-Fi for larger local context");

    input.environment = TRANSPORT_ENV_REMOTE;
    input.payload_bytes = 65u;
    ok(transport_selector_choose(&input, &route, &metrics) ==
           TRANSPORT_SELECTOR_NO_ROUTE,
       "remote payload above LoRa capacity abstains");
    ok((route.reasons & TRANSPORT_REASON_SIZE_LIMIT) != 0u,
       "capacity failure is exposed without choosing a different scope");
}

static void test_authority_and_privacy(void)
{
    transport_selector_input_t input = base_input();
    transport_route_t route;
    transport_selector_metrics_t metrics;
    memset(&metrics, 0, sizeof(metrics));

    input.physical_authorized = 0u;
    ok(transport_selector_choose(&input, &route, &metrics) ==
           TRANSPORT_SELECTOR_E_AUTHORITY,
       "transport cannot be selected without physical authority");

    input.physical_authorized = 1u;
    input.peer_authenticated = 0u;
    ok(transport_selector_choose(&input, &route, &metrics) ==
           TRANSPORT_SELECTOR_E_AUTHORITY,
       "transport cannot be selected without authenticated peer");

    input.peer_authenticated = 1u;
    input.payload_class = TRANSPORT_PAYLOAD_PERSONAL_TELEMETRY;
    ok(transport_selector_choose(&input, &route, &metrics) ==
           TRANSPORT_SELECTOR_E_PRIVACY,
       "personal telemetry cannot be shared without a separate consent");

    input.share_confirmed = 1u;
    ok(transport_selector_choose(&input, &route, &metrics) ==
           TRANSPORT_SELECTOR_OK,
       "personal telemetry can be routed only after share consent");

    input.payload_class = TRANSPORT_PAYLOAD_AUDIO;
    ok(transport_selector_choose(&input, &route, &metrics) ==
           TRANSPORT_SELECTOR_E_PRIVACY,
       "audio is outside the semantic transport contract");

    input.payload_class = TRANSPORT_PAYLOAD_LOCATION;
    ok(transport_selector_choose(&input, &route, &metrics) ==
           TRANSPORT_SELECTOR_E_PRIVACY,
       "location is outside the initial transport contract");
}

static void test_malformed_inputs(void)
{
    transport_selector_input_t input = base_input();
    transport_route_t route;
    transport_selector_metrics_t metrics;
    memset(&metrics, 0, sizeof(metrics));

    input.payload_bytes = 0u;
    ok(transport_selector_choose(&input, &route, &metrics) ==
           TRANSPORT_SELECTOR_E_FORMAT,
       "zero payload is malformed");

    input = base_input();
    input.lora_available = 2u;
    ok(transport_selector_choose(&input, &route, &metrics) ==
           TRANSPORT_SELECTOR_E_FORMAT,
       "noncanonical availability is rejected");

    input = base_input();
    input.environment = TRANSPORT_ENV_UNKNOWN;
    ok(transport_selector_choose(&input, &route, &metrics) ==
           TRANSPORT_SELECTOR_E_FORMAT,
       "unknown environment is rejected");

    ok(metrics.malformed >= 3u,
       "malformed inputs remain counted without retaining payload data");
}

int main(void)
{
    test_remote_and_urban();
    test_local_and_limits();
    test_authority_and_privacy();
    test_malformed_inputs();

    if (failures) {
        printf("%d transport selector tests failed\n", failures);
        return 1;
    }
    printf("ALL TRANSPORT SELECTOR INVARIANTS HOLD\n");
    return 0;
}
