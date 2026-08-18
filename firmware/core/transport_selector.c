#include "transport_selector.h"
#include <string.h>

static int canonical_bool(uint8_t value)
{
    return value == 0u || value == 1u;
}

static int valid_input(const transport_selector_input_t *input)
{
    if (!input) return 0;
    if (input->environment <= TRANSPORT_ENV_UNKNOWN ||
        input->environment >= TRANSPORT_ENV_COUNT ||
        input->payload_class >= TRANSPORT_PAYLOAD_COUNT ||
        !canonical_bool(input->lora_available) ||
        !canonical_bool(input->espnow_available) ||
        !canonical_bool(input->ble_available) ||
        !canonical_bool(input->wifi_available) ||
        !canonical_bool(input->peer_authenticated) ||
        !canonical_bool(input->physical_authorized) ||
        !canonical_bool(input->share_confirmed)) {
        return 0;
    }
    if (input->payload_bytes == 0u) return 0;
    if ((input->lora_available && input->lora_max_bytes == 0u) ||
        (input->espnow_available && input->espnow_max_bytes == 0u) ||
        (input->ble_available && input->ble_max_bytes == 0u) ||
        (input->wifi_available && input->wifi_max_bytes == 0u)) {
        return 0;
    }
    return 1;
}

static int can_use(uint8_t available, uint16_t capacity, uint16_t payload)
{
    return available == 1u && capacity >= payload;
}

static void select_route(transport_route_t *out, transport_kind_t kind,
                         uint16_t capacity, uint32_t reason)
{
    memset(out, 0, sizeof(*out));
    out->kind = kind;
    out->max_payload_bytes = capacity;
    out->requires_application_ack = 1u;
    out->requires_user_confirmation = 1u;
    out->reasons = reason;
}

static void note_no_route(transport_selector_metrics_t *metrics,
                          uint32_t reason)
{
    if (!metrics) return;
    metrics->abstained++;
    if (reason & TRANSPORT_REASON_SIZE_LIMIT) metrics->rejected_size++;
}

