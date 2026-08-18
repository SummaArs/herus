/*
 * HERUS transport selector.
 *
 * This module chooses a bounded transport recommendation from caller-owned facts.
 * It never opens a session, selects a peer identity, touches a key, serializes an
 * HCP, creates a frame or transmits. A recommendation is not delivery and is not
 * user consent. The caller must still use the authenticated link and delivery plan.
 */
#ifndef HERUS_TRANSPORT_SELECTOR_H
#define HERUS_TRANSPORT_SELECTOR_H

#include <stdint.h>

typedef enum {
    TRANSPORT_KIND_NONE = 0,
    TRANSPORT_KIND_LORA,
    TRANSPORT_KIND_ESPNOW,
    TRANSPORT_KIND_BLE,
    TRANSPORT_KIND_WIFI,
    TRANSPORT_KIND_COUNT
} transport_kind_t;

typedef enum {
    TRANSPORT_ENV_UNKNOWN = 0,
    TRANSPORT_ENV_REMOTE,
    TRANSPORT_ENV_URBAN,
    TRANSPORT_ENV_LOCAL,
    TRANSPORT_ENV_COUNT
} transport_environment_t;

typedef enum {
    TRANSPORT_PAYLOAD_STATE = 0,
    TRANSPORT_PAYLOAD_CARD,
    TRANSPORT_PAYLOAD_PERSONAL_TELEMETRY,
    TRANSPORT_PAYLOAD_LOCATION,
    TRANSPORT_PAYLOAD_AUDIO,
    TRANSPORT_PAYLOAD_BULK,
    TRANSPORT_PAYLOAD_COUNT
} transport_payload_class_t;

typedef enum {
    TRANSPORT_SELECTOR_OK = 0,
    TRANSPORT_SELECTOR_NO_ROUTE = 1,
    TRANSPORT_SELECTOR_E_ARG = -1,
    TRANSPORT_SELECTOR_E_FORMAT = -2,
    TRANSPORT_SELECTOR_E_AUTHORITY = -3,
    TRANSPORT_SELECTOR_E_PRIVACY = -4,
    TRANSPORT_SELECTOR_E_SIZE = -5
} transport_selector_status_t;

enum {
    TRANSPORT_REASON_NONE             = 0u,
    TRANSPORT_REASON_SELECTED_REMOTE  = 1u << 0,
    TRANSPORT_REASON_SELECTED_URBAN   = 1u << 1,
    TRANSPORT_REASON_SELECTED_LOCAL   = 1u << 2,
    TRANSPORT_REASON_FALLBACK         = 1u << 3,
    TRANSPORT_REASON_NO_ROUTE         = 1u << 4,
    TRANSPORT_REASON_PEER_REQUIRED    = 1u << 5,
    TRANSPORT_REASON_SHARE_REQUIRED   = 1u << 6,
    TRANSPORT_REASON_SIZE_LIMIT       = 1u << 7,
    TRANSPORT_REASON_UNSUPPORTED      = 1u << 8,
    TRANSPORT_REASON_NO_AUTHORITY     = 1u << 9
};

typedef struct {
    transport_environment_t environment;
    transport_payload_class_t payload_class;
    uint16_t payload_bytes;
    uint16_t lora_max_bytes;
    uint16_t espnow_max_bytes;
    uint16_t ble_max_bytes;
    uint16_t wifi_max_bytes;
    uint8_t lora_available;
    uint8_t espnow_available;
    uint8_t ble_available;
    uint8_t wifi_available;
    uint8_t peer_authenticated;
    uint8_t physical_authorized;
    uint8_t share_confirmed;
} transport_selector_input_t;

typedef struct {
    transport_kind_t kind;
    uint16_t max_payload_bytes;
    uint8_t requires_application_ack;
    uint8_t requires_user_confirmation;
    uint32_t reasons;
} transport_route_t;

typedef struct {
    uint32_t evaluations;
    uint32_t selected;
    uint32_t abstained;
    uint32_t rejected_authority;
    uint32_t rejected_privacy;
    uint32_t rejected_size;
    uint32_t malformed;
} transport_selector_metrics_t;

int transport_selector_choose(const transport_selector_input_t *input,
                              transport_route_t *out,
                              transport_selector_metrics_t *metrics);

#endif /* HERUS_TRANSPORT_SELECTOR_H */
