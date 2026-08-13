/* test_intent_gate.c — executable contract for Advance 5 confidence gateway. */
#include "intent_gate.h"
#include <stdio.h>

static int FAILED = 0;
static void ok(int cond, const char *what)
{
    printf("  %-4s %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond) FAILED = 1;
}

static intent_observation_t observation(uint32_t session, voice_command_t command,
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

int main(void)
{
    intent_observation_t o;
    intent_context_hint_t hint;
    intent_gate_result_t out;

    printf("\n== G1  session-bound confidence and bounded context ==\n");
    o = observation(7, VOICE_COMMAND_ARRIVE, 90, 20);
    ok(intent_gate_evaluate(&o, 7, NULL, &out) == INTENT_GATE_ACCEPT_DIRECT && !out.context_used,
       "G1 a high-confidence, wide-margin local command is accepted directly");

    hint.available = 1; hint.command = VOICE_COMMAND_ARRIVE;
    hint.support = 9; hint.confidence_pct = 95;
    o = observation(7, VOICE_COMMAND_ARRIVE, 79, 0);
    ok(intent_gate_evaluate(&o, 7, &hint, &out) == INTENT_GATE_LOW_CONFIDENCE,
       "G1 context cannot promote a command below the confidence floor");

    o = observation(7, VOICE_COMMAND_ARRIVE, 86, 75);
    ok(intent_gate_evaluate(&o, 7, &hint, &out) == INTENT_GATE_ACCEPT_CONTEXT && out.context_used,
       "G1 qualified local context can only disambiguate the same strong command");

    hint.command = VOICE_COMMAND_HELP;
    ok(intent_gate_evaluate(&o, 7, &hint, &out) == INTENT_GATE_AMBIGUOUS && !out.context_used,
       "G1 a different context hint cannot replace the ASR primary command");

    o = observation(6, VOICE_COMMAND_HELP, 100, 0);
    ok(intent_gate_evaluate(&o, 7, NULL, &out) == INTENT_GATE_STALE,
       "G1 a stale session result is ignored before command or confidence is trusted");

    o = observation(7, VOICE_COMMAND_HELP, 95, 0);
    o.minutes = 1;
    ok(intent_gate_evaluate(&o, 7, NULL, &out) == INTENT_GATE_REJECTED,
       "G1 malformed command parameters fail closed");

    if (FAILED) {
        printf("INTENT GATE TESTS FAILED\n");
        return 1;
    }
    printf("INTENT GATE INVARIANTS HOLD — confidence, context and session never authorize a send.\n");
    return 0;
}
