/* trust.c — explicit application-level lifecycle for Core/Nucleus trust. */
#include "trust.h"
#include <string.h>

static const uint8_t SAS_INFO[] = "HERUS/SAS/v1";
static const uint8_t PAIR_INFO[] = "HERUS/PAIR/v1";
static const uint8_t PAIR_ID_INFO[] = "HERUS/PAIR-ID/v1";

enum {
    REC_VERSION = 0u,
    REC_STATE = 1u,
    REC_GENERATION = 4u,
    REC_PAIR_ID = 8u,
    REC_KEY = 12u
};

static uint32_t elapsed(uint32_t now, uint32_t then)
{
    return (uint32_t)(now - then);
}

static void put_u32(uint8_t *p, uint32_t x)
{
    p[0] = (uint8_t)(x >> 24); p[1] = (uint8_t)(x >> 16);
    p[2] = (uint8_t)(x >> 8); p[3] = (uint8_t)x;
}

static uint32_t get_u32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static int nonzero(const uint8_t *p, unsigned n)
{
    uint8_t any = 0;
    if (!p) return 0;
    for (unsigned i = 0; i < n; i++) any |= p[i];
    return any != 0;
}

static void provisional_zero(trust_t *t)
{
    secure_zero(t->transport_secret, sizeof(t->transport_secret));
    secure_zero(t->core_nonce, sizeof(t->core_nonce));
    secure_zero(t->nucleus_nonce, sizeof(t->nucleus_nonce));
    t->offer_started_ms = 0;
    t->sas = 0;
}

static void active_zero(trust_t *t)
{
    secure_zero(t->active_blob, sizeof(t->active_blob));
}

static int record_valid(const uint8_t record[TRUST_STORE_BLOB_LEN])
{
    return record && record[REC_VERSION] == TRUST_VERSION &&
           record[REC_STATE] == TRUST_ACTIVE &&
           get_u32(record + REC_GENERATION) && get_u32(record + REC_PAIR_ID) &&
           nonzero(record + REC_KEY, SHA256_LEN);
}

static uint32_t compute_sas(const uint8_t secret[SHA256_LEN],
                            const uint8_t core_nonce[TRUST_NONCE_LEN],
                            const uint8_t nucleus_nonce[TRUST_NONCE_LEN])
{
    uint8_t input[sizeof(SAS_INFO) - 1u + 2u * TRUST_NONCE_LEN];
    uint8_t mac[SHA256_LEN];
    uint32_t value;
    memcpy(input, SAS_INFO, sizeof(SAS_INFO) - 1u);
    memcpy(input + sizeof(SAS_INFO) - 1u, core_nonce, TRUST_NONCE_LEN);
    memcpy(input + sizeof(SAS_INFO) - 1u + TRUST_NONCE_LEN, nucleus_nonce, TRUST_NONCE_LEN);
    hmac_sha256(secret, SHA256_LEN, input, sizeof(input), mac);
    value = ((uint32_t)mac[0] << 24) | ((uint32_t)mac[1] << 16) |
            ((uint32_t)mac[2] << 8) | (uint32_t)mac[3];
    secure_zero(input, sizeof(input));
    secure_zero(mac, sizeof(mac));
    return value % TRUST_SAS_MODULO;
}

static void derive_record(const trust_t *t, uint8_t out[TRUST_STORE_BLOB_LEN])
{
    uint8_t salt[2u * TRUST_NONCE_LEN];
    uint8_t pair_id_hash[SHA256_LEN];
    uint32_t generation = t->generation + 1u;
    uint32_t pair_id;
    memcpy(salt, t->core_nonce, TRUST_NONCE_LEN);
    memcpy(salt + TRUST_NONCE_LEN, t->nucleus_nonce, TRUST_NONCE_LEN);
    memset(out, 0, TRUST_STORE_BLOB_LEN);
    if (!generation) generation = 1u;
    out[REC_VERSION] = TRUST_VERSION;
    out[REC_STATE] = TRUST_ACTIVE;
    put_u32(out + REC_GENERATION, generation);
    (void)hkdf(salt, sizeof(salt), t->transport_secret, SHA256_LEN,
               PAIR_INFO, sizeof(PAIR_INFO) - 1u, out + REC_KEY, SHA256_LEN);
    hmac_sha256(out + REC_KEY, SHA256_LEN, PAIR_ID_INFO,
                sizeof(PAIR_ID_INFO) - 1u, pair_id_hash);
    pair_id = get_u32(pair_id_hash);
    if (!pair_id) pair_id = 1u;
    put_u32(out + REC_PAIR_ID, pair_id);
    secure_zero(salt, sizeof(salt));
    secure_zero(pair_id_hash, sizeof(pair_id_hash));
}

