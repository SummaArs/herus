/* test_memory_vault.c — encrypted memory-vault adversarial invariants. */
#include "memory_vault.h"
#include <stdio.h>
#include <string.h>

static int FAILED = 0;

typedef struct {
    uint8_t root[MEMORY_VAULT_ROOT_LEN];
    uint8_t blob[MEMORY_VAULT_BLOB_LEN];
    uint32_t floor;
    int have_blob;
    int fail_root;
    int fail_store;
    int fail_load;
    int fail_erase;
    int fail_load_floor;
    int fail_commit_floor;
} fake_vault_backend_t;

/* Fixture-only platform seam. Production provides this privately from protected
 * material; it is intentionally absent from every public vault header. */
static fake_vault_backend_t *ROOT_BACKEND = 0;

int memory_vault_platform_load_root(uint32_t vault_id,
                                    uint8_t out[MEMORY_VAULT_ROOT_LEN])
{
    if (!ROOT_BACKEND || ROOT_BACKEND->fail_root || vault_id != 0x48455255u)
        return -1;
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
    fake_vault_backend_t *f = (fake_vault_backend_t *)ctx;
    if (f->fail_store) return -1;
    memcpy(f->blob, blob, sizeof(f->blob));
    f->have_blob = 1;
    return 0;
}

static int fake_load(void *ctx, uint8_t blob[MEMORY_VAULT_BLOB_LEN])
{
    fake_vault_backend_t *f = (fake_vault_backend_t *)ctx;
    if (f->fail_load || !f->have_blob) return -1;
    memcpy(blob, f->blob, sizeof(f->blob));
    return 0;
}

static int fake_erase(void *ctx)
{
    fake_vault_backend_t *f = (fake_vault_backend_t *)ctx;
    if (f->fail_erase) return -1;
    memset(f->blob, 0, sizeof(f->blob));
    f->have_blob = 0;
    return 0;
}

static int fake_load_floor(void *ctx, uint32_t *out)
{
    fake_vault_backend_t *f = (fake_vault_backend_t *)ctx;
    if (f->fail_load_floor) return -1;
    *out = f->floor;
    return 0;
}

static int fake_commit_floor(void *ctx, uint32_t generation)
{
    fake_vault_backend_t *f = (fake_vault_backend_t *)ctx;
    if (f->fail_commit_floor || generation <= f->floor) return -1;
    f->floor = generation;
    return 0;
}

static memory_vault_config_t config_for(fake_vault_backend_t *f)
{
    memory_vault_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.vault_id = 0x48455255u; /* HERU, non-secret local context only. */
    cfg.storage.ctx = f;
    cfg.storage.store_sealed = fake_store;
    cfg.storage.load_sealed = fake_load;
    cfg.storage.erase_sealed = fake_erase;
    cfg.storage.load_generation_floor = fake_load_floor;
    cfg.storage.commit_generation_floor = fake_commit_floor;
    return cfg;
}

