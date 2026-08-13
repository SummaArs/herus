/* test_memory_retrieval.c — controlled typed-retrieval invariants. */
#include "memory_retrieval.h"
#include <stdio.h>
#include <string.h>

static int FAILED = 0;

static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

static memory_vault_card_t card(uint32_t id, memory_kind_t kind, uint8_t explicit,
                                uint8_t confidence, uint8_t novelty,
                                uint8_t future, uint8_t consequence)
{
    memory_vault_card_t c;
    memset(&c, 0, sizeof(c));
    c.card_id = id;
    c.review_receipt_id = id + 1000u;
    c.signal.session_authorized = 1u;
    c.signal.explicit_remember = explicit;
    c.signal.kind = kind;
    c.signal.scope = MEMORY_SCOPE_SELF;
    c.signal.sensitivity = MEMORY_SENSITIVITY_ORDINARY;
    c.signal.confidence_pct = confidence;
    c.signal.novelty_pct = novelty;
    c.signal.future_value_pct = future;
    c.signal.consequence_pct = consequence;
    c.origin = MEMORY_EXTRACT_EXPLICIT;
    c.extract_reasons = MEMORY_EXTRACT_REASON_EXPLICIT |
                        (kind == MEMORY_KIND_DECISION ? MEMORY_EXTRACT_REASON_DECISION :
                         MEMORY_EXTRACT_REASON_IDEA);
    return c;
}

static memory_consolidation_access_t access(void)
{
    memory_consolidation_access_t a;
    a.physical_session_id = 77u;
    a.physical_confirmed = 1u;
    return a;
}

static memory_retrieval_query_t decision_query(uint8_t min_confidence)
{
    memory_retrieval_query_t q;
    memset(&q, 0, sizeof(q));
    q.preferred_kind = MEMORY_KIND_DECISION;
    q.preferred_origin = MEMORY_EXTRACT_EXPLICIT;
    q.require_explicit = 1u;
    q.minimum_confidence_pct = min_confidence;
    return q;
}

