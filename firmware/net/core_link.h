/* core_link.h — authenticated companion-control envelope for Core <-> Nucleus.
 *
 * The transport (BLE or authenticated 2.4 GHz) is below this layer. This envelope
 * binds a local Nucleus ASR observation to pair, direction, sequence, expiry and
 * physical PTT session before it reaches the Core intent gateway. It transports no
 * audio, transcript, HCP message, mesh key, location or send authority.
 */
#ifndef HERUS_CORE_LINK_H
#define HERUS_CORE_LINK_H

#include <stddef.h>
#include <stdint.h>
#include "crypto.h"
#include "intent_gate.h"

#define CORE_LINK_VERSION       1u
#define CORE_LINK_HEADER_LEN   22u
#define CORE_LINK_PAYLOAD_LEN   5u
#define CORE_LINK_TAG_LEN      16u
#define CORE_LINK_WIRE_LEN (CORE_LINK_HEADER_LEN + CORE_LINK_PAYLOAD_LEN + CORE_LINK_TAG_LEN)
#define CORE_LINK_SEQ_MAX 0x00FFFFFFFFFFFFFFULL
#define CORE_LINK_MAX_TTL_MS 8000u

typedef enum {
    CORE_LINK_DIR_NUCLEUS_TO_CORE = 1u
} core_link_direction_t;

typedef struct {
    uint8_t  pair_key[SHA256_LEN]; /* target port should source from protected storage */
    uint32_t pair_id;              /* local binding id, never a mesh/BLE address */
} core_link_key_t;

typedef struct { uint64_t next_seq; } core_link_tx_t;
typedef struct { uint64_t last_seq; } core_link_rx_t;

typedef struct {
    uint32_t            session_id;
    uint32_t            expires_ms;
    intent_observation_t observation;
} core_link_intent_t;

enum {
    CORE_LINK_OK        =  0,
    CORE_LINK_E_ARG     = -1,
    CORE_LINK_E_FORMAT  = -2,
    CORE_LINK_E_PAIR    = -3,
    CORE_LINK_E_DIR     = -4,
    CORE_LINK_E_AUTH    = -5,
    CORE_LINK_E_REPLAY  = -6,
    CORE_LINK_E_EXPIRED = -7
};

void core_link_tx_init(core_link_tx_t *tx);
void core_link_rx_init(core_link_rx_t *rx);

/* Seal a Nucleus-local ASR command. Each successful call consumes exactly one
 * sequence. `out` must hold CORE_LINK_WIRE_LEN bytes. */
int core_link_seal_nucleus_intent(core_link_tx_t *tx, const core_link_key_t *key,
                                  uint32_t now_ms, uint32_t session_id,
                                  uint32_t expires_ms,
                                  const intent_observation_t *observation,
                                  uint8_t out[CORE_LINK_WIRE_LEN]);

/* Authenticate, decrypt and replay-check a Nucleus->Core envelope. A valid but
 * expired envelope consumes its sequence and returns CORE_LINK_E_EXPIRED. Any
 * other failure leaves `out` zeroed and never advances rx state. */
int core_link_open_nucleus_intent(core_link_rx_t *rx, const core_link_key_t *key,
                                  const uint8_t *wire, size_t wire_len,
                                  uint32_t now_ms, core_link_intent_t *out);

#endif /* HERUS_CORE_LINK_H */
