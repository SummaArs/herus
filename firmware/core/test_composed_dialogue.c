#include "composed_dialogue.h"

#include <stdio.h>
#include <string.h>

#define S_ALICE 1u
#define S_BOB 2u
#define S_CARA 3u
#define P_PARENT 10u
#define P_GRAND 11u
#define P_STAGE0 20u
#define P_STAGE1 21u

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static int zeroed(const void *value, size_t length)
{
    const unsigned char *bytes = value;
    unsigned char any = 0u;
    size_t i;
    for (i = 0u; i < length; ++i) any |= bytes[i];
    return any == 0u;
}

static int functional(sr_symbol_t predicate, void *user)
{
    (void)user;
    return predicate == SR_SYMBOL_LEGACY(P_STAGE1);
}

static sr_pattern_t pattern(sr_symbol_t subject, sr_symbol_t predicate,
                            sr_symbol_t object)
{
    sr_pattern_t result;
    result.subject = SR_CONST(subject);
    result.predicate = SR_CONST(predicate);
    result.object = SR_CONST(object);
    result.negated = 0u;
    return result;
}

static sp_problem_t plan_problem(void)
{
    sp_problem_t problem;
    memset(&problem, 0, sizeof(problem));
    problem.initial_count = 1u;
    problem.initial[0] = (sr_fact_t){ S_ALICE, P_STAGE0, S_BOB, 0u };
    problem.action_count = 1u;
    problem.action[0].id = 7u;
    problem.action[0].requires_confirmation = 1u;
    problem.action[0].precondition_count = 1u;
    problem.action[0].precondition[0] = problem.initial[0];
    problem.action[0].add_count = 1u;
    problem.action[0].add[0] = (sr_fact_t){ S_ALICE, P_STAGE1, S_BOB, 0u };
    problem.goal = problem.action[0].add[0];
    return problem;
}

static memory_vault_card_t card(uint32_t id, uint32_t receipt)
{
    memory_vault_card_t result;
    memset(&result, 0, sizeof(result));
    result.card_id = id;
    result.review_receipt_id = receipt;
    return result;
}

static cdh_config_t config(const intent_router_memory_t *router_memories,
                           size_t router_count, const gc_lexicon_t *lexicon,
                           const sr_reasoner_t *reasoner, const mse_index_t *memory,
                           const pa_profile_t *personal, const sr_pattern_t *query,
                           const sp_problem_t *problem, uint32_t generation)
{
    cdh_config_t result;
    memset(&result, 0, sizeof(result));
    result.router_memories = router_memories;
    result.router_memory_count = router_count;
    result.lexicon = lexicon;
    result.reasoner = reasoner;
    result.memory = memory;
    result.personal_profile = personal;
    result.default_query = *query;
    result.plan_problem = problem;
    result.current_generation = generation;
    result.lifecycle_config.turn_timeout_ms = 100u;
    result.lifecycle_config.confirmation_timeout_ms = 40u;
    return result;
}

static const intent_router_memory_t POST_ROUTER_MEMORIES[] = {
    { INTENT_ROUTER_MEMORY_ID_MEETING_OLD, INTENT_ROUTER_MEMORY_SCHEDULE,
      6u, 1u, 1u, 1u },
    { INTENT_ROUTER_MEMORY_ID_MEETING_NEW, INTENT_ROUTER_MEMORY_SCHEDULE,
      7u, 1u, 0u, 1u },
    { INTENT_ROUTER_MEMORY_ID_PREF_CONCISE, INTENT_ROUTER_MEMORY_PREFERENCE,
      7u, 1u, 0u, 1u },
    { INTENT_ROUTER_MEMORY_ID_PROJECT, INTENT_ROUTER_MEMORY_PROJECT,
      7u, 1u, 0u, 1u }
};

static const intent_router_memory_t ROUTER_MEMORIES[] = {
    { INTENT_ROUTER_MEMORY_ID_MEETING_OLD, INTENT_ROUTER_MEMORY_SCHEDULE,
      1u, 1u, 1u, 1u },
    { INTENT_ROUTER_MEMORY_ID_MEETING_NEW, INTENT_ROUTER_MEMORY_SCHEDULE,
      2u, 1u, 0u, 1u },
    { INTENT_ROUTER_MEMORY_ID_PREF_CONCISE, INTENT_ROUTER_MEMORY_PREFERENCE,
      1u, 1u, 0u, 1u },
    { INTENT_ROUTER_MEMORY_ID_PROJECT, INTENT_ROUTER_MEMORY_PROJECT,
      1u, 1u, 0u, 1u }
};

