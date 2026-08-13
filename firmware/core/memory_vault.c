/* memory_vault.c — sealed minimal semantic-card vault. */
#include "memory_vault.h"
#include "crypto.h"
#include <string.h>

#define VAULT_HEADER_LEN 28u
#define VAULT_CARD_LEN   24u
#define VAULT_TAG_LEN    16u

static const uint8_t VAULT_INFO[] = "HERUS/MEMORY-VAULT/v1";

/* Private platform seam. Production maps this to protected local material; the host
 * test supplies a fixture-only definition. No public HERUS header declares a root. */
extern int memory_vault_platform_load_root(uint32_t vault_id,
                                           uint8_t out[MEMORY_VAULT_ROOT_LEN]);

static void put_u32(uint8_t *out, uint32_t value)
{
    out[0] = (uint8_t)(value >> 24);
    out[1] = (uint8_t)(value >> 16);
    out[2] = (uint8_t)(value >> 8);
    out[3] = (uint8_t)value;
}

static uint32_t get_u32(const uint8_t *in)
{
    return ((uint32_t)in[0] << 24) | ((uint32_t)in[1] << 16) |
           ((uint32_t)in[2] << 8) | (uint32_t)in[3];
}

static int nonzero(const uint8_t *buf, size_t len)
{
    uint8_t any = 0u;
    while (len-- > 0u) any |= *buf++;
    return any != 0u;
}

static int config_valid(const memory_vault_config_t *cfg)
{
    return cfg && cfg->vault_id != 0u &&
           cfg->storage.store_sealed && cfg->storage.load_sealed &&
           cfg->storage.erase_sealed && cfg->storage.load_generation_floor &&
           cfg->storage.commit_generation_floor;
}

static int card_valid(const memory_vault_card_t *card)
{
    memory_assessment_t assessment;
    if (!card || card->card_id == 0u || card->review_receipt_id == 0u ||
        card->signal.session_authorized != 1u ||
        (card->signal.explicit_remember != 0u && card->signal.explicit_remember != 1u) ||
        card->signal.kind <= MEMORY_KIND_NONE || card->signal.kind >= MEMORY_KIND_COUNT ||
        card->signal.scope != MEMORY_SCOPE_SELF ||
        card->signal.sensitivity != MEMORY_SENSITIVITY_ORDINARY ||
        card->signal.confidence_pct > 100u ||
        card->signal.novelty_pct > 100u || card->signal.future_value_pct > 100u ||
        card->signal.consequence_pct > 100u ||
        card->origin <= MEMORY_EXTRACT_ORIGIN_NONE ||
        card->origin >= MEMORY_EXTRACT_ORIGIN_COUNT || card->extract_reasons == 0u)
        return 0;
    if (memory_policy_assess(&card->signal, &assessment) != MEMORY_POLICY_OK)
        return 0;
    return assessment.disposition == MEMORY_DISPOSITION_AUTO_ELIGIBLE;
}

static int auth_valid(const memory_vault_write_authorization_t *auth,
                      const memory_vault_card_t *card)
{
    return auth && card && auth->card_id != 0u &&
           auth->card_id == card->card_id && auth->review_receipt_id != 0u &&
           auth->review_receipt_id == card->review_receipt_id &&
           auth->human_confirmed == 1u;
}

static void pack_card(const memory_vault_card_t *card, uint8_t out[VAULT_CARD_LEN])
{
    memset(out, 0, VAULT_CARD_LEN);
    out[0] = card->signal.session_authorized;
    out[1] = card->signal.explicit_remember;
    out[2] = (uint8_t)card->signal.kind;
    out[3] = (uint8_t)card->signal.scope;
    out[4] = (uint8_t)card->signal.sensitivity;
    out[5] = card->signal.confidence_pct;
    out[6] = card->signal.novelty_pct;
    out[7] = card->signal.future_value_pct;
    out[8] = card->signal.consequence_pct;
    out[9] = (uint8_t)card->origin;
    put_u32(out + 10u, card->extract_reasons);
    put_u32(out + 14u, card->extract_reasons ^ card->card_id);
    put_u32(out + 18u, card->review_receipt_id);
}

static int unpack_card(const uint8_t in[VAULT_CARD_LEN], uint32_t card_id,
                       memory_vault_card_t *out)
{
    uint32_t reasons = get_u32(in + 10u);
    if (get_u32(in + 14u) != (reasons ^ card_id) || get_u32(in + 18u) == 0u ||
        in[22] != 0u || in[23] != 0u) return 0;
    memset(out, 0, sizeof(*out));
    out->card_id = card_id;
    out->review_receipt_id = get_u32(in + 18u);
    out->signal.session_authorized = in[0];
    out->signal.explicit_remember = in[1];
    out->signal.kind = (memory_kind_t)in[2];
    out->signal.scope = (memory_scope_t)in[3];
    out->signal.sensitivity = (memory_sensitivity_t)in[4];
    out->signal.confidence_pct = in[5];
    out->signal.novelty_pct = in[6];
    out->signal.future_value_pct = in[7];
    out->signal.consequence_pct = in[8];
    out->origin = (memory_extract_origin_t)in[9];
    out->extract_reasons = reasons;
    return card_valid(out);
}

