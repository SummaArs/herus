/* test_memory_policy.c — selective-memory relevance invariants. */
#include "memory_policy.h"
#include <stdio.h>
#include <string.h>

static int FAILED = 0;

static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

static memory_signal_t ordinary_idea(void)
{
    memory_signal_t s;
    memset(&s, 0, sizeof(s));
    s.session_authorized = 1;
    s.kind = MEMORY_KIND_IDEA;
    s.scope = MEMORY_SCOPE_SELF;
    s.sensitivity = MEMORY_SENSITIVITY_ORDINARY;
    s.confidence_pct = 90;
    s.novelty_pct = 80;
    s.future_value_pct = 85;
    s.consequence_pct = 65;
    return s;
}

int main(void)
{
    memory_signal_t s;
    memory_assessment_t a;

    printf("\n== M1  selective-memory relevance is consented, conservative and reversible ==\n");

    s = ordinary_idea();
    s.session_authorized = 0;
    ok(memory_policy_assess(&s, &a) == MEMORY_POLICY_E_NOT_AUTHORIZED &&
       a.disposition == MEMORY_DISPOSITION_DISCARD &&
       (a.reasons & MEMORY_REASON_NOT_AUTHORIZED) && a.relevance_score == 0,
       "M1 unconsented speech is discarded before relevance is considered");

    s = ordinary_idea();
    s.confidence_pct = MEMORY_POLICY_MIN_CONFIDENCE_PCT - 1u;
    s.explicit_remember = 1;
    ok(memory_policy_assess(&s, &a) == MEMORY_POLICY_OK &&
       a.disposition == MEMORY_DISPOSITION_DISCARD &&
       (a.reasons & MEMORY_REASON_LOW_CONFIDENCE) &&
       (a.reasons & MEMORY_REASON_AMBIGUOUS),
       "M1 an ambiguous extraction cannot be retained merely because it sounds important");

    s = ordinary_idea();
    s.novelty_pct = 5; s.future_value_pct = 5; s.consequence_pct = 5;
    ok(memory_policy_assess(&s, &a) == MEMORY_POLICY_OK &&
       a.disposition == MEMORY_DISPOSITION_DISCARD &&
       (a.reasons & MEMORY_REASON_LOW_RELEVANCE),
       "M1 ordinary low-value talk is intentionally forgotten");

    s = ordinary_idea();
    s.novelty_pct = 65; s.future_value_pct = 70; s.consequence_pct = 55;
    ok(memory_policy_assess(&s, &a) == MEMORY_POLICY_OK &&
       a.disposition == MEMORY_DISPOSITION_REVIEW &&
       a.relevance_score >= MEMORY_POLICY_REVIEW_SCORE &&
       a.relevance_score < MEMORY_POLICY_AUTO_SCORE,
       "M1 plausible but non-obvious value becomes a review candidate, not an automatic memory");

    s = ordinary_idea();
    s.kind = MEMORY_KIND_DECISION;
    ok(memory_policy_assess(&s, &a) == MEMORY_POLICY_OK &&
       a.disposition == MEMORY_DISPOSITION_AUTO_ELIGIBLE &&
       (a.reasons & MEMORY_REASON_DECISIONAL) &&
       a.relevance_score >= MEMORY_POLICY_AUTO_SCORE,
       "M1 confident, ordinary self decisions may be eligible for reversible consolidation");

    s = ordinary_idea();
    s.explicit_remember = 1;
    s.novelty_pct = 10; s.future_value_pct = 10; s.consequence_pct = 10;
    ok(memory_policy_assess(&s, &a) == MEMORY_POLICY_OK &&
       a.disposition == MEMORY_DISPOSITION_AUTO_ELIGIBLE &&
       (a.reasons & MEMORY_REASON_EXPLICIT),
       "M1 an explicit self-memory request is eligible even when automatic relevance is weak");

    s = ordinary_idea();
    s.explicit_remember = 1;
    s.sensitivity = MEMORY_SENSITIVITY_SENSITIVE;
    ok(memory_policy_assess(&s, &a) == MEMORY_POLICY_OK &&
       a.disposition == MEMORY_DISPOSITION_REVIEW &&
       (a.reasons & MEMORY_REASON_SENSITIVE),
       "M1 sensitive personal material is never auto-retained");

    s = ordinary_idea();
    s.explicit_remember = 1;
    s.scope = MEMORY_SCOPE_THIRD_PARTY;
    ok(memory_policy_assess(&s, &a) == MEMORY_POLICY_OK &&
       a.disposition == MEMORY_DISPOSITION_REVIEW &&
       (a.reasons & MEMORY_REASON_THIRD_PARTY),
       "M1 another person’s information is never auto-retained");

    s = ordinary_idea();
    s.kind = MEMORY_KIND_NONE;
    ok(memory_policy_assess(&s, &a) == MEMORY_POLICY_E_FORMAT &&
       a.disposition == MEMORY_DISPOSITION_DISCARD && a.reasons == 0,
       "M1 untyped candidate data is rejected without a partial decision");

    if (FAILED) {
        printf("MEMORY POLICY TESTS FAILED\n");
        return 1;
    }
    printf("MEMORY POLICY INVARIANTS HOLD — relevance is consented, conservative and cannot persist by itself.\n");
    return 0;
}
