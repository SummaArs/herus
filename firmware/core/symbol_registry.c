#include "symbol_registry.h"

#include <string.h>

#define SRREG_HASH_OFFSET 2166136261u
#define SRREG_HASH_PRIME  16777619u

static unsigned char lower_ascii(unsigned char c)
{
    return c >= (unsigned char)'A' && c <= (unsigned char)'Z'
               ? (unsigned char)(c + ((unsigned char)'a' - (unsigned char)'A'))
               : c;
}

static int valid_key(const char *text, size_t length)
{
    return text != NULL && length > 0u && length < SRREG_MAX_KEY_BYTES;
}

static int same_key(const char *a, size_t a_length,
                    const char *b, size_t b_length)
{
    if (!a || !b || a_length != b_length) return 0;
    for (size_t i = 0u; i < a_length; i++) {
        if (lower_ascii((unsigned char)a[i]) !=
            lower_ascii((unsigned char)b[i])) return 0;
    }
    return 1;
}

static void copy_key(char *destination, const char *source, size_t length)
{
    for (size_t i = 0u; i < length; i++)
        destination[i] = (char)lower_ascii((unsigned char)source[i]);
    destination[length] = '\0';
}

srreg_handle_t srreg_handle_make(uint8_t namespace_id, uint8_t version,
                                 uint16_t slot)
{
    if (namespace_id == 0u || slot == 0u) return 0u;
    return ((srreg_handle_t)namespace_id << 24) |
           ((srreg_handle_t)version << 16) | (srreg_handle_t)slot;
}

uint8_t srreg_handle_namespace(srreg_handle_t handle)
{
    return (uint8_t)(handle >> 24);
}

uint8_t srreg_handle_version(srreg_handle_t handle)
{
    return (uint8_t)(handle >> 16);
}

uint16_t srreg_handle_slot(srreg_handle_t handle)
{
    return (uint16_t)(handle & SRREG_HANDLE_SLOT_MASK);
}

uint16_t srreg_hash16(const char *text, size_t length)
{
    uint32_t hash = SRREG_HASH_OFFSET;
    if (!valid_key(text, length)) return 0u;
    for (size_t i = 0u; i < length; i++) {
        hash ^= lower_ascii((unsigned char)text[i]);
        hash *= SRREG_HASH_PRIME;
    }
    {
        uint16_t folded = (uint16_t)((hash >> 16) ^ (hash & 0xffffu));
        return folded == 0u ? 1u : folded;
    }
}

int srreg_project_legacy(srreg_handle_t handle, uint8_t active_version,
                         uint16_t *out)
{
    uint8_t namespace_id;
    uint16_t slot;
    if (!out || handle == 0u || active_version == 0u)
        return SRREG_INVALID;
    if (srreg_handle_version(handle) != active_version)
        return SRREG_VERSION_MISMATCH;
    namespace_id = srreg_handle_namespace(handle);
    slot = srreg_handle_slot(handle);
    if ((namespace_id != SRREG_NAMESPACE_FACTORY &&
         namespace_id != SRREG_NAMESPACE_PERSONAL) || slot == 0u)
        return SRREG_INVALID;
    if (slot > 0x7fffu) return SRREG_FULL;
    *out = (uint16_t)(slot |
                      (namespace_id == SRREG_NAMESPACE_PERSONAL ? 0x8000u : 0u));
    return SRREG_OK;
}

int srreg_factory_resolve(const srreg_factory_t *factory,
                          const char *text, size_t length,
                          srreg_handle_t *out)
{
    if (!factory || !out || !valid_key(text, length) ||
        factory->version == 0u || !factory->keys) return SRREG_INVALID;
    for (uint16_t i = 0u; i < factory->count; i++) {
        const char *key = factory->keys[i];
        size_t key_length = key ? strlen(key) : 0u;
        if (valid_key(key, key_length) && same_key(text, length, key, key_length)) {
            *out = srreg_handle_make(SRREG_NAMESPACE_FACTORY, factory->version,
                                     (uint16_t)(i + 1u));
            return *out == 0u ? SRREG_INVALID : SRREG_OK;
        }
    }
    return SRREG_UNKNOWN;
}

