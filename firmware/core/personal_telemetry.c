#include "personal_telemetry.h"
#include <string.h>

static void clear_pending(personal_telemetry_t *t)
{
    if (!t) return;
    memset(&t->pending, 0, sizeof(t->pending));
    t->capture_session_id = 0u;
    t->candidate_expires_ms = 0u;
}

static void abstain(personal_telemetry_t *t, personal_telemetry_state_t state,
                    uint32_t reason)
{
    if (!t) return;
    clear_pending(t);
    t->pending_reason = reason;
    t->state = state;
}

static int canonical_bool(uint8_t value)
{
    return value == 0u || value == 1u;
}

static int valid_enums(const personal_telemetry_sample_t *s)
{
    return s->kind > TELEMETRY_KIND_NONE && s->kind < TELEMETRY_KIND_COUNT &&
           s->source > TELEMETRY_SOURCE_NONE &&
           s->source < TELEMETRY_SOURCE_COUNT &&
           s->quality > TELEMETRY_QUALITY_UNKNOWN &&
           s->quality < TELEMETRY_QUALITY_COUNT;
}

static int source_allowed(telemetry_kind_t kind, telemetry_source_t source)
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

static int value_in_range(const personal_telemetry_sample_t *s)
{
    if (!s || s->unit_scale <= 0 || s->unit_scale > 1000000) return 0;
    switch (s->kind) {
    case TELEMETRY_KIND_STEPS:
        return s->unit_scale == 1 && s->value >= 0 && s->value <= 100000;
    case TELEMETRY_KIND_ACTIVE_MINUTES:
    case TELEMETRY_KIND_SLEEP_MINUTES:
        return s->unit_scale == 1 && s->value >= 0 && s->value <= 1440;
    case TELEMETRY_KIND_HEART_RATE:
        return s->unit_scale == 1 && s->value >= 20 && s->value <= 240;
    case TELEMETRY_KIND_SKIN_TEMPERATURE:
        return s->value >= -5000 && s->value <= 10000;
    case TELEMETRY_KIND_DISTANCE:
        return s->value >= 0 && s->value <= 100000000;
    case TELEMETRY_KIND_ENERGY_ESTIMATE:
        return s->value >= 0 && s->value <= 10000000;
    default:
        return 0;
    }
}

static int window_valid(const personal_telemetry_sample_t *s)
{
    uint32_t span;
    if (!s || s->window_end_ms < s->window_start_ms) return 0;
    span = s->window_end_ms - s->window_start_ms;
    return span > 0u && span <= PERSONAL_TELEMETRY_MAX_WINDOW_MS &&
           s->window_end_ms <= s->now_ms;
}

static int expired(uint32_t now_ms, uint32_t deadline_ms)
{
    return (int32_t)(now_ms - deadline_ms) >= 0;
}

void personal_telemetry_config_default(personal_telemetry_config_t *out)
{
    if (!out) return;
    out->candidate_window_ms = PERSONAL_TELEMETRY_DEFAULT_WINDOW_MS;
}

void personal_telemetry_init(personal_telemetry_t *t,
                             const personal_telemetry_config_t *cfg)
{
    if (!t) return;
    memset(t, 0, sizeof(*t));
    if (cfg) t->cfg = *cfg;
    else personal_telemetry_config_default(&t->cfg);
    if (t->cfg.candidate_window_ms == 0u) {
        t->cfg.candidate_window_ms = PERSONAL_TELEMETRY_DEFAULT_WINDOW_MS;
    }
    t->state = TELEMETRY_IDLE;
}

int personal_telemetry_begin(personal_telemetry_t *t,
                             uint32_t capture_session_id,
                             uint8_t collect_consent,
                             uint32_t now_ms)
{
    if (!t || capture_session_id == 0u || !canonical_bool(collect_consent)) {
        if (t) {
            t->metrics.malformed++;
            abstain(t, TELEMETRY_REJECTED, TELEMETRY_REASON_BAD_FORMAT);
        }
        return PERSONAL_TELEMETRY_E_ARG;
    }
    if (collect_consent != 1u) {
        t->metrics.abstained_no_consent++;
        abstain(t, TELEMETRY_REJECTED, TELEMETRY_REASON_NO_CONSENT);
        return PERSONAL_TELEMETRY_E_AUTH;
    }
    if (t->state == TELEMETRY_CAPTURING ||
        t->state == TELEMETRY_CANDIDATE) {
        return PERSONAL_TELEMETRY_E_STATE;
    }
    clear_pending(t);
    t->capture_session_id = capture_session_id;
    t->candidate_expires_ms = now_ms + t->cfg.candidate_window_ms;
    t->pending_reason = TELEMETRY_REASON_NONE;
    t->state = TELEMETRY_CAPTURING;
    t->metrics.sessions++;
    return PERSONAL_TELEMETRY_OK;
}

