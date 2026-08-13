/* trust.h — explicit Core/Nucleus companion trust lifecycle.
 *
 * This is an application-layer ceremony above BLE LE Secure Connections or OOB.
 * It receives an already-authenticated transport secret and platform-generated
 * nonces, then requires dual physical confirmation before deriving the Core/Nucleus
 * control key. It has no BLE, GPIO, RNG, filesystem, radio or transport calls.
 */
#ifndef HERUS_TRUST_H
#define HERUS_TRUST_H

#include <stdint.h>
#include "core_link.h"
#include "trust_store.h"

#define TRUST_VERSION          TRUST_STORE_VERSION
#define TRUST_NONCE_LEN       16u
#define TRUST_SAS_MODULO 1000000u
#define TRUST_OFFER_TTL_MS  60000u

typedef enum {
    TRUST_UNPAIRED = 0,
    TRUST_OFFERED,
    TRUST_CONFIRMED,
    TRUST_ACTIVE,
    TRUST_REVOKED
} trust_state_t;

typedef struct {
    trust_state_t state;
    uint32_t      generation;
    uint8_t       active_blob[TRUST_STORE_BLOB_LEN];
    uint8_t       transport_secret[SHA256_LEN];
    uint8_t       core_nonce[TRUST_NONCE_LEN];
    uint8_t       nucleus_nonce[TRUST_NONCE_LEN];
    uint32_t      offer_started_ms;
    uint32_t      sas;
} trust_t;

enum {
    TRUST_OK          =  0,
    TRUST_E_ARG       = -1,
    TRUST_E_STATE     = -2,
    TRUST_E_PHYSICAL  = -3,
    TRUST_E_SECRET    = -4,
    TRUST_E_TIMEOUT   = -5,
    TRUST_E_CONFIRM   = -6,
    TRUST_E_STORAGE   = -7,
    TRUST_E_RECORD    = -8
};

void trust_init(trust_t *t);

/* Begin only when BOTH devices are in explicit physical pairing mode. `secret`
 * is supplied by authenticated BLE LE Secure Connections or an equivalent OOB
 * channel; the caller obtains the two platform-random 128-bit nonces. */
int trust_begin(trust_t *t, int core_pair_button, int nucleus_pair_button,
                const uint8_t secret[SHA256_LEN],
                const uint8_t core_nonce[TRUST_NONCE_LEN],
                const uint8_t nucleus_nonce[TRUST_NONCE_LEN], uint32_t now_ms);

/* Return the six-digit comparison value only while an offer is pending. */
int trust_sas(const trust_t *t, uint32_t *out_sas);

/* Both device UIs must explicitly confirm and report the SAME displayed SAS.
 * Failure, divergence or failure to store leaves no active key. */
int trust_confirm(trust_t *t, int core_confirmed, uint32_t core_sas,
                  int nucleus_confirmed, uint32_t nucleus_sas,
                  uint32_t now_ms, const trust_storage_t *storage);

/* Expire a pending offer at 60 s, zeroizing all provisional material. */
int trust_tick(trust_t *t, uint32_t now_ms);
void trust_cancel(trust_t *t);

/* The key remains inside this module. These adapters are the only production
 * path from an ACTIVE companion trust into the raw A6 envelope functions. */
int trust_seal_nucleus_intent(const trust_t *t, core_link_tx_t *tx,
                              uint32_t now_ms, uint32_t session_id,
                              uint32_t expires_ms,
                              const intent_observation_t *observation,
                              uint8_t out[CORE_LINK_WIRE_LEN]);
int trust_open_nucleus_intent(const trust_t *t, core_link_rx_t *rx,
                              const uint8_t *wire, uint32_t wire_len,
                              uint32_t now_ms, core_link_intent_t *out);

/* Immediately stop export and scrub the active key plus A6 anti-replay state.
 * If erase fails the object remains REVOKED (not re-pairable) until retry succeeds. */
int trust_revoke(trust_t *t, core_link_tx_t *tx, core_link_rx_t *rx,
                 const trust_storage_t *storage);
int trust_retry_erase(trust_t *t, const trust_storage_t *storage);

/* Restore only a structurally valid ACTIVE record supplied by protected storage. */
int trust_restore(trust_t *t, const trust_storage_t *storage);

#endif /* HERUS_TRUST_H */
