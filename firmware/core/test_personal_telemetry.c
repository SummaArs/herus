#include "personal_telemetry.h"
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

static personal_telemetry_sample_t sample(uint32_t session)
{
    personal_telemetry_sample_t s;
    memset(&s, 0, sizeof(s));
    s.capture_session_id = session;
    s.capture_authorized = 1u;
    s.collect_consent = 1u;
    s.muted = 0u;
    s.kind = TELEMETRY_KIND_STEPS;
    s.source = TELEMETRY_SOURCE_ACCELEROMETER;
    s.quality = TELEMETRY_QUALITY_USABLE;
    s.value = 3200;
    s.unit_scale = 1;
    s.window_start_ms = 100u;
    s.window_end_ms = 200u;
    s.now_ms = 500u;
    return s;
}

static void begin(personal_telemetry_t *t, uint32_t session)
{
    ok(personal_telemetry_begin(t, session, 1u, 100u) ==
           PERSONAL_TELEMETRY_OK,
       "consented telemetry session begins");
    ok(personal_telemetry_state(t) == TELEMETRY_CAPTURING,
       "telemetry session enters capturing state");
}

static void test_consent_and_candidate(void)
{
    personal_telemetry_t t;
    personal_telemetry_sample_t s;
    personal_telemetry_record_t record;
    personal_telemetry_init(&t, NULL);

    ok(personal_telemetry_begin(&t, 0u, 1u, 100u) ==
           PERSONAL_TELEMETRY_E_ARG,
       "zero session cannot start telemetry");
    ok(personal_telemetry_begin(&t, 1u, 0u, 100u) ==
           PERSONAL_TELEMETRY_E_AUTH,
       "collection without consent is refused");

    begin(&t, 10u);
    s = sample(10u);
    ok(personal_telemetry_observe(&t, &s) == PERSONAL_TELEMETRY_OK,
       "usable derived sample creates a candidate");
    ok(personal_telemetry_state(&t) == TELEMETRY_CANDIDATE &&
           t.pending.persist_authorized == 0u,
       "candidate remains transient and not persistence-authorized");

    memset(&record, 0xA5, sizeof(record));
    ok(personal_telemetry_confirm(&t, 0u, 200u, &record) ==
           PERSONAL_TELEMETRY_NO_CANDIDATE,
       "physical rejection discards telemetry candidate");
    personal_telemetry_record_t zero;
    memset(&zero, 0, sizeof(zero));
    ok(memcmp(&record, &zero, sizeof(record)) == 0,
       "rejected telemetry output is scrubbed");
}

static void test_confirmation_and_session(void)
{
    personal_telemetry_t t;
    personal_telemetry_sample_t s;
    personal_telemetry_record_t record;
    personal_telemetry_init(&t, NULL);

    begin(&t, 20u);
    s = sample(21u);
    ok(personal_telemetry_observe(&t, &s) == PERSONAL_TELEMETRY_E_AUTH,
       "foreign telemetry session is rejected");
    ok(personal_telemetry_state(&t) == TELEMETRY_REJECTED,
       "foreign session cannot retain a candidate");

    begin(&t, 22u);
    s = sample(22u);
    ok(personal_telemetry_observe(&t, &s) == PERSONAL_TELEMETRY_OK,
       "new session can create a fresh candidate");
    ok(personal_telemetry_confirm(&t, 1u, 200u, &record) ==
           PERSONAL_TELEMETRY_OK,
       "physical confirmation emits retained telemetry");
    ok(record.persist_authorized == 1u &&
           record.kind == TELEMETRY_KIND_STEPS &&
           record.source == TELEMETRY_SOURCE_ACCELEROMETER &&
           record.value == 3200,
       "confirmed record carries only typed derived fields");
    ok(personal_telemetry_state(&t) == TELEMETRY_CONFIRMED,
       "confirmation is terminal");
}

static void test_quality_range_and_format(void)
{
    personal_telemetry_t t;
    personal_telemetry_sample_t s;
    personal_telemetry_init(&t, NULL);

    begin(&t, 30u);
    s = sample(30u);
    s.quality = TELEMETRY_QUALITY_LOW;
    ok(personal_telemetry_observe(&t, &s) == PERSONAL_TELEMETRY_E_QUALITY,
       "low quality sample abstains");

    begin(&t, 31u);
    s = sample(31u);
    s.kind = TELEMETRY_KIND_HEART_RATE;
    s.source = TELEMETRY_SOURCE_ACCELEROMETER;
    s.value = 80;
    ok(personal_telemetry_observe(&t, &s) == PERSONAL_TELEMETRY_E_FORMAT,
       "incompatible sensor source is rejected");

    begin(&t, 32u);
    s = sample(32u);
    s.kind = TELEMETRY_KIND_HEART_RATE;
    s.source = TELEMETRY_SOURCE_PPG;
    s.value = 300;
    ok(personal_telemetry_observe(&t, &s) == PERSONAL_TELEMETRY_E_FORMAT,
       "out-of-range heart rate is rejected");

    begin(&t, 33u);
    s = sample(33u);
    s.muted = 2u;
    ok(personal_telemetry_observe(&t, &s) == PERSONAL_TELEMETRY_E_FORMAT,
       "noncanonical mute flag is rejected");
}

static void test_expiry_mute_and_window(void)
{
    personal_telemetry_t t;
    personal_telemetry_sample_t s;
    personal_telemetry_record_t record;
    personal_telemetry_init(&t, NULL);

    begin(&t, 40u);
    s = sample(40u);
    s.window_start_ms = 700u;
    s.window_end_ms = 600u;
    ok(personal_telemetry_observe(&t, &s) == PERSONAL_TELEMETRY_E_FORMAT,
       "reversed observation window is rejected");

    begin(&t, 41u);
    s = sample(41u);
    ok(personal_telemetry_observe(&t, &s) == PERSONAL_TELEMETRY_OK,
       "candidate is available for expiry");
    ok(personal_telemetry_tick(&t, 8200u) == PERSONAL_TELEMETRY_E_EXPIRED,
       "unconfirmed telemetry expires");
    ok(personal_telemetry_confirm(&t, 1u, 8201u, &record) ==
           PERSONAL_TELEMETRY_E_STATE,
       "expired telemetry cannot be confirmed");

    begin(&t, 42u);
    s = sample(42u);
    ok(personal_telemetry_observe(&t, &s) == PERSONAL_TELEMETRY_OK,
       "candidate is available for mute");
    personal_telemetry_mute(&t);
    ok(personal_telemetry_state(&t) == TELEMETRY_MUTED,
       "mute clears candidate and becomes terminal");
    ok(personal_telemetry_confirm(&t, 1u, 200u, &record) ==
           PERSONAL_TELEMETRY_E_STATE,
       "muted telemetry cannot be confirmed");
}

int main(void)
{
    test_consent_and_candidate();
    test_confirmation_and_session();
    test_quality_range_and_format();
    test_expiry_mute_and_window();

    if (failures) {
        printf("%d personal telemetry tests failed\n", failures);
        return 1;
    }
    printf("ALL PERSONAL TELEMETRY INVARIANTS HOLD\n");
    return 0;
}
