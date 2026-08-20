/*
 * HERUS semantic_compiler — bounded controlled-natural-language compiler.
 *
 * The input is inspected transiently and the output contains only typed numeric
 * meaning. No input string, transcript, audio, embedding or authority is stored.
 * This first language is deliberately controlled; unsupported free language
 * returns an explicit error instead of a guessed interpretation.
 */
#ifndef HERUS_SEMANTIC_COMPILER_H
#define HERUS_SEMANTIC_COMPILER_H

#include "symbolic_reasoner.h"
#include "symbolic_dialogue.h"
#include "symbolic_planner.h"
#include <stdint.h>
#include <stddef.h>

#define SC_MAX_INPUT_BYTES 192u
#define SC_MAX_TOKENS       24u
#define SC_MAX_LEXEM        32u

#define SC_TERM_VARIABLE    1u
#define SC_TERM_CONSTANT    0u

#define SC_VAR(id) ((sr_term_t){ (uint16_t)(id), SC_TERM_VARIABLE })
#define SC_CONST(id) ((sr_term_t){ (uint16_t)(id), SC_TERM_CONSTANT })

typedef enum {
    SC_UNIT_NONE = 0,
    SC_UNIT_FACT,
    SC_UNIT_QUERY,
    SC_UNIT_RULE,
    SC_UNIT_GOAL,
    SC_UNIT_REJECT
} sc_unit_kind_t;

typedef enum {
    SC_OK = 0,
    SC_E_ARG = -1,
    SC_E_EMPTY = -2,
    SC_E_TOO_LONG = -3,
    SC_E_TOKEN = -4,
    SC_E_SYNTAX = -5,
    SC_E_UNSUPPORTED = -6,
    SC_E_LIMIT = -7,
    SC_E_SENSITIVE = -8
} sc_status_t;

typedef struct {
    int status;
    sc_unit_kind_t kind;
    uint8_t token_count;
    uint8_t error_token;
    uint8_t requires_confirmation;
    uint8_t exact_parse;
    uint16_t error_code;
    union {
        sr_fact_t fact;
        sr_pattern_t query;
        sr_rule_t rule;
        sr_fact_t goal;
    } meaning;
} sc_unit_t;

typedef struct {
    int status;
    uint8_t state_changed;
    uint8_t abstained;
    uint8_t confirmation_required;
    uint32_t derivation_budget;
    sd_reply_t reply;
    sp_plan_result_t plan;
} sc_bridge_result_t;

enum {
    SC_BRIDGE_OK = 0,
    SC_BRIDGE_E_ARG = -20,
    SC_BRIDGE_E_AUTH = -21,
    SC_BRIDGE_E_KIND = -22,
    SC_BRIDGE_E_ABSTAIN = -23,
    SC_BRIDGE_E_LIMIT = -24,
    SC_BRIDGE_E_NO_PLAN = -25
};

/* Apply a compiled unit to the local dialogue. Mutations require confirmation. */
int sc_apply_dialogue(sd_dialogue_t *dialogue, const sc_unit_t *unit,
                      uint8_t explicit_confirmation, uint32_t derivation_budget,
                      sc_bridge_result_t *out);

/* Turn a compiled goal into a bounded plan proposal; never executes an action. */
int sc_plan_goal(const sc_unit_t *unit, const sp_problem_t *catalog,
                 uint16_t max_nodes, uint8_t max_depth,
                 sc_bridge_result_t *out);

/* Stable role-independent symbol id; the lexeme is never retained in output. */
uint16_t sc_symbol_id(const char *text, size_t length);

/* Compile a bounded controlled Portuguese utterance into typed IR. */
int sc_compile(const char *input, size_t length, sc_unit_t *out);

#endif /* HERUS_SEMANTIC_COMPILER_H */
