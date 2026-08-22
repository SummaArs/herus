#include "knowledge_feed_cursor.h"
#include "../net/crypto.h"
#include <string.h>

static void put_u32(uint8_t *out, uint32_t value)
{
    out[0] = (uint8_t)(value & 0xffu);
    out[1] = (uint8_t)((value >> 8) & 0xffu);
    out[2] = (uint8_t)((value >> 16) & 0xffu);
    out[3] = (uint8_t)(value >> 24);
}

static uint32_t get_u32(const uint8_t *in)
{
    return (uint32_t)in[0] |
           ((uint32_t)in[1] << 8) |
           ((uint32_t)in[2] << 16) |
           ((uint32_t)in[3] << 24);
}

static int all_zero(const uint8_t *bytes, size_t len)
{
    uint8_t accumulator = 0u;
    for (size_t i = 0u; i < len; i++) accumulator |= bytes[i];
    return accumulator == 0u;
}

static void auth_input(const uint8_t record[KFC_RECORD_BYTES],
                       uint8_t out[37u])
{
    memcpy(out, record + 1u, 37u);
}

static void record_tag(const uint8_t key[32],
                       const uint8_t record[KFC_RECORD_BYTES],
                       uint8_t out[KF_AUTH_TAG_LEN])
{
    uint8_t full[SHA256_LEN];
    uint8_t input[37u];
    auth_input(record, input);
    hmac_sha256(key, 32u, input, sizeof(input), full);
    memcpy(out, full, KF_AUTH_TAG_LEN);
    secure_zero(full, sizeof(full));
    secure_zero(input, sizeof(input));
}

static void make_record(const uint8_t key[32], uint8_t registry_version,
                        uint32_t sequence,
                        const uint8_t digest[KF_DIGEST_LEN],
                        uint8_t out[KFC_RECORD_BYTES])
{
    memset(out, 0, KFC_RECORD_BYTES);
    out[0] = KFC_FORMAT_VERSION;
    out[1] = registry_version;
    put_u32(out + 2u, sequence);
    memcpy(out + 6u, digest, KF_DIGEST_LEN);
    record_tag(key, out, out + 38u);
}

static kfc_status_t decode_record(const uint8_t key[32],
                                  uint8_t registry_version,
                                  const uint8_t record[KFC_RECORD_BYTES],
                                  uint32_t *sequence,
                                  uint8_t digest[KF_DIGEST_LEN])
{
    uint8_t expected[KF_AUTH_TAG_LEN];
    if (!key || !record || !sequence || !digest) return KFC_E_ARG;
    if (record[0] != KFC_FORMAT_VERSION || record[1] != registry_version ||
        !all_zero(record + 54u, 8u))
        return KFC_E_VERSION;
    record_tag(key, record, expected);
    if (!ct_eq(expected, record + 38u, KF_AUTH_TAG_LEN)) {
        secure_zero(expected, sizeof(expected));
        return KFC_E_AUTH;
    }
    secure_zero(expected, sizeof(expected));
    *sequence = get_u32(record + 2u);
    if (*sequence == 0u) return KFC_E_FORMAT;
    memcpy(digest, record + 6u, KF_DIGEST_LEN);
    return KFC_OK;
}

static int read_slot(const kfc_storage_t *storage, uint8_t slot,
                     uint8_t record[KFC_RECORD_BYTES])
{
    int result = storage->read(storage->ctx, slot, record);
    if (result == KFC_STORE_ABSENT) return KFC_STORE_ABSENT;
    return result == 0 ? KFC_OK : KFC_E_STORAGE;
}

