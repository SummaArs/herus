#include "magic_dialogue_bridge.h"
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
    sd_dialogue_t dialogue;
    sd_dialogue_t before;
    mse_index_t memory;
    sr_reasoner_t scratch;
    magic_policy_t policy;
    magic_context_t context;
    mdb_reply_t reply;
    memory_vault_card_t reviewed = card(600u, 1600u);
    const sr_symbol_t subject = 0x07070001u;
    const sr_symbol_t predicate = SR_SYMBOL_LEGACY(50u);
    const sr_symbol_t object = SR_SYMBOL_LEGACY(51u);
    sr_pattern_t cue = {SR_CONST(subject), SR_CONST(predicate),
                        SR_CONST(object), 0u};

    sd_init(&dialogue);
    magic_policy_default(&policy);
    mse_init(&memory, NULL, NULL);
    check(&score, mse_add(&memory, &reviewed,
                          &(sr_fact_t){subject, predicate, object, 0u},
                          1u, 0u) == MSE_OK,
          "reviewed memory is available to the read-only dialogue bridge");
    context.cue = cue;
    context.privacy_class = MAGIC_PRIVACY_ORDINARY;
    context.request_kind = MAGIC_REQUEST_EXPLICIT;
    context.attention_window = 1u;
    before = dialogue;
    check(&score, mdb_propose(&dialogue, &memory, 1u, &context, &policy,
                              &scratch, &reply) == MDB_OK &&
                    reply.status == MDB_OK && reply.presentable == 1u &&
                    reply.proposal.status == MAGIC_RECALL &&
                    dialogue.turn == before.turn &&
                    dialogue.reasoner.fact_count == before.reasoner.fact_count,
          "dialogue presents local recall without advancing turn or mutating memory");
    check(&score, reply.proposal.composition.selected_card_id == 600u &&
                    reply.requires_confirmation == 0u,
          "presented recall retains minimal card provenance and no action authority");

    context.request_kind = MAGIC_REQUEST_CONTEXTUAL;
    context.attention_window = 0u;
    check(&score, mdb_propose(&dialogue, &memory, 1u, &context, &policy,
                              &scratch, &reply) == MDB_SILENT &&
                    reply.presentable == 0u,
          "dialogue bridge stays silent outside the contextual attention window");

    context.request_kind = MAGIC_REQUEST_EXPLICIT;
    context.privacy_class = MAGIC_PRIVACY_SENSITIVE;
    check(&score, mdb_propose(&dialogue, &memory, 1u, &context, &policy,
                              &scratch, &reply) == MDB_BLOCKED &&
                    reply.presentable == 0u,
          "dialogue bridge blocks sensitive context before presentation");

    context.privacy_class = MAGIC_PRIVACY_ORDINARY;
    context.cue = (sr_pattern_t){SR_CONST(subject), SR_CONST(predicate),
                                 SR_VAR(0u), 0u};
    mse_init(&memory, NULL, NULL);
    check(&score, mse_add(&memory, &reviewed,
                          &(sr_fact_t){subject, predicate, object, 0u},
                          2u, 0u) == MSE_OK &&
                    mse_add(&memory, &reviewed,
                            &(sr_fact_t){subject, predicate,
                                         SR_SYMBOL_LEGACY(52u), 0u},
                            3u, 0u) == MSE_OK,
          "dialogue receives compatible alternatives without choosing one");
    check(&score, mdb_propose(&dialogue, &memory, 3u, &context, &policy,
                              &scratch, &reply) == MDB_ABSTAIN &&
                    reply.presentable == 0u,
          "ambiguous personal context is withheld from dialogue presentation");

    context.cue = (sr_pattern_t){SR_CONST(subject), SR_CONST(predicate),
                                 SR_CONST(object), 0u};
    mse_init(&memory, NULL, NULL);
    policy.max_steps = 1u;
    check(&score, mse_add(&memory, &reviewed,
                          &(sr_fact_t){subject, predicate, object, 0u},
                          4u, 0u) == MSE_OK &&
                    mdb_propose(&dialogue, &memory, 4u, &context, &policy,
                                &scratch, &reply) == MDB_OK,
          "a direct memory proposal remains presentable under a small bounded budget");

    printf("MAGIC DIALOGUE BRIDGE: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
