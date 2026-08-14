/* memory_physical_session.c — portable purpose-bound physical-session gate. */
#include "memory_physical_session.h"
#include <limits.h>
#include <string.h>

static int purpose_valid(memory_physical_purpose_t purpose)
{
    return purpose >= MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT &&
           purpose <= MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY;
}

static int config_valid(const memory_physical_session_config_t *cfg)
{
    return cfg && cfg->window_ms != 0u &&
           cfg->window_ms <= MEMORY_PHYSICAL_SESSION_MAX_WINDOW_MS &&
           cfg->max_query_uses != 0u &&
           cfg->max_query_uses <= MEMORY_PHYSICAL_SESSION_MAX_QUERY_USES;
}

static void scrub_active(memory_physical_session_t *gate,
                         memory_physical_session_state_t next_state)
{
    gate->active_session_id = 0u;
    gate->active_event_nonce = 0u;
    gate->active_purpose = MEMORY_PHYSICAL_PURPOSE_NONE;
    gate->started_at_ms = 0u;
    gate->expires_at_ms = 0u;
    gate->uses_remaining = 0u;
    gate->state = next_state;
}

void memory_physical_session_config_default(memory_physical_session_config_t *cfg)
{
    if (!cfg) return;
    cfg->window_ms = MEMORY_PHYSICAL_SESSION_DEFAULT_WINDOW_MS;
    cfg->max_query_uses = MEMORY_PHYSICAL_SESSION_DEFAULT_QUERY_USES;
}

int memory_physical_session_init(memory_physical_session_t *gate,
                                 const memory_physical_session_config_t *cfg)
{
    if (!gate || !cfg) return MEMORY_PHYSICAL_SESSION_E_ARG;
    memset(gate, 0, sizeof(*gate));
    if (!config_valid(cfg)) {
        gate->state = MEMORY_PHYSICAL_SESSION_BLOCKED;
        return MEMORY_PHYSICAL_SESSION_E_CONFIG;
    }
    gate->cfg = *cfg;
    gate->state = MEMORY_PHYSICAL_SESSION_IDLE;
    return MEMORY_PHYSICAL_SESSION_OK;
}

int memory_physical_session_begin(memory_physical_session_t *gate,
                                  memory_physical_purpose_t purpose,
                                  uint32_t physical_session_id,
                                  uint32_t event_nonce,
                                  uint8_t physical_event_confirmed,
                                  uint8_t requested_uses,
                                  uint32_t now_ms)
{
    uint8_t uses;

    if (!gate) return MEMORY_PHYSICAL_SESSION_E_ARG;
    if (gate->state == MEMORY_PHYSICAL_SESSION_BLOCKED) {
        gate->metrics.rejected_state++;
        return MEMORY_PHYSICAL_SESSION_E_STATE;
    }
    if (gate->state == MEMORY_PHYSICAL_SESSION_ACTIVE) {
        gate->metrics.rejected_state++;
        return MEMORY_PHYSICAL_SESSION_E_STATE;
    }
    if (!purpose_valid(purpose)) {
        gate->metrics.rejected_purpose++;
        return MEMORY_PHYSICAL_SESSION_E_PURPOSE;
    }
    if (physical_session_id == 0u || event_nonce == 0u ||
        physical_event_confirmed != 1u || physical_session_id <= gate->session_floor) {
        gate->metrics.rejected_format++;
        return MEMORY_PHYSICAL_SESSION_E_FORMAT;
    }
    if (now_ms > UINT32_MAX - gate->cfg.window_ms) {
        gate->metrics.rejected_time++;
        return MEMORY_PHYSICAL_SESSION_E_TIME;
    }
    if (purpose == MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY) {
        if (requested_uses == 0u || requested_uses > gate->cfg.max_query_uses) {
            gate->metrics.rejected_assertion++;
            return MEMORY_PHYSICAL_SESSION_E_ASSERTION;
        }
        uses = requested_uses;
    } else {
        if (requested_uses != 1u) {
            gate->metrics.rejected_assertion++;
            return MEMORY_PHYSICAL_SESSION_E_ASSERTION;
        }
        uses = 1u;
    }

    gate->session_floor = physical_session_id;
    gate->active_session_id = physical_session_id;
    gate->active_event_nonce = event_nonce;
    gate->active_purpose = purpose;
    gate->started_at_ms = now_ms;
    gate->expires_at_ms = now_ms + gate->cfg.window_ms;
    gate->uses_remaining = uses;
    gate->state = MEMORY_PHYSICAL_SESSION_ACTIVE;
    gate->metrics.begun++;
    return MEMORY_PHYSICAL_SESSION_OK;
}

