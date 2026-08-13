/* interaction.c — portable event runtime; deliberately no driver or transport calls. */
#include "interaction.h"
#include <string.h>

static uint32_t elapsed(uint32_t now, uint32_t then)
{
    return (uint32_t)(now - then); /* wrap-safe for intervals below 2^31 ms */
}

static void actions_reset(interaction_t *it)
{
    memset(&it->actions, 0, sizeof(it->actions));
}

static void pending_clear(interaction_t *it)
{
    memset(&it->pending, 0, sizeof(it->pending));
    memset(&it->parsed, 0, sizeof(it->parsed));
}

static void haptic(interaction_t *it, voice_event_t event)
{
    voice_haptic_plan(event, &it->actions.haptic);
    it->actions.play_haptic = haptic_plan_safe(&it->actions.haptic) ? 1u : 0u;
}

static void enter_link_lost(interaction_t *it, uint32_t now_ms)
{
    if (it->state == INTERACTION_LISTENING) it->actions.stop_capture = 1;
    if (it->state == INTERACTION_AWAIT_CONFIRM || it->state == INTERACTION_READY_SEND)
        it->actions.clear_draft = 1;
    pending_clear(it);
    it->state = INTERACTION_LINK_LOST;
    it->metrics.source_lost++;
    it->metrics.last_latency_ms = elapsed(now_ms, it->listen_started_ms);
    haptic(it, VOICE_EVENT_REJECTED);
}

/* The only conversion from an accepted parser/command output to runtime state.
 * It deliberately stops at AWAIT_CONFIRM; no ASR result reaches READY_SEND. */
static int apply_voice_result(interaction_t *it, voice_status_t status, uint32_t now_ms)
{
    it->actions.stop_capture = 1;
    it->metrics.last_latency_ms = elapsed(now_ms, it->listen_started_ms);
    if (status == VOICE_DRAFT) {
        it->pending = it->parsed.draft;
        it->state = INTERACTION_AWAIT_CONFIRM;
        it->confirm_started_ms = now_ms;
        it->metrics.drafts++;
        it->actions.present_draft = 1;
        haptic(it, it->parsed.event);
        return INTERACTION_OK;
    }

    it->actions.clear_draft = 1;
    pending_clear(it);
    if (status == VOICE_CANCEL_LOCAL) {
        it->state = INTERACTION_CANCELLED;
        it->metrics.cancelled++;
        haptic(it, VOICE_EVENT_CANCEL);
    } else if (status == VOICE_UNKNOWN) {
        it->state = INTERACTION_REJECTED;
        it->metrics.unknown++;
        haptic(it, VOICE_EVENT_UNKNOWN);
    } else {
        it->state = INTERACTION_REJECTED;
        it->metrics.rejected++;
        haptic(it, VOICE_EVENT_REJECTED);
    }
    return INTERACTION_OK;
}

static int reject_asr_result(interaction_t *it, intent_gate_status_t status, uint32_t now_ms)
{
    it->actions.stop_capture = 1;
    it->actions.clear_draft = 1;
    pending_clear(it);
    it->state = INTERACTION_REJECTED;
    it->metrics.last_latency_ms = elapsed(now_ms, it->listen_started_ms);
    if (status == INTENT_GATE_LOW_CONFIDENCE) {
        it->metrics.asr_low_confidence++;
        it->metrics.unknown++;
        haptic(it, VOICE_EVENT_UNKNOWN);
    } else if (status == INTENT_GATE_AMBIGUOUS) {
        it->metrics.asr_ambiguous++;
        it->metrics.unknown++;
        haptic(it, VOICE_EVENT_UNKNOWN);
    } else {
        it->metrics.rejected++;
        haptic(it, VOICE_EVENT_REJECTED);
    }
    return INTERACTION_OK;
}

void interaction_config_default(interaction_config_t *out)
{
    if (!out) return;
    voice_lexicon_default(&out->lexicon);
    out->listen_timeout_ms = INTERACTION_DEFAULT_LISTEN_MS;
    out->confirm_timeout_ms = INTERACTION_DEFAULT_CONFIRM_MS;
    out->allow_test_transcript = 0;
}

void interaction_init(interaction_t *it, const interaction_config_t *cfg)
{
    interaction_config_t local;
    if (!it) return;
    if (!cfg) {
        interaction_config_default(&local);
        cfg = &local;
    }
    memset(it, 0, sizeof(*it));
    it->cfg = *cfg;
    if (!it->cfg.listen_timeout_ms) it->cfg.listen_timeout_ms = INTERACTION_DEFAULT_LISTEN_MS;
    if (!it->cfg.confirm_timeout_ms) it->cfg.confirm_timeout_ms = INTERACTION_DEFAULT_CONFIRM_MS;
    it->state = INTERACTION_IDLE;
    it->asr_available = 1; /* host and an on-Core ASR are usable until told otherwise */
}

void interaction_set_asr_available(interaction_t *it, int available, uint32_t now_ms)
{
    if (!it) return;
    actions_reset(it);
    it->asr_available = available ? 1u : 0u;
    if (!it->asr_available && (it->state == INTERACTION_LISTENING ||
                               it->state == INTERACTION_AWAIT_CONFIRM ||
                               it->state == INTERACTION_READY_SEND))
        enter_link_lost(it, now_ms);
}

