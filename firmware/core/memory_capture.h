/* memory_capture.h — explicit, bounded and transient memory-capture session.
 *
 * This module does not read a microphone, run VAD/ASR, store audio, retain text,
 * call a model, write memory or transmit. It governs the only period during which
 * a target audio adapter may prepare one transient extraction input for the future
 * memory pipeline. A physical gesture starts it; timeout, cancellation, failure or
 * delivery closes it and requests source cleanup.
 *
 * The byte buffer passed to memory_capture_deliver() is caller-owned only for the
 * synchronous callback. It is scrubbed by this module on every return path. No
 * pointer, byte, transcript, embedding, identity, location or key is retained in
 * memory_capture_t.
 */
#ifndef HERUS_MEMORY_CAPTURE_H
#define HERUS_MEMORY_CAPTURE_H

#include <stddef.h>
#include <stdint.h>

#define MEMORY_CAPTURE_DEFAULT_WINDOW_MS 12000u
#define MEMORY_CAPTURE_MAX_TRANSIENT_BYTES 4096u

typedef enum {
    MEMORY_CAPTURE_IDLE = 0,
    MEMORY_CAPTURE_CAPTURING,
    MEMORY_CAPTURE_DELIVERED,
    MEMORY_CAPTURE_CANCELLED,
    MEMORY_CAPTURE_TIMED_OUT,
    MEMORY_CAPTURE_FAILED
} memory_capture_state_t;

enum {
    MEMORY_CAPTURE_OK         =  0,
    MEMORY_CAPTURE_E_ARG      = -1,
    MEMORY_CAPTURE_E_STATE    = -2,
    MEMORY_CAPTURE_E_PHYSICAL = -3,
    MEMORY_CAPTURE_E_SESSION  = -4,
    MEMORY_CAPTURE_E_SIZE     = -5,
    MEMORY_CAPTURE_E_ADAPTER  = -6
};

/* The target uses these requested effects to gate its AFE/VAD/ASR path. `discard`
 * is asserted on every terminal path, including source error and expiry. Actions
 * contain no audio, transcript or semantic candidate. */
typedef struct {
    uint8_t start_capture;
    uint8_t stop_capture;
    uint8_t memory_indicator;
    uint8_t discard_transient;
} memory_capture_actions_t;

/* The extractor adapter may inspect the writable buffer only synchronously. It
 * must not retain the pointer or copy raw data to persistent storage. The portable
 * session clears the supplied buffer immediately after this call returns. */
typedef struct {
    void *ctx;
    int (*consume_transient)(void *ctx, uint32_t capture_session_id,
                             uint8_t *bytes, size_t len);
} memory_capture_adapter_t;

typedef struct {
    uint32_t window_ms;
} memory_capture_config_t;

/* Numeric-only diagnostics. There is intentionally no payload length, transcript,
 * candidate type, audio statistic, identity, key or location that could become a
 * product memory or behavioural log. */
typedef struct {
    uint32_t sessions_started;
    uint32_t deliveries;
    uint32_t cancelled;
    uint32_t timed_out;
    uint32_t adapter_failed;
    uint32_t rejected_start;
    uint32_t rejected_delivery;
    uint32_t scrubbed_buffers;
    uint32_t last_latency_ms;
} memory_capture_metrics_t;

typedef struct {
    memory_capture_config_t  cfg;
    memory_capture_adapter_t adapter;
    memory_capture_state_t   state;
    uint32_t                 gesture_id;       /* currently held physical gesture */
    uint32_t                 capture_session_id; /* active only; zero when terminal */
    uint32_t                 next_capture_session_id; /* monotonically advances per begin */
    uint32_t                 started_ms;
    memory_capture_actions_t actions;
    memory_capture_metrics_t metrics;
} memory_capture_t;

void memory_capture_config_default(memory_capture_config_t *out);
void memory_capture_init(memory_capture_t *m, const memory_capture_config_t *cfg,
                         const memory_capture_adapter_t *adapter);

/* Begin a memory window. `physical_gesture_id` must be nonzero and must come from
 * a currently held target gesture; zero and re-entry fail closed. It is never
 * forwarded to the adapter. */
int memory_capture_begin(memory_capture_t *m, uint32_t physical_gesture_id,
                         uint32_t now_ms);

/* Provide exactly one bounded, caller-owned transient buffer for the active capture
 * session. The adapter sees it only synchronously; the bytes are zeroized whether
 * the delivery succeeds, fails, is stale or is rejected. A successful delivery
 * terminally closes the session but does not create a candidate or persistent item. */
int memory_capture_deliver(memory_capture_t *m, uint32_t capture_session_id,
                           uint8_t *bytes, size_t len, uint32_t now_ms);

/* A physical cancel ends the capture and asks the source adapter to discard any
 * in-flight samples. It cannot preserve an unfinished utterance. */
int memory_capture_cancel(memory_capture_t *m, uint32_t physical_gesture_id,
                          uint32_t now_ms);

/* Enforce the configured window using caller-supplied monotonic milliseconds. */
int memory_capture_tick(memory_capture_t *m, uint32_t now_ms);

/* Tell the session that the local capture source has disappeared or failed. */
void memory_capture_source_failed(memory_capture_t *m, uint32_t now_ms);

uint32_t memory_capture_session_id(const memory_capture_t *m);
const memory_capture_actions_t *memory_capture_actions(const memory_capture_t *m);
const memory_capture_metrics_t *memory_capture_metrics(const memory_capture_t *m);

#endif /* HERUS_MEMORY_CAPTURE_H */
