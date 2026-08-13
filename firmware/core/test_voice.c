/* test_voice.c — proof that voice is local, bounded and cannot transmit itself. */
#include "voice.h"
#include <stdio.h>
#include <string.h>

static int FAILED = 0;
static void ok(int cond, const char *what)
{
    printf("  %-4s %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond) FAILED = 1;
}

static void test_arrival(void)
{
    voice_lexicon_t lex;
    voice_result_t r;

    printf("\n== V1  controlled Portuguese creates a semantic draft ==\n");
    voice_lexicon_default(&lex);
    ok(voice_parse_pt("Chego em dez minutos", &lex, &r) == VOICE_DRAFT &&
       r.event == VOICE_EVENT_DRAFT && r.requires_confirmation && r.minutes == 10 &&
       r.draft.tier == HCP_TIER_COMPOSED && r.draft.intent == lex.intent_arrive &&
       r.draft.nslot == 1 && r.draft.slot[0].role == lex.role_time &&
       r.draft.slot[0].filler == lex.minute_filler_base + 10,
       "V1 arrival with a spoken duration becomes one configured HCP time slot");
    ok(r.draft.seq == 0 && r.draft.ttl == 0 && r.draft.prio == 0 &&
       r.draft.flags == 0 && r.draft.pos[0] == 0,
       "V1 voice produces no transport state and cannot be sent without confirmation code");

    ok(voice_parse_pt("estou chegando", &lex, &r) == VOICE_DRAFT &&
       r.minutes == 0 && r.draft.tier == HCP_TIER_GLYPH && r.draft.nslot == 0,
       "V1 arrival without duration remains a compact semantic glyph");
}

static void test_fail_closed_and_critical(void)
{
    voice_lexicon_t lex;
    voice_result_t r;

    printf("\n== V2  cancellation, unknown speech and help fail safely ==\n");
    voice_lexicon_default(&lex);
    ok(voice_parse_pt("cancelar", &lex, &r) == VOICE_CANCEL_LOCAL &&
       r.event == VOICE_EVENT_CANCEL && !r.requires_confirmation && r.draft.intent == 0,
       "V2 cancellation discards only the local draft and never makes a frame");
    ok(voice_parse_pt("toque uma musica", &lex, &r) == VOICE_UNKNOWN &&
       r.event == VOICE_EVENT_UNKNOWN && r.draft.intent == 0,
       "V2 unknown language fails closed instead of guessing a radio intent");
    ok(voice_parse_pt("chego em noventa minutos", &lex, &r) == VOICE_REJECTED &&
       r.event == VOICE_EVENT_REJECTED && r.draft.intent == 0,
       "V2 a duration outside the configured vocabulary is rejected, not truncated");
    ok(voice_parse_pt("preciso de socorro", &lex, &r) == VOICE_DRAFT &&
       r.event == VOICE_EVENT_CRITICAL_DRAFT && r.requires_confirmation &&
       r.draft.intent == lex.intent_help && r.draft.tier != HCP_TIER_SOS,
       "V2 spoken help is a private critical draft, never an autonomous public SOS");
}

static void test_haptics(void)
{
    haptic_plan_t p;
    printf("\n== V3  haptic plans are finite and driver-independent ==\n");
    for (int event = VOICE_EVENT_DRAFT; event <= VOICE_EVENT_REJECTED; event++) {
        voice_haptic_plan((voice_event_t)event, &p);
        ok(haptic_plan_safe(&p), "V3 every standard voice event has a bounded safe vibration plan");
    }
    memset(&p, 0, sizeof(p));
    p.n = 1; p.pulse[0].on_ms = HAPTIC_MAX_ON_MS + 1;
    ok(!haptic_plan_safe(&p), "V3 an overlong motor-on command is rejected before hardware");
    p.n = HAPTIC_MAX_PULSE + 1;
    ok(!haptic_plan_safe(&p), "V3 too many pulses are rejected before hardware");
}

int main(void)
{
    test_arrival();
    test_fail_closed_and_critical();
    test_haptics();
    if (FAILED) {
        printf("VOICE TESTS FAILED\n");
        return 1;
    }
    printf("VOICE/HAPTIC INVARIANTS HOLD — speech is local, confirmed and bounded.\n");
    return 0;
}
