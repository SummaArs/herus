/* test_interaction_rig.c — deterministic adapter/lab scenarios for Advance 3. */
#include "interaction_rig.h"
#include <stdio.h>
#include <string.h>

static int FAILED = 0;
static assurance_snapshot_t safe_snapshot(void)
{
    assurance_snapshot_t s;
    memset(&s, 0, sizeof(s));
    s.source = ASSURANCE_SOURCE_CORE;
    s.physical_session_current = 1u;
    s.intent_accepted = 1u;
    s.physical_confirmation = 1u;
    s.handoff_unused = 1u;
    s.model_reply_display_only = 1u;
    return s;
}
static void ok(int cond, const char *what)
{
    printf("  %-4s %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond) FAILED = 1;
}

static void test_nominal_lab_path(void)
{
    interaction_rig_t rig;
    hcp_msg_t out;
    assurance_snapshot_t assurance = safe_snapshot();
    const interaction_rig_metrics_t *m;

    printf("\n== R1  deterministic Core/Nucleus adapter path ==\n");
    interaction_rig_init(&rig, NULL);
    ok(interaction_rig_take_send(&rig, &out) == INTERACTION_E_UNTRUSTED,
       "R1 raw adapter handoff is fail-closed without an assurance snapshot");
    ok(interaction_rig_push(&rig, 1000) == INTERACTION_OK &&
       interaction_rig_metrics(&rig)->capture_started == 1,
       "R1 physical push creates exactly one adapter capture request");
    ok(interaction_rig_transcript(&rig, "chego em dez minutos", 1500) == INTERACTION_OK &&
       rig.runtime.state == INTERACTION_AWAIT_CONFIRM &&
       interaction_rig_metrics(&rig)->capture_stopped == 1 &&
       interaction_rig_metrics(&rig)->drafts_presented == 1 &&
       interaction_rig_metrics(&rig)->last_haptic_safe,
       "R1 local ASR result stops capture, presents a draft and emits a safe haptic plan");
    ok(interaction_rig_confirm(&rig, 1, 1700) == INTERACTION_OK &&
       interaction_rig_take_send_assured(&rig, &assurance, &out) == INTERACTION_OK &&
       out.intent == rig.runtime.cfg.lexicon.intent_arrive &&
       interaction_rig_metrics(&rig)->handoffs == 1,
       "R1 one explicit confirmation yields exactly one application hand-off");
    ok(interaction_rig_take_send_assured(&rig, &assurance, &out) == INTERACTION_E_STATE,
       "R1 the rig cannot hand the same meaning to the link twice");

    interaction_rig_note_energy_uj(&rig, 12400);
    m = interaction_rig_metrics(&rig);
    ok(m->measured_energy_uj == 12400 &&
       interaction_metrics(&rig.runtime)->measured_energy_uj == 12400,
       "R1 a measured energy integral is mirrored without audio or transcript data");
}

static void test_terminal_adapter_paths(void)
{
    interaction_rig_t rig;
    hcp_msg_t out;
    assurance_snapshot_t assurance = safe_snapshot();

    printf("\n== R2  adapter failure paths cannot retain or send a draft ==\n");
    interaction_rig_init(&rig, NULL);
    interaction_rig_push(&rig, 0);
    interaction_rig_set_source(&rig, 0, 50);
    ok(rig.runtime.state == INTERACTION_LINK_LOST &&
       interaction_rig_metrics(&rig)->capture_stopped == 1 &&
       interaction_rig_take_send_assured(&rig, &assurance, &out) == INTERACTION_E_STATE,
       "R2 loss of Core/Nucleus ASR stops capture and rejects any hand-off");

    interaction_rig_set_source(&rig, 1, 100);
    interaction_rig_push(&rig, 200);
    interaction_rig_transcript(&rig, "estou chegando", 250);
    interaction_rig_tick(&rig, 250 + INTERACTION_DEFAULT_CONFIRM_MS);
    ok(rig.runtime.state == INTERACTION_CANCELLED &&
       interaction_rig_metrics(&rig)->drafts_cleared >= 1 && rig.runtime.pending.intent == 0,
       "R2 confirmation timeout clears UI and semantic pending state");

    interaction_rig_push(&rig, 9000);
    interaction_rig_tick(&rig, 9000 + INTERACTION_DEFAULT_LISTEN_MS);
    ok(rig.runtime.state == INTERACTION_TIMED_OUT &&
       interaction_rig_metrics(&rig)->capture_stopped >= 2 &&
       interaction_rig_take_send_assured(&rig, &assurance, &out) == INTERACTION_E_STATE,
       "R2 listen timeout ends the adapter session before any transcript or send");
}

int main(void)
{
    test_nominal_lab_path();
    test_terminal_adapter_paths();
    if (FAILED) {
        printf("INTERACTION RIG TESTS FAILED\n");
        return 1;
    }
    printf("INTERACTION RIG INVARIANTS HOLD — adapter sequencing is deterministic and non-transmitting.\n");
    return 0;
}
