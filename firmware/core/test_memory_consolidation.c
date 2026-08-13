/* test_memory_consolidation.c — human consolidation adversarial invariants. */
#include "memory_consolidation.h"
#include <stdio.h>
#include <string.h>

static int FAILED = 0;

typedef struct {
    uint8_t root[MEMORY_VAULT_ROOT_LEN];
    uint8_t blob[MEMORY_VAULT_BLOB_LEN];
    uint32_t floor;
    int have_blob;
    int fail_store;
    int fail_erase;
} fake_backend_t;

static fake_backend_t *ROOT_BACKEND = 0;

int memory_vault_platform_load_root(uint32_t vault_id,
                                    uint8_t out[MEMORY_VAULT_ROOT_LEN])
{
    if (!ROOT_BACKEND || vault_id != 0x48455255u) return -1;
    memcpy(out, ROOT_BACKEND->root, sizeof(ROOT_BACKEND->root));
    return 0;
}

static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

static int fake_store(void *ctx, const uint8_t blob[MEMORY_VAULT_BLOB_LEN])
{
    fake_backend_t *f = (fake_backend_t *)ctx;
    if (f->fail_store) return -1;
    memcpy(f->blob, blob, sizeof(f->blob));
    f->have_blob = 1;
    return 0;
}

static int fake_load(void *ctx, uint8_t blob[MEMORY_VAULT_BLOB_LEN])
{
    fake_backend_t *f = (fake_backend_t *)ctx;
    if (!f->have_blob) return -1;
    memcpy(blob, f->blob, sizeof(f->blob));
    return 0;
}

static int fake_erase(void *ctx)
{
    fake_backend_t *f = (fake_backend_t *)ctx;
    if (f->fail_erase) return -1;
    memset(f->blob, 0, sizeof(f->blob));
    f->have_blob = 0;
    return 0;
}

static int fake_load_floor(void *ctx, uint32_t *out)
{
    *out = ((fake_backend_t *)ctx)->floor;
    return 0;
}

static int fake_commit_floor(void *ctx, uint32_t generation)
{
    fake_backend_t *f = (fake_backend_t *)ctx;
    if (generation <= f->floor) return -1;
    f->floor = generation;
    return 0;
}

static void backend_init(fake_backend_t *f)
{
    size_t i;
    memset(f, 0, sizeof(*f));
    for (i = 0u; i < sizeof(f->root); ++i) f->root[i] = (uint8_t)(0xA3u + i);
}

static memory_vault_config_t vault_config(fake_backend_t *f)
{
    memory_vault_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.vault_id = 0x48455255u;
    cfg.storage.ctx = f;
    cfg.storage.store_sealed = fake_store;
    cfg.storage.load_sealed = fake_load;
    cfg.storage.erase_sealed = fake_erase;
    cfg.storage.load_generation_floor = fake_load_floor;
    cfg.storage.commit_generation_floor = fake_commit_floor;
    return cfg;
}

static memory_consolidation_proposal_t eligible(uint32_t card_id)
{
    memory_consolidation_proposal_t p;
    memset(&p, 0, sizeof(p));
    p.card_id = card_id;
    p.signal.session_authorized = 1u;
    p.signal.explicit_remember = 1u;
    p.signal.kind = MEMORY_KIND_DECISION;
    p.signal.scope = MEMORY_SCOPE_SELF;
    p.signal.sensitivity = MEMORY_SENSITIVITY_ORDINARY;
    p.signal.confidence_pct = 95u;
    p.signal.novelty_pct = 90u;
    p.signal.future_value_pct = 95u;
    p.signal.consequence_pct = 85u;
    p.origin = MEMORY_EXTRACT_EXPLICIT;
    p.extract_reasons = MEMORY_EXTRACT_REASON_EXPLICIT | MEMORY_EXTRACT_REASON_DECISION;
    return p;
}

static memory_consolidation_access_t physical(uint32_t session)
{
    memory_consolidation_access_t access;
    access.physical_session_id = session;
    access.physical_confirmed = 1u;
    return access;
}

