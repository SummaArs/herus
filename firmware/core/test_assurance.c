/* test_assurance.c — executable Grand Finale policy contract. */
#include "assurance.h"
#include <stdio.h>
#include <string.h>

static int FAILED = 0;
static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

static assurance_snapshot_t safe_core(void)
{
    assurance_snapshot_t s;
    memset(&s, 0, sizeof(s));
    s.source = ASSURANCE_SOURCE_CORE;
    s.physical_session_current = 1;
    s.intent_accepted = 1;
    s.physical_confirmation = 1;
    s.handoff_unused = 1;
    return s;
}

int main(void)
{
    assurance_snapshot_t s;
    assurance_decision_t d;

    printf("\n== A10.1  Grand Finale assurance composition ==\n");
    s = safe_core();
    ok(assurance_decide(&s, &d) == ASSURANCE_OK && d.handoff_permitted &&
       d.failures == ASSURANCE_FAIL_NONE,
       "A10.1 a direct-Core handoff is permitted only after all existing local gates agree");

    s = safe_core();
    s.physical_session_current = 0;
    s.intent_accepted = 0;
    s.physical_confirmation = 0;
    s.handoff_unused = 0;
    ok(assurance_decide(&s, &d) == ASSURANCE_E_BLOCKED && !d.handoff_permitted &&
       (d.failures & ASSURANCE_FAIL_PHYSICAL) && (d.failures & ASSURANCE_FAIL_INTENT) &&
       (d.failures & ASSURANCE_FAIL_CONFIRM) && (d.failures & ASSURANCE_FAIL_HANDOFF),
       "A10.1 missing PTT, accepted intent, confirmation or unused handoff denies by default");

    s = safe_core();
    s.source = ASSURANCE_SOURCE_NUCLEUS;
    s.trust_active = 1;
    s.control_link_authenticated = 1;
    s.control_link_fresh = 1;
    ok(assurance_decide(&s, &d) == ASSURANCE_OK && d.handoff_permitted,
       "A10.1 a Nucleus result needs an active, authenticated and fresh companion path");

    s.trust_revoked = 1;
    ok(assurance_decide(&s, &d) == ASSURANCE_E_BLOCKED &&
       (d.failures & ASSURANCE_FAIL_REVOKED) && !d.handoff_permitted,
       "A10.1 revocation dominates an otherwise valid companion path");

    s = safe_core();
    s.local_model_enabled = 1;
    ok(assurance_decide(&s, &d) == ASSURANCE_E_BLOCKED &&
       (d.failures & ASSURANCE_FAIL_MODEL) && (d.failures & ASSURANCE_FAIL_AGENCY),
       "A10.1 an enabled model needs A9 acceptance and an explicit display-only boundary");

    s.local_model_accepted = 1;
    s.model_reply_display_only = 1;
    ok(assurance_decide(&s, &d) == ASSURANCE_OK && d.handoff_permitted,
       "A10.1 accepted local-model evidence cannot add authority beyond the existing handoff gate");

    s.model_reply_display_only = 2;
    ok(assurance_decide(&s, &d) == ASSURANCE_E_BLOCKED &&
       (d.failures & ASSURANCE_FAIL_AGENCY),
       "A10.1 noncanonical state values are treated as unsafe, never truthy");

    if (FAILED) {
        printf("ASSURANCE TESTS FAILED\n");
        return 1;
    }
    printf("ASSURANCE INVARIANTS HOLD — composition remains physical, confirmed, revocable and non-autonomous.\n");
    return 0;
}
