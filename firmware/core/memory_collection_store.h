/* memory_collection_store.h — opaque transactional collection-storage port.
 *
 * The collection is portable C11. This port deliberately names neither an ESP-IDF
 * API nor a flash address, filesystem, secure element, eFuse or block size. A
 * production adapter may use any reviewed target backend; the host suite uses RAM.
 * Root material is absent from this public interface and loaded only by a private
 * collection implementation seam for the duration of cryptographic operations.
 */
#ifndef HERUS_MEMORY_COLLECTION_STORE_H
#define HERUS_MEMORY_COLLECTION_STORE_H

#include <stdint.h>

#define MEMORY_COLLECTION_ROOT_LEN 32u
#define MEMORY_COLLECTION_BLOB_LEN 272u

/* `load_*` callbacks return this exact positive code only when the named record is
 * absent. Zero is success; every other nonzero result is a backend failure. */
#define MEMORY_COLLECTION_STORE_ABSENT 1

typedef struct {
    void *ctx;

    /* The prepared record contains a fully authenticated candidate transaction.
     * The committed record contains a fully authenticated active collection. */
    int (*store_prepared)(void *ctx,
                          const uint8_t blob[MEMORY_COLLECTION_BLOB_LEN]);
    int (*load_prepared)(void *ctx,
                         uint8_t blob[MEMORY_COLLECTION_BLOB_LEN]);
    int (*erase_prepared)(void *ctx);
    int (*store_committed)(void *ctx,
                           const uint8_t blob[MEMORY_COLLECTION_BLOB_LEN]);
    int (*load_committed)(void *ctx,
                          uint8_t blob[MEMORY_COLLECTION_BLOB_LEN]);
    int (*erase_committed)(void *ctx);

    /* Independent anti-rollback anchor. The backend must reject decreases and
     * make a successful commit durable before reporting success. Portable tests
     * emulate this contract; target power-loss properties require real evidence. */
    int (*load_generation_floor)(void *ctx, uint32_t *out);
    int (*commit_generation_floor)(void *ctx, uint32_t generation);
} memory_collection_storage_t;

#endif /* HERUS_MEMORY_COLLECTION_STORE_H */