kfc_status_t kfc_load(const kfc_storage_t *storage,
                      const uint8_t key[32],
                      uint8_t registry_version,
                      kfc_state_t *out)
{
    uint8_t record[KFC_RECORD_BYTES];
    uint8_t digest[KF_DIGEST_LEN];
    uint32_t sequence[KFC_SLOT_COUNT] = { 0u, 0u };
    uint8_t valid[KFC_SLOT_COUNT] = { 0u, 0u };
    int present = 0;
    if (!storage || !storage->read || !key || !out || registry_version == 0u)
        return KFC_E_ARG;
    memset(out, 0, sizeof(*out));
    for (uint8_t slot = 0u; slot < KFC_SLOT_COUNT; slot++) {
        int read_result = read_slot(storage, slot, record);
        kfc_status_t decode_result;
        if (read_result == KFC_STORE_ABSENT) continue;
        if (read_result != KFC_OK) return KFC_E_STORAGE;
        present = 1;
        decode_result = decode_record(key, registry_version, record,
                                      &sequence[slot], digest);
        if (decode_result != KFC_OK) return decode_result;
        valid[slot] = 1u;
    }
    secure_zero(record, sizeof(record));
    secure_zero(digest, sizeof(digest));
    if (!present) return KFC_EMPTY;
    if (!valid[0] && !valid[1]) return KFC_E_FORMAT;
    if (valid[0] && valid[1] && sequence[0] == sequence[1]) {
        uint8_t first[KFC_RECORD_BYTES];
        uint8_t second[KFC_RECORD_BYTES];
        if (storage->read(storage->ctx, 0u, first) != 0 ||
            storage->read(storage->ctx, 1u, second) != 0 ||
            memcmp(first + 6u, second + 6u, KF_DIGEST_LEN) != 0)
            return KFC_E_ROLLBACK;
        secure_zero(first, sizeof(first));
        secure_zero(second, sizeof(second));
    }
    out->active_slot = valid[1] && (!valid[0] || sequence[1] > sequence[0])
                           ? 1u : 0u;
    out->registry_version = registry_version;
    out->generation = sequence[out->active_slot];
    out->cursor.initialized = 1u;
    out->cursor.last_sequence = out->generation;
    out->initialized = 1u;
    return KFC_OK;
}

kfc_status_t kfc_commit(const kfc_storage_t *storage,
                        const uint8_t key[32],
                        uint8_t registry_version,
                        uint32_t sequence,
                        const uint8_t payload_digest[KF_DIGEST_LEN],
                        kfc_state_t *state)
{
    uint8_t record[KFC_RECORD_BYTES];
    uint8_t readback[KFC_RECORD_BYTES];
    uint8_t digest[KF_AUTH_TAG_LEN];
    uint8_t slot;
    uint32_t recovered_sequence;
    uint8_t recovered_digest[KF_DIGEST_LEN];
    if (!storage || !storage->read || !storage->write || !key ||
        !payload_digest || !state || registry_version == 0u || sequence == 0u)
        return KFC_E_ARG;
    if (state->initialized && state->registry_version != registry_version)
        return KFC_E_VERSION;
    if (state->initialized && sequence <= state->cursor.last_sequence)
        return KFC_E_ROLLBACK;
    slot = state->initialized ? (uint8_t)(state->active_slot ^ 1u) : 0u;
    make_record(key, registry_version, sequence, payload_digest, record);
    if (storage->write(storage->ctx, slot, record) != 0) {
        secure_zero(record, sizeof(record));
        return KFC_E_STORAGE;
    }
    if (storage->read(storage->ctx, slot, readback) != 0) {
        secure_zero(record, sizeof(record));
        secure_zero(readback, sizeof(readback));
        return KFC_E_STORAGE;
    }
    if (decode_record(key, registry_version, readback,
                      &recovered_sequence, recovered_digest) != KFC_OK ||
        recovered_sequence != sequence ||
        memcmp(recovered_digest, payload_digest, KF_DIGEST_LEN) != 0) {
        secure_zero(record, sizeof(record));
        secure_zero(readback, sizeof(readback));
        secure_zero(recovered_digest, sizeof(recovered_digest));
        return KFC_E_AUTH;
    }
    state->initialized = 1u;
    state->active_slot = slot;
    state->registry_version = registry_version;
    state->generation = sequence;
    state->cursor.initialized = 1u;
    state->cursor.last_sequence = sequence;
    secure_zero(record, sizeof(record));
    secure_zero(readback, sizeof(readback));
    secure_zero(digest, sizeof(digest));
    secure_zero(recovered_digest, sizeof(recovered_digest));
    return KFC_OK;
}
