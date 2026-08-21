#include "generative_core.h"

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

static int functional(sr_symbol_t predicate, void *user)
{
    (void)user;
    return predicate == SR_SYMBOL_LEGACY(P_STAGE1);
}

static memory_vault_card_t card(uint32_t card_id, uint32_t receipt)
{
    memory_vault_card_t result;
    memset(&result, 0, sizeof(result));
    result.card_id = card_id;
    result.review_receipt_id = receipt;
    return result;
}

static sr_pattern_t pattern(sr_symbol_t subject, sr_symbol_t predicate,
                            sr_symbol_t object, uint8_t negated)
{
    sr_pattern_t result;
    result.subject = SR_CONST(subject);
    result.predicate = SR_CONST(predicate);
    result.object = SR_CONST(object);
    result.negated = negated;
    return result;
}

static sr_rule_t parent_rule(void)
{
    sr_rule_t rule;
    memset(&rule, 0, sizeof(rule));
    rule.id = 1u;
    rule.premise_count = 2u;
    rule.premise[0].subject = SR_VAR(0u);
    rule.premise[0].predicate = SR_CONST(P_PARENT);
    rule.premise[0].object = SR_VAR(1u);
    rule.premise[1].subject = SR_VAR(1u);
    rule.premise[1].predicate = SR_CONST(P_PARENT);
    rule.premise[1].object = SR_VAR(2u);
    rule.conclusion.subject = SR_VAR(0u);
    rule.conclusion.predicate = SR_CONST(P_GRAND);
    rule.conclusion.object = SR_VAR(2u);
    rule.cost = 3u;
    return rule;
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

static const gc_lexeme_t LEXEMES[] = {
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
    gc_lexicon_t lexicon = { LEXEMES, sizeof(LEXEMES) / sizeof(LEXEMES[0]) };
    sr_reasoner_t reasoner;
    sr_reasoner_t scratch;
    mse_index_t memory;
    mse_index_t expired_memory;
    mse_index_t conflict_memory;
    pa_profile_t personal;
    pa_sample_t technical = { GC_PERSONAL_FEATURE_RESPONSE_STYLE,
                              PA_STYLE_TECHNICAL, 1u, 100u };
    memory_vault_card_t memory_card = card(400u, 900u);
    memory_vault_card_t second_card = card(401u, 901u);
    sr_rule_t parent = parent_rule();
    gc_request_t request;
    gc_result_t result;
    sp_problem_t plan = plan_problem();

    sr_init(&reasoner);
    check(&score, sr_add_fact(&reasoner,
                              (sr_fact_t){ S_ALICE, P_PARENT, S_BOB, 0u }) == SR_OK,
          "ground fact enters the local reasoner");
    check(&score, sr_add_fact(&reasoner,
                              (sr_fact_t){ S_BOB, P_PARENT, S_CARA, 0u }) == SR_OK,
          "second fact enables a novel composition");
    check(&score, sr_add_rule(&reasoner, &parent) == SR_OK,
          "bounded two-premise rule enters the generator substrate");
    memset(&request, 0, sizeof(request));
    request.mode = GC_MODE_ANSWER;
    request.query = pattern(S_ALICE, P_PARENT, S_BOB, 0u);
    request.derivation_budget = 32u;
    check(&score, gc_generate(&reasoner, &lexicon, &request, &scratch, &result) == GC_STATUS_OK &&
                    result.kind == GC_KIND_DIRECT &&
                    strcmp(result.response, "alice pai bob") == 0 &&
                    result.authority == GC_AUTH_PRESENTATION_ONLY,
          "direct evidence becomes a local presentable answer");

    request.query = pattern(S_ALICE, P_GRAND, S_CARA, 0u);
    check(&score, gc_generate(&reasoner, &lexicon, &request, &scratch, &result) == GC_STATUS_OK &&
                    result.kind == GC_KIND_DERIVED &&
                    strcmp(result.response, "alice avo cara") == 0 &&
                    result.evidence_count > 0u && result.derivation_digest != 0u,
          "a novel answer is composed from a bounded derivation and evidence");

    request.mode = GC_MODE_EXPLAIN;
    check(&score, gc_generate(&reasoner, &lexicon, &request, &scratch, &result) == GC_STATUS_OK &&
                    result.kind == GC_KIND_COMPOSED &&
                    strcmp(result.response, "porque alice avo cara") == 0,
          "explanation is a distinct composition, not an authority grant");

    request.mode = GC_MODE_COUNTERFACTUAL;
    request.query = pattern(S_ALICE, P_GRAND, S_CARA, 0u);
    {
        sr_reasoner_t counterfactual_base;
        sr_init(&counterfactual_base);
        sr_add_fact(&counterfactual_base,
                    (sr_fact_t){ S_ALICE, P_PARENT, S_BOB, 0u });
        sr_add_rule(&counterfactual_base, &parent);
        check(&score, gc_generate(&counterfactual_base, &lexicon, &request,
                                   &scratch, &result) == GC_STATUS_OK &&
                        result.kind == GC_KIND_COUNTERFACTUAL &&
                        result.counterfactual == 1u &&
                        strcmp(result.response,
                               "se bob pai cara, entao alice avo cara") == 0 &&
                        scratch.fact_count == counterfactual_base.fact_count,
              "counterfactual composition is explicit and does not mutate knowledge");
    }

    request.mode = GC_MODE_PLAN;
    request.plan_problem = &plan;
    request.max_plan_nodes = 16u;
    request.max_plan_depth = 4u;
    check(&score, gc_generate(NULL, NULL, &request, &scratch, &result) == GC_STATUS_OK &&
                    result.kind == GC_KIND_PLAN && result.plan.plan_length == 1u &&
                    result.plan.action_id[0] == 7u &&
                    result.requires_confirmation == 1u &&
                    result.authority == GC_AUTH_CONFIRMATION_REQUIRED &&
                    strcmp(result.response, "plano: 7") == 0,
          "a finite plan is generated as a confirmation-bound proposal");

    request.mode = GC_MODE_ANSWER;
    request.plan_problem = NULL;
    request.query = pattern(S_CARA, P_GRAND, S_ALICE, 0u);
    check(&score, gc_generate(&reasoner, &lexicon, &request, &scratch, &result) == GC_STATUS_ABSTAIN &&
                    result.kind == GC_KIND_UNKNOWN &&
                    result.abstain_reason == GC_ABSTAIN_NO_EVIDENCE &&
                    result.response[0] == '\0',
          "missing evidence produces explicit abstention rather than fluent guessing");

    check(&score, sr_add_fact(&reasoner,
                              (sr_fact_t){ S_ALICE, P_GRAND, S_CARA, 1u }) == SR_OK,
          "contradictory evidence fixture is accepted by the substrate");
    request.query = pattern(S_ALICE, P_GRAND, S_CARA, 0u);
    check(&score, gc_generate(&reasoner, &lexicon, &request, &scratch, &result) == GC_STATUS_ABSTAIN &&
                    result.kind == GC_KIND_CONTRADICTED &&
                    result.abstain_reason == GC_ABSTAIN_CONFLICT,
          "conflicting evidence blocks a confident generated answer");

    sr_init(&reasoner);
    sr_add_fact(&reasoner, (sr_fact_t){ S_ALICE, P_PARENT, S_BOB, 0u });
    sr_add_rule(&reasoner, &parent);
    request.mode = GC_MODE_ANSWER;
    request.query = pattern(S_ALICE, P_GRAND, S_CARA, 0u);
    request.derivation_budget = 0u;
    check(&score, gc_generate(&reasoner, &lexicon, &request, &scratch, &result) == GC_STATUS_LIMIT &&
                    result.kind == GC_KIND_LIMIT &&
                    result.abstain_reason == GC_ABSTAIN_BUDGET,
          "zero derivation budget is a limit, never an implicit search");

    request.derivation_budget = 32u;
    request.policy_blocked = 1u;
    check(&score, gc_generate(&reasoner, &lexicon, &request, &scratch, &result) == GC_STATUS_ABSTAIN &&
                    result.kind == GC_KIND_POLICY_BLOCKED &&
                    result.authority == GC_AUTH_NONE,
          "policy blocks remain outside the generator and fail closed");

    request.policy_blocked = 0u;
    sr_add_fact(&reasoner, (sr_fact_t){ 99u, P_GRAND, S_CARA, 0u });
    request.query = pattern(99u, P_GRAND, S_CARA, 0u);
    check(&score, gc_generate(&reasoner, &lexicon, &request, &scratch, &result) == GC_STATUS_ABSTAIN &&
                    result.kind == GC_KIND_UNSUPPORTED && result.lexeme_missing == 1u,
          "unregistered symbols cannot become invented language");

    check(&score, reasoner.fact_count == 2u && reasoner.rule_count == 1u,
          "generation leaves the caller-owned reasoner unchanged");

    sr_init(&reasoner);
    sr_add_fact(&reasoner, (sr_fact_t){ S_ALICE, P_PARENT, S_BOB, 0u });
    sr_add_fact(&reasoner, (sr_fact_t){ S_BOB, P_PARENT, S_CARA, 0u });
    sr_add_rule(&reasoner, &parent);
    pa_init(&personal);
    check(&score, pa_update(&personal, &technical, 1u) == PA_OK &&
                    pa_update(&personal, &technical, 1u) == PA_OK,
          "two consented local style samples are available to presentation");
    request.mode = GC_MODE_EXPLAIN;
    request.query = pattern(S_ALICE, P_GRAND, S_CARA, 0u);
    request.derivation_budget = 32u;
    request.personal_profile = &personal;
    gc_generate(&reasoner, &lexicon, &request, &scratch, &result);
    check(&score, result.status == GC_STATUS_OK &&
                    result.adapted == 1u &&
                    result.adaptation.style == PA_STYLE_TECHNICAL &&
                    strcmp(result.response, "derivacao alice avo cara") == 0 &&
                    result.authority == GC_AUTH_PRESENTATION_ONLY,
          "personal adaptation changes only presentation style, never proof or authority");
    pa_reboot_quarantine(&personal);
    gc_generate(&reasoner, &lexicon, &request, &scratch, &result);
    check(&score, result.status == GC_STATUS_OK &&
                    result.adapted == 0u &&
                    strcmp(result.response, "porque alice avo cara") == 0,
          "quarantined adaptation falls back to the canonical presentation");
    request.personal_profile = NULL;

    mse_init(&memory, functional, NULL);
    check(&score, mse_add(&memory, &memory_card,
                          &(sr_fact_t){ S_ALICE, P_STAGE1, S_BOB, 0u },
                          6u, 0u) == MSE_OK,
          "an explicitly reviewed semantic card enters the grounded index");
    request.mode = GC_MODE_ANSWER;
    request.query = pattern(S_ALICE, P_STAGE1, S_BOB, 0u);
    request.derivation_budget = 32u;
    request.memory = &memory;
    request.current_generation = 6u;
    check(&score, gc_generate(&reasoner, &lexicon, &request, &scratch, &result) == GC_STATUS_OK &&
                    result.grounded == 1u && result.kind == GC_KIND_DIRECT &&
                    result.composition.selected_card_id == 400u &&
                    result.composition.selected_review_receipt_id == 900u &&
                    strcmp(result.response, "alice estagio1 bob") == 0,
          "generated language is grounded in current reviewed memory with provenance");

    mse_init(&expired_memory, functional, NULL);
    check(&score, mse_add(&expired_memory, &memory_card,
                          &(sr_fact_t){ S_ALICE, P_STAGE1, S_BOB, 0u },
                          6u, 6u) == MSE_OK,
          "an expiring semantic card is prepared");
    request.memory = &expired_memory;
    request.current_generation = 7u;
    check(&score, gc_generate(&reasoner, &lexicon, &request, &scratch, &result) == GC_STATUS_ABSTAIN &&
                    result.kind == GC_KIND_UNKNOWN &&
                    result.abstain_reason == GC_ABSTAIN_NO_EVIDENCE &&
                    result.grounded == 0u,
          "expired memory cannot ground a fresh generated answer");

    mse_init(&conflict_memory, functional, NULL);
    check(&score, mse_add(&conflict_memory, &memory_card,
                          &(sr_fact_t){ S_ALICE, P_STAGE1, S_BOB, 0u },
                          6u, 0u) == MSE_OK &&
                    mse_add(&conflict_memory, &second_card,
                            &(sr_fact_t){ S_ALICE, P_STAGE1, S_CARA, 0u },
                            7u, 0u) == MSE_OK,
          "functional memory alternatives create an explicit conflict");
    request.memory = &conflict_memory;
    request.current_generation = 7u;
    request.query = pattern(S_ALICE, P_STAGE1, S_BOB, 0u);
    check(&score, gc_generate(&reasoner, &lexicon, &request, &scratch, &result) == GC_STATUS_ABSTAIN &&
                    result.kind == GC_KIND_CONTRADICTED &&
                    result.abstain_reason == GC_ABSTAIN_CONFLICT &&
                    result.grounded == 0u,
          "conflicting personal memory blocks fluent selection");

    printf("GEN CORE: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail == 0 ? 0 : 1;
}
