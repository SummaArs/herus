/* memory_collection_index.h — private, bounded retrieval mediator over a collection.
 *
 * The index is intentionally an in-RAM control plane, not a second persistent
 * database. It asks the collection to decrypt its authenticated current record
 * only during a physically confirmed, typed query; it exposes only the existing
 * minimal retrieval result. It cannot list cards, open a card, write, erase,
 * transmit, parse free text or invoke a model.
 */
#ifndef HERUS_MEMORY_COLLECTION_INDEX_H
#define HERUS_MEMORY_COLLECTION_INDEX_H

#include <stdint.h>
#include "memory_collection.h"
#include "memory_retrieval.h"

/* A session-local query budget limits repeated typed probes in one confirmed
 * physical interaction. A future UI adapter must provide genuine fresh sessions;
 * host code proves only canonical nonzero IDs and confirmation. */
#define MEMORY_COLLECTION_INDEX_DEFAULT_MAX_QUERIES_PER_SESSION 3u
#define MEMORY_COLLECTION_INDEX_MAX_QUERIES_PER_SESSION 8u

typedef enum {
    MEMORY_COLLECTION_INDEX_UNINITIALIZED = 0,
    MEMORY_COLLECTION_INDEX_READY,
    MEMORY_COLLECTION_INDEX_BLOCKED
} memory_collection_index_state_t;

typedef struct {
    uint8_t max_queries_per_session; /* 1..8; default 3 */
} memory_collection_index_config_t;

/* Numeric aggregate counters only. No query, card ID, result property, text,
 * key, nonce, blob, collection context or physical-session ID is retained as
 * product telemetry. */
typedef struct {
    uint32_t queries;
    uint32_t matches;
    uint32_t no_match;
    uint32_t ambiguous;
    uint32_t rejected_access;
    uint32_t rejected_budget;
    uint32_t rejected_collection;
    uint32_t rejected_query;
} memory_collection_index_metrics_t;

typedef struct {
    memory_collection_index_config_t cfg;
    memory_collection_index_state_t state;
    uint32_t active_physical_session_id;
    uint8_t queries_in_active_session;
    memory_retrieval_t retrieval;
    memory_collection_index_metrics_t metrics;
} memory_collection_index_t;

enum {
    MEMORY_COLLECTION_INDEX_OK           =  0,
    MEMORY_COLLECTION_INDEX_E_ARG        = -1,
    MEMORY_COLLECTION_INDEX_E_CONFIG     = -2,
    MEMORY_COLLECTION_INDEX_E_STATE      = -3,
    MEMORY_COLLECTION_INDEX_E_ACCESS     = -4,
    MEMORY_COLLECTION_INDEX_E_BUDGET     = -5,
    MEMORY_COLLECTION_INDEX_E_QUERY      = -6,
    MEMORY_COLLECTION_INDEX_E_COLLECTION = -7
};

void memory_collection_index_config_default(memory_collection_index_config_t *cfg);
int memory_collection_index_init(memory_collection_index_t *index,
                                 const memory_collection_index_config_t *cfg);

/* Evaluates one typed query over the collection's authenticated current record.
 * `out` is cleared before every call. A MATCH contains only opaque card_id plus
 * the minimal status fields from memory_retrieval; it does not open that card.
 * NO_MATCH and AMBIGUOUS retain the existing no-winner rule. Callers must perform
 * a separate memory_collection_open under canonical physical access if they need
 * the selected minimal card. */
int memory_collection_index_query(memory_collection_index_t *index,
                                  memory_collection_t *collection,
                                  const memory_collection_access_t *access,
                                  const memory_retrieval_query_t *query,
                                  memory_retrieval_result_t *out);

const memory_collection_index_metrics_t *memory_collection_index_metrics(
    const memory_collection_index_t *index);

#endif /* HERUS_MEMORY_COLLECTION_INDEX_H */
