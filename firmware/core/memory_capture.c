/* memory_capture.c — explicit, one-shot memory-capture session. */
#include "memory_capture.h"
#include <string.h>

static void scrub(uint8_t *bytes, size_t len)
{
    volatile uint8_t *p = bytes;
    while (len-- > 0u) *p++ = 0u;
}

static int config_valid(const memory_capture_config_t *cfg)
{
    return cfg && cfg->window_ms > 0u;
}

static void clear_actions(memory_capture_t *m)
{
    memset(&m->actions, 0, sizeof(m->actions));
}

static void terminal(memory_capture_t *m, memory_capture_state_t state,
                     uint32_t now_ms)
{
    clear_actions(m);
    m->actions.stop_capture = 1u;
    m->actions.discard_transient = 1u;
    m->gesture_id = 0u;
    m->capture_session_id = 0u;
    if (now_ms >= m->started_ms) m->metrics.last_latency_ms = now_ms - m->started_ms;
    m->state = state;
}

void memory_capture_config_default(memory_capture_config_t *out)
{
    if (!out) return;
    out->window_ms = MEMORY_CAPTURE_DEFAULT_WINDOW_MS;
}

void memory_capture_init(memory_capture_t *m, const memory_capture_config_t *cfg,
                         const memory_capture_adapter_t *adapter)
{
    memory_capture_config_t defaults;
    if (!m) return;
    memory_capture_config_default(&defaults);
    memset(m, 0, sizeof(*m));
    m->cfg = config_valid(cfg) ? *cfg : defaults;
    if (adapter) m->adapter = *adapter;
    m->state = MEMORY_CAPTURE_IDLE;
}

int memory_capture_begin(memory_capture_t *m, uint32_t physical_gesture_id,
                         uint32_t now_ms)
{
    if (!m) return MEMORY_CAPTURE_E_ARG;
    if (physical_gesture_id == 0u) {
        m->metrics.rejected_start++;
        return MEMORY_CAPTURE_E_PHYSICAL;
    }
    if (m->state == MEMORY_CAPTURE_CAPTURING) {
        m->metrics.rejected_start++;
        return MEMORY_CAPTURE_E_STATE;
    }

    clear_actions(m);
    m->gesture_id = physical_gesture_id;
    /* A gesture identifier remains target-local. The active id is erased at every
     * terminal state, while this separate generation counter prevents reuse by a
     * late buffer from a prior capture. */
    m->next_capture_session_id++;
    if (m->next_capture_session_id == 0u) m->next_capture_session_id++;
    m->capture_session_id = m->next_capture_session_id;
    m->started_ms = now_ms;
    m->state = MEMORY_CAPTURE_CAPTURING;
    m->actions.start_capture = 1u;
    m->actions.memory_indicator = 1u;
    m->metrics.sessions_started++;
    return MEMORY_CAPTURE_OK;
}

int memory_capture_deliver(memory_capture_t *m, uint32_t capture_session_id,
                           uint8_t *bytes, size_t len, uint32_t now_ms)
{
    int adapter_result;

    if (!m || (!bytes && len != 0u)) return MEMORY_CAPTURE_E_ARG;
    if (len > MEMORY_CAPTURE_MAX_TRANSIENT_BYTES) {
        if (bytes) {
            scrub(bytes, len);
            if (m) m->metrics.scrubbed_buffers++;
        }
        if (m) {
            m->metrics.rejected_delivery++;
            if (m->state == MEMORY_CAPTURE_CAPTURING) {
                m->metrics.adapter_failed++;
                terminal(m, MEMORY_CAPTURE_FAILED, now_ms);
            }
        }
        return MEMORY_CAPTURE_E_SIZE;
    }
    if (m->state != MEMORY_CAPTURE_CAPTURING) {
        if (bytes) {
            scrub(bytes, len);
            m->metrics.scrubbed_buffers++;
        }
        m->metrics.rejected_delivery++;
        return MEMORY_CAPTURE_E_STATE;
    }
    if (capture_session_id == 0u || capture_session_id != m->capture_session_id) {
        if (bytes) {
            scrub(bytes, len);
            m->metrics.scrubbed_buffers++;
        }
        m->metrics.rejected_delivery++;
        return MEMORY_CAPTURE_E_SESSION;
    }
    if (now_ms - m->started_ms >= m->cfg.window_ms) {
        if (bytes) {
            scrub(bytes, len);
            m->metrics.scrubbed_buffers++;
        }
        m->metrics.timed_out++;
        terminal(m, MEMORY_CAPTURE_TIMED_OUT, now_ms);
        return MEMORY_CAPTURE_E_STATE;
    }
    if (!m->adapter.consume_transient) {
        if (bytes) {
            scrub(bytes, len);
            m->metrics.scrubbed_buffers++;
        }
        m->metrics.adapter_failed++;
        terminal(m, MEMORY_CAPTURE_FAILED, now_ms);
        return MEMORY_CAPTURE_E_ADAPTER;
    }

    adapter_result = m->adapter.consume_transient(m->adapter.ctx, capture_session_id,
                                                  bytes, len);
    if (bytes) {
        scrub(bytes, len);
        m->metrics.scrubbed_buffers++;
    }
    if (adapter_result != 0) {
        m->metrics.adapter_failed++;
        terminal(m, MEMORY_CAPTURE_FAILED, now_ms);
        return MEMORY_CAPTURE_E_ADAPTER;
    }

    m->metrics.deliveries++;
    terminal(m, MEMORY_CAPTURE_DELIVERED, now_ms);
    return MEMORY_CAPTURE_OK;
}

int memory_capture_cancel(memory_capture_t *m, uint32_t physical_gesture_id,
                          uint32_t now_ms)
{
    if (!m) return MEMORY_CAPTURE_E_ARG;
    if (m->state != MEMORY_CAPTURE_CAPTURING) return MEMORY_CAPTURE_E_STATE;
    if (physical_gesture_id == 0u || physical_gesture_id != m->gesture_id)
        return MEMORY_CAPTURE_E_PHYSICAL;
    m->metrics.cancelled++;
    terminal(m, MEMORY_CAPTURE_CANCELLED, now_ms);
    return MEMORY_CAPTURE_OK;
}

int memory_capture_tick(memory_capture_t *m, uint32_t now_ms)
{
    if (!m) return MEMORY_CAPTURE_E_ARG;
    if (m->state != MEMORY_CAPTURE_CAPTURING) return MEMORY_CAPTURE_OK;
    if (now_ms - m->started_ms < m->cfg.window_ms) return MEMORY_CAPTURE_OK;
    m->metrics.timed_out++;
    terminal(m, MEMORY_CAPTURE_TIMED_OUT, now_ms);
    return MEMORY_CAPTURE_E_STATE;
}

void memory_capture_source_failed(memory_capture_t *m, uint32_t now_ms)
{
    if (!m || m->state != MEMORY_CAPTURE_CAPTURING) return;
    m->metrics.adapter_failed++;
    terminal(m, MEMORY_CAPTURE_FAILED, now_ms);
}

uint32_t memory_capture_session_id(const memory_capture_t *m)
{
    return (m && m->state == MEMORY_CAPTURE_CAPTURING) ? m->capture_session_id : 0u;
}

const memory_capture_actions_t *memory_capture_actions(const memory_capture_t *m)
{
    return m ? &m->actions : 0;
}

const memory_capture_metrics_t *memory_capture_metrics(const memory_capture_t *m)
{
    return m ? &m->metrics : 0;
}