int memory_physical_session_validate(memory_physical_session_t *gate,
                                     memory_physical_purpose_t expected_purpose,
                                     uint32_t physical_session_id,
                                     uint32_t now_ms)
{
    if (!gate) return MEMORY_PHYSICAL_SESSION_E_ARG;
    if (!purpose_valid(expected_purpose)) {
        gate->metrics.rejected_purpose++;
        return MEMORY_PHYSICAL_SESSION_E_PURPOSE;
    }
    if (gate->state != MEMORY_PHYSICAL_SESSION_ACTIVE) {
        gate->metrics.rejected_state++;
        return MEMORY_PHYSICAL_SESSION_E_STATE;
    }
    if (now_ms < gate->started_at_ms) {
        gate->metrics.rejected_time++;
        return MEMORY_PHYSICAL_SESSION_E_TIME;
    }
    if (now_ms > gate->expires_at_ms) {
        gate->metrics.expired++;
        scrub_active(gate, MEMORY_PHYSICAL_SESSION_EXPIRED);
        return MEMORY_PHYSICAL_SESSION_E_TIME;
    }
    if (expected_purpose != gate->active_purpose) {
        gate->metrics.rejected_purpose++;
        return MEMORY_PHYSICAL_SESSION_E_PURPOSE;
    }
    if (physical_session_id == 0u || physical_session_id != gate->active_session_id) {
        gate->metrics.rejected_assertion++;
        return MEMORY_PHYSICAL_SESSION_E_ASSERTION;
    }
    if (gate->uses_remaining == 0u) {
        gate->metrics.rejected_state++;
        scrub_active(gate, MEMORY_PHYSICAL_SESSION_BLOCKED);
        return MEMORY_PHYSICAL_SESSION_E_STATE;
    }
    return MEMORY_PHYSICAL_SESSION_OK;
}

int memory_physical_session_consume(memory_physical_session_t *gate,
                                    memory_physical_purpose_t expected_purpose,
                                    uint32_t physical_session_id,
                                    uint32_t now_ms)
{
    int rc = memory_physical_session_validate(gate, expected_purpose,
                                              physical_session_id, now_ms);
    if (rc != MEMORY_PHYSICAL_SESSION_OK) return rc;
    gate->uses_remaining--;
    gate->metrics.consumed++;
    if (gate->uses_remaining == 0u)
        scrub_active(gate, MEMORY_PHYSICAL_SESSION_CONSUMED);
    return MEMORY_PHYSICAL_SESSION_OK;
}

int memory_physical_session_cancel(memory_physical_session_t *gate)
{
    if (!gate) return MEMORY_PHYSICAL_SESSION_E_ARG;
    if (gate->state != MEMORY_PHYSICAL_SESSION_ACTIVE) {
        gate->metrics.rejected_state++;
        return MEMORY_PHYSICAL_SESSION_E_STATE;
    }
    gate->metrics.cancelled++;
    scrub_active(gate, MEMORY_PHYSICAL_SESSION_CANCELLED);
    return MEMORY_PHYSICAL_SESSION_OK;
}

const memory_physical_session_metrics_t *memory_physical_session_metrics(
    const memory_physical_session_t *gate)
{
    return gate ? &gate->metrics : 0;
}
