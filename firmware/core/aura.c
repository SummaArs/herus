#include "aura.h"
#include "crypto.h"
#include <string.h>

void aura_book_init(aura_book_t *b)
{
    if (!b) return;
    memset(b, 0, sizeof *b);
}

aura_status_t aura_pair(aura_book_t *b, const uint8_t key[AURA_KEY_BYTES], uint8_t *index)
{
    if (!b || !key) return AURA_E_ARG;
    if (b->count >= AURA_MAX_PEERS) return AURA_E_FULL;
    memcpy(b->peer[b->count].key, key, AURA_KEY_BYTES);
    b->peer[b->count].epoch = 0u;
    b->peer[b->count].active = 1u;
    if (index) *index = b->count;
    b->count++;
    return AURA_OK;
}

aura_status_t aura_revoke(aura_book_t *b, uint8_t index)
{
    if (!b || index >= b->count) return AURA_E_ARG;
    b->peer[index].active = 0u;
    secure_zero(b->peer[index].key, AURA_KEY_BYTES);
    return AURA_OK;
}

void aura_token(const uint8_t key[AURA_KEY_BYTES], uint8_t out[AURA_TOKEN_BYTES])
{
    uint8_t mac[SHA256_LEN];
    if (!key || !out) return;
    hmac_sha256(key, AURA_KEY_BYTES, "aura", 4u, mac);
    memcpy(out, mac, AURA_TOKEN_BYTES);
    secure_zero(mac, sizeof mac);
}

void aura_step(uint8_t key[AURA_KEY_BYTES])
{
    uint8_t next[SHA256_LEN];
    if (!key) return;
    hmac_sha256(key, AURA_KEY_BYTES, "step", 4u, next);
    memcpy(key, next, AURA_KEY_BYTES);
    secure_zero(next, sizeof next);
}

aura_status_t aura_recognize(aura_book_t *b, const uint8_t token[AURA_TOKEN_BYTES],
                             uint8_t *peer_index, uint8_t *epochs_ahead)
{
    uint8_t i, w;
    int any_active = 0;

    if (!b || !token) return AURA_E_ARG;
    if (peer_index) *peer_index = AURA_MAX_PEERS;
    if (epochs_ahead) *epochs_ahead = 0u;

    for (i = 0; i < b->count; i++) {
        uint8_t probe[AURA_KEY_BYTES];
        if (!b->peer[i].active) continue;
        any_active = 1;
        memcpy(probe, b->peer[i].key, AURA_KEY_BYTES);
        for (w = 0; w < AURA_WINDOW; w++) {
            uint8_t t[AURA_TOKEN_BYTES];
            aura_token(probe, t);
            if (ct_eq(t, token, AURA_TOKEN_BYTES)) {
                /* consume the matched epoch: this token is now spent */
                aura_step(probe);
                memcpy(b->peer[i].key, probe, AURA_KEY_BYTES);
                b->peer[i].epoch += (uint32_t)w + 1u;
                if (peer_index) *peer_index = i;
                if (epochs_ahead) *epochs_ahead = w;
                secure_zero(probe, sizeof probe);
                return AURA_OK;
            }
            aura_step(probe);
        }
        secure_zero(probe, sizeof probe);
    }
    if (!any_active && b->count > 0u) return AURA_E_REVOKED;
    return AURA_E_UNKNOWN;
}
