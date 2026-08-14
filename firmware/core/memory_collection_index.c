/* memory_collection_index.c — bounded private retrieval over an authenticated collection. */
#include "memory_collection_index.h"
#include "memory_collection_private.h"
#include "crypto.h"
#include <string.h>

static int access_valid(const memory_collection_access_t *access)
{
    return access && access->physical_session_id != 0u &&
           access->physical_confirmed == 1u;
}

static int query_valid(const memory_retrieval_query_t *query)
{
    if (!query || query->preferred_kind >= MEMORY_KIND_COUNT ||
        query->preferred_origin >= MEMORY_EXTRACT_ORIGIN_COUNT ||
        (query->require_explicit != 0u && query->require_explicit != 1u) ||
        query->minimum_confidence_pct > 100u)
        return 0;
    return query->preferred_kind != MEMORY_KIND_NONE ||
           query->preferred_origin != MEMORY_EXTRACT_ORIGIN_NONE ||
           query->require_explicit == 1u || query->minimum_confidence_pct != 0u;
}

static int config_valid(const memory_collection_index_config_t *cfg)
{
    return cfg && cfg->max_queries_per_session >= 1u &&
           cfg->max_queries_per_session <= MEMORY_COLLECTION_INDEX_MAX_QUERIES_PER_SESSION;
}

void memory_collection_index_config_default(memory_collection_index_config_t *cfg)
{
    if (!cfg) return;
    cfg->max_queries_per_session = MEMORY_COLLECTION_INDEX_DEFAULT_MAX_QUERIES_PER_SESSION;
}

int memory_collection_index_init(memory_collection_index_t *index,
                                 const memory_collection_index_config_t *cfg)
{
    if (!index || !cfg) return MEMORY_COLLECTION_INDEX_E_ARG;
    memset(index, 0, sizeof(*index));
    if (!config_valid(cfg)) return MEMORY_COLLECTION_INDEX_E_CONFIG;
    index->cfg = *cfg;
    memory_retrieval_init(&index->retrieval);
    index->state = MEMORY_COLLECTION_INDEX_READY;
    return MEMORY_COLLECTION_INDEX_OK;
}

int memory_collection_index_query(memory_collection_index_t *index,
                                  memory_collection_t *collection,
                                  const memory_collection_access_t *access,
                                  const memory_retrieval_query_t *query,
                                  memory_retrieval_result_t *out)
{
    memory_vault_card_t cards[MEMORY_COLLECTION_MAX_CARDS];
    uint8_t card_count = 0u;
    int rc;

    if (!index || !collection || !access || !query || !out)
        return MEMORY_COLLECTION_INDEX_E_ARG;
    memset(out, 0, sizeof(*out));
    memset(cards, 0, sizeof(cards));
    if (index->state != MEMORY_COLLECTION_INDEX_READY)
        return MEMORY_COLLECTION_INDEX_E_STATE;
    if (!access_valid(access)) {
        index->metrics.rejected_access++;
        return MEMORY_COLLECTION_INDEX_E_ACCESS;
    }
    if (!query_valid(query)) {
        index->metrics.rejected_query++;
        return MEMORY_COLLECTION_INDEX_E_QUERY;
    }
    if (index->active_physical_session_id != access->physical_session_id) {
        index->active_physical_session_id = access->physical_session_id;
        index->queries_in_active_session = 0u;
    }
    if (index->queries_in_active_session >= index->cfg.max_queries_per_session) {
        index->metrics.rejected_budget++;
        return MEMORY_COLLECTION_INDEX_E_BUDGET;
    }

    /* A valid typed query consumes exactly one budget slot whether it finds a card
     * or abstains. This bounds repeated probing within the same physical session. */
    index->queries_in_active_session++;
    index->metrics.queries++;
    rc = memory_collection_copy_cards_for_index(collection, access, cards, &card_count);
    if (rc != MEMORY_COLLECTION_OK) {
        secure_zero(cards, sizeof(cards));
        index->metrics.rejected_collection++;
        index->state = MEMORY_COLLECTION_INDEX_BLOCKED;
        return MEMORY_COLLECTION_INDEX_E_COLLECTION;
    }
    if (card_count == 0u) {
        out->status = MEMORY_RETRIEVAL_NO_MATCH;
        index->metrics.no_match++;
        secure_zero(cards, sizeof(cards));
        return MEMORY_COLLECTION_INDEX_OK;
    }

    rc = memory_retrieval_query(&index->retrieval, access, query, cards, card_count, out);
    secure_zero(cards, sizeof(cards));
    if (rc != MEMORY_RETRIEVAL_OK) {
        memset(out, 0, sizeof(*out));
        index->metrics.rejected_collection++;
        index->state = MEMORY_COLLECTION_INDEX_BLOCKED;
        return MEMORY_COLLECTION_INDEX_E_COLLECTION;
    }
    if (out->status == MEMORY_RETRIEVAL_MATCH)
        index->metrics.matches++;
    else if (out->status == MEMORY_RETRIEVAL_AMBIGUOUS)
        index->metrics.ambiguous++;
    else
        index->metrics.no_match++;
    return MEMORY_COLLECTION_INDEX_OK;
}

const memory_collection_index_metrics_t *memory_collection_index_metrics(
    const memory_collection_index_t *index)
{
    return index ? &index->metrics : 0;
}
