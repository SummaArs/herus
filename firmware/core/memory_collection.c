/* memory_collection.c — bounded, transactional and portable card collection. */
#include "memory_collection.h"
#include "memory_collection_private.h"
#include "memory_collection_recovery.h"
#include "crypto.h"
#include <string.h>

#define COLLECTION_HEADER_LEN 28u
#define COLLECTION_SLOT_LEN   28u
#define COLLECTION_PAYLOAD_LEN (MEMORY_COLLECTION_MAX_CARDS * COLLECTION_SLOT_LEN)
#define COLLECTION_TAG_LEN 16u
#define COLLECTION_USED_LEN (COLLECTION_HEADER_LEN + COLLECTION_PAYLOAD_LEN + COLLECTION_TAG_LEN)

static const uint8_t COLLECTION_INFO[] = "HERUS/MEMORY-COLLECTION/v1";

/* Private platform seam. Production selects any reviewed root-protection strategy;
 * host fixtures provide this symbol. No public HERUS header declares this root. */
extern int memory_collection_platform_load_root(uint32_t collection_id,
                                                uint8_t out[MEMORY_COLLECTION_ROOT_LEN]);

typedef struct {
    memory_collection_txn_t txn;
    uint32_t generation;
    uint32_t base_generation;
    uint8_t count;
    memory_vault_card_t cards[MEMORY_COLLECTION_MAX_CARDS];
} collection_record_t;

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

static int is_zero(const uint8_t *buf, size_t len)
{
    uint8_t any = 0u;
    while (len-- > 0u) any |= *buf++;
    return any == 0u;
}

static int nonzero(const uint8_t *buf, size_t len)
{
    return !is_zero(buf, len);
}

static int access_valid(const memory_collection_access_t *access)
{
    return access && access->gate && access->physical_session_id != 0u;
}

static int access_consume(const memory_collection_access_t *access,
                          memory_physical_purpose_t purpose)
{
    return memory_physical_session_consume(access->gate, purpose,
                                           access->physical_session_id,
                                           access->observed_at_ms) ==
           MEMORY_PHYSICAL_SESSION_OK;
}

static int config_valid(const memory_collection_config_t *cfg)
{
    return cfg && cfg->collection_id != 0u &&
           cfg->storage.store_prepared && cfg->storage.load_prepared &&
           cfg->storage.erase_prepared && cfg->storage.store_committed &&
           cfg->storage.load_committed && cfg->storage.erase_committed &&
           cfg->storage.load_generation_floor && cfg->storage.commit_generation_floor;
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
        card->signal.confidence_pct > 100u || card->signal.novelty_pct > 100u ||
        card->signal.future_value_pct > 100u || card->signal.consequence_pct > 100u ||
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
    return auth && card && auth->card_id != 0u && auth->card_id == card->card_id &&
           auth->review_receipt_id != 0u &&
           auth->review_receipt_id == card->review_receipt_id &&
           auth->human_confirmed == 1u;
}

/* Serialized card layout matches the single-vault plaintext representation, but
 * collection slots carry their opaque card id separately. Struct bytes are never
 * persisted, avoiding padding/ABI dependence. */
