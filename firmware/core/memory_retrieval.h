/* memory_retrieval.h — controlled local retrieval over minimal memory cards.
 *
 * This module ranks a caller-supplied, bounded set of already-authorised vault
 * cards. It is deliberately not a vault client, database, text/voice parser,
 * embedding index, network client, LLM adapter or action dispatcher. It neither
 * reads a root nor writes, erases, sends, logs or retains a card. A future adapter
 * may obtain cards through controlled vault access; that adapter remains outside
 * this pure matching module.
 */
#ifndef HERUS_MEMORY_RETRIEVAL_H
#define HERUS_MEMORY_RETRIEVAL_H

#include <stddef.h>
#include <stdint.h>
#include "memory_consolidation.h"

#define MEMORY_RETRIEVAL_MAX_CARDS 8u
#define MEMORY_RETRIEVAL_MIN_SCORE 60u
#define MEMORY_RETRIEVAL_MIN_MARGIN 10u

typedef enum {
    MEMORY_RETRIEVAL_NO_MATCH = 0,
    MEMORY_RETRIEVAL_MATCH,
    MEMORY_RETRIEVAL_AMBIGUOUS
} memory_retrieval_status_t;

/* `NONE` means no preference for that dimension. At least one criterion must be
 * supplied: a query cannot enumerate all cards. No free-text, audio, transcript,
 * embedding, identity, location, timestamp, network or model field exists. */
typedef struct {
    memory_kind_t           preferred_kind;
    memory_extract_origin_t preferred_origin;
    uint8_t                 require_explicit;   /* exactly 1 or 0 */
    uint8_t                 minimum_confidence_pct;
} memory_retrieval_query_t;

enum {
    MEMORY_RETRIEVAL_REASON_NONE       = 0u,
    MEMORY_RETRIEVAL_REASON_KIND       = 1u << 0,
    MEMORY_RETRIEVAL_REASON_ORIGIN     = 1u << 1,
    MEMORY_RETRIEVAL_REASON_EXPLICIT   = 1u << 2,
    MEMORY_RETRIEVAL_REASON_CONFIDENCE = 1u << 3,
    MEMORY_RETRIEVAL_REASON_NOVELTY    = 1u << 4,
    MEMORY_RETRIEVAL_REASON_FUTURE     = 1u << 5,
    MEMORY_RETRIEVAL_REASON_CONSEQUENCE = 1u << 6
};

/* A presentation is not a factual claim, semantic answer, text summary or action.
 * It identifies only the selected card and why this typed query matched. On no
 * match or ambiguity card_id/kind/origin are zeroed. */
typedef struct {
    memory_retrieval_status_t status;
    uint32_t                  card_id;
    memory_kind_t             kind;
    memory_extract_origin_t   origin;
    uint8_t                   score_pct;
    uint8_t                   runner_up_score_pct;
    uint32_t                  reasons;
} memory_retrieval_result_t;

typedef struct {
    uint32_t queries;
    uint32_t matches;
    uint32_t no_match;
    uint32_t ambiguous;
    uint32_t rejected_access;
    uint32_t rejected_query;
    uint32_t rejected_source;
} memory_retrieval_metrics_t;

typedef struct {
    memory_retrieval_metrics_t metrics;
} memory_retrieval_t;

enum {
    MEMORY_RETRIEVAL_OK        =  0,
    MEMORY_RETRIEVAL_E_ARG     = -1,
    MEMORY_RETRIEVAL_E_ACCESS  = -2,
    MEMORY_RETRIEVAL_E_QUERY   = -3,
    MEMORY_RETRIEVAL_E_SOURCE  = -4,
    MEMORY_RETRIEVAL_E_CAPACITY = -5
};

void memory_retrieval_init(memory_retrieval_t *r);

/* Performs local matching only. `access` must be a canonical physical assertion;
 * it is not retained. Each source must be a safe, policy-compatible vault-card
 * representation. The result is cleared first and never exposes a losing card.
 * A score below threshold returns NO_MATCH; close scores return AMBIGUOUS. */
int memory_retrieval_query(memory_retrieval_t *r,
                           const memory_consolidation_access_t *access,
                           const memory_retrieval_query_t *query,
                           const memory_vault_card_t *cards, size_t card_count,
                           memory_retrieval_result_t *out);

const memory_retrieval_metrics_t *memory_retrieval_metrics(const memory_retrieval_t *r);

#endif /* HERUS_MEMORY_RETRIEVAL_H */
