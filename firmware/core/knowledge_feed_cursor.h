/*
 * HERUS knowledge_feed_cursor — durable anti-rollback floor for Core feeds.
 *
 * The storage adapter receives canonical byte arrays, never a C struct. Two
 * slots provide recovery from an interrupted replacement; an invalid present
 * slot blocks recovery instead of silently selecting a stale record.
 */
#ifndef HERUS_KNOWLEDGE_FEED_CURSOR_H
#define HERUS_KNOWLEDGE_FEED_CURSOR_H

#include "knowledge_feed.h"
#include <stdint.h>

#define KFC_SLOT_COUNT        2u
#define KFC_RECORD_BYTES      62u
#define KFC_FORMAT_VERSION    1u
#define KFC_STORE_ABSENT      1

typedef enum {
    KFC_OK = 0,
    KFC_EMPTY = 1,
    KFC_E_ARG = -1,
    KFC_E_STORAGE = -2,
    KFC_E_AUTH = -3,
    KFC_E_ROLLBACK = -4,
    KFC_E_VERSION = -5,
    KFC_E_FORMAT = -6
} kfc_status_t;

typedef int (*kfc_read_fn)(void *ctx, uint8_t slot,
                           uint8_t out[KFC_RECORD_BYTES]);
typedef int (*kfc_write_fn)(void *ctx, uint8_t slot,
                            const uint8_t record[KFC_RECORD_BYTES]);

typedef struct {
    void *ctx;
    kfc_read_fn read;
    kfc_write_fn write;
} kfc_storage_t;

typedef struct {
    uint8_t initialized;
    uint8_t active_slot;
    uint8_t registry_version;
    uint32_t generation;
    kf_cursor_t cursor;
} kfc_state_t;

/* Loads both slots and chooses only the newest authenticated monotonic record.
 * KFC_EMPTY is the only clean result for two absent slots. */
kfc_status_t kfc_load(const kfc_storage_t *storage,
                      const uint8_t key[32],
                      uint8_t registry_version,
                      kfc_state_t *out);

/* Writes the alternate slot, reads both slots back and promotes state only if
 * the new authenticated sequence is recovered. */
kfc_status_t kfc_commit(const kfc_storage_t *storage,
                        const uint8_t key[32],
                        uint8_t registry_version,
                        uint32_t sequence,
                        const uint8_t payload_digest[KF_DIGEST_LEN],
                        kfc_state_t *state);

#endif /* HERUS_KNOWLEDGE_FEED_CURSOR_H */
