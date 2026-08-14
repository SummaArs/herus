#include "memory_collection_index.h"
#include <stdio.h>
#include <string.h>

#define TEST_COLLECTION_ID 0x48434958u /* HCIX, non-secret test context. */
#define TEST_TAG_OFFSET 252u

typedef struct {
    uint8_t prepared[MEMORY_COLLECTION_BLOB_LEN];
    uint8_t committed[MEMORY_COLLECTION_BLOB_LEN];
    uint8_t has_prepared;
    uint8_t has_committed;
    uint32_t floor;
} fake_collection_store_t;

static uint8_t FIXTURE_ROOT[MEMORY_COLLECTION_ROOT_LEN];
static int FAILED = 0;

int memory_collection_platform_load_root(uint32_t collection_id,
                                         uint8_t out[MEMORY_COLLECTION_ROOT_LEN])
{
    if (collection_id != TEST_COLLECTION_ID) return -1;
    memcpy(out, FIXTURE_ROOT, MEMORY_COLLECTION_ROOT_LEN);
    return 0;
}

static int store_prepared(void *ctx, const uint8_t blob[MEMORY_COLLECTION_BLOB_LEN])
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    memcpy(s->prepared, blob, sizeof(s->prepared)); s->has_prepared = 1u; return 0;
}
static int load_prepared(void *ctx, uint8_t blob[MEMORY_COLLECTION_BLOB_LEN])
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    if (!s->has_prepared) return MEMORY_COLLECTION_STORE_ABSENT;
    memcpy(blob, s->prepared, sizeof(s->prepared)); return 0;
}
static int erase_prepared(void *ctx)
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    memset(s->prepared, 0, sizeof(s->prepared)); s->has_prepared = 0u; return 0;
}
static int store_committed(void *ctx, const uint8_t blob[MEMORY_COLLECTION_BLOB_LEN])
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    memcpy(s->committed, blob, sizeof(s->committed)); s->has_committed = 1u; return 0;
}
static int load_committed(void *ctx, uint8_t blob[MEMORY_COLLECTION_BLOB_LEN])
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    if (!s->has_committed) return MEMORY_COLLECTION_STORE_ABSENT;
    memcpy(blob, s->committed, sizeof(s->committed)); return 0;
}
static int erase_committed(void *ctx)
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    memset(s->committed, 0, sizeof(s->committed)); s->has_committed = 0u; return 0;
}
static int load_floor(void *ctx, uint32_t *floor)
{
    *floor = ((fake_collection_store_t *)ctx)->floor; return 0;
}
static int commit_floor(void *ctx, uint32_t generation)
{
    fake_collection_store_t *s = (fake_collection_store_t *)ctx;
    if (generation <= s->floor) return -1;
    s->floor = generation; return 0;
}

static void backend_init(fake_collection_store_t *s)
{
    size_t i;
    memset(s, 0, sizeof(*s));
    for (i = 0u; i < sizeof(FIXTURE_ROOT); ++i) FIXTURE_ROOT[i] = (uint8_t)(0x49u + i);
}

static memory_collection_config_t config_for(fake_collection_store_t *s)
{
    memory_collection_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.collection_id = TEST_COLLECTION_ID;
    cfg.storage.ctx = s;
    cfg.storage.store_prepared = store_prepared;
    cfg.storage.load_prepared = load_prepared;
    cfg.storage.erase_prepared = erase_prepared;
    cfg.storage.store_committed = store_committed;
    cfg.storage.load_committed = load_committed;
    cfg.storage.erase_committed = erase_committed;
    cfg.storage.load_generation_floor = load_floor;
    cfg.storage.commit_generation_floor = commit_floor;
    return cfg;
}

static memory_vault_card_t card_for(uint32_t card_id, uint32_t receipt,
                                    memory_kind_t kind)
{
    memory_vault_card_t card;
    memset(&card, 0, sizeof(card));
    card.card_id = card_id;
    card.review_receipt_id = receipt;
    card.signal.session_authorized = 1u;
    card.signal.explicit_remember = 1u;
    card.signal.kind = kind;
    card.signal.scope = MEMORY_SCOPE_SELF;
    card.signal.sensitivity = MEMORY_SENSITIVITY_ORDINARY;
    card.signal.confidence_pct = 95u;
    card.signal.novelty_pct = 95u;
    card.signal.future_value_pct = 95u;
    card.signal.consequence_pct = 80u;
    card.origin = MEMORY_EXTRACT_EXPLICIT;
    card.extract_reasons = MEMORY_EXTRACT_REASON_EXPLICIT | MEMORY_EXTRACT_REASON_IDEA;
    return card;
}