void trust_init(trust_t *t)
{
    if (!t) return;
    memset(t, 0, sizeof(*t));
    t->state = TRUST_UNPAIRED;
}

int trust_begin(trust_t *t, int core_pair_button, int nucleus_pair_button,
                const uint8_t secret[SHA256_LEN],
                const uint8_t core_nonce[TRUST_NONCE_LEN],
                const uint8_t nucleus_nonce[TRUST_NONCE_LEN], uint32_t now_ms)
{
    if (!t || !secret || !core_nonce || !nucleus_nonce) return TRUST_E_ARG;
    if (t->state != TRUST_UNPAIRED) return TRUST_E_STATE;
    if (!core_pair_button || !nucleus_pair_button) return TRUST_E_PHYSICAL;
    if (!nonzero(secret, SHA256_LEN) || !nonzero(core_nonce, TRUST_NONCE_LEN) ||
        !nonzero(nucleus_nonce, TRUST_NONCE_LEN)) return TRUST_E_SECRET;
    memcpy(t->transport_secret, secret, SHA256_LEN);
    memcpy(t->core_nonce, core_nonce, TRUST_NONCE_LEN);
    memcpy(t->nucleus_nonce, nucleus_nonce, TRUST_NONCE_LEN);
    t->offer_started_ms = now_ms;
    t->sas = compute_sas(t->transport_secret, t->core_nonce, t->nucleus_nonce);
    t->state = TRUST_OFFERED;
    return TRUST_OK;
}

int trust_sas(const trust_t *t, uint32_t *out_sas)
{
    if (!t || !out_sas) return TRUST_E_ARG;
    if (t->state != TRUST_OFFERED && t->state != TRUST_CONFIRMED) return TRUST_E_STATE;
    *out_sas = t->sas;
    return TRUST_OK;
}

void trust_cancel(trust_t *t)
{
    if (!t) return;
    if (t->state == TRUST_OFFERED || t->state == TRUST_CONFIRMED) {
        provisional_zero(t);
        t->state = TRUST_UNPAIRED;
    }
}

int trust_tick(trust_t *t, uint32_t now_ms)
{
    if (!t) return TRUST_E_ARG;
    if (t->state != TRUST_OFFERED && t->state != TRUST_CONFIRMED) return TRUST_E_STATE;
    if (elapsed(now_ms, t->offer_started_ms) < TRUST_OFFER_TTL_MS) return TRUST_OK;
    trust_cancel(t);
    return TRUST_E_TIMEOUT;
}

int trust_confirm(trust_t *t, int core_confirmed, uint32_t core_sas,
                  int nucleus_confirmed, uint32_t nucleus_sas,
                  uint32_t now_ms, const trust_storage_t *storage)
{
    uint8_t candidate[TRUST_STORE_BLOB_LEN];
    if (!t || !storage || !storage->store_active) return TRUST_E_ARG;
    if (t->state != TRUST_OFFERED) return TRUST_E_STATE;
    if (elapsed(now_ms, t->offer_started_ms) >= TRUST_OFFER_TTL_MS) {
        trust_cancel(t);
        return TRUST_E_TIMEOUT;
    }
    if (!core_confirmed || !nucleus_confirmed || core_sas != t->sas ||
        nucleus_sas != t->sas) {
        trust_cancel(t);
        return TRUST_E_CONFIRM;
    }
    t->state = TRUST_CONFIRMED;
    derive_record(t, candidate);
    if (!record_valid(candidate) || storage->store_active(storage->ctx, candidate)) {
        secure_zero(candidate, sizeof(candidate));
        provisional_zero(t);
        t->state = TRUST_UNPAIRED;
        return TRUST_E_STORAGE;
    }
    active_zero(t);
    memcpy(t->active_blob, candidate, sizeof(t->active_blob));
    t->generation = get_u32(candidate + REC_GENERATION);
    secure_zero(candidate, sizeof(candidate));
    provisional_zero(t);
    t->state = TRUST_ACTIVE;
    return TRUST_OK;
}

