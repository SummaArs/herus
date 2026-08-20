#include "symbolic_planner.h"
#include <string.h>

typedef struct {
    uint8_t count;
    sr_fact_t fact[SP_MAX_STATE_FACTS];
} sp_state_t;

typedef struct {
    const sp_problem_t *problem;
    uint16_t max_nodes;
    uint8_t max_depth;
    uint16_t explored;
    uint8_t path[SP_MAX_PLAN_LENGTH];
    sp_state_t ancestor[SP_MAX_PLAN_LENGTH + 1u];
    sp_plan_result_t *out;
    uint8_t depth_limited;
} sp_search_t;

static int fact_equal(sr_fact_t a, sr_fact_t b)
{
    return a.subject == b.subject && a.predicate == b.predicate &&
           a.object == b.object && a.negated == b.negated;
}

static int opposite(sr_fact_t a, sr_fact_t b)
{
    return a.subject == b.subject && a.predicate == b.predicate &&
           a.object == b.object && a.negated != b.negated;
}

static int valid_fact(sr_fact_t fact)
{
    return fact.negated == 0u || fact.negated == 1u;
}

static int state_index(const sp_state_t *state, sr_fact_t fact)
{
    for (unsigned i = 0u; i < state->count; i++) {
        if (fact_equal(state->fact[i], fact)) return (int)i;
    }
    return -1;
}

static int state_has(const sp_state_t *state, sr_fact_t fact)
{
    return state_index(state, fact) >= 0;
}

static int state_contradictory(const sp_state_t *state)
{
    for (unsigned i = 0u; i < state->count; i++) {
        for (unsigned j = i + 1u; j < state->count; j++) {
            if (opposite(state->fact[i], state->fact[j])) return 1;
        }
    }
    return 0;
}

static int state_equal(const sp_state_t *a, const sp_state_t *b)
{
    if (a->count != b->count) return 0;
    for (unsigned i = 0u; i < a->count; i++) {
        if (!state_has(b, a->fact[i])) return 0;
    }
    return 1;
}

static int state_add(sp_state_t *state, sr_fact_t fact)
{
    if (!valid_fact(fact)) return SP_E_FORMAT;
    if (state_has(state, fact)) return SP_OK;
    if (state->count >= SP_MAX_STATE_FACTS) return SP_E_LIMIT;
    state->fact[state->count++] = fact;
    return SP_OK;
}

static void state_remove(sp_state_t *state, sr_fact_t fact)
{
    int index = state_index(state, fact);
    if (index < 0) return;
    state->fact[index] = state->fact[state->count - 1u];
    state->count--;
}

static int valid_action(const sp_action_t *action)
{
    if (!action || action->precondition_count > SP_MAX_PRECONDITIONS ||
        action->add_count > SP_MAX_EFFECTS ||
        action->delete_count > SP_MAX_EFFECTS ||
        action->requires_confirmation > 1u) return 0;
    for (unsigned i = 0u; i < action->precondition_count; i++)
        if (!valid_fact(action->precondition[i])) return 0;
    for (unsigned i = 0u; i < action->add_count; i++)
        if (!valid_fact(action->add[i])) return 0;
    for (unsigned i = 0u; i < action->delete_count; i++)
        if (!valid_fact(action->delete_fact[i])) return 0;
    return 1;
}

static int valid_problem(const sp_problem_t *problem)
{
    if (!problem || problem->initial_count > SP_MAX_STATE_FACTS ||
        problem->action_count > SP_MAX_ACTIONS || !valid_fact(problem->goal))
        return 0;
    for (unsigned i = 0u; i < problem->initial_count; i++)
        if (!valid_fact(problem->initial[i])) return 0;
    for (unsigned i = 0u; i < problem->action_count; i++)
        if (!valid_action(&problem->action[i])) return 0;
    return 1;
}

static int action_applicable(const sp_state_t *state, const sp_action_t *action)
{
    for (unsigned i = 0u; i < action->precondition_count; i++) {
        if (!state_has(state, action->precondition[i])) return 0;
    }
    return 1;
}

