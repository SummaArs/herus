/* test_memory_collection.c — adversarial contract for portable multi-card memory. */
#include "memory_collection.h"
#include <stdio.h>
#include <string.h>

#define TEST_COLLECTION_ID 0x48434F4Cu /* HCOL, non-secret local context. */
#define TEST_TAG_OFFSET 252u

typedef struct {
    uint8_t prepared[MEMORY_COLLECTION_BLOB_LEN];
    uint8_t committed[MEMORY_COLLECTION_BLOB_LEN];
    uint8_t has_prepared;
    uint8_t has_committed;
    uint32_t floor;
    int fail_store_prepared;
    int fail_store_committed;
    int fail_erase_prepared;
    int fail_load_floor;
    int fail_commit_floor;
} fake_collection_store_t;

static uint8_t FIXTURE_ROOT[MEMORY_COLLECTION_ROOT_LEN];
static int ROOT_FAIL = 0;
static int FAILED = 0;

int memory_collection_platform_load_root(uint32_t collection_id,
                                         uint8_t out[MEMORY_COLLECTION_ROOT_LEN])
{
    if (ROOT_FAIL || collection_id != TEST_COLLECTION_ID) return -1;
    memcpy(out, FIXTURE_ROOT, MEMORY_COLLECTION_ROOT_LEN);
    return 0;
}

static int fake_store_prepared(void *ctx, const uint8_t blob[MEMORY_COLLECTION_BLOB_LEN])
{
    fake_collection_store_t *f = (fake_collection_store_t *)ctx;
    if (f->fail_store_prepared) return -1;
    memcpy(f->prepared, blob, sizeof(f->prepared));
    f->has_prepared = 1u;
    return 0;
}

static int fake_load_prepared(void *ctx, uint8_t blob[MEMORY_COLLECTION_BLOB_LEN])
{
    fake_collection_store_t *f = (fake_collection_store_t *)ctx;
    if (!f->has_prepared) return MEMORY_COLLECTION_STORE_ABSENT;
    memcpy(blob, f->prepared, sizeof(f->prepared));
    return 0;
}

static int fake_erase_prepared(void *ctx)
{
    fake_collection_store_t *f = (fake_collection_store_t *)ctx;
    if (f->fail_erase_prepared) return -1;
    memset(f->prepared, 0, sizeof(f->prepared));
    f->has_prepared = 0u;
    return 0;
}

static int fake_store_committed(void *ctx, const uint8_t blob[MEMORY_COLLECTION_BLOB_LEN])
{
    fake_collection_store_t *f = (fake_collection_store_t *)ctx;
    if (f->fail_store_committed) return -1;
    memcpy(f->committed, blob, sizeof(f->committed));
    f->has_committed = 1u;
    return 0;
}

static int fake_load_committed(void *ctx, uint8_t blob[MEMORY_COLLECTION_BLOB_LEN])
{
    fake_collection_store_t *f = (fake_collection_store_t *)ctx;
    if (!f->has_committed) return MEMORY_COLLECTION_STORE_ABSENT;
    memcpy(blob, f->committed, sizeof(f->committed));
    return 0;
}

static int fake_erase_committed(void *ctx)
{
    fake_collection_store_t *f = (fake_collection_store_t *)ctx;
    memset(f->committed, 0, sizeof(f->committed));
    f->has_committed = 0u;
    return 0;
}

static int fake_load_floor(void *ctx, uint32_t *out)
{
    fake_collection_store_t *f = (fake_collection_store_t *)ctx;
    if (f->fail_load_floor) return -1;
    *out = f->floor;
    return 0;
}

static int fake_commit_floor(void *ctx, uint32_t generation)
{
    fake_collection_store_t *f = (fake_collection_store_t *)ctx;
    if (f->fail_commit_floor || generation <= f->floor) return -1;
    f->floor = generation;
    return 0;
}

static void backend_init(fake_collection_store_t *f)
{
    size_t i;
    memset(f, 0, sizeof(*f));
    for (i = 0u; i < sizeof(FIXTURE_ROOT); ++i)
        FIXTURE_ROOT[i] = (uint8_t)(0x31u + i);
    ROOT_FAIL = 0;
}