static int active_control_key(const trust_t *t, core_link_key_t *out)
{
    if (!t || !out) return TRUST_E_ARG;
    if (t->state != TRUST_ACTIVE || !record_valid(t->active_blob)) {
        secure_zero(out, sizeof(*out));
        return TRUST_E_STATE;
    }
    memcpy(out->pair_key, t->active_blob + REC_KEY, sizeof(out->pair_key));
    out->pair_id = get_u32(t->active_blob + REC_PAIR_ID);
    return TRUST_OK;
}

/* HERUS_CRITICAL_SINK: nucleus-seal operation=core_link_seal_nucleus_intent( */
int trust_seal_nucleus_intent(const trust_t *t, core_link_tx_t *tx,
                              uint32_t now_ms, uint32_t session_id,
                              uint32_t expires_ms,
                              const intent_observation_t *observation,
                              uint8_t out[CORE_LINK_WIRE_LEN])
{
    core_link_key_t key;
    int rc = active_control_key(t, &key);
    if (rc) return rc;
    rc = core_link_seal_nucleus_intent(tx, &key, now_ms, session_id, expires_ms,
                                       observation, out);
    secure_zero(&key, sizeof(key));
    return rc;
}

int trust_open_nucleus_intent(const trust_t *t, core_link_rx_t *rx,
                              const uint8_t *wire, uint32_t wire_len,
                              uint32_t now_ms, core_link_intent_t *out)
{
    core_link_key_t key;
    int rc = active_control_key(t, &key);
    if (rc) {
        if (out) secure_zero(out, sizeof(*out));
        return rc;
    }
    rc = core_link_open_nucleus_intent(rx, &key, wire, wire_len, now_ms, out);
    secure_zero(&key, sizeof(key));
    return rc;
}

int trust_revoke(trust_t *t, core_link_tx_t *tx, core_link_rx_t *rx,
                 const trust_storage_t *storage)
{
    if (!t || !tx || !rx || !storage || !storage->erase) return TRUST_E_ARG;
    if (t->state != TRUST_ACTIVE && t->state != TRUST_REVOKED) return TRUST_E_STATE;
    active_zero(t);
    provisional_zero(t);
    core_link_tx_init(tx);
    core_link_rx_init(rx);
    t->state = TRUST_REVOKED;
    if (storage->erase(storage->ctx)) return TRUST_E_STORAGE;
    t->state = TRUST_UNPAIRED;
    return TRUST_OK;
}

int trust_retry_erase(trust_t *t, const trust_storage_t *storage)
{
    if (!t || !storage || !storage->erase) return TRUST_E_ARG;
    if (t->state != TRUST_REVOKED) return TRUST_E_STATE;
    if (storage->erase(storage->ctx)) return TRUST_E_STORAGE;
    t->state = TRUST_UNPAIRED;
    return TRUST_OK;
}

int trust_restore(trust_t *t, const trust_storage_t *storage)
{
    uint8_t record[TRUST_STORE_BLOB_LEN];
    if (!t || !storage || !storage->load_active) return TRUST_E_ARG;
    if (t->state != TRUST_UNPAIRED) return TRUST_E_STATE;
    memset(record, 0, sizeof(record));
    if (storage->load_active(storage->ctx, record)) return TRUST_E_STORAGE;
    if (!record_valid(record)) {
        secure_zero(record, sizeof(record));
        return TRUST_E_RECORD;
    }
    memcpy(t->active_blob, record, sizeof(t->active_blob));
    t->generation = get_u32(record + REC_GENERATION);
    secure_zero(record, sizeof(record));
    t->state = TRUST_ACTIVE;
    return TRUST_OK;
}
