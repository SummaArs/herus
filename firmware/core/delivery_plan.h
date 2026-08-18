/*
 * HERUS bounded delivery planner.
 *
 * This module is transport orchestration only. It never creates an HCP message,
 * opens a session, chooses a peer, touches a key or transmits a frame. The caller
 * must supply an already-authorised application sequence and must use a fresh
 * session frame for every SEND/RETRY event. An ACK must come from an authenticated
 * link layer; this module only checks that it matches the active sequence.
 */
#ifndef HERUS_DELIVERY_PLAN_H
#define HERUS_DELIVERY_PLAN_H

#include <stdint.h>

#define DELIVERY_PLAN_RETRY_INTERVAL_MS 1000u
#define DELIVERY_PLAN_MAX_ATTEMPTS      3u

typedef enum {
    DELIVERY_PLAN_IDLE = 0,
    DELIVERY_PLAN_WAIT_ACK,
    DELIVERY_PLAN_DELIVERED,
    DELIVERY_PLAN_EXPIRED,
    DELIVERY_PLAN_CANCELLED,
    DELIVERY_PLAN_FAILED
} delivery_plan_state_t;

typedef enum {
    DELIVERY_EVENT_NONE = 0,
    DELIVERY_EVENT_SEND,
    DELIVERY_EVENT_RETRY,
    DELIVERY_EVENT_DELIVERED,
    DELIVERY_EVENT_EXPIRED,
    DELIVERY_EVENT_CANCELLED,
    DELIVERY_EVENT_FAILED
} delivery_event_t;

enum {
    DELIVERY_PLAN_OK          = 0,
    DELIVERY_PLAN_E_ARG       = -1,
    DELIVERY_PLAN_E_AUTHORITY = -2,
    DELIVERY_PLAN_E_STATE     = -3,
    DELIVERY_PLAN_E_EXPIRED   = -4,
    DELIVERY_PLAN_E_LIMIT     = -5
};

typedef struct {
    uint16_t app_seq;
    uint8_t  max_attempts;
    uint8_t  attempts;
    uint64_t deadline_ms;
    uint64_t next_due_ms;
    delivery_plan_state_t state;
} delivery_plan_t;

typedef struct {
    uint32_t starts;
    uint32_t sends;
    uint32_t retries;
    uint32_t delivered;
    uint32_t expired;
    uint32_t cancelled;
    uint32_t failed;
    uint32_t invalid;
} delivery_plan_metrics_t;

void delivery_plan_init(delivery_plan_t *plan, delivery_plan_metrics_t *metrics);

/*
 * Begin only after the caller has completed the HERUS physical/assured handoff.
 * `authorized_handoff` must be exactly 1. `app_seq` must be nonzero, the deadline
 * must be in the future, and max_attempts is bounded to 1..3.
 */
int delivery_plan_begin(delivery_plan_t *plan, delivery_plan_metrics_t *metrics,
                       uint16_t app_seq, uint8_t max_attempts,
                       uint64_t now_ms, uint64_t deadline_ms,
                       uint8_t authorized_handoff);

/* Emit at most one transport event. The caller must create a fresh sealed frame
 * for SEND and RETRY; the planner never repeats bytes and never transmits. */
int delivery_plan_poll(delivery_plan_t *plan, delivery_plan_metrics_t *metrics,
                       uint64_t now_ms, delivery_event_t *event);

/* Call only after the authenticated link layer accepted an ACK for app_seq. */
int delivery_plan_ack(delivery_plan_t *plan, delivery_plan_metrics_t *metrics,
                      uint16_t app_seq, uint64_t now_ms,
                      delivery_event_t *event);

/* Physical/user cancellation is terminal and invalidates every future retry. */
int delivery_plan_cancel(delivery_plan_t *plan, delivery_plan_metrics_t *metrics,
                         delivery_event_t *event);

#endif /* HERUS_DELIVERY_PLAN_H */