int transport_selector_choose(const transport_selector_input_t *input,
                              transport_route_t *out,
                              transport_selector_metrics_t *metrics)
{
    if (out) memset(out, 0, sizeof(*out));
    if (metrics) metrics->evaluations++;
    if (!input || !out) {
        if (metrics) metrics->malformed++;
        return TRANSPORT_SELECTOR_E_ARG;
    }
    if (!valid_input(input)) {
        if (metrics) metrics->malformed++;
        return TRANSPORT_SELECTOR_E_FORMAT;
    }
    if (input->physical_authorized != 1u) {
        out->reasons = TRANSPORT_REASON_NO_AUTHORITY;
        if (metrics) metrics->rejected_authority++;
        return TRANSPORT_SELECTOR_E_AUTHORITY;
    }
    if (input->payload_class == TRANSPORT_PAYLOAD_AUDIO ||
        input->payload_class == TRANSPORT_PAYLOAD_LOCATION) {
        out->reasons = TRANSPORT_REASON_UNSUPPORTED;
        if (metrics) metrics->rejected_privacy++;
        return TRANSPORT_SELECTOR_E_PRIVACY;
    }
    if (input->payload_class == TRANSPORT_PAYLOAD_PERSONAL_TELEMETRY &&
        input->share_confirmed != 1u) {
        out->reasons = TRANSPORT_REASON_SHARE_REQUIRED;
        if (metrics) metrics->rejected_privacy++;
        return TRANSPORT_SELECTOR_E_PRIVACY;
    }
    if (input->peer_authenticated != 1u) {
        out->reasons = TRANSPORT_REASON_PEER_REQUIRED;
        if (metrics) metrics->rejected_authority++;
        return TRANSPORT_SELECTOR_E_AUTHORITY;
    }

    if (input->environment == TRANSPORT_ENV_REMOTE) {
        if (can_use(input->lora_available, input->lora_max_bytes,
                    input->payload_bytes)) {
            select_route(out, TRANSPORT_KIND_LORA, input->lora_max_bytes,
                         TRANSPORT_REASON_SELECTED_REMOTE);
            if (metrics) metrics->selected++;
            return TRANSPORT_SELECTOR_OK;
        }
        out->reasons = input->lora_available ? TRANSPORT_REASON_SIZE_LIMIT
                                             : TRANSPORT_REASON_NO_ROUTE;
        note_no_route(metrics, out->reasons);
        return TRANSPORT_SELECTOR_NO_ROUTE;
    }

    if (input->environment == TRANSPORT_ENV_URBAN) {
        if (can_use(input->espnow_available, input->espnow_max_bytes,
                    input->payload_bytes)) {
            select_route(out, TRANSPORT_KIND_ESPNOW, input->espnow_max_bytes,
                         TRANSPORT_REASON_SELECTED_URBAN);
            if (metrics) metrics->selected++;
            return TRANSPORT_SELECTOR_OK;
        }
        if (can_use(input->wifi_available, input->wifi_max_bytes,
                    input->payload_bytes)) {
            select_route(out, TRANSPORT_KIND_WIFI, input->wifi_max_bytes,
                         TRANSPORT_REASON_SELECTED_URBAN |
                         TRANSPORT_REASON_FALLBACK);
            if (metrics) metrics->selected++;
            return TRANSPORT_SELECTOR_OK;
        }
        if (can_use(input->ble_available, input->ble_max_bytes,
                    input->payload_bytes)) {
            select_route(out, TRANSPORT_KIND_BLE, input->ble_max_bytes,
                         TRANSPORT_REASON_SELECTED_URBAN |
                         TRANSPORT_REASON_FALLBACK);
            if (metrics) metrics->selected++;
            return TRANSPORT_SELECTOR_OK;
        }
        out->reasons = TRANSPORT_REASON_NO_ROUTE | TRANSPORT_REASON_SIZE_LIMIT;
        note_no_route(metrics, out->reasons);
        return TRANSPORT_SELECTOR_NO_ROUTE;
    }

    if (input->environment == TRANSPORT_ENV_LOCAL) {
        if (input->payload_class == TRANSPORT_PAYLOAD_STATE &&
            can_use(input->ble_available, input->ble_max_bytes,
                    input->payload_bytes)) {
            select_route(out, TRANSPORT_KIND_BLE, input->ble_max_bytes,
                         TRANSPORT_REASON_SELECTED_LOCAL);
            if (metrics) metrics->selected++;
            return TRANSPORT_SELECTOR_OK;
        }
        if (can_use(input->wifi_available, input->wifi_max_bytes,
                    input->payload_bytes)) {
            select_route(out, TRANSPORT_KIND_WIFI, input->wifi_max_bytes,
                         TRANSPORT_REASON_SELECTED_LOCAL);
            if (metrics) metrics->selected++;
            return TRANSPORT_SELECTOR_OK;
        }
        if (can_use(input->espnow_available, input->espnow_max_bytes,
                    input->payload_bytes)) {
            select_route(out, TRANSPORT_KIND_ESPNOW, input->espnow_max_bytes,
                         TRANSPORT_REASON_SELECTED_LOCAL |
                         TRANSPORT_REASON_FALLBACK);
            if (metrics) metrics->selected++;
            return TRANSPORT_SELECTOR_OK;
        }
        if (can_use(input->lora_available, input->lora_max_bytes,
                    input->payload_bytes) &&
            input->payload_class == TRANSPORT_PAYLOAD_STATE) {
            select_route(out, TRANSPORT_KIND_LORA, input->lora_max_bytes,
                         TRANSPORT_REASON_SELECTED_LOCAL |
                         TRANSPORT_REASON_FALLBACK);
            if (metrics) metrics->selected++;
            return TRANSPORT_SELECTOR_OK;
        }
        out->reasons = TRANSPORT_REASON_NO_ROUTE | TRANSPORT_REASON_SIZE_LIMIT;
        note_no_route(metrics, out->reasons);
        return TRANSPORT_SELECTOR_NO_ROUTE;
    }

    out->reasons = TRANSPORT_REASON_NO_ROUTE;
    note_no_route(metrics, out->reasons);
    return TRANSPORT_SELECTOR_NO_ROUTE;
}
