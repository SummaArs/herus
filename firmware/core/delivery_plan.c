#include "delivery_plan.h"

static void clear_plan(delivery_plan_t *plan)
{
    if (!plan) return;
    plan->app_seq = 0u;
    plan->max_attempts = 0u;
    plan->attempts = 0u;
    plan->deadline_ms = 0u;
    plan->next_due_ms = 0u;
    plan->state = DELIVERY_PLAN_IDLE;
}

static void note_invalid(delivery_plan_metrics_t *metrics)
{
    if (metrics) metrics->invalid++;
}

static int valid_begin(uint16_t app_seq, uint8_t max_attempts,
                      uint64_t now_ms, uint64_t deadline_ms,
                      uint8_t authorized_handoff)
{
    return app_seq != 0u &&
           max_attempts >= 1u && max_attempts <= DELIVERY_PLAN_MAX_ATTEMPTS &&
           deadline_ms > now_ms &&
           authorized_handoff == 1u;
}

void delivery_plan_init(delivery_plan_t *plan, delivery_plan_metrics_t *metrics)
{
    clear_plan(plan);
    if (metrics) {
        metrics->starts = 0u;
        metrics->sends = 0u;
        metrics->retries = 0u;
        metrics->delivered = 0u;
        metrics->expired = 0u;
        metrics->cancelled = 0u;
        metrics->failed = 0u;
        metrics->invalid = 0u;
    }
}

int delivery_plan_begin(delivery_plan_t *plan, delivery_plan_metrics_t *metrics,
                       uint16_t app_seq, uint8_t max_attempts,
                       uint64_t now_ms, uint64_t deadline_ms,
                       uint8_t authorized_handoff)
{
    if (!plan) {
        note_invalid(metrics);
        return DELIVERY_PLAN_E_ARG;
    }
    if (plan->state == DELIVERY_PLAN_WAIT_ACK) {
        return DELIVERY_PLAN_E_STATE;
    }
    if (!valid_begin(app_seq, max_attempts, now_ms, deadline_ms,
                     authorized_handoff)) {
        clear_plan(plan);
        note_invalid(metrics);
        return authorized_handoff == 1u ? DELIVERY_PLAN_E_ARG
                                        : DELIVERY_PLAN_E_AUTHORITY;
    }

    plan->app_seq = app_seq;
    plan->max_attempts = max_attempts;
    plan->attempts = 0u;
    plan->deadline_ms = deadline_ms;
    plan->next_due_ms = now_ms;
    plan->state = DELIVERY_PLAN_WAIT_ACK;
    if (metrics) metrics->starts++;
    return DELIVERY_PLAN_OK;
}

int delivery_plan_poll(delivery_plan_t *plan, delivery_plan_metrics_t *metrics,
                       uint64_t now_ms, delivery_event_t *event)
{
    if (event) *event = DELIVERY_EVENT_NONE;
    if (!plan || !event) {
        note_invalid(metrics);
        return DELIVERY_PLAN_E_ARG;
    }
    if (plan->state != DELIVERY_PLAN_WAIT_ACK) return DELIVERY_PLAN_E_STATE;
    if (now_ms >= plan->deadline_ms) {
        plan->state = DELIVERY_PLAN_EXPIRED;
        if (metrics) metrics->expired++;
        *event = DELIVERY_EVENT_EXPIRED;
        return DELIVERY_PLAN_OK;
    }
    if (now_ms < plan->next_due_ms) return DELIVERY_PLAN_OK;

    if (plan->attempts >= plan->max_attempts) {
        plan->state = DELIVERY_PLAN_FAILED;
        if (metrics) metrics->failed++;
        *event = DELIVERY_EVENT_FAILED;
        return DELIVERY_PLAN_OK;
    }

    plan->attempts++;
    plan->next_due_ms = now_ms + DELIVERY_PLAN_RETRY_INTERVAL_MS;
    if (plan->attempts == 1u) {
        if (metrics) metrics->sends++;
        *event = DELIVERY_EVENT_SEND;
    } else {
        if (metrics) metrics->retries++;
        *event = DELIVERY_EVENT_RETRY;
    }
    return DELIVERY_PLAN_OK;
}

int delivery_plan_ack(delivery_plan_t *plan, delivery_plan_metrics_t *metrics,
                      uint16_t app_seq, uint64_t now_ms,
                      delivery_event_t *event)
{
    if (event) *event = DELIVERY_EVENT_NONE;
    if (!plan || !event || app_seq == 0u) {
        note_invalid(metrics);
        return DELIVERY_PLAN_E_ARG;
    }
    if (plan->state != DELIVERY_PLAN_WAIT_ACK) return DELIVERY_PLAN_E_STATE;
    if (app_seq != plan->app_seq) return DELIVERY_PLAN_E_STATE;
    if (now_ms >= plan->deadline_ms) {
        plan->state = DELIVERY_PLAN_EXPIRED;
        if (metrics) metrics->expired++;
        *event = DELIVERY_EVENT_EXPIRED;
        return DELIVERY_PLAN_E_EXPIRED;
    }
    if (plan->attempts == 0u) return DELIVERY_PLAN_E_STATE;

    plan->state = DELIVERY_PLAN_DELIVERED;
    if (metrics) metrics->delivered++;
    *event = DELIVERY_EVENT_DELIVERED;
    return DELIVERY_PLAN_OK;
}

int delivery_plan_cancel(delivery_plan_t *plan, delivery_plan_metrics_t *metrics,
                         delivery_event_t *event)
{
    if (event) *event = DELIVERY_EVENT_NONE;
    if (!plan || !event) {
        note_invalid(metrics);
        return DELIVERY_PLAN_E_ARG;
    }
    if (plan->state != DELIVERY_PLAN_WAIT_ACK) return DELIVERY_PLAN_E_STATE;
    plan->state = DELIVERY_PLAN_CANCELLED;
    plan->next_due_ms = 0u;
    if (metrics) metrics->cancelled++;
    *event = DELIVERY_EVENT_CANCELLED;
    return DELIVERY_PLAN_OK;
}
