/* memory_collection.h — portable, bounded and transactional memory-card collection.
 *
 * This module stores only minimal cards already admitted by the vault contract. It
 * is not a transcript store, search index, semantic deduplicator, model memory,
 * network client or hardware backend. Every mutating operation requires canonical
 * physical access; an ambiguous or failed backend operation blocks the collection.
 */
#ifndef HERUS_MEMORY_COLLECTION_H
#define HERUS_MEMORY_COLLECTION_H

#include <stdint.h>
#include "memory_vault.h"
#include "memory_collection_store.h"
#include "memory_physical_session.h"

#define MEMORY_COLLECTION_VERSION 1u
#define MEMORY_COLLECTION_MAX_CARDS 8u

typedef enum {
    MEMORY_COLLECTION_UNINITIALIZED = 0,
    MEMORY_COLLECTION_READY,
    MEMORY_COLLECTION_BLOCKED
} memory_collection_state_t;

typedef enum {
    MEMORY_COLLECTION_TXN_NONE = 0,
    MEMORY_COLLECTION_TXN_INSERT,
    MEMORY_COLLECTION_TXN_REMOVE,
    MEMORY_COLLECTION_TXN_COMPACT
} memory_collection_txn_t;

/* A collection access names only a transient gate, opaque session ID and observed
 * monotonic time. The gate retains the adapter nonce and checks purpose/freshness
 * before an operation can proceed. This is not proof of a button, gesture,
 * biometric, identity, secure element or clock; those remain platform evidence. */
typedef struct {
    memory_physical_session_t *gate;
    uint32_t physical_session_id;
    uint32_t observed_at_ms;
} memory_collection_access_t;

typedef struct {
    uint32_t collection_id; /* non-secret local context, must be nonzero */
    memory_collection_storage_t storage;
} memory_collection_config_t;

/* Numeric-only metrics. No id, card property, text, key, nonce, address or
 * transaction payload is retained as product telemetry. */
typedef struct {
    uint32_t inserts;
    uint32_t removes;
    uint32_t compactions;
    uint32_t opens;
    uint32_t recoveries;
    uint32_t finalized_prepared;
    uint32_t discarded_prepared;
    uint32_t rejected_access;
    uint32_t rejected_card;
    uint32_t duplicate_rejections;
    uint32_t capacity_rejections;
    uint32_t authentication_failures;
    uint32_t rollback_failures;
    uint32_t backend_failures;
} memory_collection_metrics_t;

typedef struct {
    memory_collection_config_t cfg;
    memory_collection_state_t state;
    uint32_t generation;
    uint8_t count;
    memory_collection_metrics_t metrics;
} memory_collection_t;

enum {
    MEMORY_COLLECTION_OK             =  0,
    MEMORY_COLLECTION_E_ARG          = -1,
    MEMORY_COLLECTION_E_CONFIG       = -2,
    MEMORY_COLLECTION_E_STATE        = -3,
    MEMORY_COLLECTION_E_ACCESS       = -4,
    MEMORY_COLLECTION_E_AUTH         = -5,
    MEMORY_COLLECTION_E_CARD         = -6,
    MEMORY_COLLECTION_E_DUPLICATE    = -7,
    MEMORY_COLLECTION_E_CAPACITY     = -8,
    MEMORY_COLLECTION_E_NOT_FOUND    = -9,
    MEMORY_COLLECTION_E_ROOT         = -10,
    MEMORY_COLLECTION_E_STORAGE      = -11,
    MEMORY_COLLECTION_E_AUTHENTICITY = -12,
    MEMORY_COLLECTION_E_ROLLBACK     = -13,
    MEMORY_COLLECTION_E_RECOVERY     = -14
};

/* Initializes against the independent generation floor. The pure recovery oracle
 * accepts only authenticated, topology-consistent records. A prepared record bound
 * to the new floor is promoted; an old-floor prepared record is discarded; a
 * duplicate prepared copy of committed state is finalized. Any missing record other
 * than the first empty collection, malformed record, contradiction or backend
 * failure leaves the collection BLOCKED. */
int memory_collection_init(memory_collection_t *c,
                           const memory_collection_config_t *cfg);

/* Insert a card after the same explicit, card/receipt-bound human authorization
 * used by the vault and one consumed INSERT-purpose session. The collection
 * revalidates the card and authorization; no candidate, ASR, dialogue, radio or
 * model output may call this path implicitly. */
int memory_collection_insert(memory_collection_t *c,
                             const memory_vault_write_authorization_t *auth,
                             const memory_vault_card_t *card,
                             const memory_collection_access_t *access);

/* Open exactly one known opaque card identifier under one consumed OPEN-purpose
 * session. It never enumerates the collection; `out` is zeroed on every failure. */
int memory_collection_open(memory_collection_t *c, uint32_t expected_card_id,
                           const memory_collection_access_t *access,
                           memory_vault_card_t *out);

/* Logical removal only. It consumes one REMOVE-purpose session and advances the
 * collection generation. It does not claim physical-media sanitization or erase of old bytes. */
int memory_collection_remove(memory_collection_t *c, uint32_t card_id,
                             const memory_collection_access_t *access);

/* Canonically reorders the existing active cards by opaque card id under one
 * COMPACT-purpose session. It cannot add, remove, merge, summarize, deduplicate
 * semantically or alter any card field. */
int memory_collection_compact(memory_collection_t *c,
                              const memory_collection_access_t *access);

const memory_collection_metrics_t *memory_collection_metrics(
    const memory_collection_t *c);

#endif /* HERUS_MEMORY_COLLECTION_H */
