/* trust_store.h — protected persistence port for a Core/Nucleus trust binding.
 *
 * The record is an opaque fixed-size blob. Product code outside trust.c never
 * receives a pair_key field or a typed key record. A target implementation must
 * store this blob through a secure element, encrypted NVS, or equivalent trusted
 * boundary; product telemetry must never receive it. Host tests use a RAM vault.
 */
#ifndef HERUS_TRUST_STORE_H
#define HERUS_TRUST_STORE_H

#include <stdint.h>

#define TRUST_STORE_BLOB_LEN 44u
#define TRUST_STORE_VERSION  1u

/* Zero means success. `load_active` returns a blob only; trust.c alone validates
 * version, active state, generation, identifier and nonzero key material. */
typedef struct {
    void *ctx;
    int (*store_active)(void *ctx, const uint8_t blob[TRUST_STORE_BLOB_LEN]);
    int (*load_active)(void *ctx, uint8_t blob[TRUST_STORE_BLOB_LEN]);
    int (*erase)(void *ctx);
} trust_storage_t;

#endif /* HERUS_TRUST_STORE_H */
