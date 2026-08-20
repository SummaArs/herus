#include "magic_anticipation.h"
#include <stdio.h>
#include <string.h>

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static int functional(sr_symbol_t predicate, void *user)
{
    (void)user;
    return predicate == SR_SYMBOL_LEGACY(20u);
}

static memory_vault_card_t card(uint32_t id, uint32_t receipt)
{
    memory_vault_card_t out;
    memset(&out, 0, sizeof(out));
    out.card_id = id;
    out.review_receipt_id = receipt;
    return out;
}

static magic_context_t context(sr_pattern_t cue,
                               magic_privacy_class_t privacy,
                               magic_request_kind_t request,
                               uint8_t attention)
{
    magic_context_t out;
    out.cue = cue;
    out.privacy_class = privacy;
    out.request_kind = request;
    out.attention_window = attention;
    out.proactive_consent = request == MAGIC_REQUEST_CONTEXTUAL ? 1u : 0u;
    return out;
}

int main(void)
{
    score_t score = { 0, 0 };
    magic_policy_t policy;
    magic_proposal_t proposal;
    sr_reasoner_t base;
    sr_reasoner_t scratch;
    mse_index_t memory;
    memory_vault_card_t reviewed = card(400u, 1400u);
    const sr_symbol_t subject = 0x05070001u;
    const sr_symbol_t predicate = SR_SYMBOL_LEGACY(30u);
    const sr_symbol_t object = SR_SYMBOL_LEGACY(31u);
    const sr_symbol_t conclusion_predicate = SR_SYMBOL_LEGACY(33u);
    const sr_symbol_t conclusion_object = SR_SYMBOL_LEGACY(34u);
    sr_rule_t local_rule;
    sr_pattern_t cue = {SR_CONST(subject), SR_CONST(predicate),
                        SR_CONST(object), 0u};
    magic_context_t ordinary = context(cue, MAGIC_PRIVACY_ORDINARY,
                                       MAGIC_REQUEST_EXPLICIT, 1u);
    magic_context_t contextual = context(cue, MAGIC_PRIVACY_ORDINARY,
                                         MAGIC_REQUEST_CONTEXTUAL, 1u);

    magic_policy_default(&policy);
    memset(&local_rule, 0, sizeof(local_rule));
    local_rule.id = 1u;
    local_rule.premise_count = 1u;
    local_rule.premise[0] = (sr_pattern_t){SR_VAR(0u), SR_CONST(predicate),
                                           SR_CONST(object), 0u};
    local_rule.conclusion = (sr_pattern_t){SR_VAR(0u),
                                           SR_CONST(conclusion_predicate),
                                           SR_CONST(conclusion_object), 0u};
    local_rule.cost = 1u;
    sr_init(&base);
    check(&score, sr_add_rule(&base, &local_rule) == SR_OK,
          "local anticipation rule is accepted before proposal tests");
    mse_init(&memory, NULL, NULL);
    check(&score, mse_add(&memory, &reviewed,
                          &(sr_fact_t){subject, predicate, object, 0u},
                          1u, 0u) == MSE_OK,
          "ordinary reviewed memory is prepared for anticipation");
    check(&score, magic_propose(&base, &memory, 1u, &ordinary, &policy,
                                &scratch, &proposal) == MAGIC_RECALL &&
                    proposal.explanation_available == 1u &&
                    proposal.composition.selected_card_id == 400u &&
                    proposal.requires_confirmation == 0u,
          "ordinary explicit query produces an explainable local recall proposal");
    check(&score, magic_propose(&base, &memory, 1u, &contextual, &policy,
                                &scratch, &proposal) == MAGIC_RECALL,
          "ordinary contextual query can produce a bounded recall proposal");

    contextual.attention_window = 0u;
    check(&score, magic_propose(&base, &memory, 1u, &contextual, &policy,
                                &scratch, &proposal) == MAGIC_SILENT,
          "contextual anticipation stays silent outside the attention window");
    contextual.attention_window = 1u;
    contextual.proactive_consent = 0u;
    check(&score, magic_propose(&base, &memory, 1u, &contextual, &policy,
                                &scratch, &proposal) == MAGIC_SILENT,
          "revoked proactive consent suppresses contextual anticipation");
    contextual.privacy_class = MAGIC_PRIVACY_PERSONAL;
    check(&score, magic_propose(&base, &memory, 1u, &contextual, &policy,
                                &scratch, &proposal) == MAGIC_SENSITIVE_BLOCK,
          "personal contextual memory is not surfaced without explicit request");
    ordinary.privacy_class = MAGIC_PRIVACY_SENSITIVE;
    check(&score, magic_propose(&base, &memory, 1u, &ordinary, &policy,
                                &scratch, &proposal) == MAGIC_SENSITIVE_BLOCK,
          "sensitive context is blocked before local reasoning");
    ordinary.privacy_class = MAGIC_PRIVACY_THIRD_PARTY;
    check(&score, magic_propose(&base, &memory, 1u, &ordinary, &policy,
                                &scratch, &proposal) == MAGIC_SENSITIVE_BLOCK,
          "third-party context is blocked before local reasoning");

    ordinary.privacy_class = MAGIC_PRIVACY_PERSONAL;
    ordinary.request_kind = MAGIC_REQUEST_EXPLICIT;
    check(&score, magic_propose(&base, &memory, 1u, &ordinary, &policy,
                                &scratch, &proposal) == MAGIC_RECALL,
          "personal memory is available only through an explicit local request");

    mse_init(&memory, NULL, NULL);
    check(&score, magic_propose(&base, &memory, 2u, &ordinary, &policy,
                                &scratch, &proposal) == MAGIC_KNOWN_GAP &&
                    proposal.explanation_available == 1u,
          "absence of local evidence becomes a known gap, not invented recall");

    mse_init(&memory, NULL, NULL);
    check(&score, mse_add(&memory, &reviewed,
                          &(sr_fact_t){subject, predicate, object, 0u},
                          3u, 3u) == MSE_OK &&
                    magic_propose(&base, &memory, 4u, &ordinary, &policy,
                                  &scratch, &proposal) == MAGIC_KNOWN_GAP,
          "expired memory produces a known gap rather than a stale surprise");

    mse_init(&memory, NULL, NULL);
    mse_init(&memory, functional, NULL);
    check(&score, mse_add(&memory, &reviewed,
                          &(sr_fact_t){subject, SR_SYMBOL_LEGACY(20u), object, 0u},
                          5u, 0u) == MSE_OK &&
                    mse_add(&memory, &reviewed,
                            &(sr_fact_t){subject, SR_SYMBOL_LEGACY(20u),
                                         SR_SYMBOL_LEGACY(32u), 0u},
                            6u, 0u) == MSE_OK,
          "conflicting local evidence is prepared for the anti-surprise gate");
    ordinary.cue = (sr_pattern_t){SR_CONST(subject), SR_CONST(SR_SYMBOL_LEGACY(20u)),
                                  SR_VAR(0u), 0u};
    check(&score, magic_propose(&base, &memory, 6u, &ordinary, &policy,
                                &scratch, &proposal) == MAGIC_CONTRADICTION &&
                    proposal.requires_confirmation == 1u,
          "contradiction is surfaced as a confirmation-requiring proposal");

    mse_init(&memory, NULL, NULL);
    check(&score, mse_add(&memory, &reviewed,
                          &(sr_fact_t){subject, predicate, object, 0u},
                          7u, 0u) == MSE_OK &&
                    mse_add(&memory, &reviewed,
                            &(sr_fact_t){subject, predicate,
                                         SR_SYMBOL_LEGACY(32u), 0u},
                            8u, 0u) == MSE_OK,
          "compatible alternatives are prepared for ambiguity abstention");
    ordinary.cue = (sr_pattern_t){SR_CONST(subject), SR_CONST(predicate),
                                  SR_VAR(0u), 0u};
    check(&score, magic_propose(&base, &memory, 8u, &ordinary, &policy,
                                &scratch, &proposal) == MAGIC_ABSTAIN,
          "ambiguous alternatives abstain instead of selecting a favorite");

    mse_init(&memory, NULL, NULL);
    ordinary.cue = (sr_pattern_t){SR_CONST(subject),
                                  SR_CONST(conclusion_predicate),
                                  SR_CONST(conclusion_object), 0u};
    policy.max_steps = 1u;
    check(&score, mse_add(&memory, &reviewed,
                          &(sr_fact_t){subject, predicate, object, 0u},
                          9u, 0u) == MSE_OK &&
                    magic_propose(&base, &memory, 9u, &ordinary, &policy,
                                  &scratch, &proposal) == MAGIC_LIMIT,
          "bounded reasoning limit produces no magical false success");

    printf("MAGIC ANTICIPATION: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
