/* test_memory_retrieval_present.c — controlled human presentation invariants. */
#include "memory_retrieval_present.h"
#include <stdio.h>
#include <string.h>

static int FAILED = 0;

static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

static memory_consolidation_access_t access(uint32_t session)
{
    memory_consolidation_access_t a;
    a.physical_session_id = session;
    a.physical_confirmed = 1u;
    return a;
}

static memory_retrieval_result_t match_result(void)
{
    memory_retrieval_result_t r;
    memset(&r, 0, sizeof(r));
    r.status = MEMORY_RETRIEVAL_MATCH;
    r.card_id = 999u;
    r.kind = MEMORY_KIND_DECISION;
    r.origin = MEMORY_EXTRACT_EXPLICIT;
    r.score_pct = 91u;
    r.runner_up_score_pct = 70u;
    r.reasons = MEMORY_RETRIEVAL_REASON_KIND |
                MEMORY_RETRIEVAL_REASON_ORIGIN |
                MEMORY_RETRIEVAL_REASON_EXPLICIT;
    return r;
}

static memory_retrieval_result_t no_match_result(void)
{
    memory_retrieval_result_t r;
    memset(&r, 0, sizeof(r));
    r.status = MEMORY_RETRIEVAL_NO_MATCH;
    r.score_pct = 55u;
    return r;
}

static memory_retrieval_result_t ambiguous_result(void)
{
    memory_retrieval_result_t r;
    memset(&r, 0, sizeof(r));
    r.status = MEMORY_RETRIEVAL_AMBIGUOUS;
    r.score_pct = 82u;
    r.runner_up_score_pct = 77u;
    return r;
}

