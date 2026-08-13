/* memory_vault_store.h — opaque sealed-record storage port.
 *
 * The root is deliberately absent from every public type. memory_vault.c obtains it
 * only through its private platform adapter for the duration of an operation. This
 * port stores the fixed sealed blob and the independent generation floor. The host
 * test backend uses RAM; an ESP32-S3 backend may map these operations to NVS
 * encryption, HMAC/eFuse or a secure element only after target-specific review.
 */
#ifndef HERUS_MEMORY_VAULT_STORE_H
#define HERUS_MEMORY_VAULT_STORE_H

#include <stdint.h>

#define MEMORY_VAULT_ROOT_LEN 32u
#define MEMORY_VAULT_BLOB_LEN 68u

typedef struct {
    void *ctx;
    /* Opaque fixed-size record only. Zero means success. */
    int (*store_sealed)(void *ctx, const uint8_t blob[MEMORY_VAULT_BLOB_LEN]);
    int (*load_sealed)(void *ctx, uint8_t blob[MEMORY_VAULT_BLOB_LEN]);
    int (*erase_sealed)(void *ctx);
    /* Trusted monotonic floor, independent from the replaceable blob. The backend
     * must reject decreases; it is the mandatory anti-rollback anchor. Host tests
     * emulate it, while target hardware must provide a reviewed durable mapping. */
    int (*load_generation_floor)(void *ctx, uint32_t *out);
    int (*commit_generation_floor)(void *ctx, uint32_t generation);
} memory_vault_storage_t;

#endif /* HERUS_MEMORY_VAULT_STORE_H */
