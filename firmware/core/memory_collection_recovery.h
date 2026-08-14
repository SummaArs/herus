/* memory_collection_recovery.h — pure crash-recovery decision for a collection.
 *
 * This is a portable, side-effect-free recovery oracle. It classifies only an
 * already-decoded view of PREPARED, COMMITTED and an independent monotonic floor.
 * It never reads storage, decrypts blobs, holds a key, exposes a card, performs
 * I/O or makes a platform claim. The collection owns authentication and any
 * actual store/erase operation after this oracle returns a bounded decision.
 */
#ifndef HERUS_MEMORY_COLLECTION_RECOVERY_H
#define HERUS_MEMORY_COLLECTION_RECOVERY_H

#include <stdint.h>

typedef enum {
    MEMORY_COLLECTION_RECOVERY_EMPTY = 0,
    MEMORY_COLLECTION_RECOVERY_USE_COMMITTED,
    MEMORY_COLLECTION_RECOVERY_PROMOTE_PREPARED,
    MEMORY_COLLECTION_RECOVERY_FINALIZE_PREPARED,
    MEMORY_COLLECTION_RECOVERY_DISCARD_PREPARED,
    MEMORY_COLLECTION_RECOVERY_BLOCKED
} memory_collection_recovery_action_t;

/* All booleans are canonical: exactly 0 or 1. Generations are nonzero whenever
 * a record is present. `prepared_matches_committed` is evaluated by the caller
 * only after both authenticated records were decoded; it is required only for
 * an interrupted cleanup where both names carry the same generation. */
typedef struct {
    uint8_t committed_present;
    uint8_t prepared_present;
    uint8_t committed_authenticated;
    uint8_t prepared_authenticated;
    uint8_t prepared_matches_committed;
    uint32_t committed_generation;
    uint32_t prepared_generation;
    uint32_t prepared_base_generation;
    uint32_t durable_generation_floor;
} memory_collection_recovery_snapshot_t;

enum {
    MEMORY_COLLECTION_RECOVERY_OK       =  0,
    MEMORY_COLLECTION_RECOVERY_E_ARG    = -1,
    MEMORY_COLLECTION_RECOVERY_E_INVALID = -2
};

/* Decides one and only one safe recovery action. Any malformed snapshot,
 * unauthenticated present record, impossible generation relationship or floor
 * contradiction returns E_INVALID and BLOCKED. The caller must fail closed. */
int memory_collection_recovery_assess(
    const memory_collection_recovery_snapshot_t *snapshot,
    memory_collection_recovery_action_t *out_action);

#endif /* HERUS_MEMORY_COLLECTION_RECOVERY_H */