int interaction_push_to_talk(interaction_t *it, uint32_t now_ms)
{
    if (!it) return INTERACTION_E_ARG;
    actions_reset(it);
    if (it->state == INTERACTION_LISTENING || it->state == INTERACTION_AWAIT_CONFIRM ||
        it->state == INTERACTION_READY_SEND) return INTERACTION_E_STATE;
    if (!it->asr_available) {
        enter_link_lost(it, now_ms);
        return INTERACTION_E_SOURCE;
    }
    pending_clear(it);
    it->session_id++;
    if (!it->session_id) it->session_id++; /* reserve zero as invalid */
    it->state = INTERACTION_LISTENING;
    it->listen_started_ms = now_ms;
    it->metrics.sessions++;
    it->actions.start_capture = 1;
    return INTERACTION_OK;
}

int interaction_transcript(interaction_t *it, const char *text, uint32_t now_ms)
{
    voice_status_t status;
    if (!it || !text) return INTERACTION_E_ARG;
    if (it->state != INTERACTION_LISTENING) return INTERACTION_E_STATE;
    if (!it->cfg.allow_test_transcript) return INTERACTION_E_UNTRUSTED;

    actions_reset(it);
    status = voice_parse_pt(text, &it->cfg.lexicon, &it->parsed);
    return apply_voice_result(it, status, now_ms);
}

int interaction_asr_result(interaction_t *it, const intent_observation_t *obs,
                           const intent_context_hint_t *hint, uint32_t now_ms)
{
    intent_gate_result_t gate;
    voice_status_t status;
    if (!it || !obs) return INTERACTION_E_ARG;
    if (it->state != INTERACTION_LISTENING) return INTERACTION_E_STATE;

    actions_reset(it);
    (void)intent_gate_evaluate(obs, it->session_id, hint, &gate);
    if (gate.status == INTENT_GATE_STALE) {
        it->metrics.asr_stale++;
        return INTERACTION_OK; /* leave active capture and UX untouched */
    }
    if (gate.status != INTENT_GATE_ACCEPT_DIRECT && gate.status != INTENT_GATE_ACCEPT_CONTEXT)
        return reject_asr_result(it, gate.status, now_ms);

    status = voice_from_command(obs->command, obs->minutes, &it->cfg.lexicon, &it->parsed);
    if (status != VOICE_DRAFT && status != VOICE_CANCEL_LOCAL)
        return reject_asr_result(it, INTENT_GATE_REJECTED, now_ms);
    if (gate.status == INTENT_GATE_ACCEPT_CONTEXT) it->metrics.asr_context_assisted++;
    return apply_voice_result(it, status, now_ms);
}

uint32_t interaction_session_id(const interaction_t *it)
{
    return it ? it->session_id : 0u;
}

int interaction_confirm(interaction_t *it, int accepted, uint32_t now_ms)
{
    if (!it) return INTERACTION_E_ARG;
    if (it->state != INTERACTION_AWAIT_CONFIRM) return INTERACTION_E_STATE;
    actions_reset(it);
    if (!it->asr_available) {
        enter_link_lost(it, now_ms);
        return INTERACTION_E_SOURCE;
    }
    if (!accepted) {
        it->state = INTERACTION_CANCELLED;
        it->metrics.cancelled++;
        it->actions.clear_draft = 1;
        pending_clear(it);
        haptic(it, VOICE_EVENT_CANCEL);
        return INTERACTION_OK;
    }
    it->state = INTERACTION_READY_SEND;
    it->metrics.confirms++;
    haptic(it, VOICE_EVENT_DRAFT);
    return INTERACTION_OK;
}

int interaction_take_send(interaction_t *it, hcp_msg_t *out)
{
    if (!it || !out) return INTERACTION_E_ARG;
    if (it->state != INTERACTION_READY_SEND) return INTERACTION_E_STATE;
    actions_reset(it);
    *out = it->pending;
    pending_clear(it);
    it->state = INTERACTION_IDLE;
    it->metrics.sent++;
    it->actions.clear_draft = 1;
    return INTERACTION_OK;
}

int interaction_tick(interaction_t *it, uint32_t now_ms)
{
    if (!it) return INTERACTION_E_ARG;
    actions_reset(it);
    if (it->state == INTERACTION_LISTENING &&
        elapsed(now_ms, it->listen_started_ms) >= it->cfg.listen_timeout_ms) {
        it->state = INTERACTION_TIMED_OUT;
        it->metrics.timed_out++;
        it->metrics.last_latency_ms = elapsed(now_ms, it->listen_started_ms);
        it->actions.stop_capture = 1;
        it->actions.clear_draft = 1;
        pending_clear(it);
        haptic(it, VOICE_EVENT_REJECTED);
    } else if (it->state == INTERACTION_AWAIT_CONFIRM &&
               elapsed(now_ms, it->confirm_started_ms) >= it->cfg.confirm_timeout_ms) {
        it->state = INTERACTION_CANCELLED;
        it->metrics.cancelled++;
        it->actions.clear_draft = 1;
        pending_clear(it);
        haptic(it, VOICE_EVENT_CANCEL);
    }
    return INTERACTION_OK;
}

void interaction_note_energy_uj(interaction_t *it, uint32_t energy_uj)
{
    if (it) it->metrics.measured_energy_uj += energy_uj;
}

const interaction_actions_t *interaction_actions(const interaction_t *it)
{
    return it ? &it->actions : NULL;
}

const interaction_metrics_t *interaction_metrics(const interaction_t *it)
{
    return it ? &it->metrics : NULL;
}