int personal_telemetry_observe(personal_telemetry_t *t,
                               const personal_telemetry_sample_t *s)
{
    if (!t || !s) {
        if (t) t->metrics.malformed++;
        return PERSONAL_TELEMETRY_E_ARG;
    }
    t->metrics.observations++;
    if (t->state != TELEMETRY_CAPTURING) return PERSONAL_TELEMETRY_E_STATE;
    if (s->capture_session_id == 0u ||
        s->capture_session_id != t->capture_session_id ||
        s->capture_authorized != 1u) {
        t->metrics.abstained_no_session++;
        abstain(t, TELEMETRY_REJECTED, TELEMETRY_REASON_NO_SESSION);
        return PERSONAL_TELEMETRY_E_AUTH;
    }
    if (!canonical_bool(s->collect_consent) ||
        !canonical_bool(s->muted) || !valid_enums(s)) {
        t->metrics.malformed++;
        abstain(t, TELEMETRY_REJECTED, TELEMETRY_REASON_BAD_FORMAT);
        return PERSONAL_TELEMETRY_E_FORMAT;
    }
    if (s->collect_consent != 1u) {
        t->metrics.abstained_no_consent++;
        abstain(t, TELEMETRY_REJECTED, TELEMETRY_REASON_NO_CONSENT);
        return PERSONAL_TELEMETRY_E_AUTH;
    }
    if (s->muted != 0u) {
        t->metrics.abstained_muted++;
        abstain(t, TELEMETRY_MUTED, TELEMETRY_REASON_MUTED);
        return PERSONAL_TELEMETRY_NO_CANDIDATE;
    }
    if (s->quality != TELEMETRY_QUALITY_USABLE) {
        t->metrics.abstained_low_quality++;
        abstain(t, TELEMETRY_REJECTED, TELEMETRY_REASON_LOW_QUALITY);
        return PERSONAL_TELEMETRY_E_QUALITY;
    }
    if (!source_allowed(s->kind, s->source) || !window_valid(s) ||
        !value_in_range(s)) {
        t->metrics.out_of_range++;
        abstain(t, TELEMETRY_REJECTED, TELEMETRY_REASON_OUT_OF_RANGE);
        return PERSONAL_TELEMETRY_E_FORMAT;
    }

    memset(&t->pending, 0, sizeof(t->pending));
    t->pending.persist_authorized = 0u;
    t->pending.kind = s->kind;
    t->pending.source = s->source;
    t->pending.quality = s->quality;
    t->pending.value = s->value;
    t->pending.unit_scale = s->unit_scale;
    t->pending.window_start_ms = s->window_start_ms;
    t->pending.window_end_ms = s->window_end_ms;
    t->pending_reason = TELEMETRY_REASON_NONE;
    t->state = TELEMETRY_CANDIDATE;
    t->metrics.candidates++;
    return PERSONAL_TELEMETRY_OK;
}

int personal_telemetry_confirm(personal_telemetry_t *t,
                               uint8_t accepted,
                               uint32_t now_ms,
                               personal_telemetry_record_t *out)
{
    if (out) memset(out, 0, sizeof(*out));
    if (!t || !out || !canonical_bool(accepted)) {
        if (t) t->metrics.malformed++;
        return PERSONAL_TELEMETRY_E_ARG;
    }
    if (t->state != TELEMETRY_CANDIDATE) return PERSONAL_TELEMETRY_E_STATE;
    if (expired(now_ms, t->candidate_expires_ms)) {
        t->metrics.expired++;
        abstain(t, TELEMETRY_EXPIRED, TELEMETRY_REASON_EXPIRED);
        return PERSONAL_TELEMETRY_E_EXPIRED;
    }
    if (accepted == 0u) {
        t->metrics.rejected++;
        abstain(t, TELEMETRY_REJECTED, TELEMETRY_REASON_USER_REJECTED);
        return PERSONAL_TELEMETRY_NO_CANDIDATE;
    }
    t->pending.persist_authorized = 1u;
    *out = t->pending;
    clear_pending(t);
    t->pending_reason = TELEMETRY_REASON_NONE;
    t->state = TELEMETRY_CONFIRMED;
    t->metrics.confirmed++;
    return PERSONAL_TELEMETRY_OK;
}

int personal_telemetry_tick(personal_telemetry_t *t, uint32_t now_ms)
{
    if (!t) return PERSONAL_TELEMETRY_E_ARG;
    if (t->state != TELEMETRY_CANDIDATE) return PERSONAL_TELEMETRY_E_STATE;
    if (!expired(now_ms, t->candidate_expires_ms)) return PERSONAL_TELEMETRY_OK;
    t->metrics.expired++;
    abstain(t, TELEMETRY_EXPIRED, TELEMETRY_REASON_EXPIRED);
    return PERSONAL_TELEMETRY_E_EXPIRED;
}

void personal_telemetry_mute(personal_telemetry_t *t)
{
    if (!t) return;
    t->metrics.abstained_muted++;
    abstain(t, TELEMETRY_MUTED, TELEMETRY_REASON_MUTED);
}

void personal_telemetry_forget(personal_telemetry_t *t)
{
    if (!t) return;
    clear_pending(t);
    t->pending_reason = TELEMETRY_REASON_NONE;
    t->state = TELEMETRY_IDLE;
}

const personal_telemetry_metrics_t *personal_telemetry_metrics(
    const personal_telemetry_t *t)
{
    return t ? &t->metrics : NULL;
}

personal_telemetry_state_t personal_telemetry_state(
    const personal_telemetry_t *t)
{
    return t ? t->state : TELEMETRY_REJECTED;
}

uint32_t personal_telemetry_reason(const personal_telemetry_t *t)
{
    return t ? t->pending_reason : TELEMETRY_REASON_BAD_FORMAT;
}
