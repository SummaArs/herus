#include "watch_memory_frontend.h"
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

static watch_memory_observation_t observation(uint32_t session,
                                               uint8_t speaker,
                                               uint8_t runner,
                                               uint8_t relevance)
{
    watch_memory_observation_t o;
    memset(&o, 0, sizeof(o));
    o.capture_session_id = session;
    o.capture_authorized = 1u;
    o.wake_detected = 1u;
    o.muted = 0u;
    o.speaker_score_pct = speaker;
    o.speaker_runner_up_pct = runner;
    o.relevance_score_pct = relevance;
    o.novelty_pct = 80u;
    o.future_value_pct = 85u;
    o.consequence_pct = 70u;
    o.kind = MEMORY_KIND_IDEA;
    o.scope = MEMORY_SCOPE_SELF;
    o.sensitivity = MEMORY_SENSITIVITY_ORDINARY;
    o.now_ms = 100u;
    return o;
}

static void begin_and_expect(watch_memory_frontend_t *f, uint32_t session)
{
    ok(watch_memory_begin(f, session, 100u) == WATCH_MEMORY_OK,
       "new physical capture session begins");
    ok(watch_memory_state(f) == WATCH_MEMORY_LISTENING,
       "new session enters listening state");
}

static void test_authority_and_session(void)
{
    watch_memory_frontend_t f;
    watch_memory_frontend_init(&f, NULL);
    ok(watch_memory_begin(&f, 0u, 100u) == WATCH_MEMORY_E_AUTH,
       "zero session cannot open memory front-end");
    ok(watch_memory_state(&f) == WATCH_MEMORY_ABSTAIN,
       "missing session fails closed into abstention");

    begin_and_expect(&f, 9u);
    ok(watch_memory_begin(&f, 10u, 100u) == WATCH_MEMORY_E_STATE,
       "active listening session cannot be overwritten");

    watch_memory_observation_t o = observation(10u, 95u, 40u, 90u);
    ok(watch_memory_observe(&f, &o) == WATCH_MEMORY_E_AUTH,
       "observation from another session is rejected");
    ok(watch_memory_state(&f) == WATCH_MEMORY_ABSTAIN,
       "foreign session produces no candidate");
}

static void test_voice_and_relevance_abstention(void)
{
    watch_memory_frontend_t f;
    watch_memory_observation_t o;
    watch_memory_frontend_init(&f, NULL);

    begin_and_expect(&f, 20u);
    o = observation(20u, 95u, 40u, 90u);
    o.muted = 1u;
    ok(watch_memory_observe(&f, &o) == WATCH_MEMORY_NO_CANDIDATE,
       "mute blocks observation");
    ok(watch_memory_state(&f) == WATCH_MEMORY_MUTED,
       "mute is visible as a terminal session state");

    begin_and_expect(&f, 21u);
    o = observation(21u, 95u, 30u, 90u);
    o.wake_detected = 0u;
    ok(watch_memory_observe(&f, &o) == WATCH_MEMORY_NO_CANDIDATE,
       "absence of wake word abstains without format failure");
    ok(watch_memory_reason(&f) == WATCH_MEMORY_REASON_NO_WAKE,
       "absence of wake word is classified separately");

    begin_and_expect(&f, 22u);
    o = observation(22u, 79u, 20u, 90u);
    ok(watch_memory_observe(&f, &o) == WATCH_MEMORY_NO_CANDIDATE,
       "low speaker score abstains");
    ok(watch_memory_state(&f) == WATCH_MEMORY_ABSTAIN,
       "low speaker score cannot create a candidate");

    begin_and_expect(&f, 23u);
    o = observation(23u, 90u, 80u, 90u);
    ok(watch_memory_observe(&f, &o) == WATCH_MEMORY_NO_CANDIDATE,
       "ambiguous speaker score abstains");
    ok(watch_memory_state(&f) == WATCH_MEMORY_ABSTAIN,
       "ambiguous speaker cannot create a candidate");

    begin_and_expect(&f, 231u);
    o = observation(231u, 85u, 90u, 90u);
    ok(watch_memory_observe(&f, &o) == WATCH_MEMORY_NO_CANDIDATE,
       "runner-up above primary speaker abstains safely");
    ok(watch_memory_reason(&f) == WATCH_MEMORY_REASON_AMBIGUOUS_SPEAKER,
       "inverted speaker ranking is classified as ambiguous");

    begin_and_expect(&f, 24u);
    o = observation(24u, 95u, 30u, 59u);
    ok(watch_memory_observe(&f, &o) == WATCH_MEMORY_NO_CANDIDATE,
       "low relevance abstains");
    ok(watch_memory_state(&f) == WATCH_MEMORY_ABSTAIN,
       "low relevance cannot create a candidate");
}

