/* test_interaction.c — executable state-machine contract for Advance 2. */
#include "interaction.h"
#include <stdio.h>
#include <string.h>

static int FAILED = 0;
static void ok(int cond, const char *what)
{
    printf("  %-4s %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond) FAILED = 1;
}

static void test_only_confirmed_draft_sends(void)
{
    interaction_t it;
    hcp_msg_t out;

    printf("\n== I1  push-to-talk -> local ASR -> confirmation -> one send ==\n");
    interaction_init(&it, NULL);
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
    interaction_init(&it, NULL);
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

static void test_confirmation_timeout_and_telemetry(void)
{
    interaction_t it;
    const interaction_metrics_t *m;

    printf("\n== I3  confirmation deadline and privacy-preserving telemetry ==\n");
    interaction_init(&it, NULL);
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
    test_only_confirmed_draft_sends();
    test_terminal_paths_clear_drafts();
    test_confirmation_timeout_and_telemetry();
    if (FAILED) {
        printf("INTERACTION TESTS FAILED\n");
        return 1;
    }
    printf("INTERACTION INVARIANTS HOLD — local speech is push-to-talk, confirmed and one-shot.\n");
    return 0;
}
