/* test_memory_extract.c — typed, conservative memory-candidate invariants. */
#include "memory_extract.h"
#include <stdio.h>
#include <string.h>

static int FAILED = 0;

static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

static memory_capture_t live_capture(uint32_t gesture, uint32_t now)
{
    memory_capture_t c;
    memory_capture_config_t cfg;
    memory_capture_config_default(&cfg);
    cfg.window_ms = 1000u;
    memory_capture_init(&c, &cfg, 0);
    (void)memory_capture_begin(&c, gesture, now);
    return c;
}

int main(void)
{
    memory_extract_t e;
    memory_capture_t c;
    memory_candidate_t candidate;
    memory_candidate_t saved;
    memory_assessment_t assessment;
    char idea[] = "lembre esta ideia: nucleo como segundo cerebro";
    const char *decision = "decidimos: vamos priorizar memoria local";
    const char *other = "lembre fato de projeto: saude de outra pessoa";
    const char *plain = "o tempo esta bonito hoje";
    uint32_t sid;

    printf("\n== M3 candidate extraction is typed, uncertain and non-retaining ==\n");
    memory_extract_init(&e);
    c = live_capture(10u, 100u);
    sid = memory_capture_session_id(&c);

    ok(memory_extract_text(&e, &c, 0u, idea, strlen(idea), 95u, &candidate) ==
       MEMORY_EXTRACT_E_SESSION && candidate.origin == MEMORY_EXTRACT_ORIGIN_NONE,
       "M3 no candidate is produced without a live capture-session identifier");

    ok(memory_extract_text(&e, &c, sid, plain, strlen(plain), 95u, &candidate) ==
       MEMORY_EXTRACT_NO_CANDIDATE && candidate.origin == MEMORY_EXTRACT_ORIGIN_NONE &&
       candidate.signal.kind == MEMORY_KIND_NONE,
       "M3 free conversation outside the controlled grammar is discarded");

    ok(memory_extract_text(&e, &c, sid, idea, strlen(idea), 95u, &candidate) ==
       MEMORY_EXTRACT_OK && candidate.origin == MEMORY_EXTRACT_EXPLICIT &&
       candidate.signal.explicit_remember == 1u && candidate.signal.kind == MEMORY_KIND_IDEA &&
       candidate.signal.scope == MEMORY_SCOPE_SELF &&
       candidate.signal.sensitivity == MEMORY_SENSITIVITY_ORDINARY &&
       (candidate.reasons & MEMORY_EXTRACT_REASON_EXPLICIT) &&
       (candidate.reasons & MEMORY_EXTRACT_REASON_IDEA),
       "M3 explicit own idea becomes a typed candidate without carrying its text");
    saved = candidate;
    memset(idea, 'x', strlen(idea));
    ok(memcmp(&candidate, &saved, sizeof(candidate)) == 0,
       "M3 changing the caller text after extraction cannot alter retained extractor state");
    ok(memory_extract_assess(&candidate, &assessment) == MEMORY_POLICY_OK &&
       assessment.disposition == MEMORY_DISPOSITION_AUTO_ELIGIBLE,
       "M3 policy may make an ordinary explicit candidate eligible but does not persist it");

    ok(memory_extract_text(&e, &c, sid, decision, strlen(decision), 88u, &candidate) ==
       MEMORY_EXTRACT_OK && candidate.origin == MEMORY_EXTRACT_CONTROLLED_INFERENCE &&
       candidate.signal.kind == MEMORY_KIND_DECISION &&
       (candidate.reasons & MEMORY_EXTRACT_REASON_DECISION),
       "M3 controlled decision syntax produces an uncertain typed interpretation, not a fact");

    ok(memory_extract_text(&e, &c, sid, other, strlen(other), 90u, &candidate) ==
       MEMORY_EXTRACT_OK && candidate.signal.scope == MEMORY_SCOPE_THIRD_PARTY &&
       candidate.signal.sensitivity == MEMORY_SENSITIVITY_SENSITIVE &&
       (candidate.reasons & MEMORY_EXTRACT_REASON_THIRD_PARTY) &&
       (candidate.reasons & MEMORY_EXTRACT_REASON_SENSITIVE) &&
       memory_extract_assess(&candidate, &assessment) == MEMORY_POLICY_OK &&
       assessment.disposition == MEMORY_DISPOSITION_REVIEW,
       "M3 sensitive information about another person is marked and forced to review");

    ok(memory_extract_text(&e, &c, sid, decision, strlen(decision), 50u, &candidate) ==
       MEMORY_EXTRACT_OK && (candidate.reasons & MEMORY_EXTRACT_REASON_AMBIGUOUS) &&
       memory_extract_assess(&candidate, &assessment) == MEMORY_POLICY_OK &&
       assessment.disposition == MEMORY_DISPOSITION_DISCARD,
       "M3 a low-confidence local-ASR observation cannot become memory by grammar alone");

    /* A bounded non-NUL fixture makes the max-length check independent of C string
     * termination and demonstrates that the extractor uses its supplied length. */
    {
        char too_long[MEMORY_EXTRACT_TEXT_MAX + 1u];
        memset(too_long, 'a', sizeof(too_long));
        ok(memory_extract_text(&e, &c, sid, too_long, sizeof(too_long), 90u, &candidate) ==
           MEMORY_EXTRACT_E_LENGTH && candidate.signal.kind == MEMORY_KIND_NONE,
           "M3 oversized input is rejected before grammar or policy evaluation");
    }

    (void)memory_capture_cancel(&c, 10u, 200u);
    ok(memory_extract_text(&e, &c, sid, decision, strlen(decision), 90u, &candidate) ==
       MEMORY_EXTRACT_E_SESSION && candidate.signal.kind == MEMORY_KIND_NONE,
       "M3 an expired or cancelled capture cannot be reused for later extraction");

    ok(e.metrics.calls == 8u && e.metrics.candidates == 4u &&
       e.metrics.no_candidate == 1u && e.metrics.rejected_session == 2u &&
       e.metrics.low_confidence == 1u && e.metrics.sensitive_or_other == 1u,
       "M3 metrics are numeric-only and account for candidate, rejection and risk paths");

    if (FAILED) {
        printf("MEMORY EXTRACT TESTS FAILED\n");
        return 1;
    }
    printf("MEMORY EXTRACT INVARIANTS HOLD — grammar is conservative, output is typed and no text is retained.\n");
    return 0;
}
