/* memory_policy.h — selective-memory relevance contract for the HERUS Nucleus.
 *
 * This module is deliberately a policy, not a recorder, ASR engine, LLM, database
 * or network client. It accepts only compact, typed signals produced during an
 * explicitly authorised memory session. It has no field for audio, transcript,
 * embedding, identity, location, key, message content or model prompt.
 *
 * Its result is advisory: it may mark a semantic candidate as discard, review or
 * auto-eligible. It cannot persist, send, query a model or alter a user record.
 */
#ifndef HERUS_MEMORY_POLICY_H
#define HERUS_MEMORY_POLICY_H

#include <stdint.h>

#define MEMORY_POLICY_MIN_CONFIDENCE_PCT 70u
#define MEMORY_POLICY_REVIEW_SCORE       60u
#define MEMORY_POLICY_AUTO_SCORE         80u

typedef enum {
    MEMORY_KIND_NONE = 0,
    MEMORY_KIND_IDEA,
    MEMORY_KIND_DECISION,
    MEMORY_KIND_COMMITMENT,
    MEMORY_KIND_PREFERENCE,
    MEMORY_KIND_PROJECT_FACT,
    MEMORY_KIND_ROUTINE,
    MEMORY_KIND_COUNT
} memory_kind_t;

typedef enum {
    MEMORY_SCOPE_NONE = 0,
    MEMORY_SCOPE_SELF,
    MEMORY_SCOPE_THIRD_PARTY,
    MEMORY_SCOPE_MIXED,
    MEMORY_SCOPE_COUNT
} memory_scope_t;

typedef enum {
    MEMORY_SENSITIVITY_NONE = 0,
    MEMORY_SENSITIVITY_ORDINARY,
    MEMORY_SENSITIVITY_PERSONAL,
    MEMORY_SENSITIVITY_SENSITIVE,
    MEMORY_SENSITIVITY_COUNT
} memory_sensitivity_t;

/* Typed extractor output only. Percentages are bounded to 0..100. `session_authorized`
 * and `explicit_remember` are canonical booleans: only exactly 1 means true. */
typedef struct {
    uint8_t              session_authorized;
    uint8_t              explicit_remember;
    memory_kind_t        kind;
    memory_scope_t       scope;
    memory_sensitivity_t sensitivity;
    uint8_t              confidence_pct;
    uint8_t              novelty_pct;
    uint8_t              future_value_pct;
    uint8_t              consequence_pct;
} memory_signal_t;

typedef enum {
    MEMORY_DISPOSITION_DISCARD = 0,
    MEMORY_DISPOSITION_REVIEW,
    MEMORY_DISPOSITION_AUTO_ELIGIBLE
} memory_disposition_t;

enum {
    MEMORY_REASON_NONE          = 0u,
    MEMORY_REASON_NOT_AUTHORIZED = 1u << 0,
    MEMORY_REASON_LOW_CONFIDENCE = 1u << 1,
    MEMORY_REASON_LOW_RELEVANCE  = 1u << 2,
    MEMORY_REASON_EXPLICIT       = 1u << 3,
    MEMORY_REASON_DECISIONAL     = 1u << 4,
    MEMORY_REASON_FUTURE_VALUE   = 1u << 5,
    MEMORY_REASON_NOVEL          = 1u << 6,
    MEMORY_REASON_CONSEQUENTIAL  = 1u << 7,
    MEMORY_REASON_SENSITIVE      = 1u << 8,
    MEMORY_REASON_THIRD_PARTY    = 1u << 9,
    MEMORY_REASON_AMBIGUOUS      = 1u << 10
};

typedef struct {
    memory_disposition_t disposition;
    uint8_t              relevance_score; /* 0..100; not a truth score */
    uint32_t             reasons;         /* OR of MEMORY_REASON_* */
} memory_assessment_t;

enum {
    MEMORY_POLICY_OK = 0,
    MEMORY_POLICY_E_ARG = -1,
    MEMORY_POLICY_E_FORMAT = -2,
    MEMORY_POLICY_E_NOT_AUTHORIZED = -3
};

/* Assess relevance without retaining any input. A result of AUTO_ELIGIBLE is not
 * persistence: later lifecycle code must still provide a reversible write path,
 * provenance, expiry and user controls. Personal-sensitive or third-party data is
 * always REVIEW, even when explicit and highly relevant. */
int memory_policy_assess(const memory_signal_t *signal, memory_assessment_t *out);

#endif /* HERUS_MEMORY_POLICY_H */
