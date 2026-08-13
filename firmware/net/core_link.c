/* core_link.c — authenticated Core/Nucleus companion-control envelope. */
#include "core_link.h"
#include <string.h>

#define OFF_VER       0u
#define OFF_DIR       1u
#define OFF_PAIR      2u
#define OFF_SEQ       6u
#define OFF_SESSION  14u
#define OFF_EXPIRES  18u
#define OFF_PAYLOAD  CORE_LINK_HEADER_LEN
#define OFF_TAG     (CORE_LINK_HEADER_LEN + CORE_LINK_PAYLOAD_LEN)

static const uint8_t KDF_INFO[] = "HERUS/CORE-NUCLEUS/CONTROL/v1";

static void put_u32(uint8_t *p, uint32_t x)
{
    p[0] = (uint8_t)(x >> 24); p[1] = (uint8_t)(x >> 16);
    p[2] = (uint8_t)(x >> 8);  p[3] = (uint8_t)x;
}

static uint32_t get_u32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static void put_u64_56(uint8_t *p, uint64_t x)
{
    for (unsigned i = 0; i < 7; i++) p[i] = (uint8_t)(x >> (48u - 8u * i));
}

static uint64_t get_u64_56(const uint8_t *p)
{
    uint64_t x = 0;
    for (unsigned i = 0; i < 7; i++) x = (x << 8) | p[i];
    return x;
}

static int key_valid(const core_link_key_t *key)
{
    uint8_t zero[SHA256_LEN] = {0};
    return key && key->pair_id && !ct_eq(key->pair_key, zero, sizeof(zero));
}

static int command_valid(voice_command_t command, uint8_t minutes)
{
    if (command == VOICE_COMMAND_ARRIVE) return minutes <= 60u;
    if (command == VOICE_COMMAND_HELP || command == VOICE_COMMAND_CANCEL) return minutes == 0u;
    return 0;
}

static int observation_valid(const intent_observation_t *o)
{
    return o && o->source == INTENT_SOURCE_NUCLEUS && command_valid(o->command, o->minutes) &&
           o->confidence_pct <= 100u && o->runner_up_pct <= o->confidence_pct;
}

static int derive_key_nonce(const core_link_key_t *key, uint8_t direction, uint64_t seq,
                            uint8_t out_key[SHA256_LEN], uint8_t nonce[12])
{
    uint8_t salt[4];
    if (!key_valid(key) || !seq || seq > CORE_LINK_SEQ_MAX) return -1;
    put_u32(salt, key->pair_id);
    if (hkdf(salt, sizeof(salt), key->pair_key, sizeof(key->pair_key),
             KDF_INFO, sizeof(KDF_INFO) - 1u, out_key, SHA256_LEN)) return -1;
    put_u32(nonce, key->pair_id);
    nonce[4] = direction;
    put_u64_56(nonce + 5, seq);
    return 0;
}

void core_link_tx_init(core_link_tx_t *tx)
{
    if (tx) tx->next_seq = 1u;
}

void core_link_rx_init(core_link_rx_t *rx)
{
    if (rx) rx->last_seq = 0u;
}

