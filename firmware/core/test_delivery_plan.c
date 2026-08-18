#include "delivery_plan.h"
#include <stdio.h>
#include <string.h>

static int failures;

static void ok(int condition, const char *message)
{
    if (!condition) {
        printf("FAIL: %s\n", message);
        failures++;
    }
}

static void expect_event(delivery_event_t got, delivery_event_t expected,
                         const char *message)
{
    ok(got == expected, message);
}

static void test_authority_and_arguments(void)
{
    delivery_plan_t p;
    delivery_plan_metrics_t m;
    delivery_event_t ev;

    delivery_plan_init(&p, &m);
    ok(delivery_plan_begin(&p, &m, 7u, 2u, 100u, 500u, 0u) ==
           DELIVERY_PLAN_E_AUTHORITY,
       "unconfirmed handoff cannot begin delivery");
    ok(p.state == DELIVERY_PLAN_IDLE && p.app_seq == 0u,
       "rejected begin clears plan");

    ok(delivery_plan_begin(&p, &m, 0u, 2u, 100u, 500u, 1u) ==
           DELIVERY_PLAN_E_ARG,
       "zero application sequence is rejected");
    ok(delivery_plan_begin(&p, &m, 7u, 4u, 100u, 500u, 1u) ==
           DELIVERY_PLAN_E_ARG,
       "attempt limit above bound is rejected");
    ok(delivery_plan_begin(&p, &m, 7u, 2u, 500u, 500u, 1u) ==
           DELIVERY_PLAN_E_ARG,
       "non-future deadline is rejected");
    ok(delivery_plan_begin(&p, &m, 7u, 2u, 100u, 500u, 1u) ==
           DELIVERY_PLAN_OK,
       "valid plan arms after rejected starts");
    ok(delivery_plan_begin(&p, &m, 8u, 2u, 100u, 500u, 1u) ==
           DELIVERY_PLAN_E_STATE,
       "active plan cannot be overwritten");
    ok(p.app_seq == 7u && p.state == DELIVERY_PLAN_WAIT_ACK,
       "active plan remains intact after overwrite attempt");
    ok(m.invalid == 4u,
       "only malformed or unauthorised starts are counted as invalid");

    ok(delivery_plan_poll(&p, &m, 100u, &ev) == DELIVERY_PLAN_OK,
       "active plan can produce its first send");
    expect_event(ev, DELIVERY_EVENT_SEND, "armed plan emits SEND");
    delivery_plan_init(&p, &m);
    ok(delivery_plan_poll(&p, &m, 100u, &ev) == DELIVERY_PLAN_E_STATE,
       "polling an idle plan is rejected");
}

static void test_bounded_send_retry_and_ack(void)
{
    delivery_plan_t p;
    delivery_plan_metrics_t m;
    delivery_event_t ev;

    delivery_plan_init(&p, &m);
    ok(delivery_plan_begin(&p, &m, 42u, 3u, 1000u, 10000u, 1u) ==
           DELIVERY_PLAN_OK,
       "authorised plan begins");

    ok(delivery_plan_poll(&p, &m, 1000u, &ev) == DELIVERY_PLAN_OK,
       "initial poll succeeds");
    expect_event(ev, DELIVERY_EVENT_SEND, "first due event is SEND");
    ok(p.attempts == 1u, "first event consumes one attempt");

    ok(delivery_plan_poll(&p, &m, 1500u, &ev) == DELIVERY_PLAN_OK,
       "early poll succeeds without event");
    expect_event(ev, DELIVERY_EVENT_NONE, "early poll does not duplicate SEND");

    ok(delivery_plan_poll(&p, &m, 2000u, &ev) == DELIVERY_PLAN_OK,
       "retry poll succeeds");
    expect_event(ev, DELIVERY_EVENT_RETRY, "second due event is RETRY");
    ok(p.attempts == 2u, "retry consumes one bounded attempt");

    ok(delivery_plan_ack(&p, &m, 41u, 2200u, &ev) == DELIVERY_PLAN_E_STATE,
       "ACK for another application sequence is rejected");
    expect_event(ev, DELIVERY_EVENT_NONE, "wrong ACK creates no event");
    ok(p.state == DELIVERY_PLAN_WAIT_ACK,
       "wrong ACK does not terminate the active plan");

    ok(delivery_plan_ack(&p, &m, 42u, 2200u, &ev) == DELIVERY_PLAN_OK,
       "matching authenticated ACK is accepted");
    expect_event(ev, DELIVERY_EVENT_DELIVERED, "matching ACK emits DELIVERED");
    ok(p.state == DELIVERY_PLAN_DELIVERED && m.delivered == 1u,
       "delivery becomes terminal exactly once");
    ok(delivery_plan_poll(&p, &m, 3000u, &ev) == DELIVERY_PLAN_E_STATE,
       "delivered plan cannot produce another retry");
}

