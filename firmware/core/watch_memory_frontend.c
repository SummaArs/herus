#include "watch_memory_frontend.h"
#include <string.h>

static void clear_pending(watch_memory_frontend_t *f)
{
    if (!f) return;
    memset(&f->pending, 0, sizeof(f->pending));
    f->capture_session_id = 0u;
    f->candidate_expires_ms = 0u;
}

static void abstain(watch_memory_frontend_t *f, watch_memory_state_t state,
                    uint32_t reason)
{
    if (!f) return;
    clear_pending(f);
    f->pending_reason = reason;
    f->state = state;
}

static int boolean_is_canonical(uint8_t value)
{
    return value == 0u || value == 1u;
}

static int enum_values_valid(const watch_memory_observation_t *o)
{
    return o->kind > MEMORY_KIND_NONE && o->kind < MEMORY_KIND_COUNT &&
           o->scope > MEMORY_SCOPE_NONE && o->scope < MEMORY_SCOPE_COUNT &&
           o->sensitivity > MEMORY_SENSITIVITY_NONE &&
           o->sensitivity < MEMORY_SENSITIVITY_COUNT;
}

static int expired(uint32_t now_ms, uint32_t deadline_ms)
{
    return (int32_t)(now_ms - deadline_ms) >= 0;
}

void watch_memory_config_default(watch_memory_config_t *out)
{
    if (!out) return;
    out->candidate_window_ms = WATCH_MEMORY_DEFAULT_CANDIDATE_MS;
}

void watch_memory_frontend_init(watch_memory_frontend_t *f,
                                const watch_memory_config_t *cfg)
{
    if (!f) return;
    memset(f, 0, sizeof(*f));
    if (cfg) f->cfg = *cfg;
    else watch_memory_config_default(&f->cfg);
    if (f->cfg.candidate_window_ms == 0u) {
        f->cfg.candidate_window_ms = WATCH_MEMORY_DEFAULT_CANDIDATE_MS;
    }
    f->state = WATCH_MEMORY_IDLE;
}

int watch_memory_begin(watch_memory_frontend_t *f, uint32_t capture_session_id,
                      uint32_t now_ms)
{
    if (!f || capture_session_id == 0u) {
        if (f) {
            f->metrics.abstained_no_session++;
            abstain(f, WATCH_MEMORY_ABSTAIN, WATCH_MEMORY_REASON_NO_SESSION);
        }
        return WATCH_MEMORY_E_AUTH;
    }
    if (f->state == WATCH_MEMORY_LISTENING ||
        f->state == WATCH_MEMORY_CANDIDATE) {
        return WATCH_MEMORY_E_STATE;
    }

    clear_pending(f);
    f->capture_session_id = capture_session_id;
    f->candidate_expires_ms = now_ms + f->cfg.candidate_window_ms;
    f->pending_reason = WATCH_MEMORY_REASON_NONE;
    f->state = WATCH_MEMORY_LISTENING;
    f->metrics.sessions++;
    return WATCH_MEMORY_OK;
}

