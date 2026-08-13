/* intent_gate.c — no-autonomy confidence and ambiguity gate for local ASR. */
#include "intent_gate.h"
#include <string.h>

static int command_valid(voice_command_t command, uint8_t minutes)
{
    if (command == VOICE_COMMAND_ARRIVE) return minutes <= 60u;
    if (command == VOICE_COMMAND_HELP || command == VOICE_COMMAND_CANCEL) return minutes == 0u;
    return 0;
}

static int hint_qualifies(const intent_context_hint_t *hint, voice_command_t command)
{
    return hint && hint->available == 1u && hint->command == command &&
           hint->support >= INTENT_GATE_CONTEXT_MIN_SUPPORT &&
           hint->confidence_pct >= INTENT_GATE_CONTEXT_MIN_CONF_PCT;
}

intent_gate_status_t intent_gate_evaluate(const intent_observation_t *obs,
                                          uint32_t active_session,
                                          const intent_context_hint_t *hint,
                                          intent_gate_result_t *out)
{
    uint8_t margin;
    if (out) memset(out, 0, sizeof(*out));
    if (!obs || !out || !active_session || obs->session_id != active_session) {
        if (out) out->status = INTENT_GATE_STALE;
        return INTENT_GATE_STALE;
    }
    if ((obs->source != INTENT_SOURCE_CORE && obs->source != INTENT_SOURCE_NUCLEUS) ||
        !command_valid(obs->command, obs->minutes) ||
        obs->confidence_pct > 100u || obs->runner_up_pct > 100u ||
        obs->runner_up_pct > obs->confidence_pct) {
        out->status = INTENT_GATE_REJECTED;
        return out->status;
    }
    if (obs->confidence_pct < INTENT_GATE_MIN_CONFIDENCE_PCT) {
        out->status = INTENT_GATE_LOW_CONFIDENCE;
        return out->status;
    }

    margin = (uint8_t)(obs->confidence_pct - obs->runner_up_pct);
    if (margin >= INTENT_GATE_MIN_MARGIN_PCT) {
        out->status = INTENT_GATE_ACCEPT_DIRECT;
        return out->status;
    }
    if (hint_qualifies(hint, obs->command)) {
        out->status = INTENT_GATE_ACCEPT_CONTEXT;
        out->context_used = 1u;
        return out->status;
    }
    out->status = INTENT_GATE_AMBIGUOUS;
    return out->status;
}