static const gc_lexeme_t LEXICON[] = {
    { S_ALICE, "alice", 5u },
    { S_BOB, "bob", 3u },
    { S_CARA, "cara", 4u },
    { P_PARENT, "pai", 3u },
    { P_GRAND, "avo", 3u },
    { P_STAGE0, "estagio0", 8u },
    { P_STAGE1, "estagio1", 8u }
};

int main(void)
{
    score_t score = { 0, 0 };
    sr_reasoner_t empty_reasoner;
    sr_pattern_t query = pattern(S_ALICE, P_GRAND, S_CARA);
    sp_problem_t plan = plan_problem();
    mse_index_t memory;
    mse_index_t expired_memory;
    mse_index_t post_reboot_memory;
    mse_index_t bad_floor_memory;
    memory_vault_card_t current_card = card(400u, 900u);
    memory_vault_card_t expired_card = card(401u, 901u);
    memory_vault_card_t post_reboot_card = card(402u, 902u);
    memory_vault_card_t bad_floor_card = card(403u, 903u);
    sr_fact_t grand = { S_ALICE, P_GRAND, S_CARA, 0u };
    pa_profile_t personal;
    pa_sample_t technical = { GC_PERSONAL_FEATURE_RESPONSE_STYLE,
                              PA_STYLE_TECHNICAL, 1u, 100u };
    cdh_config_t cfg;
    cdh_t dialogue;
    uint8_t round_trip_before;
    const char *recall = "Me diga o que ficou registrado sobre a reuniao.";

    sr_init(&empty_reasoner);
    mse_init(&memory, functional, NULL);
    check(&score, mse_add(&memory, &current_card, &grand, 5u, 0u) == MSE_OK,
          "current reviewed memory enters the composed fixture");
    cfg = config(ROUTER_MEMORIES, sizeof(ROUTER_MEMORIES) / sizeof(ROUTER_MEMORIES[0]),
                 &(gc_lexicon_t){ LEXICON, sizeof(LEXICON) / sizeof(LEXICON[0]) },
                 &empty_reasoner, &memory, NULL, &query, &plan, 5u);
    cdh_init(&dialogue, &cfg);
    check(&score, cdh_start(&dialogue, recall, strlen(recall), 0u, 99u) == CDH_E_PHYSICAL &&
                    dialogue.state == CDH_IDLE,
          "a composed turn cannot start without a physical session");
    check(&score, cdh_start(&dialogue, recall, strlen(recall), 41u, 100u) == CDH_OK &&
                    dialogue.state == CDH_ROUTED &&
                    dialogue.route.intent == INTENT_ROUTER_RECALL_MEMORY,
          "recall observation routes into a fresh composed turn");
    round_trip_before = dialogue.route.evidence_count;
    check(&score, cdh_generate_present(&dialogue, 101u) == CDH_OK &&
                    dialogue.state == CDH_PRESENTED &&
                    dialogue.generated.grounded == 1u &&
                    dialogue.generated.composition.selected_card_id == 400u &&
                    dialogue.generated.composition.selected_review_receipt_id == 900u &&
                    dialogue.generated.composition.selected_generation == 5u &&
                    dialogue.lifecycle.signal.event.scope == HL_SCOPE_MEM &&
                    dialogue.lifecycle.signal.actionable == 0u,
          "current memory is grounded, presented and still non-actionable");
    check(&score, round_trip_before == 1u && dialogue.route.evidence_count == 1u,
          "router evidence remains bounded across generation");
    check(&score, cdh_forget(&dialogue) == CDH_OK && dialogue.state == CDH_CLEARED &&
                    dialogue.lifecycle.physical_session_id == 0u,
          "forget ends the first turn without a live session");

    pa_init(&personal);
    check(&score, pa_update(&personal, &technical, 1u) == PA_OK &&
                    pa_update(&personal, &technical, 1u) == PA_OK,
          "personal style is learned before the next composed turn");
    cfg = config(ROUTER_MEMORIES, sizeof(ROUTER_MEMORIES) / sizeof(ROUTER_MEMORIES[0]),
                 &(gc_lexicon_t){ LEXICON, sizeof(LEXICON) / sizeof(LEXICON[0]) },
                 &empty_reasoner, &memory, &personal, &query, &plan, 5u);
    cdh_init(&dialogue, &cfg);
    check(&score, cdh_start(&dialogue, recall, strlen(recall), 42u, 200u) == CDH_OK &&
                    cdh_generate_present(&dialogue, 201u) == CDH_OK &&
                    dialogue.generated.adapted == 1u &&
                    dialogue.generated.adaptation.style == PA_STYLE_TECHNICAL &&
                    dialogue.generated.grounded == 1u,
          "adaptation composes with grounded recall without changing its proof");
    check(&score, cdh_forget(&dialogue) == CDH_OK &&
                    dialogue.state == CDH_CLEARED,
          "adapted response is erased with the transient turn");

    cfg = config(ROUTER_MEMORIES, sizeof(ROUTER_MEMORIES) / sizeof(ROUTER_MEMORIES[0]),
                 &(gc_lexicon_t){ LEXICON, sizeof(LEXICON) / sizeof(LEXICON[0]) },
                 &empty_reasoner, NULL, NULL, &query, &plan, 5u);
    cdh_init(&dialogue, &cfg);
    check(&score, cdh_start(&dialogue, "Ligue para essa pessoa imediatamente.",
                             strlen("Ligue para essa pessoa imediatamente."), 43u, 300u) == CDH_OK &&
                    dialogue.route.intent == INTENT_ROUTER_ACTION_REQUEST &&
                    cdh_generate_present(&dialogue, 301u) == CDH_OK &&
                    dialogue.state == CDH_CONFIRMATION_PENDING &&
                    dialogue.lifecycle.signal.confirmation_required == 1u &&
                    dialogue.lifecycle.signal.actionable == 0u,
          "action intent becomes a pending plan and never an automatic action");
    check(&score, cdh_confirm(&dialogue, 99u, 302u) == CDH_E_PHYSICAL &&
                    dialogue.state == CDH_CONFIRMATION_PENDING,
          "wrong session cannot confirm the composed plan");
    check(&score, cdh_confirm(&dialogue, 43u, 303u) == CDH_OK &&
                    dialogue.state == CDH_CONFIRMED &&
                    dialogue.lifecycle.physical_session_id == 0u,
          "matching session confirms exactly one pending proposal");
    check(&score, cdh_confirm(&dialogue, 43u, 304u) == CDH_E_CONFIRMATION,
          "confirmation replay is rejected by the composed boundary");
    check(&score, cdh_forget(&dialogue) == CDH_OK && dialogue.state == CDH_CLEARED,
          "confirmed proposal still requires explicit cleanup");

    cdh_init(&dialogue, &cfg);
    check(&score, cdh_start(&dialogue, "Voce conhece o codigo secreto que nunca te contei?",
                             strlen("Voce conhece o codigo secreto que nunca te contei?"),
                             44u, 400u) == CDH_OK &&
                    cdh_generate_present(&dialogue, 401u) == CDH_OK &&
                    dialogue.state == CDH_ABSTAINED &&
                    dialogue.generated.abstain_reason == GC_ABSTAIN_NO_EVIDENCE &&
                    dialogue.lifecycle.signal.abstained == 1u,
          "unknown intent becomes an explicit multi-turn abstention");
    check(&score, cdh_forget(&dialogue) == CDH_OK,
          "unknown turn can be cleared without retaining a transcript");

    cdh_init(&dialogue, &cfg);
    check(&score, cdh_start(&dialogue,
                             "Qual horario devo considerar quando as anotacoes divergem?",
                             strlen("Qual horario devo considerar quando as anotacoes divergem?"),
                             45u, 500u) == CDH_OK &&
                    cdh_generate_present(&dialogue, 501u) == CDH_OK &&
                    dialogue.state == CDH_ABSTAINED &&
                    dialogue.generated.kind == GC_KIND_CONTRADICTED &&
                    dialogue.lifecycle.signal.event.class_code == HL_CLASS_ALERT,
          "conflict intent never selects a side during composition");
    check(&score, cdh_forget(&dialogue) == CDH_OK,
          "conflict turn is explicitly cleared");

    mse_init(&expired_memory, functional, NULL);
    check(&score, mse_add(&expired_memory, &expired_card, &grand, 1u, 2u) == MSE_OK,
          "pre-reboot evidence is inserted with a finite validity generation");
    cfg = config(ROUTER_MEMORIES, sizeof(ROUTER_MEMORIES) / sizeof(ROUTER_MEMORIES[0]),
                 &(gc_lexicon_t){ LEXICON, sizeof(LEXICON) / sizeof(LEXICON[0]) },
                 &empty_reasoner, &expired_memory, NULL, &query, &plan, 3u);
    cdh_init(&dialogue, &cfg);
    check(&score, cdh_start(&dialogue, recall, strlen(recall), 46u, 600u) == CDH_OK &&
                    cdh_generate_present(&dialogue, 601u) == CDH_OK &&
                    dialogue.state == CDH_ABSTAINED &&
                    dialogue.generated.grounded == 0u &&
                    dialogue.lifecycle.signal.abstained == 1u,
          "expired pre-reboot evidence cannot become ghost memory");
    check(&score, cdh_tick(&dialogue, 650u) == CDH_OK &&
                    dialogue.state == CDH_ABSTAINED,
          "an abstaining turn remains stable below its timeout boundary");
    check(&score, cdh_forget(&dialogue) == CDH_OK &&
                    zeroed(&dialogue.generated, sizeof(dialogue.generated)) &&
                    zeroed(&dialogue.request, sizeof(dialogue.request)),
          "final cleanup erases generated metadata and request state");

    check(&score, cdh_start(&dialogue, recall, strlen(recall), 47u, 700u) == CDH_OK &&
                    cdh_generate_present(&dialogue, 701u) == CDH_OK &&
                    dialogue.state == CDH_ABSTAINED &&
                    dialogue.route.evidence_count == 1u,
          "an active pre-reboot turn provides transient state to scrub");
    mse_init(&post_reboot_memory, functional, NULL);
    check(&score, mse_set_generation_floor(&post_reboot_memory, 6u) == MSE_OK &&
                    mse_add(&post_reboot_memory, &post_reboot_card, &grand,
                            7u, 0u) == MSE_OK,
          "post-reboot index accepts only fresh evidence above the recovered floor");
    check(&score, cdh_reboot(&dialogue, 6u) == CDH_OK &&
                    dialogue.memory_quarantined == 1u &&
                    dialogue.recovered_generation == 6u &&
                    dialogue.lifecycle.physical_session_id == 0u &&
                    zeroed(&dialogue.route, sizeof(dialogue.route)) &&
                    zeroed(&dialogue.request, sizeof(dialogue.request)) &&
                    zeroed(&dialogue.generated, sizeof(dialogue.generated)),
          "reboot scrubs the composed session and enters memory quarantine");
    check(&score, cdh_start(&dialogue, recall, strlen(recall), 47u, 800u) == CDH_E_STATE,
          "old configuration cannot start a turn while post-reboot quarantine is active");
    check(&score, cdh_rearm(&dialogue, POST_ROUTER_MEMORIES,
                             sizeof(POST_ROUTER_MEMORIES) / sizeof(POST_ROUTER_MEMORIES[0]),
                             &post_reboot_memory, 6u) == CDH_E_STATE,
          "rearm at the recovered generation is rejected as replay");
    mse_init(&bad_floor_memory, functional, NULL);
    check(&score, mse_set_generation_floor(&bad_floor_memory, 5u) == MSE_OK &&
                    mse_add(&bad_floor_memory, &bad_floor_card, &grand,
                            7u, 0u) == MSE_OK &&
                    cdh_rearm(&dialogue, POST_ROUTER_MEMORIES,
                              sizeof(POST_ROUTER_MEMORIES) / sizeof(POST_ROUTER_MEMORIES[0]),
                              &bad_floor_memory, 7u) == CDH_E_STATE,
          "rearm with a divergent recovered floor is rejected");
    check(&score, cdh_rearm(&dialogue, POST_ROUTER_MEMORIES,
                             sizeof(POST_ROUTER_MEMORIES) / sizeof(POST_ROUTER_MEMORIES[0]),
                             &post_reboot_memory, 7u) == CDH_OK &&
                    dialogue.memory_quarantined == 0u && dialogue.state == CDH_IDLE,
          "explicit rearm at a strictly newer generation restores idle operation");
    check(&score, cdh_start(&dialogue, recall, strlen(recall), 48u, 900u) == CDH_OK &&
                    cdh_generate_present(&dialogue, 901u) == CDH_OK &&
                    dialogue.state == CDH_PRESENTED && dialogue.generated.grounded == 1u &&
                    dialogue.generated.composition.selected_card_id == 402u,
          "post-reboot recall grounds only the newly admitted semantic card");
    check(&score, cdh_forget(&dialogue) == CDH_OK && dialogue.state == CDH_CLEARED &&
                    dialogue.lifecycle.physical_session_id == 0u,
          "post-reboot turn still requires explicit final cleanup");

    printf("COMPOSED DIALOGUE: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail == 0 ? 0 : 1;
}