int watch_memory_observe(watch_memory_frontend_t *f,
                        const watch_memory_observation_t *o)
{
    if (!f || !o) {
        if (f) f->metrics.malformed++;
        return WATCH_MEMORY_E_ARG;
    }
    f->metrics.observations++;
    if (f->state != WATCH_MEMORY_LISTENING) return WATCH_MEMORY_E_STATE;
    if (o->capture_session_id == 0u ||
        o->capture_session_id != f->capture_session_id ||
        o->capture_authorized != 1u) {
        f->metrics.abstained_no_session++;
        abstain(f, WATCH_MEMORY_ABSTAIN, WATCH_MEMORY_REASON_NO_SESSION);
        return WATCH_MEMORY_E_AUTH;
    }
    if (!boolean_is_canonical(o->wake_detected) ||
        !boolean_is_canonical(o->muted) || !enum_values_valid(o) ||
        o->speaker_score_pct > 100u || o->speaker_runner_up_pct > 100u ||
        o->relevance_score_pct > 100u || o->novelty_pct > 100u ||
        o->future_value_pct > 100u || o->consequence_pct > 100u) {
        f->metrics.malformed++;
        abstain(f, WATCH_MEMORY_ABSTAIN, WATCH_MEMORY_REASON_BAD_FORMAT);
        return WATCH_MEMORY_E_FORMAT;
    }
    if (o->muted != 0u) {
        f->metrics.abstained_muted++;
        abstain(f, WATCH_MEMORY_MUTED, WATCH_MEMORY_REASON_MUTED);
        return WATCH_MEMORY_NO_CANDIDATE;
    }
    if (o->wake_detected == 0u) {
        f->metrics.abstained_no_wake++;
        abstain(f, WATCH_MEMORY_ABSTAIN, WATCH_MEMORY_REASON_NO_WAKE);
        return WATCH_MEMORY_NO_CANDIDATE;
    }
    if (o->speaker_score_pct < WATCH_MEMORY_SPEAKER_MIN_PCT) {
        f->metrics.abstained_low_speaker++;
        abstain(f, WATCH_MEMORY_ABSTAIN, WATCH_MEMORY_REASON_LOW_SPEAKER);
        return WATCH_MEMORY_NO_CANDIDATE;
    }
    if (o->speaker_score_pct < o->speaker_runner_up_pct ||
        (uint8_t)(o->speaker_score_pct - o->speaker_runner_up_pct) <
        WATCH_MEMORY_SPEAKER_MARGIN_MIN_PCT) {
        f->metrics.abstained_ambiguous_speaker++;
        abstain(f, WATCH_MEMORY_ABSTAIN,
                WATCH_MEMORY_REASON_AMBIGUOUS_SPEAKER);
        return WATCH_MEMORY_NO_CANDIDATE;
    }
    if (o->relevance_score_pct < WATCH_MEMORY_RELEVANCE_MIN_PCT) {
        f->metrics.abstained_low_relevance++;
        abstain(f, WATCH_MEMORY_ABSTAIN, WATCH_MEMORY_REASON_LOW_RELEVANCE);
        return WATCH_MEMORY_NO_CANDIDATE;
    }

    memset(&f->pending, 0, sizeof(f->pending));
    f->pending.session_authorized = 1u;
    f->pending.explicit_remember = 0u;
    f->pending.kind = o->kind;
    f->pending.scope = o->scope;
    f->pending.sensitivity = o->sensitivity;
    f->pending.confidence_pct = o->speaker_score_pct;
    f->pending.novelty_pct = o->novelty_pct;
    f->pending.future_value_pct = o->future_value_pct;
    f->pending.consequence_pct = o->consequence_pct;
    f->pending_reason = WATCH_MEMORY_REASON_NONE;
    f->state = WATCH_MEMORY_CANDIDATE;
    f->metrics.candidates++;
    return WATCH_MEMORY_OK;
}

int watch_memory_confirm(watch_memory_frontend_t *f, uint8_t accepted,
                         uint32_t now_ms, memory_signal_t *out)
{
    if (out) memset(out, 0, sizeof(*out));
    if (!f || !out || !boolean_is_canonical(accepted)) {
        if (f) f->metrics.malformed++;
        return WATCH_MEMORY_E_ARG;
    }
    if (f->state != WATCH_MEMORY_CANDIDATE) return WATCH_MEMORY_E_STATE;
    if (expired(now_ms, f->candidate_expires_ms)) {
        f->metrics.expired++;
        abstain(f, WATCH_MEMORY_EXPIRED, WATCH_MEMORY_REASON_EXPIRED);
        return WATCH_MEMORY_E_EXPIRED;
    }
    if (accepted == 0u) {
        f->metrics.rejected++;
        abstain(f, WATCH_MEMORY_REJECTED, WATCH_MEMORY_REASON_USER_REJECTED);
        return WATCH_MEMORY_NO_CANDIDATE;
    }
    f->pending.explicit_remember = 1u;
    *out = f->pending;
    clear_pending(f);
    f->pending_reason = WATCH_MEMORY_REASON_NONE;
    f->state = WATCH_MEMORY_CONFIRMED;
    f->metrics.confirmed++;
    return WATCH_MEMORY_OK;
}

int watch_memory_tick(watch_memory_frontend_t *f, uint32_t now_ms)
{
    if (!f) return WATCH_MEMORY_E_ARG;
    if (f->state != WATCH_MEMORY_CANDIDATE) return WATCH_MEMORY_E_STATE;
    if (!expired(now_ms, f->candidate_expires_ms)) return WATCH_MEMORY_OK;
    f->metrics.expired++;
    abstain(f, WATCH_MEMORY_EXPIRED, WATCH_MEMORY_REASON_EXPIRED);
    return WATCH_MEMORY_E_EXPIRED;
}

void watch_memory_mute(watch_memory_frontend_t *f)
{
    if (!f) return;
    f->metrics.abstained_muted++;
    abstain(f, WATCH_MEMORY_MUTED, WATCH_MEMORY_REASON_MUTED);
}

void watch_memory_forget(watch_memory_frontend_t *f)
{
    if (!f) return;
    clear_pending(f);
    f->pending_reason = WATCH_MEMORY_REASON_NONE;
    f->state = WATCH_MEMORY_IDLE;
}

const watch_memory_metrics_t *watch_memory_metrics(const watch_memory_frontend_t *f)
{
    return f ? &f->metrics : NULL;
}

watch_memory_state_t watch_memory_state(const watch_memory_frontend_t *f)
{
    return f ? f->state : WATCH_MEMORY_REJECTED;
}

uint32_t watch_memory_reason(const watch_memory_frontend_t *f)
{
    return f ? f->pending_reason : WATCH_MEMORY_REASON_BAD_FORMAT;
}
