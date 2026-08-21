#include "knowledge_feed_cursor.h"
#include <stdio.h>
#include <string.h>

typedef struct { int pass; int fail; } score_t;

typedef struct {
    uint8_t present[KFC_SLOT_COUNT];
    uint8_t record[KFC_SLOT_COUNT][KFC_RECORD_BYTES];
    int partial_write_bytes;
    unsigned write_count;
    int corrupt_read_slot;
    int corrupt_byte;
    uint8_t corrupt_mask;
} store_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static int store_read(void *ctx, uint8_t slot, uint8_t out[KFC_RECORD_BYTES])
{
    store_t *store = (store_t *)ctx;
    if (!store || slot >= KFC_SLOT_COUNT || !out) return -1;
    if (!store->present[slot]) return KFC_STORE_ABSENT;
    memcpy(out, store->record[slot], KFC_RECORD_BYTES);
    if (store->corrupt_read_slot == (int)slot &&
        store->corrupt_byte >= 0 && store->corrupt_byte < (int)KFC_RECORD_BYTES)
        out[store->corrupt_byte] ^= store->corrupt_mask;
    return 0;
}

static int store_write(void *ctx, uint8_t slot,
                       const uint8_t record[KFC_RECORD_BYTES])
{
    store_t *store = (store_t *)ctx;
    size_t bytes;
    if (!store || slot >= KFC_SLOT_COUNT || !record) return -1;
    bytes = KFC_RECORD_BYTES;
    store->write_count++;
    if (store->partial_write_bytes >= 0 && store->write_count == 2u)
        bytes = (size_t)store->partial_write_bytes;
    memcpy(store->record[slot], record, bytes);
    store->present[slot] = 1u;
    return 0;
}

static void reset_store(store_t *store)
{
    memset(store, 0, sizeof(*store));
    memset(store->record, 0xff, sizeof(store->record));
    store->partial_write_bytes = -1;
    store->corrupt_read_slot = -1;
    store->corrupt_byte = -1;
}

int main(void)
{
    score_t score = { 0, 0 };
    const uint8_t key[32] = {
        0x42u, 0x17u, 0x90u, 0xabu, 0x01u, 0x55u, 0xceu, 0x33u,
        0x72u, 0x19u, 0x04u, 0xdeu, 0x88u, 0x61u, 0xacu, 0x0fu,
        0x21u, 0x08u, 0x9au, 0x73u, 0x14u, 0x66u, 0xb0u, 0x52u,
        0x3du, 0xe1u, 0x0au, 0x7cu, 0x49u, 0x2fu, 0xd8u, 0x95u
    };
    uint8_t digest_a[KF_DIGEST_LEN];
    uint8_t digest_b[KF_DIGEST_LEN];
    kfc_storage_t storage;
    store_t store;
    kfc_state_t state;
    kfc_state_t before;
    kfc_state_t recovered;

    memset(digest_a, 0x11u, sizeof(digest_a));
    memset(digest_b, 0x22u, sizeof(digest_b));
    storage = (kfc_storage_t){ &store, store_read, store_write };

    reset_store(&store);
    memset(&state, 0, sizeof(state));
    check(&score, kfc_commit(&storage, key, 7u, 10u, digest_a, &state) == KFC_OK,
          "an authenticated floor is installed before power-fault injection");

    {
        int all_rejected = 1;
        int all_unchanged = 1;
        for (int prefix = 0; prefix < (int)KFC_RECORD_BYTES; prefix++) {
            kfc_state_t candidate;
            kfc_state_t rebooted;
            store_t fault_store;
            kfc_storage_t fault_storage;
            reset_store(&fault_store);
            fault_storage = (kfc_storage_t){ &fault_store, store_read, store_write };
            memset(&candidate, 0, sizeof(candidate));
            if (kfc_commit(&fault_storage, key, 7u, 10u, digest_a, &candidate) != KFC_OK) {
                all_rejected = 0;
                all_unchanged = 0;
                continue;
            }
            fault_store.partial_write_bytes = prefix;
            before = candidate;
            {
                kfc_status_t commit_status = kfc_commit(&fault_storage, key, 7u,
                                                         11u, digest_b, &candidate);
                if (commit_status != KFC_E_AUTH) {
                    printf("  DEBUG prefix=%d commit_status=%d\\n", prefix, commit_status);
                    all_rejected = 0;
                }
            }
            if (memcmp(&before, &candidate, sizeof(candidate)) != 0) {
                printf("  DEBUG prefix=%d RAM state changed\\n", prefix);
                all_unchanged = 0;
            }
            memset(&rebooted, 0xa5, sizeof(rebooted));
            {
                kfc_status_t load_status = kfc_load(&fault_storage, key, 7u,
                                                      &rebooted);
                if (load_status == KFC_OK || rebooted.initialized != 0u) {
                    printf("  DEBUG prefix=%d load_status=%d initialized=%u\\n",
                           prefix, load_status, rebooted.initialized);
                    all_rejected = 0;
                }
            }
        }
        check(&score, all_rejected,
              "every interrupted write prefix is rejected and cannot be promoted on reboot");
        check(&score, all_unchanged,
              "every interrupted write leaves the in-RAM cursor state unchanged");
    }

    reset_store(&store);
    memset(&state, 0, sizeof(state));
    check(&score, kfc_commit(&storage, key, 7u, 10u, digest_a, &state) == KFC_OK &&
                    kfc_commit(&storage, key, 7u, 11u, digest_b, &state) == KFC_OK,
          "a complete successor is committed before bit-flip injection");
    {
        int all_rejected = 1;
        for (int byte = 0; byte < (int)KFC_RECORD_BYTES; byte++) {
            for (unsigned bit = 0u; bit < 8u; bit++) {
                store.corrupt_read_slot = (int)state.active_slot;
                store.corrupt_byte = byte;
                store.corrupt_mask = (uint8_t)(1u << bit);
                memset(&recovered, 0xa5, sizeof(recovered));
                if (kfc_load(&storage, key, 7u, &recovered) == KFC_OK ||
                    recovered.initialized != 0u)
                    all_rejected = 0;
            }
        }
        store.corrupt_read_slot = -1;
        store.corrupt_byte = -1;
        check(&score, all_rejected,
              "all 496 single-bit corruptions of the active journal record fail closed");
    }

    reset_store(&store);
    memset(&state, 0, sizeof(state));
    check(&score, kfc_commit(&storage, key, 7u, 10u, digest_a, &state) == KFC_OK,
          "a fresh floor is available for interrupted RAM promotion");
    {
        kfc_state_t candidate = state;
        kfc_state_t old_state = state;
        check(&score, kfc_commit(&storage, key, 7u, 11u, digest_b, &candidate) == KFC_OK &&
                        memcmp(&state, &old_state, sizeof(state)) == 0 &&
                        kfc_load(&storage, key, 7u, &recovered) == KFC_OK &&
                        recovered.generation == 11u,
              "if power fails after durable readback but before RAM promotion, reboot recovers the authenticated successor");
    }

    printf("KNOWLEDGE FEED CURSOR POWERFAIL: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
