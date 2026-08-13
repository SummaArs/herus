/* interaction.h — safe runtime between push-to-talk, local ASR, HCP and haptics.
 *
 * The runtime accepts events; it does not own a microphone, ASR engine, haptic
 * driver, radio, key or clock. It can therefore prove that no draft reaches the
 * link layer before physical confirmation, independent of the target platform.
 */
#ifndef HERUS_INTERACTION_H
#define HERUS_INTERACTION_H

#include <stdint.h>
#include "voice.h"
#include "intent_gate.h"

#define INTERACTION_DEFAULT_LISTEN_MS   6000u
#define INTERACTION_DEFAULT_CONFIRM_MS  8000u

typedef enum {
    INTERACTION_IDLE = 0,
    INTERACTION_LISTENING,
    INTERACTION_AWAIT_CONFIRM,
    INTERACTION_READY_SEND,
    INTERACTION_CANCELLED,
    INTERACTION_REJECTED,
    INTERACTION_TIMED_OUT,
    INTERACTION_LINK_LOST
} interaction_state_t;

enum {
    INTERACTION_OK       =  0,
    INTERACTION_E_ARG    = -1,
    INTERACTION_E_STATE  = -2,
    INTERACTION_E_SOURCE = -3,
    INTERACTION_E_UNTRUSTED = -4
};

/* The portable runtime writes these desired effects after each event. A target
 * adapter may start/stop its AFE/ASR, render a draft, clear UX and apply haptics.
 * None of these fields gives an adapter permission to transmit. */
typedef struct {
    uint8_t start_capture;
    uint8_t stop_capture;
    uint8_t present_draft;
    uint8_t clear_draft;
    uint8_t play_haptic;
    haptic_plan_t haptic;
} interaction_actions_t;

/* Counters and measured energy only. No transcript, raw audio, semantic content,
 * key, address or location appears here, so this structure is safe to retain as
 * local diagnostic telemetry. */
typedef struct {
    uint32_t sessions;
    uint32_t drafts;
    uint32_t confirms;
    uint32_t sent;
    uint32_t cancelled;
    uint32_t unknown;
    uint32_t rejected;
    uint32_t timed_out;
    uint32_t source_lost;
    uint32_t asr_stale;
    uint32_t asr_low_confidence;
    uint32_t asr_ambiguous;
    uint32_t asr_context_assisted;
    uint32_t last_latency_ms;     /* push-to-talk -> draft or terminal result */
    uint64_t measured_energy_uj;  /* caller-supplied PMIC/bench measurement only */
} interaction_metrics_t;

typedef struct {
    voice_lexicon_t lexicon;
    uint32_t listen_timeout_ms;
    uint32_t confirm_timeout_ms;
    uint8_t  allow_test_transcript; /* host-only parser path; production defaults to 0 */
} interaction_config_t;

typedef struct {
    interaction_config_t cfg;
    interaction_state_t  state;
    uint8_t              asr_available; /* local Core or trusted local Nucleus */
    uint32_t             session_id;     /* nonzero, advances only on physical PTT */
    uint32_t             listen_started_ms;
    uint32_t             confirm_started_ms;
    hcp_msg_t            pending;       /* zero unless confirmation is pending/ready */
    voice_result_t       parsed;        /* current local result, never serialized */
    interaction_actions_t actions;
    interaction_metrics_t metrics;
} interaction_t;

void interaction_config_default(interaction_config_t *out);
void interaction_init(interaction_t *it, const interaction_config_t *cfg);

/* Report availability of the local ASR source. If it disappears during a live
 * session, the runtime stops capture, wipes the draft and enters LINK_LOST. */
void interaction_set_asr_available(interaction_t *it, int available, uint32_t now_ms);

/* Physical button press is the sole transition that starts capture. */
int interaction_push_to_talk(interaction_t *it, uint32_t now_ms);

/* Feed a local ASR transcript only while LISTENING. This is disabled by default
 * and exists only for host tests until a typed local-ASR adapter is wired. */
int interaction_transcript(interaction_t *it, const char *text, uint32_t now_ms);

/* Feed a typed observation from an on-Core or local-Nucleus ASR adapter. The
 * gateway binds it to the active push-to-talk session, fails closed on low
 * confidence/ambiguity, and cannot bypass physical confirmation. */
int interaction_asr_result(interaction_t *it, const intent_observation_t *obs,
                           const intent_context_hint_t *hint, uint32_t now_ms);

/* Return the active/recent physical PTT session id for adapter tagging. */
uint32_t interaction_session_id(const interaction_t *it);

/* The physical confirmation handler may release a pending draft exactly once. */
int interaction_confirm(interaction_t *it, int accepted, uint32_t now_ms);

/* Move the one confirmed semantic draft to the application. The caller must set
 * seq/ttl/prio and call link_send separately. On success the runtime returns IDLE
 * immediately, so a second call cannot send the same meaning again. */
int interaction_take_send(interaction_t *it, hcp_msg_t *out);

/* Enforce listen and confirmation deadlines using caller-supplied monotonic ms. */
int interaction_tick(interaction_t *it, uint32_t now_ms);

/* Register measured—not estimated—energy from a PMIC or bench instrument. */
void interaction_note_energy_uj(interaction_t *it, uint32_t energy_uj);

const interaction_actions_t *interaction_actions(const interaction_t *it);
const interaction_metrics_t *interaction_metrics(const interaction_t *it);

#endif /* HERUS_INTERACTION_H */