static int apply_action(const sp_state_t *state, const sp_action_t *action,
                        sp_state_t *out)
{
    *out = *state;
    for (unsigned i = 0u; i < action->delete_count; i++)
        state_remove(out, action->delete_fact[i]);
    for (unsigned i = 0u; i < action->add_count; i++) {
        if (state_add(out, action->add[i]) != SP_OK) return SP_E_LIMIT;
    }
    return state_contradictory(out) ? SP_E_CONTRADICTION : SP_OK;
}

static int search(sp_search_t *searcher, const sp_state_t *state,
                  uint8_t depth, uint16_t cost, uint8_t confirmations)
{
    if (searcher->explored >= searcher->max_nodes) return SP_E_LIMIT;
    searcher->explored++;
    if (state_has(state, searcher->problem->goal)) {
        searcher->out->status = SP_OK;
        searcher->out->plan_length = depth;
        searcher->out->confirmation_count = confirmations;
        searcher->out->cost = cost;
        for (unsigned i = 0u; i < depth; i++)
            searcher->out->action_id[i] = searcher->path[i];
        return SP_OK;
    }
    if (depth >= searcher->max_depth || depth >= SP_MAX_PLAN_LENGTH) {
        searcher->depth_limited = 1u;
        return SP_NO_PLAN;
    }

    for (unsigned i = 0u; i < searcher->problem->action_count; i++) {
        const sp_action_t *action = &searcher->problem->action[i];
        sp_state_t next;
        int seen = 0;
        if (!action_applicable(state, action)) continue;
        if (apply_action(state, action, &next) != SP_OK) continue;
        for (unsigned ancestor = 0u; ancestor <= depth; ancestor++) {
            if (state_equal(&next, &searcher->ancestor[ancestor])) {
                seen = 1;
                break;
            }
        }
        if (seen) continue;
        searcher->path[depth] = action->id;
        searcher->ancestor[depth + 1u] = next;
        {
            int result = search(searcher, &next, (uint8_t)(depth + 1u),
                                (uint16_t)(cost + action->cost),
                                (uint8_t)(confirmations +
                                          (action->requires_confirmation ? 1u : 0u)));
            if (result == SP_OK) return SP_OK;
            if (result == SP_E_LIMIT && searcher->explored >= searcher->max_nodes)
                return SP_E_LIMIT;
        }
    }
    return SP_NO_PLAN;
}

int sp_plan(const sp_problem_t *problem, uint16_t max_nodes,
            uint8_t max_depth, sp_plan_result_t *out)
{
    sp_search_t searcher;
    sp_state_t initial;
    int result;
    if (!out || !valid_problem(problem) || max_nodes == 0u || max_depth == 0u)
        return SP_E_ARG;
    memset(out, 0, sizeof(*out));
    out->status = SP_NO_PLAN;
    memset(&initial, 0, sizeof(initial));
    for (unsigned i = 0u; i < problem->initial_count; i++) {
        if (state_add(&initial, problem->initial[i]) != SP_OK) {
            out->status = SP_E_LIMIT;
            return SP_E_LIMIT;
        }
    }
    if (state_contradictory(&initial)) {
        out->status = SP_E_CONTRADICTION;
        return SP_E_CONTRADICTION;
    }
    memset(&searcher, 0, sizeof(searcher));
    searcher.problem = problem;
    searcher.max_nodes = max_nodes;
    searcher.max_depth = max_depth;
    searcher.out = out;
    searcher.ancestor[0] = initial;
    result = search(&searcher, &initial, 0u, 0u, 0u);
    out->explored_nodes = searcher.explored;
    if (result == SP_OK) return SP_OK;
    if (searcher.explored >= max_nodes || searcher.depth_limited) {
        out->status = SP_E_LIMIT;
        return SP_E_LIMIT;
    }
    out->status = SP_NO_PLAN;
    return SP_NO_PLAN;
}