static void test_format_and_candidate(void)
{
    watch_memory_frontend_t f;
    watch_memory_observation_t o;
    watch_memory_frontend_init(&f, NULL);

    begin_and_expect(&f, 30u);
    o = observation(30u, 95u, 30u, 90u);
    o.wake_detected = 2u;
    ok(watch_memory_observe(&f, &o) == WATCH_MEMORY_E_FORMAT,
       "noncanonical wake flag is rejected");
    ok(watch_memory_state(&f) == WATCH_MEMORY_ABSTAIN,
       "malformed observation cannot create a candidate");

    begin_and_expect(&f, 31u);
    o = observation(31u, 95u, 30u, 90u);
    ok(watch_memory_observe(&f, &o) == WATCH_MEMORY_OK,
       "qualified observation creates a candidate");
    ok(watch_memory_state(&f) == WATCH_MEMORY_CANDIDATE,
       "qualified observation enters transient candidate state");
}

static void test_confirmation_and_rejection(void)
{
    watch_memory_frontend_t f;
    watch_memory_observation_t o;
    memory_signal_t signal;
    watch_memory_frontend_init(&f, NULL);

    begin_and_expect(&f, 40u);
    o = observation(40u, 95u, 30u, 90u);
    ok(watch_memory_observe(&f, &o) == WATCH_MEMORY_OK,
       "candidate is available for rejection test");
    memset(&signal, 0xA5, sizeof(signal));
    ok(watch_memory_confirm(&f, 0u, 200u, &signal) == WATCH_MEMORY_NO_CANDIDATE,
       "physical rejection discards candidate");
    memory_signal_t zero;
    memset(&zero, 0, sizeof(zero));
    ok(memcmp(&signal, &zero, sizeof(signal)) == 0,
       "rejected candidate output is scrubbed");
    ok(watch_memory_state(&f) == WATCH_MEMORY_REJECTED,
       "rejection is terminal");

    begin_and_expect(&f, 41u);
    o = observation(41u, 95u, 30u, 90u);
    ok(watch_memory_observe(&f, &o) == WATCH_MEMORY_OK,
       "candidate is available for confirmation test");
    memset(&signal, 0, sizeof(signal));
    ok(watch_memory_confirm(&f, 1u, 200u, &signal) == WATCH_MEMORY_OK,
       "physical confirmation emits memory signal");
    ok(signal.session_authorized == 1u && signal.explicit_remember == 1u &&
           signal.kind == MEMORY_KIND_IDEA &&
           signal.scope == MEMORY_SCOPE_SELF,
       "confirmed signal carries only typed policy fields");
    ok(watch_memory_state(&f) == WATCH_MEMORY_CONFIRMED,
       "confirmation is terminal");
    ok(watch_memory_confirm(&f, 1u, 201u, &signal) == WATCH_MEMORY_E_STATE,
       "confirmed candidate cannot be confirmed twice");
}

static void test_expiry_and_mute(void)
{
    watch_memory_frontend_t f;
    watch_memory_observation_t o;
    memory_signal_t signal;
    watch_memory_frontend_init(&f, NULL);

    begin_and_expect(&f, 50u);
    o = observation(50u, 95u, 30u, 90u);
    ok(watch_memory_observe(&f, &o) == WATCH_MEMORY_OK,
       "candidate is available for expiry test");
    ok(watch_memory_tick(&f, 8100u) == WATCH_MEMORY_E_EXPIRED,
       "unconfirmed candidate expires");
    ok(watch_memory_state(&f) == WATCH_MEMORY_EXPIRED,
       "expiry is terminal");
    memset(&signal, 0, sizeof(signal));
    ok(watch_memory_confirm(&f, 1u, 8101u, &signal) == WATCH_MEMORY_E_STATE,
       "expired candidate cannot be confirmed");

    begin_and_expect(&f, 51u);
    o = observation(51u, 95u, 30u, 90u);
    ok(watch_memory_observe(&f, &o) == WATCH_MEMORY_OK,
       "candidate is available for mute test");
    watch_memory_mute(&f);
    ok(watch_memory_state(&f) == WATCH_MEMORY_MUTED,
       "mute scrubs candidate and is terminal");
    ok(watch_memory_confirm(&f, 1u, 200u, &signal) == WATCH_MEMORY_E_STATE,
       "muted candidate cannot be confirmed");
}

int main(void)
{
    test_authority_and_session();
    test_voice_and_relevance_abstention();
    test_format_and_candidate();
    test_confirmation_and_rejection();
    test_expiry_and_mute();

    if (failures) {
        printf("%d watch memory front-end tests failed\n", failures);
        return 1;
    }
    printf("ALL WATCH MEMORY FRONT-END INVARIANTS HOLD\n");
    return 0;
}
