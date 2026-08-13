/* test_nucleus.c — executable privacy and behaviour contract for the Nucleus. */
#include "nucleus.h"
#include <stdio.h>
#include <string.h>

static int FAILED = 0;
static void ok(int cond, const char *what)
{
    printf("  %-4s %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond) FAILED = 1;
}

static hcp_msg_t glyph(uint16_t intent, uint8_t flags, uint16_t seq,
                       uint8_t ttl, uint8_t prio)
{
    hcp_msg_t m;
    memset(&m, 0, sizeof(m));
    m.ver = HCP_VERSION;
    m.tier = HCP_TIER_GLYPH;
    m.flags = flags;
    m.intent = intent;
    m.seq = seq;
    m.ttl = ttl;
    m.prio = prio;
    return m;
}

static void test_consent_and_transition(void)
{
    nucleus_t n;
    nucleus_suggestion_t out[NUC_SUGGESTION_CAP];
    hcp_msg_t a = glyph(10, 1, 41, 7, 3);
    hcp_msg_t b = glyph(20, 2, 42, 6, 2);
    hcp_msg_t c = glyph(30, 0, 43, 5, 1);

    printf("\n== N1  consent, semantic transition and explainable confidence ==\n");
    nucleus_init(&n);
    ok(nucleus_observe(&n, &a, 1) == NUC_DISABLED && nucleus_rule_count(&n) == 0,
       "N1 learning is off by default and stores no unconsented observation");

    nucleus_set_learning(&n, 1);
    /* Learn A -> B three times. The reverse B -> A is also a real observation,
     * but it must not pollute the A-context ranking. */
    ok(nucleus_observe(&n, &a, 2) == NUC_OK, "N1 first authorised meaning arms context only");
    nucleus_observe(&n, &b, 3);
    nucleus_observe(&n, &a, 4);
    nucleus_observe(&n, &b, 5);
    nucleus_observe(&n, &a, 6);
    nucleus_observe(&n, &b, 7);
    nucleus_observe(&n, &a, 8);
    nucleus_observe(&n, &c, 9); /* A -> C once: below suggestion threshold. */

    unsigned got = nucleus_suggest(&n, &a, 10, 20, out, NUC_SUGGESTION_CAP);
    ok(got == 1 && out[0].template.intent == b.intent &&
       out[0].support == 3 && out[0].observations == 4 &&
       out[0].confidence_pct == 75,
       "N1 only repeated transitions become ranked suggestions with explicit confidence");
    ok(out[0].template.seq == 0 && out[0].template.ttl == 0 &&
       out[0].template.prio == 0 && out[0].template.flags == b.flags,
       "N1 a suggestion preserves meaning but strips transport state before confirmation");

    unsigned again = nucleus_suggest(&n, &a, 10, 20, out, NUC_SUGGESTION_CAP);
    ok(again == got && out[0].support == 3 && nucleus_rule_count(&n) == 3,
       "N1 querying a prediction never trains, transmits or changes the learned state");
}

static void test_rejection_and_erasure(void)
{
    nucleus_t n;
    hcp_msg_t a = glyph(10, 0, 1, 1, 1);
    hcp_msg_t bad = glyph(11, 0, 2, 1, 1);

    printf("\n== N2  malformed input, self-loop suppression, expiry and erasure ==\n");
    nucleus_init(&n);
    nucleus_set_learning(&n, 1);
    bad.tier = HCP_TIER_VOICE;
    ok(nucleus_observe(&n, &bad, 1) == NUC_REJECTED && nucleus_rule_count(&n) == 0,
       "N2 non-semantic tiers are rejected before they can enter local memory");

    nucleus_observe(&n, &a, 2);
    nucleus_observe(&n, &a, 3);
    ok(nucleus_rule_count(&n) == 0,
       "N2 repeated identical meanings refresh context but never create self-suggestions");

    hcp_msg_t b = glyph(12, 0, 3, 1, 1);
    nucleus_observe(&n, &b, 4);
    ok(nucleus_rule_count(&n) == 1 && nucleus_expire(&n, 15, 10) == 1,
       "N2 stale knowledge has a deterministic expiry path");

    nucleus_observe(&n, &a, 20);
    nucleus_observe(&n, &b, 21);
    ok(nucleus_expire(&n, 21, 0) == 1 && nucleus_rule_count(&n) == 0,
       "N2 a zero retention policy erases even freshly learned context immediately");
    nucleus_forget(&n);
    ok(nucleus_rule_count(&n) == 0 && n.context == 0 && !n.learning_enabled,
       "N2 forget erases every learned template, pending context and consent state");
}

static void test_mobile_base_governor(void)
{
    nucleus_telemetry_t t = { 1, 95, -220, 0, 800, 1 };
    nucleus_governance_t g;

    printf("\n== N3  mobile-base governance stays advisory and conservative ==\n");
    nucleus_govern(&t, &g);
    ok(g.state == NUC_BASE_HEALTHY && g.relay_recommended &&
       g.charge_recommended && !g.owner_alert,
       "N3 healthy telemetry recommends relay and charge but does not actuate either");

    t.pdr_pct = 70;
    nucleus_govern(&t, &g);
    ok(g.state == NUC_BASE_REPOSITION && g.relay_recommended && g.owner_alert,
       "N3 weak delivery asks the owner to reposition the puck before changing protocol");

    t.pdr_pct = 100;
    t.battery_permille = 199;
    nucleus_govern(&t, &g);
    ok(g.state == NUC_BASE_SAVE_POWER && !g.relay_recommended && g.owner_alert,
       "N3 low battery suppresses relay recommendation rather than spending the last reserve");

    t.battery_permille = 800;
    t.core_link_ok = 0;
    nucleus_govern(&t, &g);
    ok(g.state == NUC_BASE_CORE_UNREACHABLE && !g.relay_recommended && g.owner_alert,
       "N3 a lost Core link never triggers autonomous traffic or charging control");
}

int main(void)
{
    test_consent_and_transition();
    test_rejection_and_erasure();
    test_mobile_base_governor();
    if (FAILED) {
        printf("NUCLEUS TESTS FAILED\n");
        return 1;
    }
    printf("NUCLEUS INVARIANTS HOLD — local intelligence is bounded, opt-in and non-autonomous.\n");
    return 0;
}
