/*
 * HERUS symbolic_reasoner — bounded local generative inference.
 *
 * This is not a language model and it does not pretend to be one. It generates
 * new typed propositions by applying caller-owned rules to ground facts, records
 * the derivation, detects contradiction and abstains on ambiguity or limits.
 * It never allocates, performs I/O, stores text/audio, opens a radio or grants
 * authority. A response is an explanation candidate, not an action.
 */
#ifndef HERUS_SYMBOLIC_REASONER_H
#define HERUS_SYMBOLIC_REASONER_H

#include <stdint.h>
#include <stddef.h>

#define SR_MAX_FACTS       96u
#define SR_MAX_RULES       48u
#define SR_MAX_PREMISES     4u
#define SR_MAX_VARIABLES    8u
#define SR_MAX_PARENTS      4u
#define SR_MAX_DEPTH       16u
#define SR_MAX_QUERY_HITS   8u

#define SR_TERM_CONSTANT    0u
#define SR_TERM_VARIABLE    1u

#define SR_VAR(id) ((sr_term_t){ (uint16_t)(id), SR_TERM_VARIABLE })
#define SR_CONST(id) ((sr_term_t){ (uint16_t)(id), SR_TERM_CONSTANT })

/* Rule and fact status are intentionally small and stable for firmware ABI use. */
enum {
    SR_OK = 0,
    SR_NO_CHANGE = 1,
    SR_E_ARG = -1,
    SR_E_FULL = -2,
    SR_E_FORMAT = -3,
    SR_E_LIMIT = -4,
    SR_E_CONTRADICTION = -5,
    SR_E_AMBIGUOUS = -6,
    SR_E_NO_EVIDENCE = -7
};

typedef struct {
    uint16_t value;
    uint8_t kind;
} sr_term_t;

typedef struct {
    uint16_t subject;
    uint16_t predicate;
    uint16_t object;
    uint8_t negated;
} sr_fact_t;

typedef struct {
    sr_term_t subject;
    sr_term_t predicate;
    sr_term_t object;
    uint8_t negated;
} sr_pattern_t;

typedef struct {
    uint8_t id;
    uint8_t premise_count;
    sr_pattern_t premise[SR_MAX_PREMISES];
    sr_pattern_t conclusion;
    uint16_t cost;
} sr_rule_t;

typedef enum {
    SR_ORIGIN_INPUT = 0,
    SR_ORIGIN_RULE = 1
} sr_origin_t;

typedef struct {
    uint8_t origin;
    uint8_t depth;
    uint8_t rule_id;
    uint8_t parent_count;
    uint8_t parent[SR_MAX_PARENTS];
    uint16_t derivation_cost;
} sr_fact_meta_t;

typedef enum {
    SR_ANSWER_NONE = 0,
    SR_ANSWER_DIRECT,
    SR_ANSWER_DERIVED,
    SR_ANSWER_ABSENT,
    SR_ANSWER_CONTRADICTED,
    SR_ANSWER_AMBIGUOUS,
    SR_ANSWER_LIMIT
} sr_answer_kind_t;

typedef enum {
    SR_ABDUCTION_NONE = 0,
    SR_ABDUCTION_FOUND,
    SR_ABDUCTION_AMBIGUOUS,
    SR_ABDUCTION_LIMIT,
    SR_ABDUCTION_E_ARG
} sr_abduction_status_t;

typedef struct {
    sr_abduction_status_t status;
    sr_fact_t missing_fact;
    uint8_t rule_id;
    uint8_t missing_premise;
    uint8_t supporting_count;
    uint16_t derivation_cost;
    uint32_t candidates_examined;
} sr_abduction_t;

typedef struct {
    sr_answer_kind_t kind;
    sr_fact_t fact;
    uint8_t fact_index;
    uint8_t hit_count;
    uint8_t depth;
    uint8_t rule_id;
    uint16_t derivation_cost;
    uint8_t evidence_count;
    uint8_t evidence[SR_MAX_PARENTS];
} sr_answer_t;

typedef struct {
    sr_fact_t facts[SR_MAX_FACTS];
    sr_fact_meta_t meta[SR_MAX_FACTS];
    sr_rule_t rules[SR_MAX_RULES];
    uint8_t fact_count;
    uint8_t rule_count;
    uint8_t contradiction_count;
    uint8_t saturation_truncated;
    uint32_t derivation_steps;
} sr_reasoner_t;

void sr_init(sr_reasoner_t *r);

int sr_add_fact(sr_reasoner_t *r, sr_fact_t fact);
int sr_add_rule(sr_reasoner_t *r, const sr_rule_t *rule);

/* Apply rules until a fixed point or the caller's step budget is exhausted. */
int sr_saturate(sr_reasoner_t *r, uint32_t max_steps);

/* Query is exact for ground patterns and conservative for variable patterns. */
int sr_query(const sr_reasoner_t *r, const sr_pattern_t *query,
             sr_answer_t *out);

/* Find one missing ground fact that would make a ground goal derivable.
 * This is a proposal only: it never mutates the reasoner. Multiple valid
 * explanations return SR_ABDUCTION_AMBIGUOUS instead of an arbitrary guess. */
sr_abduction_status_t sr_abduce(const sr_reasoner_t *r,
                                const sr_pattern_t *ground_goal,
                                uint32_t max_candidates,
                                sr_abduction_t *out);

unsigned sr_fact_count(const sr_reasoner_t *r);
unsigned sr_rule_count(const sr_reasoner_t *r);
unsigned sr_contradiction_count(const sr_reasoner_t *r);

#endif /* HERUS_SYMBOLIC_REASONER_H */