static void make_nonce(uint32_t vault_id, uint32_t generation, uint8_t nonce[12])
{
    nonce[0] = 'H'; nonce[1] = 'M'; nonce[2] = 'V'; nonce[3] = '1';
    put_u32(nonce + 4u, vault_id);
    put_u32(nonce + 8u, generation);
}

static int derive_key(memory_vault_t *v, uint32_t generation,
                      uint8_t key[MEMORY_VAULT_ROOT_LEN])
{
    uint8_t root[MEMORY_VAULT_ROOT_LEN];
    uint8_t salt[8];
    int rc;
    memset(root, 0, sizeof(root));
    if (memory_vault_platform_load_root(v->cfg.vault_id, root) != 0 ||
        !nonzero(root, sizeof(root))) {
        secure_zero(root, sizeof(root));
        return MEMORY_VAULT_E_ROOT;
    }
    put_u32(salt, v->cfg.vault_id);
    put_u32(salt + 4u, generation);
    rc = hkdf(salt, sizeof(salt), root, sizeof(root), VAULT_INFO,
              sizeof(VAULT_INFO) - 1u, key, MEMORY_VAULT_ROOT_LEN);
    secure_zero(root, sizeof(root));
    return rc == 0 ? MEMORY_VAULT_OK : MEMORY_VAULT_E_ROOT;
}

int memory_vault_init(memory_vault_t *v, const memory_vault_config_t *cfg)
{
    if (!v || !cfg) return MEMORY_VAULT_E_ARG;
    memset(v, 0, sizeof(*v));
    if (!config_valid(cfg)) return MEMORY_VAULT_E_CONFIG;
    v->cfg = *cfg;
    if (v->cfg.storage.load_generation_floor(v->cfg.storage.ctx,
                                             &v->current_generation) != 0) {
        v->state = MEMORY_VAULT_BLOCKED;
        v->metrics.backend_failures++;
        return MEMORY_VAULT_E_STORAGE;
    }
    v->state = MEMORY_VAULT_READY;
    return MEMORY_VAULT_OK;
}

int memory_vault_seal(memory_vault_t *v, const memory_vault_write_authorization_t *auth,
                      const memory_vault_card_t *card)
{
    uint8_t blob[MEMORY_VAULT_BLOB_LEN];
    uint8_t key[MEMORY_VAULT_ROOT_LEN];
    uint8_t plain[VAULT_CARD_LEN];
    uint8_t nonce[12];
    uint32_t generation;
    int rc;

    if (!v || !auth || !card) return MEMORY_VAULT_E_ARG;
    if (v->state != MEMORY_VAULT_READY && v->state != MEMORY_VAULT_SEALED)
        return MEMORY_VAULT_E_STATE;
    if (!auth_valid(auth, card)) {
        v->metrics.rejected_authorization++;
        return MEMORY_VAULT_E_AUTH;
    }
    if (!card_valid(card)) {
        v->metrics.rejected_card++;
        return MEMORY_VAULT_E_CARD;
    }
    if (v->current_generation == UINT32_MAX) {
        v->state = MEMORY_VAULT_BLOCKED;
        return MEMORY_VAULT_E_STATE;
    }
    generation = v->current_generation + 1u;
    if (generation == 0u) {
        v->state = MEMORY_VAULT_BLOCKED;
        return MEMORY_VAULT_E_STATE;
    }

    memset(blob, 0, sizeof(blob));
    memset(key, 0, sizeof(key));
    memset(plain, 0, sizeof(plain));
    blob[0] = MEMORY_VAULT_VERSION;
    put_u32(blob + 4u, v->cfg.vault_id);
    put_u32(blob + 8u, card->card_id);
    put_u32(blob + 12u, generation);
    make_nonce(v->cfg.vault_id, generation, nonce);
    memcpy(blob + 16u, nonce, sizeof(nonce));
    pack_card(card, plain);

    rc = derive_key(v, generation, key);
    if (rc != MEMORY_VAULT_OK) {
        v->metrics.backend_failures++;
        v->state = MEMORY_VAULT_BLOCKED;
        secure_zero(key, sizeof(key));
        secure_zero(plain, sizeof(plain));
        secure_zero(blob, sizeof(blob));
        return rc;
    }
    aead_encrypt(key, nonce, blob, VAULT_HEADER_LEN, plain, sizeof(plain),
                 blob + VAULT_HEADER_LEN, blob + VAULT_HEADER_LEN + VAULT_CARD_LEN,
                 VAULT_TAG_LEN);
    rc = v->cfg.storage.store_sealed(v->cfg.storage.ctx, blob);
    secure_zero(key, sizeof(key));
    secure_zero(plain, sizeof(plain));
    secure_zero(blob, sizeof(blob));
    if (rc != 0 || v->cfg.storage.commit_generation_floor(v->cfg.storage.ctx,
                                                          generation) != 0) {
        v->metrics.backend_failures++;
        v->state = MEMORY_VAULT_BLOCKED;
        return MEMORY_VAULT_E_STORAGE;
    }
    v->current_generation = generation;
    v->state = MEMORY_VAULT_SEALED;
    v->metrics.seals++;
    return MEMORY_VAULT_OK;
}

