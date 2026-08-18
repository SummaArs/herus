/*
 * HERUS personal telemetry gate.
 *
 * This module accepts only a derived, typed sample from a future local sensor
 * adapter. It stores no raw sensor signal, audio, identity, location or key. A
 * qualified sample is a transient candidate. Only physical confirmation emits a
 * record with persist_authorized == 1. Sharing is a separate consent owned by the
 * transport/policy caller and is never implied by local retention.
 */
#ifndef HERUS_PERSONAL_TELEMETRY_H
#define HERUS_PERSONAL_TELEMETRY_H

#include <stdint.h>

#define PERSONAL_TELEMETRY_DEFAULT_WINDOW_MS 8000u
#define PERSONAL_TELEMETRY_MAX_WINDOW_MS     86400000u

typedef enum {
    TELEMETRY_KIND_NONE = 0,
    TELEMETRY_KIND_STEPS,
    TELEMETRY_KIND_ACTIVE_MINUTES,
    TELEMETRY_KIND_HEART_RATE,
    TELEMETRY_KIND_SLEEP_MINUTES,
    TELEMETRY_KIND_SKIN_TEMPERATURE,
    TELEMETRY_KIND_DISTANCE,
    TELEMETRY_KIND_ENERGY_ESTIMATE,
    TELEMETRY_KIND_COUNT
} telemetry_kind_t;

typedef enum {
    TELEMETRY_SOURCE_NONE = 0,
    TELEMETRY_SOURCE_ACCELEROMETER,
    TELEMETRY_SOURCE_PPG,
    TELEMETRY_SOURCE_TEMPERATURE,
    TELEMETRY_SOURCE_GNSS,
    TELEMETRY_SOURCE_DERIVED,
    TELEMETRY_SOURCE_MANUAL,
    TELEMETRY_SOURCE_COUNT
} telemetry_source_t;

typedef enum {
    TELEMETRY_QUALITY_UNKNOWN = 0,
    TELEMETRY_QUALITY_LOW,
    TELEMETRY_QUALITY_USABLE,
    TELEMETRY_QUALITY_COUNT
} telemetry_quality_t;

typedef enum {
    TELEMETRY_IDLE = 0,
    TELEMETRY_CAPTURING,
    TELEMETRY_CANDIDATE,
    TELEMETRY_CONFIRMED,
    TELEMETRY_REJECTED,
    TELEMETRY_EXPIRED,
    TELEMETRY_MUTED
} personal_telemetry_state_t;

enum {
    PERSONAL_TELEMETRY_OK = 0,
    PERSONAL_TELEMETRY_NO_CANDIDATE = 1,
    PERSONAL_TELEMETRY_E_ARG = -1,
    PERSONAL_TELEMETRY_E_STATE = -2,
    PERSONAL_TELEMETRY_E_AUTH = -3,
    PERSONAL_TELEMETRY_E_FORMAT = -4,
    PERSONAL_TELEMETRY_E_QUALITY = -5,
    PERSONAL_TELEMETRY_E_EXPIRED = -6
};

enum {
    TELEMETRY_REASON_NONE = 0u,
    TELEMETRY_REASON_NO_SESSION = 1u << 0,
    TELEMETRY_REASON_NO_CONSENT = 1u << 1,
    TELEMETRY_REASON_MUTED = 1u << 2,
    TELEMETRY_REASON_LOW_QUALITY = 1u << 3,
    TELEMETRY_REASON_BAD_FORMAT = 1u << 4,
    TELEMETRY_REASON_EXPIRED = 1u << 5,
    TELEMETRY_REASON_USER_REJECTED = 1u << 6,
    TELEMETRY_REASON_OUT_OF_RANGE = 1u << 7
};

typedef struct {
    uint32_t capture_session_id;
    uint8_t capture_authorized;
    uint8_t collect_consent;
    uint8_t muted;
    telemetry_kind_t kind;
    telemetry_source_t source;
    telemetry_quality_t quality;
    int32_t value;
    int32_t unit_scale;
    uint32_t window_start_ms;
    uint32_t window_end_ms;
    uint32_t now_ms;
} personal_telemetry_sample_t;

typedef struct {
    uint8_t persist_authorized;
    telemetry_kind_t kind;
    telemetry_source_t source;
    telemetry_quality_t quality;
    int32_t value;
    int32_t unit_scale;
    uint32_t window_start_ms;
    uint32_t window_end_ms;
} personal_telemetry_record_t;

typedef struct {
    uint32_t sessions;
    uint32_t observations;
    uint32_t candidates;
    uint32_t confirmed;
    uint32_t rejected;
    uint32_t expired;
    uint32_t abstained_no_session;
    uint32_t abstained_no_consent;
    uint32_t abstained_muted;
    uint32_t abstained_low_quality;
    uint32_t malformed;
    uint32_t out_of_range;
} personal_telemetry_metrics_t;

typedef struct {
    uint32_t candidate_window_ms;
} personal_telemetry_config_t;

typedef struct {
    personal_telemetry_config_t cfg;
    personal_telemetry_state_t state;
    uint32_t capture_session_id;
    uint32_t candidate_expires_ms;
    personal_telemetry_record_t pending;
    uint32_t pending_reason;
    personal_telemetry_metrics_t metrics;
} personal_telemetry_t;

void personal_telemetry_config_default(personal_telemetry_config_t *out);
void personal_telemetry_init(personal_telemetry_t *t,
                             const personal_telemetry_config_t *cfg);

int personal_telemetry_begin(personal_telemetry_t *t,
                             uint32_t capture_session_id,
                             uint8_t collect_consent,
                             uint32_t now_ms);

int personal_telemetry_observe(personal_telemetry_t *t,
                               const personal_telemetry_sample_t *sample);

int personal_telemetry_confirm(personal_telemetry_t *t,
                               uint8_t accepted,
                               uint32_t now_ms,
                               personal_telemetry_record_t *out);

int personal_telemetry_tick(personal_telemetry_t *t, uint32_t now_ms);
void personal_telemetry_mute(personal_telemetry_t *t);
void personal_telemetry_forget(personal_telemetry_t *t);

const personal_telemetry_metrics_t *personal_telemetry_metrics(
    const personal_telemetry_t *t);
personal_telemetry_state_t personal_telemetry_state(
    const personal_telemetry_t *t);
uint32_t personal_telemetry_reason(const personal_telemetry_t *t);

#endif /* HERUS_PERSONAL_TELEMETRY_H */
