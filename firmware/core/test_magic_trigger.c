#include "magic_trigger.h"
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

int main(void)
{
    score_t score = { 0, 0 };
    magic_trigger_t trigger;
    magic_policy_t policy;
    magic_proposal_t proposal;
    sr_reasoner_t base;
    sr_reasoner_t scratch;
    mse_index_t memory;
    memory_vault_card_t reviewed = card(500u, 1500u);
    magic_context_t context;
    sr_pattern_t cue;
    const sr_symbol_t subject = 0x06070001u;
    const sr_symbol_t predicate = SR_SYMBOL_LEGACY(40u);
    const sr_symbol_t object = SR_SYMBOL_LEGACY(41u);

    magic_policy_default(&policy);
    sr_init(&base);
    mse_init(&memory, NULL, NULL);
    check(&score, mse_add(&memory, &reviewed,
                          &(sr_fact_t){subject, predicate, object, 0u},
                          10u, 0u) == MSE_OK,
          "reviewed local fact is prepared for a temporal trigger");
    cue = (sr_pattern_t){SR_CONST(subject), SR_CONST(predicate),
                         SR_CONST(object), 0u};
    context.cue = cue;
    context.privacy_class = MAGIC_PRIVACY_ORDINARY;
    context.request_kind = MAGIC_REQUEST_CONTEXTUAL;
    context.attention_window = 1u;

    check(&score, magic_trigger_begin(&trigger, &context, 10u, 3u, 2u) ==
                    MAGIC_TRIGGER_OK && trigger.active == 1u &&
                    trigger.expires_generation == 13u,
          "attention window opens with explicit TTL and presentation budget");
    check(&score, magic_trigger_offer(&trigger, &base, &memory, 10u, &policy,
                                      &scratch, &proposal) == MAGIC_TRIGGER_OK &&
                    proposal.status == MAGIC_RECALL &&
                    trigger.proposals_served == 1u,
          "first contextual offer is a bounded local recall");
    check(&score, magic_trigger_offer(&trigger, &base, &memory, 11u, &policy,
                                      &scratch, &proposal) == MAGIC_TRIGGER_OK &&
                    trigger.proposals_served == 2u,
          "second offer consumes the final bounded presentation slot");
    check(&score, magic_trigger_offer(&trigger, &base, &memory, 12u, &policy,
                                      &scratch, &proposal) == MAGIC_TRIGGER_SILENT,
          "presentation budget prevents repetitive magical interruption");
    check(&score, magic_trigger_offer(&trigger, &base, &memory, 14u, &policy,
                                      &scratch, &proposal) == MAGIC_TRIGGER_SILENT,
          "expired generation window becomes silent");

    magic_trigger_close(&trigger);
    check(&score, trigger.active == 0u && trigger.proposals_served == 0u,
          "closing the trigger scrubs its transient context and budget");
    context.attention_window = 0u;
    check(&score, magic_trigger_begin(&trigger, &context, 20u, 2u, 1u) ==
                    MAGIC_TRIGGER_SILENT && trigger.active == 0u,
          "no attention window opens no proactive context");
    context.attention_window = 1u;
    check(&score, magic_trigger_begin(&trigger, &context, 20u, 0u, 1u) ==
                    MAGIC_TRIGGER_E_ARG,
          "zero TTL is rejected");
    check(&score, magic_trigger_begin(&trigger, &context, UINT32_MAX, 1u, 1u) ==
                    MAGIC_TRIGGER_E_FORMAT,
          "generation overflow is rejected before activation");
    check(&score, magic_trigger_begin(&trigger, &context, 20u, 2u, 0u) ==
                    MAGIC_TRIGGER_E_ARG,
          "zero presentation budget is rejected");

    printf("MAGIC TRIGGER: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