int srreg_personal_init(srreg_personal_t *registry, uint8_t version,
                       uint16_t capacity)
{
    if (!registry || version == 0u || capacity > SRREG_MAX_ENTRIES)
        return SRREG_INVALID;
    memset(registry, 0, sizeof(*registry));
    registry->version = version;
    registry->capacity = capacity;
    return SRREG_OK;
}

int srreg_personal_resolve(srreg_personal_t *registry,
                           const char *text, size_t length,
                           uint8_t explicit_confirmation,
                           srreg_handle_t *out)
{
    if (!registry || !out || !valid_key(text, length)) return SRREG_INVALID;
    for (uint16_t i = 0u; i < registry->count; i++) {
        srreg_personal_entry_t *entry = &registry->entry[i];
        if (entry->used && same_key(text, length, entry->key, entry->length)) {
            *out = srreg_handle_make(SRREG_NAMESPACE_PERSONAL, registry->version,
                                     (uint16_t)(i + 1u));
            return *out == 0u ? SRREG_INVALID : SRREG_OK;
        }
    }
    if (explicit_confirmation != 1u) return SRREG_AUTH;
    if (registry->count >= registry->capacity ||
        registry->count >= SRREG_MAX_ENTRIES) return SRREG_FULL;
    {
        srreg_personal_entry_t *entry = &registry->entry[registry->count];
        entry->used = 1u;
        entry->length = (uint8_t)length;
        copy_key(entry->key, text, length);
        registry->count++;
        *out = srreg_handle_make(SRREG_NAMESPACE_PERSONAL, registry->version,
                                 registry->count);
    }
    return *out == 0u ? SRREG_INVALID : SRREG_OK;
}

int srreg_personal_accept(const srreg_personal_t *registry,
                          srreg_handle_t handle)
{
    uint16_t slot;
    if (!registry || srreg_handle_namespace(handle) != SRREG_NAMESPACE_PERSONAL)
        return SRREG_INVALID;
    if (srreg_handle_version(handle) != registry->version)
        return SRREG_VERSION_MISMATCH;
    slot = srreg_handle_slot(handle);
    if (slot == 0u || slot > registry->count ||
        registry->entry[slot - 1u].used == 0u) return SRREG_UNKNOWN;
    return SRREG_OK;
}

int srreg_personal_migrate(const srreg_personal_t *source,
                           uint8_t new_version,
                           srreg_personal_t *destination)
{
    srreg_handle_t handle;
    if (!source || !destination || new_version == 0u ||
        srreg_personal_init(destination, new_version, source->capacity) != SRREG_OK)
        return SRREG_INVALID;
    for (uint16_t i = 0u; i < source->count; i++) {
        const srreg_personal_entry_t *entry = &source->entry[i];
        if (!entry->used) return SRREG_INVALID;
        if (srreg_personal_resolve(destination, entry->key, entry->length,
                                   1u, &handle) != SRREG_OK)
            return SRREG_FULL;
    }
    return SRREG_OK;
}

unsigned srreg_personal_export(const srreg_personal_t *registry,
                               srreg_handle_t *out, unsigned capacity)
{
    unsigned count;
    if (!registry || !out || capacity == 0u) return 0u;
    count = registry->count < capacity ? registry->count : capacity;
    for (unsigned i = 0u; i < count; i++)
        out[i] = srreg_handle_make(SRREG_NAMESPACE_PERSONAL,
                                   registry->version, (uint16_t)(i + 1u));
    return count;
}

unsigned srreg_personal_count(const srreg_personal_t *registry)
{
    return registry ? registry->count : 0u;
}