int main(void)
{
    memory_retrieval_t r;
    memory_retrieval_query_t q;
    memory_retrieval_result_t out;
    memory_consolidation_access_t a;
    memory_vault_card_t cards[3];
    memory_vault_card_t reversed[3];
    memory_vault_card_t too_many[MEMORY_RETRIEVAL_MAX_CARDS + 1u];
    size_t i;

    printf("\n== M6 controlled semantic retrieval is local, bounded and ambiguity-safe ==\n");
    memory_retrieval_init(&r);
    cards[0] = card(101u, MEMORY_KIND_DECISION, 1u, 96u, 96u, 96u, 96u);
    cards[1] = card(202u, MEMORY_KIND_DECISION, 1u, 72u, 55u, 55u, 55u);
    cards[2] = card(303u, MEMORY_KIND_IDEA, 1u, 93u, 90u, 90u, 90u);
    a = access();
    q = decision_query(90u);

    ok(memory_retrieval_query(&r, &a, &q, cards, 3u, &out) == MEMORY_RETRIEVAL_OK &&
       out.status == MEMORY_RETRIEVAL_MATCH && out.card_id == 101u &&
       out.kind == MEMORY_KIND_DECISION && out.origin == MEMORY_EXTRACT_EXPLICIT &&
       out.score_pct >= MEMORY_RETRIEVAL_MIN_SCORE &&
       (out.reasons & MEMORY_RETRIEVAL_REASON_KIND) &&
       (out.reasons & MEMORY_RETRIEVAL_REASON_EXPLICIT),
       "M6 a constrained typed query returns only its highest eligible card with match reasons");

    a.physical_confirmed = 0u;
    ok(memory_retrieval_query(&r, &a, &q, cards, 3u, &out) == MEMORY_RETRIEVAL_E_ACCESS &&
       out.card_id == 0u && out.status == MEMORY_RETRIEVAL_NO_MATCH,
       "M6 recovery refuses a non-canonical physical access assertion and exposes no card");
    a = access();

    memset(&q, 0, sizeof(q));
    ok(memory_retrieval_query(&r, &a, &q, cards, 3u, &out) == MEMORY_RETRIEVAL_E_QUERY &&
       out.card_id == 0u,
       "M6 an unbounded query cannot enumerate every card in memory");

    q = decision_query(0u);
    ok(memory_retrieval_query(&r, &a, &q, cards, 3u, &out) == MEMORY_RETRIEVAL_OK &&
       out.status == MEMORY_RETRIEVAL_AMBIGUOUS && out.card_id == 0u &&
       out.score_pct >= out.runner_up_score_pct &&
       (uint32_t)out.score_pct - (uint32_t)out.runner_up_score_pct <
       MEMORY_RETRIEVAL_MIN_MARGIN,
       "M6 close typed scores return ambiguity instead of silently selecting a memory");

    q = decision_query(90u);
    q.preferred_kind = MEMORY_KIND_ROUTINE;
    ok(memory_retrieval_query(&r, &a, &q, cards, 3u, &out) == MEMORY_RETRIEVAL_OK &&
       out.status == MEMORY_RETRIEVAL_NO_MATCH && out.card_id == 0u &&
       out.score_pct == 0u,
       "M6 a constrained query with no eligible source returns no match rather than a near guess");

    reversed[0] = cards[2];
    reversed[1] = cards[1];
    reversed[2] = cards[0];
    q = decision_query(90u);
    ok(memory_retrieval_query(&r, &a, &q, reversed, 3u, &out) == MEMORY_RETRIEVAL_OK &&
       out.status == MEMORY_RETRIEVAL_MATCH && out.card_id == 101u,
       "M6 a unique winner is independent of caller ordering and has no hidden recency preference");

    cards[1].signal.sensitivity = MEMORY_SENSITIVITY_SENSITIVE;
    ok(memory_retrieval_query(&r, &a, &q, cards, 3u, &out) == MEMORY_RETRIEVAL_E_SOURCE &&
       out.card_id == 0u,
       "M6 a sensitive or policy-incompatible source invalidates the set before ranking");
    cards[1] = card(202u, MEMORY_KIND_DECISION, 1u, 72u, 55u, 55u, 55u);
    cards[1].card_id = cards[0].card_id;
    ok(memory_retrieval_query(&r, &a, &q, cards, 3u, &out) == MEMORY_RETRIEVAL_E_SOURCE &&
       out.card_id == 0u,
       "M6 duplicate identifiers are rejected instead of producing ambiguous provenance");
    cards[1] = card(202u, MEMORY_KIND_DECISION, 1u, 72u, 55u, 55u, 55u);

    for (i = 0u; i < sizeof(too_many) / sizeof(too_many[0]); ++i)
        too_many[i] = card((uint32_t)(500u + i), MEMORY_KIND_DECISION, 1u,
                            95u, 90u, 90u, 90u);
    ok(memory_retrieval_query(&r, &a, &q, too_many,
                              sizeof(too_many) / sizeof(too_many[0]), &out) ==
       MEMORY_RETRIEVAL_E_CAPACITY && out.card_id == 0u,
       "M6 source cardinality is bounded before scoring, allocation or any external lookup");

    ok(memory_retrieval_metrics(&r)->matches == 2u &&
       memory_retrieval_metrics(&r)->ambiguous == 1u &&
       memory_retrieval_metrics(&r)->no_match == 1u &&
       memory_retrieval_metrics(&r)->rejected_access == 1u &&
       memory_retrieval_metrics(&r)->rejected_query == 1u &&
       memory_retrieval_metrics(&r)->rejected_source == 3u,
       "M6 numeric-only metrics account for match, ambiguity and rejected boundaries without card content");

    if (FAILED) {
        printf("MEMORY RETRIEVAL TESTS FAILED\n");
        return 1;
    }
    printf("MEMORY RETRIEVAL INVARIANTS HOLD — typed local matching is bounded, explainable and never autonomous.\n");
    return 0;
}
