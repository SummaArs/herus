/* memory_vault.h — encrypted local vault for minimal semantic memory cards.
 *
 * This is deliberately not a memory database. It seals one minimal, typed card
 * after explicit human authorization; it cannot accept text/audio, a raw extractor
 * candidate, an LLM result, a network packet or a radio command. The public API
 * never returns a root key, derived key, nonce, ciphertext blob, audio, text,
 * transcript, embedding, identity, location or raw capture payload.
 */
#ifndef HERUS_MEMORY_VAULT_H
#define HERUS_MEMORY_VAULT_H

#include <stdint.h>
#include "memory_policy.h"
#include "memory_extract.h"
#include "memory_vault_store.h"

#define MEMORY_VAULT_VERSION 1u

typedef enum {
    MEMORY_VAULT_UNINITIALIZED = 0,
    MEMORY_VAULT_READY,
    MEMORY_VAULT_SEALED,
    MEMORY_VAULT_BLOCKED
} memory_vault_state_t;

/* A card intentionally has no string, audio, embedding, identity, location, key,
 * timestamp or network field. It preserves only typed provenance needed for a
 * later human-controlled review/consolidation lifecycle. */
typedef struct {
    uint32_t             card_id;
    uint32_t             review_receipt_id; /* opaque local proof of prior human review */
    memory_signal_t      signal;
    memory_extract_origin_t origin;
    uint32_t             extract_reasons;
} memory_vault_card_t;

/* This is a deliberate authority boundary. Future Step 5 will issue it only after
 * its review/consolidation policy. The vault rejects zero/non-canonical fields and
 * never derives this authorization from a candidate, model, dialogue or radio. */
typedef struct {
    uint32_t card_id;
    uint32_t review_receipt_id;
    uint8_t  human_confirmed; /* exactly 1 is true */
} memory_vault_write_authorization_t;

/* Destructive erasure has a separate, vault-bound authority. It cannot be
 * synthesized from write authority or vault state alone. */
typedef struct {
    uint32_t vault_id;
    uint32_t physical_session_id;
    uint8_t  human_confirmed; /* exactly 1 is true */
} memory_vault_erase_authorization_t;

typedef struct {
    uint32_t vault_id;       /* non-secret local partition/context identifier */
    memory_vault_storage_t storage;
} memory_vault_config_t;

/* Numeric-only diagnostics; no card id, plaintext property, key, nonce, address or
 * semantic content is retained as telemetry. */
typedef struct {
    uint32_t seals;
    uint32_t opens;
    uint32_t erases;
    uint32_t rejected_authorization;
    uint32_t rejected_card;
    uint32_t authentication_failures;
    uint32_t backend_failures;
    uint32_t rollback_failures;
} memory_vault_metrics_t;

typedef struct {
    memory_vault_config_t cfg;
    memory_vault_state_t  state;
    uint32_t              current_generation;
    memory_vault_metrics_t metrics;
} memory_vault_t;

enum {
    MEMORY_VAULT_OK              =  0,
    MEMORY_VAULT_E_ARG           = -1,
    MEMORY_VAULT_E_CONFIG        = -2,
    MEMORY_VAULT_E_STATE         = -3,
    MEMORY_VAULT_E_AUTH          = -4,
    MEMORY_VAULT_E_CARD          = -5,
    MEMORY_VAULT_E_ROOT          = -6,
    MEMORY_VAULT_E_STORAGE       = -7,
    MEMORY_VAULT_E_AUTHENTICITY  = -8,
    MEMORY_VAULT_E_ROLLBACK      = -9,
    MEMORY_VAULT_E_ERASE         = -10
};

/* Initialises a vault in READY state only if ports and nonzero vault_id are present.
 * No root material is loaded, but the independent durable generation floor is read. */
int memory_vault_init(memory_vault_t *v, const memory_vault_config_t *cfg);

/* Seal one minimal card. The caller supplies explicit authorization; no memory
 * candidate can call this API implicitly. A card may be sealed only if it is a
 * self/ordinary, session-authorised, high-confidence AUTO_ELIGIBLE-compatible
 * observation. The plaintext working copy and derived key are scrubbed on return. */
int memory_vault_seal(memory_vault_t *v, const memory_vault_write_authorization_t *auth,
                      const memory_vault_card_t *card);

/* Open and validate the most recent sealed card into caller-provided RAM. Any
 * malformed header, wrong context, AEAD failure, stale generation or malformed
 * plaintext fails closed and leaves `out` zeroed. */
int memory_vault_open(memory_vault_t *v, uint32_t expected_card_id,
                      memory_vault_card_t *out);

/* Erase persistent record only under explicit, vault-bound human authority. If
 * the backend erase fails, the vault becomes BLOCKED; no later open or seal may
 * use possibly stale state. */
int memory_vault_erase(memory_vault_t *v,
                       const memory_vault_erase_authorization_t *auth);

const memory_vault_metrics_t *memory_vault_metrics(const memory_vault_t *v);

#endif /* HERUS_MEMORY_VAULT_H */
