/* test_semantic_ir_bridge.c — real parser/adapter convergence checks. */
#include "voice.h"
#include <stdio.h>
#include <string.h>

static int FAILED;

static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

static int same_semantic(const voice_result_t *a, const voice_result_t *b)
{
    if (!a || !b) return 0;
    if (a->status != b->status || a->event != b->event ||
        a->requires_confirmation != b->requires_confirmation ||
        a->minutes != b->minutes) return 0;
    if (a->draft.tier != b->draft.tier || a->draft.intent != b->draft.intent ||
        a->draft.nslot != b->draft.nslot) return 0;
    for (unsigned i = 0; i < a->draft.nslot; i++) {
        if (a->draft.slot[i].role != b->draft.slot[i].role ||
            a->draft.slot[i].filler != b->draft.slot[i].filler) return 0;
    }
    return 1;
}

static void test_text_and_typed_arrival(void)
{
    voice_lexicon_t lex;
    voice_result_t text;
    voice_result_t typed;
    voice_lexicon_default(&lex);

    ok(voice_parse_pt("Chego em dez minutos", &lex, &text) == VOICE_DRAFT &&
       voice_from_command(VOICE_COMMAND_ARRIVE, 10, &lex, &typed) == VOICE_DRAFT &&
       same_semantic(&text, &typed),
       "text arrival and typed arrival converge to the same semantic draft");

    ok(voice_parse_pt("estou chegando", &lex, &text) == VOICE_DRAFT &&
       voice_from_command(VOICE_COMMAND_ARRIVE, 0, &lex, &typed) == VOICE_DRAFT &&
       same_semantic(&text, &typed),
       "text arrival without duration and typed arrival without duration converge");
}

static void test_text_and_typed_control_events(void)
{
    voice_lexicon_t lex;
    voice_result_t text;
    voice_result_t typed;
    voice_lexicon_default(&lex);

    ok(voice_parse_pt("preciso de socorro", &lex, &text) == VOICE_DRAFT &&
       voice_from_command(VOICE_COMMAND_HELP, 0, &lex, &typed) == VOICE_DRAFT &&
       same_semantic(&text, &typed),
       "text help and typed help converge to the same private critical draft");

    ok(voice_parse_pt("cancelar", &lex, &text) == VOICE_CANCEL_LOCAL &&
       voice_from_command(VOICE_COMMAND_CANCEL, 0, &lex, &typed) == VOICE_CANCEL_LOCAL &&
       same_semantic(&text, &typed),
       "text cancellation and typed cancellation converge to the same local event");
}

static void test_rejection_is_not_convergence(void)
{
    voice_lexicon_t lex;
    voice_result_t result;
    voice_lexicon_default(&lex);

    ok(voice_parse_pt("toque uma musica", &lex, &result) == VOICE_UNKNOWN &&
       result.draft.intent == 0,
       "unknown text is rejected without a semantic command");
    ok(voice_parse_pt("chego em noventa minutos", &lex, &result) == VOICE_REJECTED &&
       result.draft.intent == 0,
       "out-of-domain duration is rejected without truncation");
    ok(voice_from_command(VOICE_COMMAND_HELP, 1, &lex, &result) == VOICE_REJECTED &&
       voice_from_command(VOICE_COMMAND_CANCEL, 1, &lex, &result) == VOICE_REJECTED,
       "typed control commands reject an unsupported duration");
}

int main(void)
{
    printf("\n== SIR1 real parser/adapter convergence ==\n");
    test_text_and_typed_arrival();
    test_text_and_typed_control_events();
    test_rejection_is_not_convergence();
    if (FAILED) {
        printf("SEMANTIC IR BRIDGE TESTS FAILED\n");
        return 1;
    }
    printf("SEMANTIC IR BRIDGE INVARIANTS HOLD\n");
    return 0;
}