static memory_collection_config_t config_for(fake_collection_store_t *f)
{
    memory_collection_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.collection_id = TEST_COLLECTION_ID;
    cfg.storage.ctx = f;
    cfg.storage.store_prepared = fake_store_prepared;
    cfg.storage.load_prepared = fake_load_prepared;
    cfg.storage.erase_prepared = fake_erase_prepared;
    cfg.storage.store_committed = fake_store_committed;
    cfg.storage.load_committed = fake_load_committed;
    cfg.storage.erase_committed = fake_erase_committed;
    cfg.storage.load_generation_floor = fake_load_floor;
    cfg.storage.commit_generation_floor = fake_commit_floor;
    return cfg;
}

static memory_vault_card_t eligible_card(uint32_t card_id, uint32_t receipt)
{
    memory_vault_card_t card;
    memset(&card, 0, sizeof(card));
    card.card_id = card_id;
    card.review_receipt_id = receipt;
    card.signal.session_authorized = 1u;
    card.signal.explicit_remember = 1u;
    card.signal.kind = MEMORY_KIND_IDEA;
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

static memory_collection_access_t physical_access(memory_physical_session_t *gate,
                                                  memory_physical_purpose_t purpose,
                                                  uint32_t session)
{
    memory_collection_access_t access;
    uint8_t uses = purpose == MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY ? 3u : 1u;
    (void)memory_physical_session_begin(gate, purpose, session, session ^ 0xA55Au,
                                        1u, uses, session * 100u);
    access.gate = gate;
    access.physical_session_id = session;
    access.observed_at_ms = session * 100u + 1u;
    return access;
}

static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

int main(void)
{
    fake_collection_store_t f;
    memory_collection_config_t cfg;
    memory_collection_t c;
    memory_collection_t reopened;
    memory_vault_card_t card;
    memory_vault_card_t out;
    memory_vault_write_authorization_t auth;
    memory_collection_access_t access;
    memory_physical_session_t gate;
    memory_physical_session_config_t gate_cfg;
    uint8_t old_committed[MEMORY_COLLECTION_BLOB_LEN];
    uint32_t old_floor;
    uint8_t i;

    printf("\n== T10 multi-card collection remains human-gated, bounded and transactional ==\n");
    backend_init(&f);
    cfg = config_for(&f);
    memory_physical_session_config_default(&gate_cfg);
    gate_cfg.window_ms = 1000u;
    ok(memory_physical_session_init(&gate, &gate_cfg) == MEMORY_PHYSICAL_SESSION_OK,
       "T10 fixture physical-session gate initializes without a physical-device claim");
    access = physical_access(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 77u);
    ok(memory_collection_init(&c, &cfg) == MEMORY_COLLECTION_OK &&
       c.state == MEMORY_COLLECTION_READY && c.generation == 0u && c.count == 0u,
       "T10 empty collection initialises only from an empty floor and empty records");

    card = eligible_card(101u, 1001u);
    auth = auth_for(&card);
    ok(memory_collection_insert(&c, &auth, &card, &access) == MEMORY_COLLECTION_OK &&
       c.generation == 1u && c.count == 1u && f.floor == 1u &&
       f.has_committed && !f.has_prepared,
       "T10 authorised card is prepared, floor-committed, promoted and cleaned up");
    access = physical_access(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_OPEN, 78u);
    memset(&out, 0xA5, sizeof(out));
    ok(memory_collection_open(&c, 101u, &access, &out) == MEMORY_COLLECTION_OK &&
       out.card_id == 101u && out.review_receipt_id == 1001u,
       "T10 physical access opens exactly one known minimal card");

    access.gate = 0;
    card = eligible_card(102u, 1002u);
    auth = auth_for(&card);
    ok(memory_collection_insert(&c, &auth, &card, &access) == MEMORY_COLLECTION_E_ACCESS &&
       c.count == 1u && c.generation == 1u,
       "T10 noncanonical physical access cannot add a card");
    access = physical_access(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 80u);
    auth.human_confirmed = 0u;
    ok(memory_collection_insert(&c, &auth, &card, &access) == MEMORY_COLLECTION_E_AUTH &&
       c.count == 1u && memory_physical_session_cancel(&gate) == MEMORY_PHYSICAL_SESSION_OK,
       "T10 missing human write authorization cannot be replaced by collection access or silently reused");
    auth = auth_for(&card);
    access = physical_access(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 81u);
    card.signal.sensitivity = MEMORY_SENSITIVITY_SENSITIVE;
    ok(memory_collection_insert(&c, &auth, &card, &access) == MEMORY_COLLECTION_E_CARD &&
       c.count == 1u && memory_physical_session_cancel(&gate) == MEMORY_PHYSICAL_SESSION_OK,
       "T10 sensitive card never crosses the collection boundary automatically or leaves reusable authority");
    card = eligible_card(101u, 2001u);
    auth = auth_for(&card);
    access = physical_access(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 82u);
    ok(memory_collection_insert(&c, &auth, &card, &access) == MEMORY_COLLECTION_E_DUPLICATE &&
       c.count == 1u,
       "T10 duplicate opaque card id is rejected rather than overwritten or merged");

    for (i = 0u; i < MEMORY_COLLECTION_MAX_CARDS - 1u; ++i) {
        card = eligible_card(200u + (uint32_t)i, 3000u + (uint32_t)i);
        auth = auth_for(&card);
        access = physical_access(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT,
                                 83u + (uint32_t)i);
        ok(memory_collection_insert(&c, &auth, &card, &access) == MEMORY_COLLECTION_OK,
           "T10 bounded collection accepts each independently authorised card within capacity");
    }
    card = eligible_card(999u, 9999u);
    auth = auth_for(&card);
    access = physical_access(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 90u);
    ok(c.count == MEMORY_COLLECTION_MAX_CARDS &&
       memory_collection_insert(&c, &auth, &card, &access) == MEMORY_COLLECTION_E_CAPACITY &&
       c.count == MEMORY_COLLECTION_MAX_CARDS,
       "T10 capacity exhaustion does not evict or overwrite an existing memory");

    access = physical_access(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_REMOVE, 91u);
    ok(memory_collection_remove(&c, 203u, &access) == MEMORY_COLLECTION_OK &&
       c.count == MEMORY_COLLECTION_MAX_CARDS - 1u,
       "T10 physical removal is logical, generation-advancing and leaves no success ambiguity");
    access = physical_access(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_OPEN, 92u);
    memset(&out, 0xA5, sizeof(out));
    ok(memory_collection_open(&c, 203u, &access, &out) == MEMORY_COLLECTION_E_NOT_FOUND &&
       out.card_id == 0u,
       "T10 removed identifiers cannot be recovered and failure output is scrubbed");
    access = physical_access(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_COMPACT, 93u);
    ok(memory_collection_compact(&c, &access) == MEMORY_COLLECTION_OK &&
       c.count == MEMORY_COLLECTION_MAX_CARDS - 1u,
       "T10 compaction only rewrites canonical active cards under physical access");

    backend_init(&f);
    cfg = config_for(&f);
    ok(memory_collection_init(&c, &cfg) == MEMORY_COLLECTION_OK,
       "T10 fresh fixture initialises for transaction-recovery scenarios");
    card = eligible_card(401u, 4001u);
    auth = auth_for(&card);
    f.fail_store_committed = 1;
    access = physical_access(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 100u);
    ok(memory_collection_insert(&c, &auth, &card, &access) == MEMORY_COLLECTION_E_STORAGE &&
       c.state == MEMORY_COLLECTION_BLOCKED && f.has_prepared && f.floor == 1u,
       "T10 commit failure after prepared/floor write blocks instead of claiming persistence");
    f.fail_store_committed = 0;
    ok(memory_collection_init(&reopened, &cfg) == MEMORY_COLLECTION_OK &&
       reopened.state == MEMORY_COLLECTION_READY && reopened.generation == 1u &&
       reopened.count == 1u && reopened.metrics.recoveries == 1u &&
       f.has_committed && !f.has_prepared,
       "T10 authenticated unambiguous prepared state is promoted deterministically on reopen");

    card = eligible_card(402u, 4002u);
    auth = auth_for(&card);
    f.fail_commit_floor = 1;
    access = physical_access(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 101u);
    ok(memory_collection_insert(&reopened, &auth, &card, &access) == MEMORY_COLLECTION_E_STORAGE &&
       reopened.state == MEMORY_COLLECTION_BLOCKED && f.has_prepared && f.floor == 1u,
       "T10 interruption before floor commit leaves only a discardable authenticated preparation");
    f.fail_commit_floor = 0;
    ok(memory_collection_init(&c, &cfg) == MEMORY_COLLECTION_OK && c.generation == 1u &&
       c.count == 1u && !f.has_prepared && c.metrics.discarded_prepared == 1u,
       "T10 old floor discards unfinished successor and preserves prior committed collection");

    memcpy(old_committed, f.committed, sizeof(old_committed));
    old_floor = f.floor;
    access = physical_access(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 102u);
    ok(memory_collection_insert(&c, &auth, &card, &access) == MEMORY_COLLECTION_OK &&
       f.floor == old_floor + 1u,
       "T10 second committed mutation advances an independent collection generation");
    memcpy(f.committed, old_committed, sizeof(old_committed));
    f.floor = old_floor + 1u;
    ok(memory_collection_init(&reopened, &cfg) == MEMORY_COLLECTION_E_ROLLBACK &&
       reopened.state == MEMORY_COLLECTION_BLOCKED,
       "T10 restored older committed collection is rejected against independent floor");

    backend_init(&f);
    cfg = config_for(&f);
    ok(memory_collection_init(&c, &cfg) == MEMORY_COLLECTION_OK,
       "T10 fresh fixture initialises for post-commit cleanup interruption");
    card = eligible_card(451u, 4501u);
    auth = auth_for(&card);
    f.fail_erase_prepared = 1;
    access = physical_access(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 103u);
    ok(memory_collection_insert(&c, &auth, &card, &access) == MEMORY_COLLECTION_E_STORAGE &&
       c.state == MEMORY_COLLECTION_BLOCKED && f.has_committed && f.has_prepared && f.floor == 1u,
       "T10 interruption after committed write keeps matching prepared state for cleanup only");
    f.fail_erase_prepared = 0;
    ok(memory_collection_init(&reopened, &cfg) == MEMORY_COLLECTION_OK &&
       reopened.generation == 1u && reopened.count == 1u && !f.has_prepared &&
       reopened.metrics.finalized_prepared == 1u,
       "T10 matching committed/prepared state finalizes cleanup without replaying mutation");

    backend_init(&f);
    cfg = config_for(&f);
    ok(memory_collection_init(&c, &cfg) == MEMORY_COLLECTION_OK,
       "T10 fresh fixture initialises for authenticity scenario");
    card = eligible_card(501u, 5001u);
    auth = auth_for(&card);
    access = physical_access(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 104u);
    ok(memory_collection_insert(&c, &auth, &card, &access) == MEMORY_COLLECTION_OK,
       "T10 authenticity fixture stores one card");
    f.committed[TEST_TAG_OFFSET] ^= 0x80u;
    ok(memory_collection_init(&reopened, &cfg) == MEMORY_COLLECTION_E_AUTHENTICITY &&
       reopened.state == MEMORY_COLLECTION_BLOCKED,
       "T10 altered collection tag is never decoded as a card set");

    backend_init(&f);
    cfg = config_for(&f);
    ROOT_FAIL = 1;
    ok(memory_collection_init(&c, &cfg) == MEMORY_COLLECTION_OK,
       "T10 empty collection does not pretend to test a root before a cryptographic operation");
    card = eligible_card(601u, 6001u);
    auth = auth_for(&card);
    access = physical_access(&gate, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT, 105u);
    ok(memory_collection_insert(&c, &auth, &card, &access) != MEMORY_COLLECTION_OK &&
       c.state == MEMORY_COLLECTION_BLOCKED,
       "T10 root-load failure blocks mutation with no false persistence result");

    if (FAILED) {
        printf("MEMORY COLLECTION TESTS FAILED\n");
        return 1;
    }
    printf("MEMORY COLLECTION INVARIANTS HOLD — bounded records remain authorised, authenticated and fail-closed.\n");
    return 0;
}
