/* generative_core.h — bounded hybrid local generation.
 *
 * This layer composes typed symbolic facts, bounded inference and finite plans
 * into a presentable local candidate. It is not a hosted model, does not store
 * raw input, does not mutate the caller's reasoner and never executes authority.
 */
#ifndef HERUS_GENERATIVE_CORE_H
#define HERUS_GENERATIVE_CORE_H

#include "memory_reasoning_bridge.h"
#include "personal_adapter.h"
#include "symbolic_planner.h"
#include <stddef.h>
#include <stdint.h>

#define GC_MAX_RESPONSE_BYTES 256u
#define GC_MAX_LEXEM_BYTES     32u
#define GC_MAX_LEXEM_ENTRIES   32u
#define GC_MAX_EVIDENCE_ROOTS   4u
#define GC_PERSONAL_FEATURE_RESPONSE_STYLE 1u

/* The generator operates on typed requests; a language adapter remains outside. */
typedef enum {
    GC_MODE_ANSWER = 0,
    GC_MODE_EXPLAIN,
    GC_MODE_COUNTERFACTUAL,
    GC_MODE_PLAN
} gc_mode_t;

typedef enum {
    GC_STATUS_OK = 0,
    GC_STATUS_ABSTAIN = 1,
    GC_STATUS_LIMIT = 2,
    GC_E_ARG = -1,
    GC_E_FORMAT = -2,
    GC_E_OUTPUT = -3
} gc_status_t;

typedef enum {
    GC_KIND_NONE = 0,
    GC_KIND_DIRECT,
    GC_KIND_DERIVED,
    GC_KIND_COMPOSED,
    GC_KIND_COUNTERFACTUAL,
    GC_KIND_PLAN,
    GC_KIND_UNKNOWN,
    GC_KIND_AMBIGUOUS,
    GC_KIND_CONTRADICTED,
    GC_KIND_UNSUPPORTED,
    GC_KIND_LIMIT,
    GC_KIND_POLICY_BLOCKED
} gc_kind_t;

typedef enum {
    GC_ABSTAIN_NONE = 0,
    GC_ABSTAIN_NO_EVIDENCE,
    GC_ABSTAIN_AMBIGUITY,
    GC_ABSTAIN_CONFLICT,
    GC_ABSTAIN_UNSUPPORTED,
    GC_ABSTAIN_BUDGET,
    GC_ABSTAIN_POLICY,
    GC_ABSTAIN_NO_PLAN
} gc_abstain_reason_t;

typedef enum {
    GC_AUTH_NONE = 0,
    GC_AUTH_PRESENTATION_ONLY,
    GC_AUTH_CONFIRMATION_REQUIRED
} gc_authority_state_t;

typedef struct {
    sr_symbol_t symbol;
    const char *text;
    uint8_t length;
} gc_lexeme_t;

typedef struct {
    const gc_lexeme_t *entries;
    size_t count;
} gc_lexicon_t;

typedef struct {
    gc_mode_t mode;
    sr_pattern_t query;
    const sp_problem_t *plan_problem;
    uint32_t derivation_budget;
    uint16_t max_plan_nodes;
    uint8_t max_plan_depth;
    uint8_t policy_blocked;
    const mse_index_t *memory;
    uint32_t current_generation;
    const pa_profile_t *personal_profile;
} gc_request_t;

typedef struct {
    gc_status_t status;
    gc_kind_t kind;
    gc_abstain_reason_t abstain_reason;
    gc_authority_state_t authority;
    uint8_t requires_confirmation;
    uint8_t counterfactual;
    uint8_t grounded;
    uint8_t adapted;
    uint8_t lexeme_missing;
    uint8_t evidence_count;
    uint8_t evidence[GC_MAX_EVIDENCE_ROOTS];
    uint32_t derivation_digest;
    uint16_t response_length;
    char response[GC_MAX_RESPONSE_BYTES];
    sr_answer_t answer;
    sr_abduction_t abduction;
    sp_plan_result_t plan;
    mrb_meta_t composition;
    pa_prediction_t adaptation;
} gc_result_t;

/* Compose a local candidate from a caller-owned reasoner and bounded request.
 * `scratch` is overwritten with a private working copy; `base` is unchanged.
 * No output path executes, transmits, persists or grants authority. */
gc_status_t gc_generate(const sr_reasoner_t *base,
                        const gc_lexicon_t *lexicon,
                        const gc_request_t *request,
                        sr_reasoner_t *scratch,
                        gc_result_t *out);

#endif /* HERUS_GENERATIVE_CORE_H */
