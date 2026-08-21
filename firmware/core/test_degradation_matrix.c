#include "magic_anticipation.h"
#include <stdio.h>
#include <string.h>

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static memory_vault_card_t card(uint32_t id, uint32_t receipt)
{
    memory_vault_card_t out;
    memset(&out, 0, sizeof(out));
    out.card_id = id;
    out.review_receipt_id = receipt;
    return out;
}

static int functional(sr_symbol_t predicate, void *user)
{
    (void)user;
    return predicate == SR_SYMBOL_LEGACY(20u);
}

static magic_context_t make_context(sr_pattern_t cue, magic_request_kind_t request,
                                    uint8_t consent)
{
    magic_context_t out;
    memset(&out, 0, sizeof(out));
    out.cue = cue;
    out.privacy_class = MAGIC_PRIVACY_ORDINARY;
    out.request_kind = request;
    out.attention_window = 1u;
    out.proactive_consent = consent;
    return out;
}

int main(void)
{
    score_t score = { 0, 0 };
    magic_policy_t policy;
    magic_proposal_t proposal;
    sr_reasoner_t base;
    sr_reasoner_t scratch;
    memory_vault_card_t reviewed = card(900u, 1900u);
    const sr_symbol_t fill_predicate = SR_SYMBOL_LEGACY(30u);
    const sr_symbol_t unknown_subject = 0x7f070001u;
    const sr_symbol_t unknown_object = 0x7f070002u;
    sr_pattern_t unknown_cue;
    magic_context_t context;
    mse_index_t memory;

    magic_policy_default(&policy);
    sr_init(&base);
    unknown_cue = (sr_pattern_t){SR_CONST(unknown_subject),
                                SR_CONST(fill_predicate),
                                SR_CONST(unknown_object), 0u};

    mse_init(&memory, NULL, NULL);
    {
        int filled = 1;
        for (uint16_t i = 0u; i < MSE_MAX_EVIDENCE; i++) {
            sr_fact_t fact = {
                0x70000000u + (sr_symbol_t)i,
                fill_predicate,
                0x71000000u + (sr_symbol_t)i,
                0u
            };
            if (mse_add(&memory, &reviewed, &fact, (uint32_t)i + 1u, 0u) != MSE_OK)
                filled = 0;
        }
        {
            const uint16_t before = memory.evidence_count;
            sr_fact_t overflow = {unknown_subject, fill_predicate, unknown_object, 0u};
            check(&score, filled && before == MSE_MAX_EVIDENCE &&
                            mse_add(&memory, &reviewed, &overflow, 100u, 0u) == MSE_E_FULL &&
                            memory.evidence_count == before,
                  "full semantic memory rejects a new fact without overwriting existing evidence");
        }
    }

    context = make_context(unknown_cue, MAGIC_REQUEST_CONTEXTUAL, 1u);
    check(&score, magic_propose(&base, &memory, 100u, &context, &policy,
                                &scratch, &proposal) == MAGIC_KNOWN_GAP &&
                    proposal.explanation_available == 1u &&
                    proposal.answer.kind != SR_ANSWER_DIRECT &&
                    proposal.answer.kind != SR_ANSWER_DERIVED,
          "a contextual query at capacity with no matching evidence becomes a known gap");

    context.proactive_consent = 0u;
    check(&score, magic_propose(&base, &memory, 100u, &context, &policy,
                                &scratch, &proposal) == MAGIC_SILENT &&
                    proposal.status == MAGIC_SILENT,
          "revoked proactive consent stays silent even while memory is full and context is active");

    mse_init(&memory, functional, NULL);
    {
        sr_fact_t first = {0x52000001u, SR_SYMBOL_LEGACY(20u),
                           SR_SYMBOL_LEGACY(31u), 0u};
        sr_fact_t second = {0x52000001u, SR_SYMBOL_LEGACY(20u),
                            SR_SYMBOL_LEGACY(32u), 0u};
        sr_pattern_t cue = {SR_CONST(0x52000001u),
                            SR_CONST(SR_SYMBOL_LEGACY(20u)), SR_VAR(0u), 0u};
        context = make_context(cue, MAGIC_REQUEST_EXPLICIT, 1u);
        check(&score, mse_add(&memory, &reviewed, &first, 1u, 0u) == MSE_OK &&
                        mse_add(&memory, &reviewed, &second, 2u, 0u) == MSE_OK &&
                        magic_propose(&base, &memory, 2u, &context, &policy,
                                      &scratch, &proposal) == MAGIC_CONTRADICTION &&
                        proposal.requires_confirmation == 1u,
              "functional conflict becomes a confirmation-required contradiction, never a chosen fact");
    }

    mse_init(&memory, NULL, NULL);
    {
        sr_fact_t first = {0x53000001u, fill_predicate,
                           SR_SYMBOL_LEGACY(41u), 0u};
        sr_fact_t second = {0x53000001u, fill_predicate,
                            SR_SYMBOL_LEGACY(42u), 0u};
        sr_pattern_t cue = {SR_CONST(0x53000001u), SR_CONST(fill_predicate),
                            SR_VAR(0u), 0u};
        context = make_context(cue, MAGIC_REQUEST_CONTEXTUAL, 1u);
        check(&score, mse_add(&memory, &reviewed, &first, 1u, 0u) == MSE_OK &&
                        mse_add(&memory, &reviewed, &second, 2u, 0u) == MSE_OK &&
                        magic_propose(&base, &memory, 2u, &context, &policy,
                                      &scratch, &proposal) == MAGIC_ABSTAIN &&
                        proposal.answer.kind == SR_ANSWER_NONE,
              "compatible alternatives under active context abstain instead of selecting a favorite");
    }

    mse_init(&memory, NULL, NULL);
    context = make_context(unknown_cue, MAGIC_REQUEST_CONTEXTUAL, 1u);
    check(&score, magic_propose(&base, &memory, 1u, &context, &policy,
                                &scratch, &proposal) == MAGIC_KNOWN_GAP &&
                    proposal.requires_confirmation == 0u,
          "Core absence represented by empty local evidence cannot create authority or invented recall");

    printf("DEGRADATION MATRIX: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