static void backend_init(fake_vault_backend_t *f)
{
    size_t i;
    memset(f, 0, sizeof(*f));
    for (i = 0u; i < sizeof(f->root); ++i) f->root[i] = (uint8_t)(0x51u + i);
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

static memory_vault_write_authorization_t authorization_for(const memory_vault_card_t *card)
{
    memory_vault_write_authorization_t auth;
    auth.card_id = card->card_id;
    auth.review_receipt_id = card->review_receipt_id;
    auth.human_confirmed = 1u;
    return auth;
}

static memory_vault_erase_authorization_t erase_authorization_for(
    const memory_vault_t *vault, uint32_t session_id)
{
    memory_vault_erase_authorization_t auth;
    auth.vault_id = vault->cfg.vault_id;
    auth.physical_session_id = session_id;
    auth.human_confirmed = 1u;
    return auth;
}

int main(void)
{
    fake_vault_backend_t f;
    fake_vault_backend_t wrong_key;
    memory_vault_config_t cfg;
    memory_vault_t v;
    memory_vault_t restored;
    memory_vault_card_t card;
    memory_vault_card_t card2;
    memory_vault_card_t out;
    memory_vault_write_authorization_t auth;
    memory_vault_erase_authorization_t erase_auth;
    uint8_t prior[MEMORY_VAULT_BLOB_LEN];

    printf("\n== M4 encrypted memory vault is authorised, authenticated and fail-closed ==\n");
    backend_init(&f);
    ROOT_BACKEND = &f;
    cfg = config_for(&f);
    ok(memory_vault_init(&v, &cfg) == MEMORY_VAULT_OK &&
       v.state == MEMORY_VAULT_READY && v.current_generation == 0u,
       "M4 initialisation loads an independent monotonic floor without loading a root key");

    card = eligible_card(101u, 9001u);
    auth = authorization_for(&card);
    auth.human_confirmed = 0u;
    ok(memory_vault_seal(&v, &auth, &card) == MEMORY_VAULT_E_AUTH && !f.have_blob &&
       memory_vault_metrics(&v)->rejected_authorization == 1u,
       "M4 an eligible card cannot persist without canonical explicit human authorization");

    auth = authorization_for(&card);
    card.signal.sensitivity = MEMORY_SENSITIVITY_SENSITIVE;
    ok(memory_vault_seal(&v, &auth, &card) == MEMORY_VAULT_E_CARD && !f.have_blob &&
       memory_vault_metrics(&v)->rejected_card == 1u,
       "M4 sensitive or third-party-shaped candidates cannot bypass policy into the vault");
    card = eligible_card(101u, 9001u);
    auth = authorization_for(&card);
    ok(memory_vault_seal(&v, &auth, &card) == MEMORY_VAULT_OK && f.have_blob &&
       f.floor == 1u && v.current_generation == 1u && v.state == MEMORY_VAULT_SEALED,
       "M4 an authorised minimal card seals with a committed monotonic generation");
    memset(&out, 0, sizeof(out));
    ok(memory_vault_open(&v, card.card_id, &out) == MEMORY_VAULT_OK &&
       memcmp(&out, &card, sizeof(card)) == 0,
       "M4 a valid sealed card opens only under its expected local identifier");

    f.blob[MEMORY_VAULT_BLOB_LEN - 1u] ^= 0x80u;
    ok(memory_vault_open(&v, card.card_id, &out) == MEMORY_VAULT_E_AUTHENTICITY &&
       v.state == MEMORY_VAULT_BLOCKED && out.card_id == 0u,
       "M4 tag alteration is rejected and releases no plaintext card");
    f.blob[MEMORY_VAULT_BLOB_LEN - 1u] ^= 0x80u;

    ok(memory_vault_init(&restored, &cfg) == MEMORY_VAULT_OK,
       "M4 a fresh instance obtains the durable floor before accepting a record");
    f.blob[4] ^= 0x01u;
    ok(memory_vault_open(&restored, card.card_id, &out) == MEMORY_VAULT_E_AUTHENTICITY &&
       restored.state == MEMORY_VAULT_BLOCKED,
       "M4 AAD/header-context alteration fails before any card can be returned");
    f.blob[4] ^= 0x01u;

    backend_init(&wrong_key);
    memcpy(wrong_key.blob, f.blob, sizeof(f.blob));
    wrong_key.have_blob = 1;
    wrong_key.floor = f.floor;
    wrong_key.root[0] ^= 0x55u;
    ROOT_BACKEND = &wrong_key;
    cfg = config_for(&wrong_key);
    ok(memory_vault_init(&restored, &cfg) == MEMORY_VAULT_OK &&
       memory_vault_open(&restored, card.card_id, &out) == MEMORY_VAULT_E_AUTHENTICITY &&
       out.card_id == 0u,
       "M4 a different root material cannot authenticate the sealed record");

    ROOT_BACKEND = &f;
    cfg = config_for(&f);
    ok(memory_vault_init(&v, &cfg) == MEMORY_VAULT_OK,
       "M4 original backend restores after a rejected foreign-root attempt");
    memcpy(prior, f.blob, sizeof(prior));
    card2 = eligible_card(202u, 9002u);
    auth = authorization_for(&card2);
    ok(memory_vault_seal(&v, &auth, &card2) == MEMORY_VAULT_OK && f.floor == 2u,
       "M4 replacement advances the protected generation floor");
    memcpy(f.blob, prior, sizeof(prior));
    ok(memory_vault_init(&restored, &cfg) == MEMORY_VAULT_OK &&
       memory_vault_open(&restored, card.card_id, &out) == MEMORY_VAULT_E_ROLLBACK &&
       memory_vault_metrics(&restored)->rollback_failures == 1u,
       "M4 restoring an authenticated old blob is rejected against the separate durable floor");
    memcpy(f.blob, prior, sizeof(prior));

    /* Recover test fixture by resealing at generation 3; the old generation must
     * never be reused, even after a failed/erased record lifecycle. */
    f.have_blob = 0;
    card2 = eligible_card(303u, 9003u);
    auth = authorization_for(&card2);
    ok(memory_vault_init(&v, &cfg) == MEMORY_VAULT_OK &&
       memory_vault_seal(&v, &auth, &card2) == MEMORY_VAULT_OK && f.floor == 3u,
       "M4 a later seal derives a new generation-specific key rather than reusing erased state");

    erase_auth = erase_authorization_for(&v, 7001u);
    erase_auth.human_confirmed = 0u;
    ok(memory_vault_erase(&v, &erase_auth) == MEMORY_VAULT_E_AUTH && f.have_blob &&
       v.state == MEMORY_VAULT_SEALED,
       "M4 destructive erase is rejected without canonical explicit human authority");
    erase_auth = erase_authorization_for(&v, 7001u);
    f.fail_erase = 1;
    ok(memory_vault_erase(&v, &erase_auth) == MEMORY_VAULT_E_ERASE && v.state == MEMORY_VAULT_BLOCKED &&
       memory_vault_seal(&v, &auth, &card2) == MEMORY_VAULT_E_STATE,
       "M4 erase failure blocks both reading and writing instead of assuming deletion");
    f.fail_erase = 0;
    erase_auth = erase_authorization_for(&v, 7002u);
    ok(memory_vault_init(&v, &cfg) == MEMORY_VAULT_OK && memory_vault_erase(&v, &erase_auth) == MEMORY_VAULT_OK &&
       v.state == MEMORY_VAULT_READY && v.current_generation == 3u && !f.have_blob,
       "M4 successful erase removes the record but never lowers the anti-rollback floor");

    card = eligible_card(404u, 9004u);
    auth = authorization_for(&card);
    f.fail_store = 1;
    ok(memory_vault_seal(&v, &auth, &card) == MEMORY_VAULT_E_STORAGE &&
       v.state == MEMORY_VAULT_BLOCKED,
       "M4 failed record persistence blocks the vault before a generation can be claimed");
    f.fail_store = 0;

    if (FAILED) {
        printf("MEMORY VAULT TESTS FAILED\n");
        return 1;
    }
    printf("MEMORY VAULT INVARIANTS HOLD — human authority, full AEAD authentication, durable generation and fail-closed deletion are enforced in host simulation.\n");
    return 0;
}
