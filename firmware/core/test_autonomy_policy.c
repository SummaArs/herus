#include "autonomy_policy.h"
#include <stdio.h>
#include <string.h>

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static herus_autonomy_envelope_t envelope(herus_autonomy_level_t level,
                                          herus_action_scope_t scope)
{
    herus_autonomy_envelope_t out;
    memset(&out, 0, sizeof(out));
    out.level = level;
    out.scope = scope;
    out.proposal_id = 100u;
    out.confirmation_id = 200u;
    return out;
}

int main(void)
{
    score_t score = { 0, 0 };
    herus_autonomy_envelope_t e;

    e = envelope(HERUS_A0_SILENT, HERUS_SCOPE_NONE);
    check(&score, herus_policy_validate(&e) == HERUS_POLICY_OK &&
                    herus_policy_classify(&e) == HERUS_POLICY_SILENT,
          "A0 is a valid silent envelope");

    e = envelope(HERUS_A1_REACTIVE, HERUS_SCOPE_PRESENT);
    check(&score, herus_policy_classify(&e) == HERUS_POLICY_OK,
          "A1 may present an explicit reactive result");

    e = envelope(HERUS_A2_CONTEXTUAL, HERUS_SCOPE_PRESENT);
    e.proactive = 1u;
    e.attention_window = 1u;
    e.proactive_consent = 1u;
    check(&score, herus_policy_classify(&e) == HERUS_POLICY_OK,
          "A2 may present a consented contextual proposal");

    e.proactive_consent = 0u;
    check(&score, herus_policy_classify(&e) == HERUS_POLICY_SILENT,
          "A2 without proactive consent remains silent");

    e = envelope(HERUS_A3_PREPARATORY, HERUS_SCOPE_PREPARE);
    check(&score, herus_policy_classify(&e) == HERUS_POLICY_NEEDS_CONFIRMATION,
          "A3 preparation cannot proceed without exact confirmation");

    e = envelope(HERUS_A4_CONFIRMED, HERUS_SCOPE_PREPARE);
    e.explicit_confirmation = 1u;
    check(&score, herus_policy_classify(&e) == HERUS_POLICY_NEEDS_CONFIRMATION,
          "A4 still requires an unconsumed matching confirmation");
    check(&score, herus_policy_consume_confirmation(&e, 101u, 201u) ==
                    HERUS_POLICY_REVOKED && e.confirmation_consumed == 0u,
          "a mismatched confirmation cannot consume an untouched proposal");
    check(&score, herus_policy_consume_confirmation(&e, 100u, 200u) ==
                    HERUS_POLICY_OK && e.confirmation_consumed == 1u &&
                    e.confirmed_scope == HERUS_SCOPE_PREPARE &&
                    herus_policy_classify(&e) == HERUS_POLICY_OK,
          "matching confirmation promotes exactly one bounded proposal");
    e.scope = HERUS_SCOPE_TRANSMIT;
    check(&score, herus_policy_classify(&e) == HERUS_POLICY_REVOKED,
          "changing scope after confirmation revokes the old authority");
    e.scope = HERUS_SCOPE_PREPARE;
    check(&score, herus_policy_consume_confirmation(&e, 100u, 200u) ==
                    HERUS_POLICY_REVOKED,
          "the same confirmation cannot be reused");
    check(&score, herus_policy_consume_confirmation(&e, 101u, 201u) ==
                    HERUS_POLICY_REVOKED,
          "a confirmation for another proposal is rejected");

    e = envelope(HERUS_A2_CONTEXTUAL, HERUS_SCOPE_PRESENT);
    e.proactive = 1u;
    e.attention_window = 1u;
    e.proactive_consent = 1u;
    e.sensitive_context = 1u;
    check(&score, herus_policy_validate(&e) == HERUS_POLICY_REJECTED,
          "sensitive context is rejected by the primary validator");
    check(&score, herus_policy_classify(&e) == HERUS_POLICY_REJECTED,
          "sensitive contextual initiative is rejected");
    e.sensitive_context = 0u;
    e.third_party_context = 1u;
    check(&score, herus_policy_classify(&e) == HERUS_POLICY_REJECTED,
          "third-party contextual initiative is rejected");

    e = envelope(HERUS_A0_SILENT, HERUS_SCOPE_PRESENT);
    check(&score, herus_policy_validate(&e) == HERUS_POLICY_SCOPE,
          "silent level cannot carry an action scope");
    e = envelope(HERUS_A4_CONFIRMED, HERUS_SCOPE_NONE);
    check(&score, herus_policy_validate(&e) == HERUS_POLICY_SCOPE,
          "confirmed level cannot be detached from a scope");

    e = envelope(HERUS_A2_CONTEXTUAL, HERUS_SCOPE_PRESENT);
    e.proactive = 2u;
    check(&score, herus_policy_validate(&e) == HERUS_POLICY_FORMAT,
          "non-canonical policy bits fail closed");

    printf("AUTONOMY POLICY: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
