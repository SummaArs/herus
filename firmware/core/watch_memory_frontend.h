/*
 * HERUS Watch memory front-end.
 *
 * This is a typed, transient gate between a future local VAD/wake-word/speaker
 * adapter and the existing memory policy. It stores no audio, text, transcript,
 * embedding, identity, location, key or model prompt. A qualified observation is
 * only a candidate. The only path that emits a memory_signal_t requires an explicit
 * physical confirmation supplied by the caller.
 */
#ifndef HERUS_WATCH_MEMORY_FRONTEND_H
#define HERUS_WATCH_MEMORY_FRONTEND_H

#include <stdint.h>
#include "memory_policy.h"

#define WATCH_MEMORY_SPEAKER_MIN_PCT       80u
#define WATCH_MEMORY_SPEAKER_MARGIN_MIN_PCT 15u
#define WATCH_MEMORY_RELEVANCE_MIN_PCT     60u
#define WATCH_MEMORY_DEFAULT_CANDIDATE_MS  8000u

typedef enum {
    WATCH_MEMORY_IDLE = 0,
    WATCH_MEMORY_LISTENING,
    WATCH_MEMORY_CANDIDATE,
    WATCH_MEMORY_ABSTAIN,
    WATCH_MEMORY_CONFIRMED,
    WATCH_MEMORY_REJECTED,
    WATCH_MEMORY_EXPIRED,
    WATCH_MEMORY_MUTED
} watch_memory_state_t;

typedef enum {
    WATCH_MEMORY_REASON_NONE = 0u,
    WATCH_MEMORY_REASON_NO_SESSION = 1u << 0,
    WATCH_MEMORY_REASON_MUTED = 1u << 1,
    WATCH_MEMORY_REASON_NO_WAKE = 1u << 2,
    WATCH_MEMORY_REASON_LOW_SPEAKER = 1u << 3,
    WATCH_MEMORY_REASON_AMBIGUOUS_SPEAKER = 1u << 4,
    WATCH_MEMORY_REASON_LOW_RELEVANCE = 1u << 5,
    WATCH_MEMORY_REASON_EXPIRED = 1u << 6,
    WATCH_MEMORY_REASON_USER_REJECTED = 1u << 7,
    WATCH_MEMORY_REASON_BAD_FORMAT = 1u << 8
} watch_memory_reason_t;

typedef struct {
    uint32_t capture_session_id;
    uint8_t  capture_authorized;
    uint8_t  wake_detected;
    uint8_t  muted;
    uint8_t  speaker_score_pct;
    uint8_t  speaker_runner_up_pct;
    uint8_t  relevance_score_pct;
    uint8_t  novelty_pct;
    uint8_t  future_value_pct;
    uint8_t  consequence_pct;
    memory_kind_t        kind;
    memory_scope_t       scope;
    memory_sensitivity_t sensitivity;
    uint32_t now_ms;
} watch_memory_observation_t;

typedef struct {
    uint32_t sessions;
    uint32_t observations;
    uint32_t candidates;
    uint32_t confirmed;
    uint32_t rejected;
    uint32_t expired;
    uint32_t abstained_no_session;
    uint32_t abstained_muted;
    uint32_t abstained_no_wake;
    uint32_t abstained_low_speaker;
    uint32_t abstained_ambiguous_speaker;
    uint32_t abstained_low_relevance;
    uint32_t malformed;
} watch_memory_metrics_t;

typedef struct {
    uint32_t candidate_window_ms;
} watch_memory_config_t;

typedef struct {
    watch_memory_config_t cfg;
    watch_memory_state_t state;
    uint32_t capture_session_id;
    uint32_t candidate_expires_ms;
    memory_signal_t pending;
    uint32_t pending_reason;
    watch_memory_metrics_t metrics;
} watch_memory_frontend_t;

enum {
    WATCH_MEMORY_OK           =  0,
    WATCH_MEMORY_NO_CANDIDATE =  1,
    WATCH_MEMORY_E_ARG        = -1,
    WATCH_MEMORY_E_STATE      = -2,
    WATCH_MEMORY_E_AUTH       = -3,
    WATCH_MEMORY_E_FORMAT     = -4,
    WATCH_MEMORY_E_EXPIRED    = -5
};

void watch_memory_config_default(watch_memory_config_t *out);
void watch_memory_frontend_init(watch_memory_frontend_t *f,
                                const watch_memory_config_t *cfg);

/* Bind one transient observation window to an already-authorised capture session. */
int watch_memory_begin(watch_memory_frontend_t *f, uint32_t capture_session_id,
                      uint32_t now_ms);

/* Inspect typed local-adapter output. This creates only a transient candidate. */
int watch_memory_observe(watch_memory_frontend_t *f,
                        const watch_memory_observation_t *observation);

/* Explicit physical confirmation is the sole path that emits a memory signal. */
int watch_memory_confirm(watch_memory_frontend_t *f, uint8_t accepted,
                         uint32_t now_ms, memory_signal_t *out);

/* Expire a candidate using a caller-owned monotonic clock. */
int watch_memory_tick(watch_memory_frontend_t *f, uint32_t now_ms);

/* Mute/forget clears pending typed state and blocks observation until a new begin. */
void watch_memory_mute(watch_memory_frontend_t *f);
void watch_memory_forget(watch_memory_frontend_t *f);

const watch_memory_metrics_t *watch_memory_metrics(const watch_memory_frontend_t *f);
watch_memory_state_t watch_memory_state(const watch_memory_frontend_t *f);
uint32_t watch_memory_reason(const watch_memory_frontend_t *f);

#endif /* HERUS_WATCH_MEMORY_FRONTEND_H */