int main(void)
{
    memory_retrieval_present_t p;
    memory_retrieval_presentation_t out;
    memory_consolidation_access_t a;
    memory_retrieval_result_t r;

    printf("\n== M7 retrieval presentation preserves uncertainty and human control ==\n");
    memory_retrieval_present_init(&p);
    a = access(41u);
    r = match_result();
    ok(memory_retrieval_present_show(&p, &a, &r, &out) == MEMORY_RETRIEVAL_PRESENT_OK &&
       p.state == MEMORY_RETRIEVAL_PRESENT_SHOWN &&
       out.phrase == MEMORY_RETRIEVAL_PHRASE_MATCH_AVAILABLE &&
       out.kind == MEMORY_KIND_DECISION && out.origin == MEMORY_EXTRACT_EXPLICIT &&
       out.reasons == r.reasons && haptic_plan_safe(&out.haptic),
       "M7 a unique match becomes one bounded symbolic status with only typed provenance");
    ok(memory_retrieval_present_show(&p, &a, &r, &out) == MEMORY_RETRIEVAL_PRESENT_E_STATE &&
       out.phrase == MEMORY_RETRIEVAL_PHRASE_NONE &&
       memory_retrieval_present_metrics(&p)->shown_match == 1u,
       "M7 presentation is one-shot and cannot repeat a retrieval result silently");
    ok(memory_retrieval_present_dismiss(&p, &(memory_consolidation_access_t){42u, 1u}) ==
       MEMORY_RETRIEVAL_PRESENT_E_ACCESS && p.state == MEMORY_RETRIEVAL_PRESENT_SHOWN,
       "M7 a different physical session cannot dismiss or reset a shown status");
    ok(memory_retrieval_present_dismiss(&p, &a) == MEMORY_RETRIEVAL_PRESENT_OK &&
       p.state == MEMORY_RETRIEVAL_PRESENT_IDLE && p.pending.phrase == MEMORY_RETRIEVAL_PHRASE_NONE &&
       p.active_physical_session_id == 0u,
       "M7 same-session dismissal scrubs transient presentation state without changing retrieval");

    r = no_match_result();
    ok(memory_retrieval_present_show(&p, &a, &r, &out) == MEMORY_RETRIEVAL_PRESENT_OK &&
       out.phrase == MEMORY_RETRIEVAL_PHRASE_NO_MATCH &&
       out.kind == MEMORY_KIND_NONE && out.origin == MEMORY_EXTRACT_ORIGIN_NONE &&
       out.reasons == 0u && haptic_plan_safe(&out.haptic),
       "M7 no-match is presented as absence under criteria and never leaks a near card");
    ok(memory_retrieval_present_dismiss(&p, &a) == MEMORY_RETRIEVAL_PRESENT_OK,
       "M7 no-match can be explicitly dismissed and returns to idle");

    r = ambiguous_result();
    ok(memory_retrieval_present_show(&p, &a, &r, &out) == MEMORY_RETRIEVAL_PRESENT_OK &&
       out.phrase == MEMORY_RETRIEVAL_PHRASE_AMBIGUOUS_REVIEW &&
       out.kind == MEMORY_KIND_NONE && out.origin == MEMORY_EXTRACT_ORIGIN_NONE &&
       out.reasons == 0u && haptic_plan_safe(&out.haptic),
       "M7 ambiguity becomes a review status without selecting or exposing either contender");
    ok(memory_retrieval_present_dismiss(&p, &a) == MEMORY_RETRIEVAL_PRESENT_OK,
       "M7 ambiguity does not create a follow-on action when dismissed");

    a.physical_confirmed = 0u;
    r = match_result();
    ok(memory_retrieval_present_show(&p, &a, &r, &out) == MEMORY_RETRIEVAL_PRESENT_E_ACCESS &&
       p.state == MEMORY_RETRIEVAL_PRESENT_IDLE && out.phrase == MEMORY_RETRIEVAL_PHRASE_NONE,
       "M7 non-canonical physical access produces no local status or retained presentation");
    a = access(41u);

    r = match_result();
    r.reasons = 0u;
    ok(memory_retrieval_present_show(&p, &a, &r, &out) == MEMORY_RETRIEVAL_PRESENT_E_RESULT &&
       p.state == MEMORY_RETRIEVAL_PRESENT_BLOCKED && p.pending.phrase == MEMORY_RETRIEVAL_PHRASE_NONE,
       "M7 malformed match result fails closed before any voice, haptic or visual effect");
    r = match_result();
    ok(memory_retrieval_present_show(&p, &a, &r, &out) == MEMORY_RETRIEVAL_PRESENT_E_STATE &&
       out.phrase == MEMORY_RETRIEVAL_PHRASE_NONE,
       "M7 blocked presenter cannot retry a result autonomously after malformed input");

    memory_retrieval_present_init(&p);
    r = no_match_result();
    r.card_id = 99u;
    ok(memory_retrieval_present_show(&p, &a, &r, &out) == MEMORY_RETRIEVAL_PRESENT_E_RESULT &&
       p.state == MEMORY_RETRIEVAL_PRESENT_BLOCKED,
       "M7 no-match carrying a hidden winner is rejected rather than exposed to the adapter");

    memory_retrieval_present_init(&p);
    r = ambiguous_result();
    r.kind = MEMORY_KIND_IDEA;
    ok(memory_retrieval_present_show(&p, &a, &r, &out) == MEMORY_RETRIEVAL_PRESENT_E_RESULT &&
       p.state == MEMORY_RETRIEVAL_PRESENT_BLOCKED,
       "M7 ambiguity carrying winner provenance is rejected rather than resolved by presentation");

    memory_retrieval_present_init(&p);
    r = match_result();
    ok(memory_retrieval_present_show(&p, &a, &r, &out) == MEMORY_RETRIEVAL_PRESENT_OK &&
       memory_retrieval_present_metrics(&p)->shown_match == 1u &&
       memory_retrieval_present_metrics(&p)->rejected_access == 0u &&
       memory_retrieval_present_metrics(&p)->rejected_result == 0u,
       "M7 numeric-only metrics count presentation state without card identifier or content");

    if (FAILED) {
        printf("MEMORY RETRIEVAL PRESENTATION TESTS FAILED\n");
        return 1;
    }
    printf("MEMORY RETRIEVAL PRESENTATION INVARIANTS HOLD — status is bounded, one-shot and has zero authority.\n");
    return 0;
}