static memory_vault_write_authorization_t auth_for(const memory_vault_card_t *card)
{
    memory_vault_write_authorization_t auth;
    auth.card_id = card->card_id;
    auth.review_receipt_id = card->review_receipt_id;
    auth.human_confirmed = 1u;
    return auth;
}

static memory_collection_access_t access_for(memory_physical_session_t *gate,
                                           memory_physical_purpose_t purpose,
                                           uint32_t session)
{
    memory_collection_access_t access;
    uint8_t uses = purpose == MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY ? 3u : 1u;
    (void)memory_physical_session_begin(gate, purpose, session, session ^ 0x55AAu, 1u,
                                        uses, session * 100u);
    access.gate = gate;
    access.physical_session_id = session;
    access.observed_at_ms = session * 100u + 1u;
    return access;
}

static memory_retrieval_query_t kind_query(memory_kind_t kind)
{
    memory_retrieval_query_t query;
    memset(&query, 0, sizeof(query));
    query.preferred_kind = kind;
    return query;
}

static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

int main(void)
{
    fake_collection_store_t store;
    memory_collection_config_t collection_cfg;
    memory_collection_index_config_t index_cfg;
    memory_collection_t collection;
    memory_collection_t bad_collection;
    memory_collection_index_t index;
    memory_collection_index_t bad_index;
    memory_collection_access_t access;
    memory_physical_session_t gate;
    memory_physical_session_config_t gate_cfg;
    memory_vault_card_t card;
    memory_vault_write_authorization_t auth;
    memory_retrieval_query_t query;
    memory_retrieval_result_t result;
    uint32_t opens_before;

    printf("\n== T11 private collection index remains bounded, abstention-safe and zero-authority ==\n");
    backend_init(&store);
    collection_cfg = config_for(&store);
    memory_collection_index_config_default(&index_cfg);
    index_cfg.max_queries_per_session = 2u;
    memory_physical_session_config_default(&gate_cfg);
    gate_cfg.window_ms = 1000u;
    ok(memory_physical_session_init(&gate, &gate_cfg) == MEMORY_PHYSICAL_SESSION_OK,
       "T11 fixture session gate initializes without claiming a real control");
    memset(&access, 0, sizeof(access));
    ok(memory_collection_init(&collection, &collection_cfg) == MEMORY_COLLECTION_OK &&
       memory_collection_index_init(&index, &index_cfg) == MEMORY_COLLECTION_INDEX_OK,
       "T11 collection and portable in-RAM index initialise without a query or card cache");

    card = card_for(101u, 1001u, MEMORY_KIND_IDEA);
    auth = auth_for(&card);
    access = access_for(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 500u);
    ok(memory_collection_insert(&collection, &auth, &card, &access) == MEMORY_COLLECTION_OK,
       "T11 authorised first minimal card enters the encrypted collection");
    query = kind_query(MEMORY_KIND_IDEA);
    access = access_for(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY, 501u);
    opens_before = collection.metrics.opens;
    memset(&result, 0xA5, sizeof(result));
    ok(memory_collection_index_query(&index, &collection, &access, &query, &result) ==
           MEMORY_COLLECTION_INDEX_OK && result.status == MEMORY_RETRIEVAL_MATCH &&
       result.card_id == 101u && result.kind == MEMORY_KIND_IDEA &&
       collection.metrics.opens == opens_before &&
       memory_physical_session_cancel(&gate) == MEMORY_PHYSICAL_SESSION_OK,
       "T11 index returns a minimal opaque match without automatically opening a card");

    card = card_for(102u, 1002u, MEMORY_KIND_DECISION);
    auth = auth_for(&card);
    access = access_for(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 502u);
    ok(memory_collection_insert(&collection, &auth, &card, &access) == MEMORY_COLLECTION_OK,
       "T11 independently authorised second card enters without changing first card");
    query = kind_query(MEMORY_KIND_DECISION);
    access = access_for(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY, 503u);
    ok(memory_collection_index_query(&index, &collection, &access, &query, &result) ==
           MEMORY_COLLECTION_INDEX_OK && result.status == MEMORY_RETRIEVAL_MATCH &&
       result.card_id == 102u && memory_physical_session_cancel(&gate) == MEMORY_PHYSICAL_SESSION_OK,
       "T11 typed kind filter selects only the matching opaque identifier");

    card = card_for(103u, 1003u, MEMORY_KIND_IDEA);
    auth = auth_for(&card);
    access = access_for(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 504u);
    ok(memory_collection_insert(&collection, &auth, &card, &access) == MEMORY_COLLECTION_OK,
       "T11 third independently authorised card creates a genuine same-type contender");
    query = kind_query(MEMORY_KIND_IDEA);
    access = access_for(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY, 505u);
    ok(memory_collection_index_query(&index, &collection, &access, &query, &result) ==
           MEMORY_COLLECTION_INDEX_OK && result.status == MEMORY_RETRIEVAL_AMBIGUOUS &&
           result.card_id == 0u && result.kind == MEMORY_KIND_NONE &&
       result.origin == MEMORY_EXTRACT_ORIGIN_NONE && result.reasons == 0u &&
       memory_physical_session_cancel(&gate) == MEMORY_PHYSICAL_SESSION_OK,
       "T11 close contenders remain ambiguous and never expose a winner");

    query = kind_query(MEMORY_KIND_COMMITMENT);
    access = access_for(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY, 506u);
    ok(memory_collection_index_query(&index, &collection, &access, &query, &result) ==
           MEMORY_COLLECTION_INDEX_OK && result.status == MEMORY_RETRIEVAL_NO_MATCH &&
       result.card_id == 0u && result.reasons == 0u &&
       memory_physical_session_cancel(&gate) == MEMORY_PHYSICAL_SESSION_OK,
       "T11 unmatched typed request abstains without enumerating collection contents");

    query = kind_query(MEMORY_KIND_DECISION);
    access = access_for(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY, 507u);
    ok(memory_collection_index_query(&index, &collection, &access, &query, &result) ==
           MEMORY_COLLECTION_INDEX_OK && memory_collection_index_query(&index, &collection,
           &access, &query, &result) == MEMORY_COLLECTION_INDEX_OK &&
           memory_collection_index_query(&index, &collection, &access, &query, &result) ==
           MEMORY_COLLECTION_INDEX_E_BUDGET && result.card_id == 0u &&
       memory_physical_session_cancel(&gate) == MEMORY_PHYSICAL_SESSION_OK,
       "T11 each physical session has a bounded probe budget and a rejected probe clears output");

    access.gate = 0;
    ok(memory_collection_index_query(&index, &collection, &access, &query, &result) ==
           MEMORY_COLLECTION_INDEX_E_ACCESS && result.card_id == 0u,
       "T11 noncanonical physical access cannot read the private index");
    access = access_for(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY, 508u);
    memset(&query, 0, sizeof(query));
    ok(memory_collection_index_query(&index, &collection, &access, &query, &result) ==
           MEMORY_COLLECTION_INDEX_E_QUERY && result.card_id == 0u,
       "T11 empty query cannot enumerate every card in the collection");

    store.committed[TEST_TAG_OFFSET] ^= 0x40u;
    ok(memory_collection_init(&bad_collection, &collection_cfg) ==
           MEMORY_COLLECTION_E_AUTHENTICITY && bad_collection.state == MEMORY_COLLECTION_BLOCKED &&
       memory_collection_index_init(&bad_index, &index_cfg) == MEMORY_COLLECTION_INDEX_OK &&
       memory_collection_index_query(&bad_index, &bad_collection, &access, &query, &result) ==
           MEMORY_COLLECTION_INDEX_E_QUERY,
       "T11 corrupted collection is blocked before any malformed query can become retrieval");
    query = kind_query(MEMORY_KIND_IDEA);
    ok(memory_collection_index_query(&bad_index, &bad_collection, &access, &query, &result) ==
           MEMORY_COLLECTION_INDEX_E_COLLECTION && bad_index.state == MEMORY_COLLECTION_INDEX_BLOCKED &&
       result.card_id == 0u,
       "T11 collection authenticity failure blocks index rather than returning stale candidates");

    if (FAILED) {
        printf("COLLECTION INDEX INVARIANTS FAILED\n");
        return 1;
    }
    printf("COLLECTION INDEX INVARIANTS HOLD — bounded typed queries stay private, abstention-safe and non-authoritative.\n");
    return 0;
}
