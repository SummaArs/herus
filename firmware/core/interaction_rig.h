/* interaction_rig.h — deterministic host adapters for the HERUS interaction lab.
 *
 * This is not a hardware implementation. It applies the declarative actions
 * produced by interaction.[ch] to counters so host tests can prove adapter
 * sequencing before ESP-SR, GPIO, a motor driver or link_send are introduced.
 */
#ifndef HERUS_INTERACTION_RIG_H
#define HERUS_INTERACTION_RIG_H

#include "interaction.h"

typedef struct {
    uint32_t capture_started;
    uint32_t capture_stopped;
    uint32_t drafts_presented;
    uint32_t drafts_cleared;
    uint32_t haptic_plans;
    uint32_t handoffs;
    uint32_t measured_energy_uj;
    uint8_t  last_haptic_safe;
} interaction_rig_metrics_t;

typedef struct {
    interaction_t             runtime;
    interaction_rig_metrics_t rig;
} interaction_rig_t;

void interaction_rig_init(interaction_rig_t *rig, const interaction_config_t *cfg);
int interaction_rig_push(interaction_rig_t *rig, uint32_t now_ms);
int interaction_rig_transcript(interaction_rig_t *rig, const char *text, uint32_t now_ms);
int interaction_rig_confirm(interaction_rig_t *rig, int accepted, uint32_t now_ms);
int interaction_rig_tick(interaction_rig_t *rig, uint32_t now_ms);
void interaction_rig_set_source(interaction_rig_t *rig, int available, uint32_t now_ms);
/* Legacy raw handoff is fail-closed and always rejects. */
int interaction_rig_take_send(interaction_rig_t *rig, hcp_msg_t *out);
/* The lab adapter must receive the same nonsecret assurance snapshot used by
 * the production integration path. Assurance may only block the one-time
 * handoff; it cannot create a draft or authorize transport. */
int interaction_rig_take_send_assured(interaction_rig_t *rig,
                                      const assurance_snapshot_t *snapshot,
                                      hcp_msg_t *out);
void interaction_rig_note_energy_uj(interaction_rig_t *rig, uint32_t energy_uj);

const interaction_rig_metrics_t *interaction_rig_metrics(const interaction_rig_t *rig);

#endif /* HERUS_INTERACTION_RIG_H */
