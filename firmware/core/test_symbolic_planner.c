#include "symbolic_planner.h"
#include <stdio.h>
#include <string.h>

#define F_IDLE       1u
#define F_FOCUS      2u
#define F_NOTE       3u
#define F_RESPONSE   4u
#define P_STATE      10u
#define O_SELF       20u
#define O_TRUE       21u

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static sr_fact_t fact(uint16_t object)
{
    return (sr_fact_t){ O_SELF, P_STATE, object, 0u };
}

static sp_action_t action(uint8_t id, uint16_t need, uint16_t add,
                          uint16_t remove, uint16_t cost,
                          uint8_t confirmation)
{
    sp_action_t a;
    memset(&a, 0, sizeof(a));
    a.id = id;
    a.requires_confirmation = confirmation;
    a.precondition_count = 1u;
    a.add_count = 1u;
    a.delete_count = remove == 0u ? 0u : 1u;
    a.cost = cost;
    a.precondition[0] = fact(need);
    a.add[0] = fact(add);
    if (remove != 0u) a.delete_fact[0] = fact(remove);
    return a;
}

int main(void)
{
    sp_problem_t problem;
    sp_plan_result_t result;
    score_t score = { 0, 0 };

    memset(&problem, 0, sizeof(problem));
    problem.initial_count = 1u;
    problem.initial[0] = fact(F_IDLE);
    problem.goal = fact(F_RESPONSE);
    problem.action_count = 3u;
    problem.action[0] = action(10u, F_IDLE, F_FOCUS, F_IDLE, 2u, 0u);
    problem.action[1] = action(11u, F_FOCUS, F_NOTE, F_FOCUS, 3u, 1u);
    problem.action[2] = action(12u, F_NOTE, F_RESPONSE, F_NOTE, 4u, 0u);

    check(&score, sp_plan(&problem, 32u, 8u, &result) == SP_OK,
          "planner generates a bounded path to a symbolic goal");
    check(&score, result.plan_length == 3u && result.action_id[0] == 10u &&
                    result.action_id[1] == 11u && result.action_id[2] == 12u,
          "generated plan preserves causal action order");
    check(&score, result.cost == 9u && result.confirmation_count == 1u,
          "plan exposes cost and physical-confirmation boundary");
    check(&score, result.explored_nodes > 0u,
          "planner reports explored search nodes");

    problem.action_count = 4u;
    problem.action[3] = action(13u, F_FOCUS, F_FOCUS, 0u, 1u, 0u);
    check(&score, sp_plan(&problem, 32u, 8u, &result) == SP_OK &&
                    result.plan_length == 3u,
          "a self-loop is ignored without changing the valid plan");

    problem.goal = fact(99u);
    check(&score, sp_plan(&problem, 32u, 8u, &result) == SP_NO_PLAN &&
                    result.status == SP_NO_PLAN,
          "an unreachable goal is reported as no plan, not hallucinated");

    problem.goal = fact(F_RESPONSE);
    check(&score, sp_plan(&problem, 1u, 8u, &result) == SP_E_LIMIT &&
                    result.explored_nodes == 1u,
          "node budget prevents unbounded combinatorial search");

    memset(&problem, 0, sizeof(problem));
    problem.initial_count = 2u;
    problem.initial[0] = fact(F_IDLE);
    problem.initial[1] = (sr_fact_t){ O_SELF, P_STATE, F_IDLE, 1u };
    problem.goal = fact(F_RESPONSE);
    check(&score, sp_plan(&problem, 32u, 8u, &result) == SP_E_CONTRADICTION &&
                    result.status == SP_E_CONTRADICTION,
          "contradictory initial state blocks plan generation");

    printf("SYMBOLIC PLANNER: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