static void pack_card(const memory_vault_card_t *card, uint8_t out[24])
{
    memset(out, 0, 24u);
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

static int unpack_card(const uint8_t in[24], uint32_t card_id,
                       memory_vault_card_t *out)
{
    uint32_t reasons;
    if (card_id == 0u || in[22] != 0u || in[23] != 0u) return 0;
    reasons = get_u32(in + 10u);
    if (get_u32(in + 14u) != (reasons ^ card_id) || get_u32(in + 18u) == 0u)
        return 0;
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

static void make_nonce(uint32_t collection_id, uint32_t generation,
                       uint8_t nonce[12])
{
    nonce[0] = 'H'; nonce[1] = 'M'; nonce[2] = 'C'; nonce[3] = '1';
    put_u32(nonce + 4u, collection_id);
    put_u32(nonce + 8u, generation);
}

static int derive_key(const memory_collection_t *c, uint32_t generation,
                      uint8_t key[MEMORY_COLLECTION_ROOT_LEN])
{
    uint8_t root[MEMORY_COLLECTION_ROOT_LEN];
    uint8_t salt[8];
    int rc;
    memset(root, 0, sizeof(root));
    if (memory_collection_platform_load_root(c->cfg.collection_id, root) != 0 ||
        !nonzero(root, sizeof(root))) {
        secure_zero(root, sizeof(root));
        return MEMORY_COLLECTION_E_ROOT;
    }
    put_u32(salt, c->cfg.collection_id);
    put_u32(salt + 4u, generation);
    rc = hkdf(salt, sizeof(salt), root, sizeof(root), COLLECTION_INFO,
              sizeof(COLLECTION_INFO) - 1u, key, MEMORY_COLLECTION_ROOT_LEN);
    secure_zero(root, sizeof(root));
    secure_zero(salt, sizeof(salt));
    return rc == 0 ? MEMORY_COLLECTION_OK : MEMORY_COLLECTION_E_ROOT;
}

static int find_card(const collection_record_t *r, uint32_t card_id)
{
    uint8_t i;
    for (i = 0u; i < r->count; ++i)
        if (r->cards[i].card_id == card_id) return (int)i;
    return -1;
}

static int record_valid(const collection_record_t *r)
{
    uint8_t i;
    if (!r || r->count > MEMORY_COLLECTION_MAX_CARDS ||
        r->generation == 0u || r->txn < MEMORY_COLLECTION_TXN_NONE ||
        r->txn > MEMORY_COLLECTION_TXN_COMPACT ||
        (r->txn == MEMORY_COLLECTION_TXN_NONE && r->base_generation != 0u))
        return 0;
    for (i = 0u; i < r->count; ++i) {
        if (!card_valid(&r->cards[i]) || find_card(r, r->cards[i].card_id) != (int)i)
            return 0;
    }
    for (i = r->count; i < MEMORY_COLLECTION_MAX_CARDS; ++i)
        if (r->cards[i].card_id != 0u || r->cards[i].review_receipt_id != 0u)
            return 0;
    return 1;
}

static int build_blob(const memory_collection_t *c, const collection_record_t *r,
                      uint8_t blob[MEMORY_COLLECTION_BLOB_LEN])
{
    uint8_t plain[COLLECTION_PAYLOAD_LEN];
    uint8_t key[MEMORY_COLLECTION_ROOT_LEN];
    uint8_t nonce[12];
    uint8_t i;
    int rc;
    if (!record_valid(r)) return MEMORY_COLLECTION_E_CARD;
    memset(blob, 0, MEMORY_COLLECTION_BLOB_LEN);
    memset(plain, 0, sizeof(plain));
    memset(key, 0, sizeof(key));
    blob[0] = MEMORY_COLLECTION_VERSION;
    blob[1] = (uint8_t)r->txn;
    blob[2] = r->count;
    put_u32(blob + 4u, c->cfg.collection_id);
    put_u32(blob + 8u, r->generation);
    put_u32(blob + 12u, r->base_generation);
    make_nonce(c->cfg.collection_id, r->generation, nonce);
    memcpy(blob + 16u, nonce, sizeof(nonce));
    for (i = 0u; i < r->count; ++i) {
        put_u32(plain + (size_t)i * COLLECTION_SLOT_LEN, r->cards[i].card_id);
        pack_card(&r->cards[i], plain + (size_t)i * COLLECTION_SLOT_LEN + 4u);
    }
    rc = derive_key(c, r->generation, key);
    if (rc == MEMORY_COLLECTION_OK)
        aead_encrypt(key, nonce, blob, COLLECTION_HEADER_LEN, plain, sizeof(plain),
                     blob + COLLECTION_HEADER_LEN,
                     blob + COLLECTION_HEADER_LEN + COLLECTION_PAYLOAD_LEN,
                     COLLECTION_TAG_LEN);
    secure_zero(plain, sizeof(plain));
    secure_zero(key, sizeof(key));
    secure_zero(nonce, sizeof(nonce));
    return rc;
}

static int decode_blob(const memory_collection_t *c,
                       const uint8_t blob[MEMORY_COLLECTION_BLOB_LEN],
                       collection_record_t *r)
{
    uint8_t plain[COLLECTION_PAYLOAD_LEN];
    uint8_t key[MEMORY_COLLECTION_ROOT_LEN];
    uint8_t nonce[12];
    uint8_t i;
    int rc;
    if (!blob || !r || blob[0] != MEMORY_COLLECTION_VERSION || blob[3] != 0u ||
        blob[1] > (uint8_t)MEMORY_COLLECTION_TXN_COMPACT ||
        blob[2] > MEMORY_COLLECTION_MAX_CARDS ||
        get_u32(blob + 4u) != c->cfg.collection_id || get_u32(blob + 8u) == 0u ||
        !is_zero(blob + COLLECTION_USED_LEN,
                 MEMORY_COLLECTION_BLOB_LEN - COLLECTION_USED_LEN))
        return MEMORY_COLLECTION_E_AUTHENTICITY;
    make_nonce(c->cfg.collection_id, get_u32(blob + 8u), nonce);
    if (!ct_eq(blob + 16u, nonce, sizeof(nonce))) {
        secure_zero(nonce, sizeof(nonce));
        return MEMORY_COLLECTION_E_AUTHENTICITY;
    }
    memset(plain, 0, sizeof(plain));
    memset(key, 0, sizeof(key));
    rc = derive_key(c, get_u32(blob + 8u), key);
    if (rc == MEMORY_COLLECTION_OK &&
        aead_decrypt(key, nonce, blob, COLLECTION_HEADER_LEN,
                     blob + COLLECTION_HEADER_LEN, sizeof(plain),
                     blob + COLLECTION_HEADER_LEN + COLLECTION_PAYLOAD_LEN,
                     COLLECTION_TAG_LEN, plain) != 0)
        rc = MEMORY_COLLECTION_E_AUTHENTICITY;
    if (rc != MEMORY_COLLECTION_OK) {
        secure_zero(plain, sizeof(plain));
        secure_zero(key, sizeof(key));
        secure_zero(nonce, sizeof(nonce));
        return rc;
    }
    memset(r, 0, sizeof(*r));
    r->txn = (memory_collection_txn_t)blob[1];
    r->count = blob[2];
    r->generation = get_u32(blob + 8u);
    r->base_generation = get_u32(blob + 12u);
    for (i = 0u; i < r->count; ++i) {
        const uint8_t *slot = plain + (size_t)i * COLLECTION_SLOT_LEN;
        if (!unpack_card(slot + 4u, get_u32(slot), &r->cards[i])) rc = MEMORY_COLLECTION_E_AUTHENTICITY;
    }
    for (i = r->count; i < MEMORY_COLLECTION_MAX_CARDS; ++i) {
        const uint8_t *slot = plain + (size_t)i * COLLECTION_SLOT_LEN;
        if (!is_zero(slot, COLLECTION_SLOT_LEN)) rc = MEMORY_COLLECTION_E_AUTHENTICITY;
    }
    if (rc == MEMORY_COLLECTION_OK && !record_valid(r))
        rc = MEMORY_COLLECTION_E_AUTHENTICITY;
    secure_zero(plain, sizeof(plain));
    secure_zero(key, sizeof(key));
    secure_zero(nonce, sizeof(nonce));
    if (rc != MEMORY_COLLECTION_OK) secure_zero(r, sizeof(*r));
    return rc;
}

static void block(memory_collection_t *c, int backend, int rollback, int auth)
{
    if (!c) return;
    c->state = MEMORY_COLLECTION_BLOCKED;
    if (backend) c->metrics.backend_failures++;
    if (rollback) c->metrics.rollback_failures++;
    if (auth) c->metrics.authentication_failures++;
}

static int load_floor(memory_collection_t *c, uint32_t *floor)
{
    if (c->cfg.storage.load_generation_floor(c->cfg.storage.ctx, floor) != 0) {
        block(c, 1, 0, 0);
        return MEMORY_COLLECTION_E_STORAGE;
    }
    return MEMORY_COLLECTION_OK;
}

/* Loads the active committed collection and binds it to the current durable floor.
 * Initial empty state is the only legal state without a committed record. */
static int load_current(memory_collection_t *c, collection_record_t *out)
{
    uint8_t blob[MEMORY_COLLECTION_BLOB_LEN];
    uint32_t floor;
    int rc;
    memset(out, 0, sizeof(*out));
    if (load_floor(c, &floor) != MEMORY_COLLECTION_OK) return MEMORY_COLLECTION_E_STORAGE;
    if (floor != c->generation) {
        block(c, 0, 1, 0);
        return MEMORY_COLLECTION_E_ROLLBACK;
    }
    memset(blob, 0, sizeof(blob));
    rc = c->cfg.storage.load_committed(c->cfg.storage.ctx, blob);
    if (rc == MEMORY_COLLECTION_STORE_ABSENT && floor == 0u) {
        secure_zero(blob, sizeof(blob));
        return MEMORY_COLLECTION_OK;
    }
    if (rc != 0) {
        secure_zero(blob, sizeof(blob));
        block(c, 1, 0, 0);
        return MEMORY_COLLECTION_E_STORAGE;
    }
    rc = decode_blob(c, blob, out);
    secure_zero(blob, sizeof(blob));
    if (rc != MEMORY_COLLECTION_OK || out->txn != MEMORY_COLLECTION_TXN_NONE ||
        out->generation != floor) {
        secure_zero(out, sizeof(*out));
        block(c, rc == MEMORY_COLLECTION_E_AUTHENTICITY ? 0 : 1,
              out->generation != floor, rc == MEMORY_COLLECTION_E_AUTHENTICITY);
        return rc == MEMORY_COLLECTION_E_AUTHENTICITY ? rc : MEMORY_COLLECTION_E_ROLLBACK;
    }
    return MEMORY_COLLECTION_OK;
}

static int commit_record(memory_collection_t *c, memory_collection_txn_t txn,
                         const collection_record_t *next)
{
    collection_record_t prepared;
    collection_record_t committed;
    uint8_t blob[MEMORY_COLLECTION_BLOB_LEN];
    int rc;
    if (!c || !next || c->generation == UINT32_MAX) {
        if (c) block(c, 0, 1, 0);
        return MEMORY_COLLECTION_E_STATE;
    }
    memset(&prepared, 0, sizeof(prepared));
    prepared = *next;
    prepared.txn = txn;
    prepared.base_generation = c->generation;
    prepared.generation = c->generation + 1u;
    if (prepared.generation == 0u || !record_valid(&prepared)) {
        secure_zero(&prepared, sizeof(prepared));
        block(c, 0, 1, 0);
        return MEMORY_COLLECTION_E_STATE;
    }
    memset(blob, 0, sizeof(blob));
    rc = build_blob(c, &prepared, blob);
    if (rc != MEMORY_COLLECTION_OK ||
        c->cfg.storage.store_prepared(c->cfg.storage.ctx, blob) != 0) {
        secure_zero(blob, sizeof(blob));
        secure_zero(&prepared, sizeof(prepared));
        block(c, 1, 0, rc == MEMORY_COLLECTION_E_AUTHENTICITY);
        return rc == MEMORY_COLLECTION_OK ? MEMORY_COLLECTION_E_STORAGE : rc;
    }
    if (c->cfg.storage.commit_generation_floor(c->cfg.storage.ctx,
                                               prepared.generation) != 0) {
        secure_zero(blob, sizeof(blob));
        secure_zero(&prepared, sizeof(prepared));
        block(c, 1, 0, 0);
        return MEMORY_COLLECTION_E_STORAGE;
    }
    committed = prepared;
    committed.txn = MEMORY_COLLECTION_TXN_NONE;
    committed.base_generation = 0u;
    memset(blob, 0, sizeof(blob));
    rc = build_blob(c, &committed, blob);
    if (rc != MEMORY_COLLECTION_OK ||
        c->cfg.storage.store_committed(c->cfg.storage.ctx, blob) != 0) {
        secure_zero(blob, sizeof(blob));
        secure_zero(&prepared, sizeof(prepared));
        secure_zero(&committed, sizeof(committed));
        block(c, 1, 0, rc == MEMORY_COLLECTION_E_AUTHENTICITY);
        return rc == MEMORY_COLLECTION_OK ? MEMORY_COLLECTION_E_STORAGE : rc;
    }
    if (c->cfg.storage.erase_prepared(c->cfg.storage.ctx) != 0) {
        secure_zero(blob, sizeof(blob));
        secure_zero(&prepared, sizeof(prepared));
        secure_zero(&committed, sizeof(committed));
        block(c, 1, 0, 0);
        return MEMORY_COLLECTION_E_STORAGE;
    }
    c->generation = committed.generation;
    c->count = committed.count;
    secure_zero(blob, sizeof(blob));
    secure_zero(&prepared, sizeof(prepared));
    secure_zero(&committed, sizeof(committed));
    return MEMORY_COLLECTION_OK;
}

static int records_equal(const collection_record_t *a, const collection_record_t *b)
{
    uint8_t i;
    if (a->count != b->count) return 0;
    for (i = 0u; i < a->count; ++i)
        if (memcmp(&a->cards[i], &b->cards[i], sizeof(a->cards[i])) != 0)
            return 0;
    return 1;
}

static int recover_prepared(memory_collection_t *c, const collection_record_t *prepared,
                            memory_collection_recovery_action_t action)
{
    collection_record_t promote;
    uint8_t blob[MEMORY_COLLECTION_BLOB_LEN];
    int rc;
    if (!c || !prepared || prepared->txn == MEMORY_COLLECTION_TXN_NONE ||
        (action != MEMORY_COLLECTION_RECOVERY_PROMOTE_PREPARED &&
         action != MEMORY_COLLECTION_RECOVERY_FINALIZE_PREPARED))
        return MEMORY_COLLECTION_E_RECOVERY;
    if (action == MEMORY_COLLECTION_RECOVERY_FINALIZE_PREPARED) {
        if (c->cfg.storage.erase_prepared(c->cfg.storage.ctx) != 0)
            return MEMORY_COLLECTION_E_STORAGE;
        c->generation = prepared->generation;
        c->count = prepared->count;
        c->metrics.finalized_prepared++;
        return MEMORY_COLLECTION_OK;
    }
    promote = *prepared;
    promote.txn = MEMORY_COLLECTION_TXN_NONE;
    promote.base_generation = 0u;
    memset(blob, 0, sizeof(blob));
    rc = build_blob(c, &promote, blob);
    if (rc != MEMORY_COLLECTION_OK ||
        c->cfg.storage.store_committed(c->cfg.storage.ctx, blob) != 0 ||
        c->cfg.storage.erase_prepared(c->cfg.storage.ctx) != 0) {
        secure_zero(blob, sizeof(blob));
        secure_zero(&promote, sizeof(promote));
        return rc == MEMORY_COLLECTION_OK ? MEMORY_COLLECTION_E_STORAGE : rc;
    }
    c->generation = promote.generation;
    c->count = promote.count;
    c->metrics.recoveries++;
    secure_zero(blob, sizeof(blob));
    secure_zero(&promote, sizeof(promote));
    return MEMORY_COLLECTION_OK;
}

int memory_collection_init(memory_collection_t *c,
                           const memory_collection_config_t *cfg)
{
    uint8_t committed_blob[MEMORY_COLLECTION_BLOB_LEN];
    uint8_t prepared_blob[MEMORY_COLLECTION_BLOB_LEN];
    collection_record_t committed;
    collection_record_t prepared;
    uint32_t floor;
    int committed_rc;
    int prepared_rc;
    int rc;
    memory_collection_recovery_snapshot_t recovery_snapshot;
    memory_collection_recovery_action_t recovery_action;
    if (!c || !cfg) return MEMORY_COLLECTION_E_ARG;
    memset(c, 0, sizeof(*c));
    if (!config_valid(cfg)) return MEMORY_COLLECTION_E_CONFIG;
    c->cfg = *cfg;
    if (load_floor(c, &floor) != MEMORY_COLLECTION_OK) return MEMORY_COLLECTION_E_STORAGE;
    memset(committed_blob, 0, sizeof(committed_blob));
    memset(prepared_blob, 0, sizeof(prepared_blob));
    memset(&committed, 0, sizeof(committed));
    memset(&prepared, 0, sizeof(prepared));
    committed_rc = c->cfg.storage.load_committed(c->cfg.storage.ctx, committed_blob);
    prepared_rc = c->cfg.storage.load_prepared(c->cfg.storage.ctx, prepared_blob);
    if ((committed_rc != 0 && committed_rc != MEMORY_COLLECTION_STORE_ABSENT) ||
        (prepared_rc != 0 && prepared_rc != MEMORY_COLLECTION_STORE_ABSENT)) {
        secure_zero(committed_blob, sizeof(committed_blob));
        secure_zero(prepared_blob, sizeof(prepared_blob));
        block(c, 1, 0, 0);
        return MEMORY_COLLECTION_E_STORAGE;
    }
    if (committed_rc == 0 &&
        (decode_blob(c, committed_blob, &committed) != MEMORY_COLLECTION_OK ||
         committed.txn != MEMORY_COLLECTION_TXN_NONE)) {
        secure_zero(committed_blob, sizeof(committed_blob));
        secure_zero(prepared_blob, sizeof(prepared_blob));
        secure_zero(&committed, sizeof(committed));
        block(c, 0, 0, 1);
        return MEMORY_COLLECTION_E_AUTHENTICITY;
    }
    if (prepared_rc == 0 && decode_blob(c, prepared_blob, &prepared) != MEMORY_COLLECTION_OK) {
        secure_zero(committed_blob, sizeof(committed_blob));
        secure_zero(prepared_blob, sizeof(prepared_blob));
        secure_zero(&committed, sizeof(committed));
        secure_zero(&prepared, sizeof(prepared));
        block(c, 0, 0, 1);
        return MEMORY_COLLECTION_E_AUTHENTICITY;
    }
    secure_zero(committed_blob, sizeof(committed_blob));
    secure_zero(prepared_blob, sizeof(prepared_blob));
    memset(&recovery_snapshot, 0, sizeof(recovery_snapshot));
    recovery_snapshot.committed_present = committed_rc == 0 ? 1u : 0u;
    recovery_snapshot.prepared_present = prepared_rc == 0 ? 1u : 0u;
    recovery_snapshot.committed_authenticated = committed_rc == 0 ? 1u : 0u;
    recovery_snapshot.prepared_authenticated = prepared_rc == 0 ? 1u : 0u;
    recovery_snapshot.committed_generation = committed.generation;
    recovery_snapshot.prepared_generation = prepared.generation;
    recovery_snapshot.prepared_base_generation = prepared.base_generation;
    recovery_snapshot.durable_generation_floor = floor;
    if (committed_rc == 0 && prepared_rc == 0 &&
        committed.generation == prepared.generation &&
        records_equal(&prepared, &committed))
        recovery_snapshot.prepared_matches_committed = 1u;
    rc = memory_collection_recovery_assess(&recovery_snapshot, &recovery_action);
    if (rc != MEMORY_COLLECTION_RECOVERY_OK) {
        int only_committed_floor_mismatch = prepared_rc == MEMORY_COLLECTION_STORE_ABSENT &&
            committed_rc == 0 && committed.generation != floor;
        secure_zero(&prepared, sizeof(prepared));
        secure_zero(&committed, sizeof(committed));
        block(c, 0, 1, 0);
        return only_committed_floor_mismatch ? MEMORY_COLLECTION_E_ROLLBACK :
            MEMORY_COLLECTION_E_RECOVERY;
    }
    if (recovery_action == MEMORY_COLLECTION_RECOVERY_EMPTY) {
        c->state = MEMORY_COLLECTION_READY;
        secure_zero(&prepared, sizeof(prepared));
        secure_zero(&committed, sizeof(committed));
        return MEMORY_COLLECTION_OK;
    }
    if (recovery_action == MEMORY_COLLECTION_RECOVERY_USE_COMMITTED) {
        c->generation = committed.generation;
        c->count = committed.count;
        c->state = MEMORY_COLLECTION_READY;
        secure_zero(&prepared, sizeof(prepared));
        secure_zero(&committed, sizeof(committed));
        return MEMORY_COLLECTION_OK;
    }
    if (recovery_action == MEMORY_COLLECTION_RECOVERY_DISCARD_PREPARED) {
        if (c->cfg.storage.erase_prepared(c->cfg.storage.ctx) != 0) {
            secure_zero(&prepared, sizeof(prepared));
            secure_zero(&committed, sizeof(committed));
            block(c, 1, 0, 0);
            return MEMORY_COLLECTION_E_STORAGE;
        }
        c->generation = committed_rc == 0 ? committed.generation : 0u;
        c->count = committed_rc == 0 ? committed.count : 0u;
        c->metrics.discarded_prepared++;
        c->state = MEMORY_COLLECTION_READY;
        secure_zero(&prepared, sizeof(prepared));
        secure_zero(&committed, sizeof(committed));
        return MEMORY_COLLECTION_OK;
    }
    rc = recover_prepared(c, &prepared, recovery_action);
    secure_zero(&prepared, sizeof(prepared));
    secure_zero(&committed, sizeof(committed));
    if (rc != MEMORY_COLLECTION_OK) {
        block(c, rc == MEMORY_COLLECTION_E_STORAGE, rc == MEMORY_COLLECTION_E_RECOVERY, 0);
        return rc;
    }
    c->state = MEMORY_COLLECTION_READY;
    return MEMORY_COLLECTION_OK;
}

int memory_collection_insert(memory_collection_t *c,
                             const memory_vault_write_authorization_t *auth,
                             const memory_vault_card_t *card,
                             const memory_collection_access_t *access)
{
    collection_record_t current;
    if (!c || !auth || !card || !access) return MEMORY_COLLECTION_E_ARG;
    if (c->state != MEMORY_COLLECTION_READY) return MEMORY_COLLECTION_E_STATE;
    if (!access_valid(access)) {
        c->metrics.rejected_access++;
        return MEMORY_COLLECTION_E_ACCESS;
    }
    if (!auth_valid(auth, card)) return MEMORY_COLLECTION_E_AUTH;
    if (!card_valid(card)) {
        c->metrics.rejected_card++;
        return MEMORY_COLLECTION_E_CARD;
    }
    if (!access_consume(access, MEMORY_PHYSICAL_PURPOSE_COLLECTION_INSERT)) {
        c->metrics.rejected_access++;
        return MEMORY_COLLECTION_E_ACCESS;
    }
    if (load_current(c, &current) != MEMORY_COLLECTION_OK) return MEMORY_COLLECTION_E_STATE;
    if (find_card(&current, card->card_id) >= 0) {
        c->metrics.duplicate_rejections++;
        secure_zero(&current, sizeof(current));
        return MEMORY_COLLECTION_E_DUPLICATE;
    }
    if (current.count >= MEMORY_COLLECTION_MAX_CARDS) {
        c->metrics.capacity_rejections++;
        secure_zero(&current, sizeof(current));
        return MEMORY_COLLECTION_E_CAPACITY;
    }
    current.cards[current.count++] = *card;
    if (commit_record(c, MEMORY_COLLECTION_TXN_INSERT, &current) != MEMORY_COLLECTION_OK) {
        secure_zero(&current, sizeof(current));
        return MEMORY_COLLECTION_E_STORAGE;
    }
    c->metrics.inserts++;
    secure_zero(&current, sizeof(current));
    return MEMORY_COLLECTION_OK;
}

int memory_collection_open(memory_collection_t *c, uint32_t expected_card_id,
                           const memory_collection_access_t *access,
                           memory_vault_card_t *out)
{
    collection_record_t current;
    int index;
    if (!c || !access || !out || expected_card_id == 0u) return MEMORY_COLLECTION_E_ARG;
    memset(out, 0, sizeof(*out));
    if (c->state != MEMORY_COLLECTION_READY) return MEMORY_COLLECTION_E_STATE;
    if (!access_valid(access)) {
        c->metrics.rejected_access++;
        return MEMORY_COLLECTION_E_ACCESS;
    }
    if (!access_consume(access, MEMORY_PHYSICAL_PURPOSE_COLLECTION_OPEN)) {
        c->metrics.rejected_access++;
        return MEMORY_COLLECTION_E_ACCESS;
    }
    if (load_current(c, &current) != MEMORY_COLLECTION_OK) return MEMORY_COLLECTION_E_STATE;
    index = find_card(&current, expected_card_id);
    if (index < 0) {
        secure_zero(&current, sizeof(current));
        return MEMORY_COLLECTION_E_NOT_FOUND;
    }
    *out = current.cards[index];
    c->metrics.opens++;
    secure_zero(&current, sizeof(current));
    return MEMORY_COLLECTION_OK;
}

int memory_collection_copy_cards_for_index(
    memory_collection_t *c, const memory_collection_access_t *access,
    memory_vault_card_t out[MEMORY_COLLECTION_MAX_CARDS], uint8_t *out_count)
{
    collection_record_t current;
    if (!c || !access || !out || !out_count) return MEMORY_COLLECTION_E_ARG;
    memset(out, 0, sizeof(memory_vault_card_t) * MEMORY_COLLECTION_MAX_CARDS);
    *out_count = 0u;
    if (c->state != MEMORY_COLLECTION_READY) return MEMORY_COLLECTION_E_STATE;
    if (!access_valid(access)) {
        c->metrics.rejected_access++;
        return MEMORY_COLLECTION_E_ACCESS;
    }
    if (!access_consume(access, MEMORY_PHYSICAL_PURPOSE_COLLECTION_QUERY)) {
        c->metrics.rejected_access++;
        return MEMORY_COLLECTION_E_ACCESS;
    }
    if (load_current(c, &current) != MEMORY_COLLECTION_OK) return MEMORY_COLLECTION_E_STATE;
    memcpy(out, current.cards, sizeof(current.cards));
    *out_count = current.count;
    secure_zero(&current, sizeof(current));
    return MEMORY_COLLECTION_OK;
}

int memory_collection_remove(memory_collection_t *c, uint32_t card_id,
                             const memory_collection_access_t *access)
{
    collection_record_t current;
    int index;
    uint8_t i;
    if (!c || !access || card_id == 0u) return MEMORY_COLLECTION_E_ARG;
    if (c->state != MEMORY_COLLECTION_READY) return MEMORY_COLLECTION_E_STATE;
    if (!access_valid(access)) {
        c->metrics.rejected_access++;
        return MEMORY_COLLECTION_E_ACCESS;
    }
    if (!access_consume(access, MEMORY_PHYSICAL_PURPOSE_COLLECTION_REMOVE)) {
        c->metrics.rejected_access++;
        return MEMORY_COLLECTION_E_ACCESS;
    }
    if (load_current(c, &current) != MEMORY_COLLECTION_OK) return MEMORY_COLLECTION_E_STATE;
    index = find_card(&current, card_id);
    if (index < 0) {
        secure_zero(&current, sizeof(current));
        return MEMORY_COLLECTION_E_NOT_FOUND;
    }
    for (i = (uint8_t)index; i + 1u < current.count; ++i)
        current.cards[i] = current.cards[i + 1u];
    memset(&current.cards[current.count - 1u], 0, sizeof(current.cards[0]));
    current.count--;
    if (commit_record(c, MEMORY_COLLECTION_TXN_REMOVE, &current) != MEMORY_COLLECTION_OK) {
        secure_zero(&current, sizeof(current));
        return MEMORY_COLLECTION_E_STORAGE;
    }
    c->metrics.removes++;
    secure_zero(&current, sizeof(current));
    return MEMORY_COLLECTION_OK;
}

int memory_collection_compact(memory_collection_t *c,
                              const memory_collection_access_t *access)
{
    collection_record_t current;
    memory_vault_card_t tmp;
    uint8_t i;
    uint8_t j;
    if (!c || !access) return MEMORY_COLLECTION_E_ARG;
    if (c->state != MEMORY_COLLECTION_READY) return MEMORY_COLLECTION_E_STATE;
    if (!access_valid(access)) {
        c->metrics.rejected_access++;
        return MEMORY_COLLECTION_E_ACCESS;
    }
    if (!access_consume(access, MEMORY_PHYSICAL_PURPOSE_COLLECTION_COMPACT)) {
        c->metrics.rejected_access++;
        return MEMORY_COLLECTION_E_ACCESS;
    }
    if (load_current(c, &current) != MEMORY_COLLECTION_OK) return MEMORY_COLLECTION_E_STATE;
    for (i = 1u; i < current.count; ++i) {
        tmp = current.cards[i];
        j = i;
        while (j > 0u && current.cards[j - 1u].card_id > tmp.card_id) {
            current.cards[j] = current.cards[j - 1u];
            --j;
        }
        current.cards[j] = tmp;
    }
    if (commit_record(c, MEMORY_COLLECTION_TXN_COMPACT, &current) != MEMORY_COLLECTION_OK) {
        secure_zero(&current, sizeof(current));
        secure_zero(&tmp, sizeof(tmp));
        return MEMORY_COLLECTION_E_STORAGE;
    }
    c->metrics.compactions++;
    secure_zero(&current, sizeof(current));
    secure_zero(&tmp, sizeof(tmp));
    return MEMORY_COLLECTION_OK;
}

const memory_collection_metrics_t *memory_collection_metrics(
    const memory_collection_t *c)
{
    return c ? &c->metrics : 0;
}
