/* interaction_rig.c — host-only adapter counters for Advance 3. */
#include "interaction_rig.h"
#include <string.h>

static void apply_actions(interaction_rig_t *rig)
{
    const interaction_actions_t *a = interaction_actions(&rig->runtime);
    if (!a) return;
    if (a->start_capture) rig->rig.capture_started++;
    if (a->stop_capture) rig->rig.capture_stopped++;
    if (a->present_draft) rig->rig.drafts_presented++;
    if (a->clear_draft) rig->rig.drafts_cleared++;
    if (a->play_haptic) {
        rig->rig.haptic_plans++;
        rig->rig.last_haptic_safe = haptic_plan_safe(&a->haptic) ? 1u : 0u;
    }
}

void interaction_rig_init(interaction_rig_t *rig, const interaction_config_t *cfg)
{
    if (!rig) return;
    memset(rig, 0, sizeof(*rig));
    interaction_init(&rig->runtime, cfg);
}

int interaction_rig_push(interaction_rig_t *rig, uint32_t now_ms)
{
    int rc;
    if (!rig) return INTERACTION_E_ARG;
    rc = interaction_push_to_talk(&rig->runtime, now_ms);
    apply_actions(rig);
    return rc;
}

int interaction_rig_transcript(interaction_rig_t *rig, const char *text, uint32_t now_ms)
{
    int rc;
    if (!rig) return INTERACTION_E_ARG;
    rc = interaction_transcript(&rig->runtime, text, now_ms);
    apply_actions(rig);
    return rc;
}

int interaction_rig_confirm(interaction_rig_t *rig, int accepted, uint32_t now_ms)
{
    int rc;
    if (!rig) return INTERACTION_E_ARG;
    rc = interaction_confirm(&rig->runtime, accepted, now_ms);
    apply_actions(rig);
    return rc;
}

int interaction_rig_tick(interaction_rig_t *rig, uint32_t now_ms)
{
    int rc;
    if (!rig) return INTERACTION_E_ARG;
    rc = interaction_tick(&rig->runtime, now_ms);
    apply_actions(rig);
    return rc;
}

void interaction_rig_set_source(interaction_rig_t *rig, int available, uint32_t now_ms)
{
    if (!rig) return;
    interaction_set_asr_available(&rig->runtime, available, now_ms);
    apply_actions(rig);
}

int interaction_rig_take_send(interaction_rig_t *rig, hcp_msg_t *out)
{
    int rc;
    if (!rig) return INTERACTION_E_ARG;
    rc = interaction_take_send(&rig->runtime, out);
    apply_actions(rig);
    if (rc == INTERACTION_OK) rig->rig.handoffs++;
    return rc;
}

void interaction_rig_note_energy_uj(interaction_rig_t *rig, uint32_t energy_uj)
{
    if (!rig) return;
    interaction_note_energy_uj(&rig->runtime, energy_uj);
    rig->rig.measured_energy_uj += energy_uj;
}

const interaction_rig_metrics_t *interaction_rig_metrics(const interaction_rig_t *rig)
{
    return rig ? &rig->rig : NULL;
}
