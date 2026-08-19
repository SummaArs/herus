/*
 * HERUS symbolic_planner — bounded generative plan search.
 *
 * It generates a sequence of typed actions from a finite symbolic state. It does
 * not execute actions, transmit, persist memory or treat a low-cost plan as an
 * authorization. Actions that would require a human gesture remain marked in the
 * returned plan for the caller to confirm.
 */
#ifndef HERUS_SYMBOLIC_PLANNER_H
#define HERUS_SYMBOLIC_PLANNER_H

#include "symbolic_reasoner.h"

#define SP_MAX_ACTIONS       24u
#define SP_MAX_PRECONDITIONS  4u
#define SP_MAX_EFFECTS        4u
#define SP_MAX_PLAN_LENGTH   12u
#define SP_MAX_NODES         128u
#define SP_MAX_STATE_FACTS   32u

enum {
    SP_OK = 0,
    SP_NO_PLAN = 1,
    SP_E_ARG = -1,
    SP_E_LIMIT = -2,
    SP_E_FORMAT = -3,
    SP_E_CONTRADICTION = -4
};

typedef struct {
    uint8_t id;
    uint8_t requires_confirmation;
    uint8_t precondition_count;
    uint8_t add_count;
    uint8_t delete_count;
    uint16_t cost;
    sr_fact_t precondition[SP_MAX_PRECONDITIONS];
    sr_fact_t add[SP_MAX_EFFECTS];
    sr_fact_t delete_fact[SP_MAX_EFFECTS];
} sp_action_t;

typedef struct {
    uint8_t initial_count;
    sr_fact_t initial[SP_MAX_STATE_FACTS];
    uint8_t action_count;
    sp_action_t action[SP_MAX_ACTIONS];
    sr_fact_t goal;
} sp_problem_t;

typedef struct {
    int status;
    uint8_t plan_length;
    uint8_t confirmation_count;
    uint16_t cost;
    uint16_t explored_nodes;
    uint8_t action_id[SP_MAX_PLAN_LENGTH];
} sp_plan_result_t;

int sp_plan(const sp_problem_t *problem, uint16_t max_nodes,
            uint8_t max_depth, sp_plan_result_t *out);

#endif /* HERUS_SYMBOLIC_PLANNER_H */