int main(void)
{
    fake_backend_t f;
    memory_vault_config_t vcfg;
    memory_vault_t vault;
    memory_consolidation_config_t ccfg;
    memory_consolidation_t c;
    memory_consolidation_proposal_t p;
    memory_consolidation_access_t access;
    memory_vault_card_t out;

    printf("\n== M5 human consolidation is bounded, explicit and non-autonomous ==\n");
    backend_init(&f);
    ROOT_BACKEND = &f;
    vcfg = vault_config(&f);
    ok(memory_vault_init(&vault, &vcfg) == MEMORY_VAULT_OK,
       "M5 fixture vault starts from a valid protected-generation contract");
    memory_consolidation_config_default(&ccfg);
    ccfg.review_window_ms = 100u;
    ok(memory_consolidation_init(&c, &ccfg) == MEMORY_CONSOLIDATION_OK &&
       c.state == MEMORY_CONSOLIDATION_IDLE && c.next_review_receipt_id == 1u,
       "M5 consolidation begins idle with a bounded transient review window");

    p = eligible(10u);
    p.signal.sensitivity = MEMORY_SENSITIVITY_SENSITIVE;
    ok(memory_consolidation_begin(&c, &p, 9u, 0u) == MEMORY_CONSOLIDATION_E_PROPOSAL &&
       c.state == MEMORY_CONSOLIDATION_IDLE && !f.have_blob,
       "M5 sensitive or policy-ineligible proposals cannot enter a human review queue");

    p = eligible(11u);
    ok(memory_consolidation_begin(&c, &p, 20u, 1000u) == MEMORY_CONSOLIDATION_OK &&
       c.state == MEMORY_CONSOLIDATION_REVIEWING && !f.have_blob,
       "M5 beginning review retains only a transient typed proposal and writes nothing");
    access = physical(21u);
    ok(memory_consolidation_confirm_store(&c, &vault, &access, 1001u) ==
       MEMORY_CONSOLIDATION_E_ACCESS && c.state == MEMORY_CONSOLIDATION_REVIEWING &&
       !f.have_blob,
       "M5 a physical confirmation from another session cannot authorize persistence");
    ok(memory_consolidation_expire(&c, 1100u) == MEMORY_CONSOLIDATION_E_EXPIRED &&
       c.state == MEMORY_CONSOLIDATION_EXPIRED && c.pending.card_id == 0u && !f.have_blob,
       "M5 an expired review scrubs the proposal and never turns timeout into retention");
    ok(memory_consolidation_cancel(&c) == MEMORY_CONSOLIDATION_OK &&
       c.state == MEMORY_CONSOLIDATION_IDLE,
       "M5 cancellation returns control to the person without creating a receipt or record");

    p = eligible(12u);
    ok(memory_consolidation_begin(&c, &p, 30u, 2000u) == MEMORY_CONSOLIDATION_OK &&
       memory_consolidation_mark_conflict(&c, 77u, 2001u) == MEMORY_CONSOLIDATION_OK &&
       c.state == MEMORY_CONSOLIDATION_CONFLICTED,
       "M5 an incompatible existing card becomes an explicit conflict, not an automatic choice");
    access = physical(30u);
    ok(memory_consolidation_confirm_store(&c, &vault, &access, 2002u) ==
       MEMORY_CONSOLIDATION_E_CONFLICT && !f.have_blob,
       "M5 conflict blocks even a valid physical confirmation until the proposal is cancelled");
    ok(memory_consolidation_cancel(&c) == MEMORY_CONSOLIDATION_OK &&
       c.state == MEMORY_CONSOLIDATION_IDLE && c.pending.card_id == 0u,
       "M5 cancelling conflict zeroizes the pending proposal rather than resolving it silently");

    p = eligible(42u);
    access = physical(40u);
    ok(memory_consolidation_begin(&c, &p, access.physical_session_id, 3000u) ==
       MEMORY_CONSOLIDATION_OK &&
       memory_consolidation_confirm_store(&c, &vault, &access, 3001u) ==
       MEMORY_CONSOLIDATION_OK && c.state == MEMORY_CONSOLIDATION_IDLE &&
       f.have_blob && f.floor == 1u && c.next_review_receipt_id == 2u,
       "M5 same-session canonical confirmation is the only path that issues a receipt and seals a card");
    ok(memory_vault_open(&vault, 42u, &out) == MEMORY_VAULT_OK &&
       out.card_id == 42u && out.review_receipt_id == 1u &&
       out.signal.kind == MEMORY_KIND_DECISION,
       "M5 the persisted card carries typed provenance and review receipt, not free content");

    access.physical_confirmed = 0u;
    ok(memory_consolidation_recall(&c, &vault, 42u, &access, &out) ==
       MEMORY_CONSOLIDATION_E_ACCESS && out.card_id == 0u,
       "M5 controlled recovery refuses a non-canonical physical access assertion");
    access = physical(501u);
    ok(memory_consolidation_recall(&c, &vault, 42u, &access, &out) ==
       MEMORY_CONSOLIDATION_OK && out.card_id == 42u &&
       memory_consolidation_metrics(&c)->recalled == 1u,
       "M5 recovery is an identifier-bound vault read, not semantic search or model inference");
    ok(memory_consolidation_erase(&c, &vault, &access) == MEMORY_CONSOLIDATION_OK &&
       !f.have_blob && vault.current_generation == 1u &&
       memory_consolidation_metrics(&c)->erased == 1u,
       "M5 authorised removal calls the vault erase while preserving its anti-rollback floor");

    p = eligible(55u);
    access = physical(60u);
    ok(memory_consolidation_begin(&c, &p, 60u, 4000u) == MEMORY_CONSOLIDATION_OK &&
       memory_consolidation_confirm_store(&c, &vault, &access, 4001u) ==
       MEMORY_CONSOLIDATION_OK && f.have_blob && f.floor == 2u,
       "M5 a later reviewed record uses the next vault generation and a new receipt");
    f.fail_erase = 1;
    ok(memory_consolidation_erase(&c, &vault, &access) == MEMORY_CONSOLIDATION_E_VAULT &&
       c.state == MEMORY_CONSOLIDATION_FAILED,
       "M5 erase backend failure becomes a failed state rather than a false deletion claim");
    f.fail_erase = 0;

    ok(memory_consolidation_init(&c, &ccfg) == MEMORY_CONSOLIDATION_OK,
       "M5 an explicit reinitialisation is required after a vault failure; no automatic retry occurs");
    p = eligible(66u);
    access = physical(70u);
    f.fail_store = 1;
    ok(memory_consolidation_begin(&c, &p, 70u, 5000u) == MEMORY_CONSOLIDATION_OK &&
       memory_consolidation_confirm_store(&c, &vault, &access, 5001u) ==
       MEMORY_CONSOLIDATION_E_VAULT && c.state == MEMORY_CONSOLIDATION_FAILED &&
       c.pending.card_id == 0u,
       "M5 failed vault write scrubs review state and cannot retry without a new review");
    f.fail_store = 0;

    ok(memory_consolidation_metrics(&c)->vault_failures == 1u,
       "M5 numeric-only metrics account for vault failure without recording card content");

    if (FAILED) {
        printf("MEMORY CONSOLIDATION TESTS FAILED\n");
        return 1;
    }
    printf("MEMORY CONSOLIDATION INVARIANTS HOLD — review is human, bounded, conflict-safe and reversible without model authority.\n");
    return 0;
}
