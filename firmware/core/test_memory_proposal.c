/* test_memory_proposal.c — executable contract for model-proposal compilation. */
#include "memory_proposal.h"
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

static memory_model_proposal_t valid_proposal(void)
{
    memory_model_proposal_t p;
    memset(&p, 0, sizeof(p));
    p.schema_version = MEMORY_PROPOSAL_SCHEMA_VERSION;
    p.kind = MEMORY_KIND_IDEA;
    p.scope = MEMORY_SCOPE_SELF;
    p.sensitivity = MEMORY_SENSITIVITY_ORDINARY;
    p.confidence_pct = 92u;
    p.novelty_pct = 80u;
    p.future_value_pct = 90u;
    p.consequence_pct = 60u;
    return p;
}

int main(void)
{
    memory_proposal_t compiler;
    memory_capture_t capture;
    memory_model_proposal_t proposal;
    memory_candidate_t candidate;
    memory_assessment_t assessment;
    uint32_t session;

    printf("\n== model proposal compiler is typed, versioned and fail-closed ==\n");
    memory_proposal_init(&compiler);
    capture = live_capture(17u, 100u);
    session = memory_capture_session_id(&capture);
    proposal = valid_proposal();

    ok(memory_proposal_compile(&compiler, &capture, 0u, &proposal, &candidate) ==
           MEMORY_PROPOSAL_E_SESSION &&
       candidate.origin == MEMORY_EXTRACT_ORIGIN_NONE &&
       candidate.signal.kind == MEMORY_KIND_NONE,
       "P1 proposal cannot compile without the active capture session");

    proposal.schema_version = MEMORY_PROPOSAL_SCHEMA_VERSION + 1u;
    ok(memory_proposal_compile(&compiler, &capture, session, &proposal, &candidate) ==
           MEMORY_PROPOSAL_E_SCHEMA &&
       candidate.signal.kind == MEMORY_KIND_NONE,
       "P1 unknown proposal schema is rejected before compilation");

    proposal = valid_proposal();
    proposal.abstain = 1u;
    ok(memory_proposal_compile(&compiler, &capture, session, &proposal, &candidate) ==
           MEMORY_PROPOSAL_NO_CANDIDATE &&
       candidate.origin == MEMORY_EXTRACT_ORIGIN_NONE &&
       candidate.signal.kind == MEMORY_KIND_NONE,
       "P1 explicit model abstention clears the candidate and creates no memory");

    proposal = valid_proposal();
    proposal.abstain = 2u;
    ok(memory_proposal_compile(&compiler, &capture, session, &proposal, &candidate) ==
           MEMORY_PROPOSAL_E_VALUE &&
       candidate.signal.kind == MEMORY_KIND_NONE,
       "P1 noncanonical booleans are rejected fail-closed");

    proposal = valid_proposal();
    proposal.kind = MEMORY_KIND_NONE;
    ok(memory_proposal_compile(&compiler, &capture, session, &proposal, &candidate) ==
           MEMORY_PROPOSAL_E_VALUE &&
       candidate.signal.kind == MEMORY_KIND_NONE,
       "P1 unsupported semantic kind is rejected without a partial candidate");

    proposal = valid_proposal();
    ok(memory_proposal_compile(&compiler, &capture, session, &proposal, &candidate) ==
           MEMORY_PROPOSAL_OK &&
       candidate.origin == MEMORY_EXTRACT_CONTROLLED_INFERENCE &&
       candidate.signal.kind == MEMORY_KIND_IDEA &&
       candidate.signal.session_authorized == 1u &&
       candidate.signal.explicit_remember == 0u &&
       candidate.signal.scope == MEMORY_SCOPE_SELF &&
       candidate.signal.sensitivity == MEMORY_SENSITIVITY_ORDINARY &&
       memory_extract_assess(&candidate, &assessment) == MEMORY_POLICY_OK &&
       assessment.disposition == MEMORY_DISPOSITION_REVIEW,
       "P1 valid model output compiles to an advisory candidate, never human confirmation");

    proposal = valid_proposal();
    proposal.scope = MEMORY_SCOPE_THIRD_PARTY;
    proposal.sensitivity = MEMORY_SENSITIVITY_SENSITIVE;
    ok(memory_proposal_compile(&compiler, &capture, session, &proposal, &candidate) ==
           MEMORY_PROPOSAL_OK &&
       (candidate.reasons & MEMORY_EXTRACT_REASON_THIRD_PARTY) != 0u &&
       (candidate.reasons & MEMORY_EXTRACT_REASON_SENSITIVE) != 0u &&
       memory_extract_assess(&candidate, &assessment) == MEMORY_POLICY_OK &&
       assessment.disposition == MEMORY_DISPOSITION_REVIEW,
       "P1 third-party or sensitive proposals remain review-only");

    proposal = valid_proposal();
    proposal.confidence_pct = 50u;
    ok(memory_proposal_compile(&compiler, &capture, session, &proposal, &candidate) ==
           MEMORY_PROPOSAL_OK &&
       (candidate.reasons & MEMORY_EXTRACT_REASON_AMBIGUOUS) != 0u &&
       memory_extract_assess(&candidate, &assessment) == MEMORY_POLICY_OK &&
       assessment.disposition == MEMORY_DISPOSITION_DISCARD,
       "P1 low-confidence proposals are preserved as uncertain and discarded by policy");

    (void)memory_capture_cancel(&capture, 17u, 200u);
    proposal = valid_proposal();
    ok(memory_proposal_compile(&compiler, &capture, session, &proposal, &candidate) ==
           MEMORY_PROPOSAL_E_SESSION &&
       candidate.signal.kind == MEMORY_KIND_NONE,
       "P1 cancelled sessions cannot be reused to compile a proposal");

    ok(compiler.metrics.calls == 9u && compiler.metrics.compiled == 3u &&
       compiler.metrics.abstained == 1u && compiler.metrics.rejected_session == 2u &&
       compiler.metrics.rejected_schema == 1u && compiler.metrics.rejected_value == 2u,
       "P1 metrics are numeric-only and account for compile, abstain and reject paths");

    if (FAILED) {
        printf("MEMORY PROPOSAL TESTS FAILED\n");
        return 1;
    }
    printf("MEMORY PROPOSAL INVARIANTS HOLD — model output is a bounded proposal, not authority.\n");
    return 0;
}
