/*
 * HERUS symbol_registry — versioned collision-aware symbol handles.
 *
 * This boundary is deliberately independent from the legacy reasoner ABI. It
 * emits opaque 32-bit handles only after exact membership or exact interning.
 * Personal key bytes live in a bounded private runtime table and are never
 * exported as facts, proofs, logs or telemetry.
 */
#ifndef HERUS_SYMBOL_REGISTRY_H
#define HERUS_SYMBOL_REGISTRY_H

#include <stddef.h>
#include <stdint.h>

#define SRREG_NAMESPACE_FACTORY  1u
#define SRREG_NAMESPACE_PERSONAL 2u
#define SRREG_MAX_KEY_BYTES      32u
#define SRREG_MAX_ENTRIES        32u
#define SRREG_HANDLE_SLOT_MASK   0xffffu

typedef uint32_t srreg_handle_t;

typedef enum {
    SRREG_OK = 0,
    SRREG_UNKNOWN = 1,
    SRREG_AUTH = 2,
    SRREG_COLLISION = 3,
    SRREG_FULL = 4,
    SRREG_VERSION_MISMATCH = 5,
    SRREG_INVALID = 6
} srreg_status_t;

typedef struct {
    uint8_t version;
    const char *const *keys;
    uint16_t count;
} srreg_factory_t;

typedef struct {
    uint8_t used;
    uint8_t length;
    char key[SRREG_MAX_KEY_BYTES];
} srreg_personal_entry_t;

typedef struct {
    uint8_t version;
    uint16_t capacity;
    uint16_t count;
    srreg_personal_entry_t entry[SRREG_MAX_ENTRIES];
} srreg_personal_t;

srreg_handle_t srreg_handle_make(uint8_t namespace_id, uint8_t version,
                                 uint16_t slot);
uint8_t srreg_handle_namespace(srreg_handle_t handle);
uint8_t srreg_handle_version(srreg_handle_t handle);
uint16_t srreg_handle_slot(srreg_handle_t handle);

uint16_t srreg_hash16(const char *text, size_t length);
int srreg_factory_resolve(const srreg_factory_t *factory,
                          const char *text, size_t length,
                          srreg_handle_t *out);
int srreg_project_legacy(srreg_handle_t handle, uint8_t active_version,
                         uint16_t *out);

int srreg_personal_init(srreg_personal_t *registry, uint8_t version,
                       uint16_t capacity);
int srreg_personal_resolve(srreg_personal_t *registry,
                           const char *text, size_t length,
                           uint8_t explicit_confirmation,
                           srreg_handle_t *out);
int srreg_personal_accept(const srreg_personal_t *registry,
                          srreg_handle_t handle);
int srreg_personal_migrate(const srreg_personal_t *source,
                           uint8_t new_version,
                           srreg_personal_t *destination);
unsigned srreg_personal_export(const srreg_personal_t *registry,
                               srreg_handle_t *out, unsigned capacity);
unsigned srreg_personal_count(const srreg_personal_t *registry);

#endif /* HERUS_SYMBOL_REGISTRY_H */