int core_link_seal_nucleus_intent(core_link_tx_t *tx, const core_link_key_t *key,
                                  uint32_t now_ms, uint32_t session_id,
                                  uint32_t expires_ms,
                                  const intent_observation_t *observation,
                                  uint8_t out[CORE_LINK_WIRE_LEN])
{
    uint8_t control_key[SHA256_LEN], nonce[12], payload[CORE_LINK_PAYLOAD_LEN];
    uint32_t ttl;
    uint64_t seq;
    if (!tx || !out || !session_id || !expires_ms || !observation_valid(observation) ||
        observation->session_id != session_id)
        return CORE_LINK_E_ARG;
    ttl = (uint32_t)(expires_ms - now_ms);
    if ((int32_t)ttl <= 0 || ttl > CORE_LINK_MAX_TTL_MS) return CORE_LINK_E_ARG;
    seq = tx->next_seq;
    if (!seq || seq > CORE_LINK_SEQ_MAX || derive_key_nonce(key, CORE_LINK_DIR_NUCLEUS_TO_CORE,
                                                              seq, control_key, nonce))
        return CORE_LINK_E_ARG;

    memset(out, 0, CORE_LINK_WIRE_LEN);
    out[OFF_VER] = CORE_LINK_VERSION;
    out[OFF_DIR] = CORE_LINK_DIR_NUCLEUS_TO_CORE;
    put_u32(out + OFF_PAIR, key->pair_id);
    put_u64_56(out + OFF_SEQ, seq);
    put_u32(out + OFF_SESSION, session_id);
    put_u32(out + OFF_EXPIRES, expires_ms);
    payload[0] = (uint8_t)observation->source;
    payload[1] = (uint8_t)observation->command;
    payload[2] = observation->minutes;
    payload[3] = observation->confidence_pct;
    payload[4] = observation->runner_up_pct;
    aead_encrypt(control_key, nonce, out, CORE_LINK_HEADER_LEN, payload, sizeof(payload),
                 out + OFF_PAYLOAD, out + OFF_TAG, CORE_LINK_TAG_LEN);
    secure_zero(control_key, sizeof(control_key));
    secure_zero(payload, sizeof(payload));
    tx->next_seq++;
    return CORE_LINK_OK;
}

int core_link_open_nucleus_intent(core_link_rx_t *rx, const core_link_key_t *key,
                                  const uint8_t *wire, size_t wire_len,
                                  uint32_t now_ms, core_link_intent_t *out)
{
    uint8_t control_key[SHA256_LEN], nonce[12], payload[CORE_LINK_PAYLOAD_LEN];
    uint64_t seq;
    uint32_t pair, session, expires;
    int rc = CORE_LINK_E_FORMAT;
    if (out) memset(out, 0, sizeof(*out));
    if (!rx || !key || !wire || !out || wire_len != CORE_LINK_WIRE_LEN) return CORE_LINK_E_ARG;
    if (wire[OFF_VER] != CORE_LINK_VERSION) return CORE_LINK_E_FORMAT;
    if (wire[OFF_DIR] != CORE_LINK_DIR_NUCLEUS_TO_CORE) return CORE_LINK_E_DIR;
    pair = get_u32(wire + OFF_PAIR);
    if (!key_valid(key) || pair != key->pair_id) return CORE_LINK_E_PAIR;
    seq = get_u64_56(wire + OFF_SEQ);
    if (!seq || seq > CORE_LINK_SEQ_MAX || seq <= rx->last_seq) return CORE_LINK_E_REPLAY;
    if (derive_key_nonce(key, wire[OFF_DIR], seq, control_key, nonce)) return CORE_LINK_E_ARG;
    if (aead_decrypt(control_key, nonce, wire, CORE_LINK_HEADER_LEN,
                     wire + OFF_PAYLOAD, CORE_LINK_PAYLOAD_LEN, wire + OFF_TAG,
                     CORE_LINK_TAG_LEN, payload)) {
        rc = CORE_LINK_E_AUTH;
        goto done;
    }
    session = get_u32(wire + OFF_SESSION);
    expires = get_u32(wire + OFF_EXPIRES);
    if (!session || !expires) {
        rc = CORE_LINK_E_FORMAT;
        goto done;
    }
    out->session_id = session;
    out->expires_ms = expires;
    out->observation.source = (intent_source_t)payload[0];
    out->observation.session_id = session;
    out->observation.command = (voice_command_t)payload[1];
    out->observation.minutes = payload[2];
    out->observation.confidence_pct = payload[3];
    out->observation.runner_up_pct = payload[4];
    if (!observation_valid(&out->observation)) {
        memset(out, 0, sizeof(*out));
        rc = CORE_LINK_E_FORMAT;
        goto done;
    }
    /* Authenticated packets consume their sequence even if stale in time: otherwise
     * an attacker can repeatedly force parsing of the same expired ciphertext. */
    rx->last_seq = seq;
    if ((int32_t)(expires - now_ms) <= 0) {
        memset(out, 0, sizeof(*out));
        rc = CORE_LINK_E_EXPIRED;
        goto done;
    }
    rc = CORE_LINK_OK;
done:
    secure_zero(control_key, sizeof(control_key));
    secure_zero(payload, sizeof(payload));
    return rc;
}