static void test_attempt_exhaustion_and_cancel(void)
{
    delivery_plan_t p;
    delivery_plan_metrics_t m;
    delivery_event_t ev;

    delivery_plan_init(&p, &m);
    ok(delivery_plan_begin(&p, &m, 9u, 2u, 0u, 10000u, 1u) ==
           DELIVERY_PLAN_OK,
       "second plan begins");
    ok(delivery_plan_poll(&p, &m, 0u, &ev) == DELIVERY_PLAN_OK,
       "second plan sends");
    expect_event(ev, DELIVERY_EVENT_SEND, "second plan initial send");
    ok(delivery_plan_poll(&p, &m, 1000u, &ev) == DELIVERY_PLAN_OK,
       "second plan retries");
    expect_event(ev, DELIVERY_EVENT_RETRY, "second plan bounded retry");
    ok(delivery_plan_poll(&p, &m, 2000u, &ev) == DELIVERY_PLAN_OK,
       "exhaustion poll succeeds");
    expect_event(ev, DELIVERY_EVENT_FAILED, "attempt exhaustion is terminal");
    ok(p.state == DELIVERY_PLAN_FAILED && m.failed == 1u,
       "attempt exhaustion cannot silently continue");

    delivery_plan_init(&p, &m);
    ok(delivery_plan_begin(&p, &m, 10u, 3u, 100u, 10000u, 1u) ==
           DELIVERY_PLAN_OK,
       "cancel plan begins");
    ok(delivery_plan_cancel(&p, &m, &ev) == DELIVERY_PLAN_OK,
       "active plan can be cancelled");
    expect_event(ev, DELIVERY_EVENT_CANCELLED, "cancel emits terminal event");
    ok(delivery_plan_poll(&p, &m, 5000u, &ev) == DELIVERY_PLAN_E_STATE,
       "cancelled plan has no future retry");
    ok(m.cancelled == 1u, "cancellation is counted");
}

static void test_expiry_and_ack_order(void)
{
    delivery_plan_t p;
    delivery_plan_metrics_t m;
    delivery_event_t ev;

    delivery_plan_init(&p, &m);
    ok(delivery_plan_begin(&p, &m, 11u, 1u, 100u, 500u, 1u) ==
           DELIVERY_PLAN_OK,
       "expiry plan begins");
    ok(delivery_plan_ack(&p, &m, 11u, 200u, &ev) == DELIVERY_PLAN_E_STATE,
       "ACK before any send is rejected");
    expect_event(ev, DELIVERY_EVENT_NONE, "early ACK creates no event");
    ok(delivery_plan_poll(&p, &m, 100u, &ev) == DELIVERY_PLAN_OK,
       "expiry plan sends once");
    expect_event(ev, DELIVERY_EVENT_SEND, "expiry plan initial send");
    ok(delivery_plan_poll(&p, &m, 500u, &ev) == DELIVERY_PLAN_OK,
       "deadline poll succeeds");
    expect_event(ev, DELIVERY_EVENT_EXPIRED, "deadline is terminal");
    ok(p.state == DELIVERY_PLAN_EXPIRED && m.expired == 1u,
       "expired plan cannot be delivered later");
    ok(delivery_plan_ack(&p, &m, 11u, 501u, &ev) == DELIVERY_PLAN_E_STATE,
       "late ACK cannot reopen an expired plan");
}

int main(void)
{
    test_authority_and_arguments();
    test_bounded_send_retry_and_ack();
    test_attempt_exhaustion_and_cancel();
    test_expiry_and_ack_order();

    if (failures) {
        printf("%d delivery-plan tests failed\n", failures);
        return 1;
    }
    printf("ALL DELIVERY PLAN INVARIANTS HOLD\n");
    return 0;
}
