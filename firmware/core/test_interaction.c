/* test_interaction.c — executable state-machine contract for Advance 2. */
#include "interaction.h"
#include <stdio.h>
#include <string.h>

static int FAILED = 0;

static void init_test(interaction_t *it)
{
    interaction_config_t cfg;
    interaction_config_default(&cfg);
    cfg.allow_test_transcript = 1;
    interaction_init(it, &cfg);
}

static void ok(int cond, const char *what)
{
    printf("  %-4s %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond) FAILED = 1;
}

static void test_production_rejects_text_path(void)
{
    interaction_t it;
    printf("\n== I0  production accepts typed local ASR, not raw test text ==\n");
    interaction_init(&it, NULL);
    interaction_push_to_talk(&it, 10);
    ok(interaction_transcript(&it, "chego", 20) == INTERACTION_E_UNTRUSTED &&
       it.state == INTERACTION_LISTENING && interaction_actions(&it)->start_capture,
       "I0 raw transcript input is disabled by default and cannot end the live session");
}

static void test_only_confirmed_draft_sends(void)
{
    interaction_t it;
    hcp_msg_t out;

    printf("\n== I1  push-to-talk -> local ASR -> confirmation -> one send ==\n");
    init_test(&it);
    ok(interaction_push_to_talk(&it, 100) == INTERACTION_OK &&
       it.state == INTERACTION_LISTENING && interaction_actions(&it)->start_capture,
       "I1 only a push-to-talk event starts the local capture window");
    ok(interaction_take_send(&it, &out) == INTERACTION_E_STATE,
       "I1 no message can leave while the runtime is listening");

    ok(interaction_transcript(&it, "chego em vinte e cinco minutos", 600) == INTERACTION_OK &&
       it.state == INTERACTION_AWAIT_CONFIRM && interaction_actions(&it)->stop_capture &&
       interaction_actions(&it)->present_draft && interaction_actions(&it)->play_haptic,
       "I1 a valid local transcript stops capture and presents a haptic draft");
    ok(interaction_take_send(&it, &out) == INTERACTION_E_STATE,
       "I1 a valid draft still cannot be transmitted before physical confirmation");

    ok(interaction_confirm(&it, 1, 650) == INTERACTION_OK && it.state == INTERACTION_READY_SEND,
       "I1 only an explicit positive confirmation opens the one-time send gate");
    ok(interaction_take_send(&it, &out) == INTERACTION_OK && it.state == INTERACTION_IDLE &&
       out.intent == it.cfg.lexicon.intent_arrive && out.nslot == 1 &&
       out.slot[0].filler == it.cfg.lexicon.minute_filler_base + 25 &&
       out.seq == 0 && out.ttl == 0 && out.prio == 0,
       "I1 the application receives semantic content once, with transport fields still unset");
    ok(interaction_take_send(&it, &out) == INTERACTION_E_STATE &&
       interaction_metrics(&it)->sent == 1 && interaction_metrics(&it)->last_latency_ms == 500,
       "I1 taking the send returns to idle and prevents duplicate transmission");
}

static void test_terminal_paths_clear_drafts(void)
{
    interaction_t it;
    hcp_msg_t out;

    printf("\n== I2  cancellation, timeout, source loss and help fail safely ==\n");
    init_test(&it);
    interaction_push_to_talk(&it, 0);
    interaction_tick(&it, INTERACTION_DEFAULT_LISTEN_MS);
    ok(it.state == INTERACTION_TIMED_OUT && interaction_actions(&it)->stop_capture &&
       interaction_actions(&it)->clear_draft && interaction_take_send(&it, &out) == INTERACTION_E_STATE,
       "I2 listening timeout stops capture, erases state and never opens send");

    interaction_push_to_talk(&it, 7000);
    interaction_transcript(&it, "preciso de socorro", 7100);
    ok(it.state == INTERACTION_AWAIT_CONFIRM && it.pending.tier != HCP_TIER_SOS &&
       interaction_take_send(&it, &out) == INTERACTION_E_STATE,
       "I2 spoken help remains a private draft and cannot become public SOS or auto-send");
    interaction_confirm(&it, 0, 7200);
    ok(it.state == INTERACTION_CANCELLED && interaction_actions(&it)->clear_draft &&
       it.pending.intent == 0,
       "I2 declining confirmation clears the critical draft locally");

    interaction_push_to_talk(&it, 8000);
    interaction_set_asr_available(&it, 0, 8050);
    ok(it.state == INTERACTION_LINK_LOST && interaction_actions(&it)->stop_capture &&
       interaction_take_send(&it, &out) == INTERACTION_E_STATE,
       "I2 loss of the local ASR source stops capture and cannot leave a stale draft");
    interaction_set_asr_available(&it, 1, 8100);
    ok(interaction_push_to_talk(&it, 8200) == INTERACTION_OK,
       "I2 a new explicit button press is required after source recovery");
    interaction_transcript(&it, "cancelar", 8250);
    ok(it.state == INTERACTION_CANCELLED && interaction_metrics(&it)->cancelled == 2,
       "I2 spoken cancellation remains local and is accounted for as a terminal event");
}

static intent_observation_t asr(uint32_t session, voice_command_t command,
                                 uint8_t confidence, uint8_t runner_up)
{
    intent_observation_t o;
    o.source = INTENT_SOURCE_CORE;
    o.session_id = session;
    o.command = command;
    o.minutes = 0;
    o.confidence_pct = confidence;
    o.runner_up_pct = runner_up;
    return o;
}

static void test_typed_asr_is_still_confirmed(void)
{
    interaction_t it;
    hcp_msg_t out;
    intent_observation_t o;
    intent_context_hint_t hint;

    printf("\n== I4  typed ASR confidence gate remains non-transmitting ==\n");
    interaction_init(&it, NULL);
    interaction_push_to_talk(&it, 100);
    o = asr(interaction_session_id(&it) - 1u, VOICE_COMMAND_ARRIVE, 100, 0);
    ok(interaction_asr_result(&it, &o, NULL, 120) == INTERACTION_OK &&
       it.state == INTERACTION_LISTENING && interaction_metrics(&it)->asr_stale == 1,
       "I4 stale ASR output is ignored without ending the current physical session");

    o = asr(interaction_session_id(&it), VOICE_COMMAND_ARRIVE, 75, 0);
    ok(interaction_asr_result(&it, &o, NULL, 140) == INTERACTION_OK &&
       it.state == INTERACTION_REJECTED && interaction_metrics(&it)->asr_low_confidence == 1 &&
       interaction_take_send(&it, &out) == INTERACTION_E_STATE,
       "I4 low confidence stops capture and cannot create a sendable draft");

    interaction_push_to_talk(&it, 200);
    hint.available = 1; hint.command = VOICE_COMMAND_ARRIVE;
    hint.support = 3; hint.confidence_pct = 70;
    o = asr(interaction_session_id(&it), VOICE_COMMAND_ARRIVE, 86, 75);
    ok(interaction_asr_result(&it, &o, &hint, 300) == INTERACTION_OK &&
       it.state == INTERACTION_AWAIT_CONFIRM && interaction_metrics(&it)->asr_context_assisted == 1 &&
       interaction_take_send(&it, &out) == INTERACTION_E_STATE,
       "I4 qualified context may disambiguate only into a confirmed local draft");
    ok(interaction_confirm(&it, 1, 320) == INTERACTION_OK &&
       interaction_take_send(&it, &out) == INTERACTION_OK && out.intent == it.cfg.lexicon.intent_arrive,
       "I4 even a context-assisted command still needs physical confirmation for one handoff");
}

static void test_confirmation_timeout_and_telemetry(void)
{
    interaction_t it;
    const interaction_metrics_t *m;

    printf("\n== I3  confirmation deadline and privacy-preserving telemetry ==\n");
    init_test(&it);
    interaction_push_to_talk(&it, 1000);
    interaction_transcript(&it, "estou chegando", 1100);
    interaction_tick(&it, 1100 + INTERACTION_DEFAULT_CONFIRM_MS);
    ok(it.state == INTERACTION_CANCELLED && interaction_actions(&it)->clear_draft &&
       it.pending.intent == 0,
       "I3 an unanswered draft expires and is cleared before a future session");

    interaction_note_energy_uj(&it, 1234);
    interaction_note_energy_uj(&it, 66);
    m = interaction_metrics(&it);
    ok(m->sessions == 1 && m->drafts == 1 && m->cancelled == 1 &&
       m->measured_energy_uj == 1300,
       "I3 telemetry keeps counts, latency and measured energy without transcript storage");
}

int main(void)
{
    test_production_rejects_text_path();
    test_only_confirmed_draft_sends();
    test_terminal_paths_clear_drafts();
    test_typed_asr_is_still_confirmed();
    test_confirmation_timeout_and_telemetry();
    if (FAILED) {
        printf("INTERACTION TESTS FAILED\n");
        return 1;
    }
    printf("INTERACTION INVARIANTS HOLD — local speech is push-to-talk, confirmed and one-shot.\n");
    return 0;
}