int memory_vault_open(memory_vault_t *v, uint32_t expected_card_id,
                      memory_vault_card_t *out)
{
    uint8_t blob[MEMORY_VAULT_BLOB_LEN];
    uint8_t key[MEMORY_VAULT_ROOT_LEN];
    uint8_t plain[VAULT_CARD_LEN];
    uint8_t nonce[12];
    uint32_t vault_id;
    uint32_t card_id;
    uint32_t generation;
    int rc;

    if (!v || !out || expected_card_id == 0u) return MEMORY_VAULT_E_ARG;
    memset(out, 0, sizeof(*out));
    if (v->state != MEMORY_VAULT_READY && v->state != MEMORY_VAULT_SEALED)
        return MEMORY_VAULT_E_STATE;
    memset(blob, 0, sizeof(blob));
    memset(key, 0, sizeof(key));
    memset(plain, 0, sizeof(plain));
    if (v->cfg.storage.load_sealed(v->cfg.storage.ctx, blob) != 0) {
        v->metrics.backend_failures++;
        v->state = MEMORY_VAULT_BLOCKED;
        secure_zero(blob, sizeof(blob));
        return MEMORY_VAULT_E_STORAGE;
    }
    vault_id = get_u32(blob + 4u);
    card_id = get_u32(blob + 8u);
    generation = get_u32(blob + 12u);
    if (blob[0] != MEMORY_VAULT_VERSION || blob[1] != 0u || blob[2] != 0u ||
        blob[3] != 0u || vault_id != v->cfg.vault_id || card_id != expected_card_id ||
        generation == 0u) {
        v->metrics.authentication_failures++;
        v->state = MEMORY_VAULT_BLOCKED;
        secure_zero(blob, sizeof(blob));
        return MEMORY_VAULT_E_AUTHENTICITY;
    }
    if (generation != v->current_generation) {
        v->metrics.rollback_failures++;
        v->state = MEMORY_VAULT_BLOCKED;
        secure_zero(blob, sizeof(blob));
        return MEMORY_VAULT_E_ROLLBACK;
    }
    make_nonce(vault_id, generation, nonce);
    if (!ct_eq(nonce, blob + 16u, sizeof(nonce))) {
        v->metrics.authentication_failures++;
        v->state = MEMORY_VAULT_BLOCKED;
        secure_zero(blob, sizeof(blob));
        return MEMORY_VAULT_E_AUTHENTICITY;
    }
    rc = derive_key(v, generation, key);
    if (rc != MEMORY_VAULT_OK) {
        v->metrics.backend_failures++;
        v->state = MEMORY_VAULT_BLOCKED;
        secure_zero(key, sizeof(key));
        secure_zero(blob, sizeof(blob));
        return rc;
    }
    rc = aead_decrypt(key, nonce, blob, VAULT_HEADER_LEN,
                      blob + VAULT_HEADER_LEN, VAULT_CARD_LEN,
                      blob + VAULT_HEADER_LEN + VAULT_CARD_LEN, VAULT_TAG_LEN, plain);
    secure_zero(key, sizeof(key));
    secure_zero(blob, sizeof(blob));
    if (rc != 0 || !unpack_card(plain, card_id, out)) {
        v->metrics.authentication_failures++;
        v->state = MEMORY_VAULT_BLOCKED;
        secure_zero(plain, sizeof(plain));
        memset(out, 0, sizeof(*out));
        return MEMORY_VAULT_E_AUTHENTICITY;
    }
    secure_zero(plain, sizeof(plain));
    v->current_generation = generation;
    v->state = MEMORY_VAULT_SEALED;
    v->metrics.opens++;
    return MEMORY_VAULT_OK;
}

int memory_vault_erase(memory_vault_t *v)
{
    if (!v) return MEMORY_VAULT_E_ARG;
    if (v->state == MEMORY_VAULT_UNINITIALIZED) return MEMORY_VAULT_E_STATE;
    if (v->cfg.storage.erase_sealed(v->cfg.storage.ctx) != 0) {
        v->metrics.backend_failures++;
        v->state = MEMORY_VAULT_BLOCKED;
        return MEMORY_VAULT_E_ERASE;
    }
    v->state = MEMORY_VAULT_READY;
    v->metrics.erases++;
    return MEMORY_VAULT_OK;
}

const memory_vault_metrics_t *memory_vault_metrics(const memory_vault_t *v)
{
    return v ? &v->metrics : 0;
}
