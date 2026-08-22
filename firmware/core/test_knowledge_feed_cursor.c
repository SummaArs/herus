#include "knowledge_feed_cursor.h"
#include <stdio.h>
#include <string.h>

typedef struct { int pass; int fail; } score_t;

typedef struct {
    uint8_t present[KFC_SLOT_COUNT];
    uint8_t record[KFC_SLOT_COUNT][KFC_RECORD_BYTES];
    int fail_write;
    int corrupt_read_slot;
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
    if (store->corrupt_read_slot == (int)slot) out[38] ^= 1u;
    return 0;
}

static int store_write(void *ctx, uint8_t slot,
                       const uint8_t record[KFC_RECORD_BYTES])
{
    store_t *store = (store_t *)ctx;
    if (!store || slot >= KFC_SLOT_COUNT || !record || store->fail_write)
        return -1;
    memcpy(store->record[slot], record, KFC_RECORD_BYTES);
    store->present[slot] = 1u;
    return 0;
}

int main(void)
{
    score_t score = { 0, 0 };
    store_t store;
    kfc_storage_t storage;
    kfc_state_t state;
    kfc_state_t recovered;
    kfc_state_t before;
    uint8_t key[32];
    uint8_t digest_a[KF_DIGEST_LEN];
    uint8_t digest_b[KF_DIGEST_LEN];
    memset(&store, 0, sizeof(store));
    store.corrupt_read_slot = -1;
    memset(key, 0x5au, sizeof(key));
    memset(digest_a, 0x11u, sizeof(digest_a));
    memset(digest_b, 0x22u, sizeof(digest_b));
    storage.ctx = &store;
    storage.read = store_read;
    storage.write = store_write;

    check(&score, kfc_load(&storage, key, 7u, &recovered) == KFC_EMPTY,
          "two absent cursor slots produce a clean empty state");
    memset(&state, 0, sizeof(state));
    check(&score, kfc_commit(&storage, key, 7u, 10u, digest_a, &state) == KFC_OK &&
                    state.initialized == 1u && state.generation == 10u &&
                    state.active_slot == 0u,
          "first cursor commit is authenticated and occupies slot zero");
    check(&score, kfc_load(&storage, key, 7u, &recovered) == KFC_OK &&
                    recovered.generation == 10u && recovered.active_slot == 0u &&
                    recovered.cursor.last_sequence == 10u,
          "reboot recovery loads the authenticated monotonic floor");

    check(&score, kfc_commit(&storage, key, 7u, 11u, digest_b, &state) == KFC_OK &&
                    state.generation == 11u && state.active_slot == 1u,
          "next commit writes the alternate slot and advances the floor");
    memset(&recovered, 0, sizeof(recovered));
    check(&score, kfc_load(&storage, key, 7u, &recovered) == KFC_OK &&
                    recovered.generation == 11u && recovered.active_slot == 1u,
          "reboot chooses the newest authenticated slot");

    before = state;
    check(&score, kfc_commit(&storage, key, 7u, 11u, digest_a, &state) ==
                    KFC_E_ROLLBACK && memcmp(&before, &state, sizeof(state)) == 0,
          "same-sequence replacement is rejected without mutating RAM state");
    before = state;
    check(&score, kfc_commit(&storage, key, 7u, 9u, digest_a, &state) ==
                    KFC_E_ROLLBACK && memcmp(&before, &state, sizeof(state)) == 0,
          "older sequence rollback is rejected without changing the floor");

    store.record[state.active_slot][38] ^= 1u;
    check(&score, kfc_load(&storage, key, 7u, &recovered) == KFC_E_AUTH,
          "a present but corrupted active slot blocks recovery instead of falling back");
    store.record[state.active_slot][38] ^= 1u;

    store.fail_write = 1;
    before = state;
    check(&score, kfc_commit(&storage, key, 7u, 12u, digest_a, &state) ==
                    KFC_E_STORAGE && memcmp(&before, &state, sizeof(state)) == 0,
          "interrupted slot write leaves the in-RAM floor unchanged");
    store.fail_write = 0;

    store.corrupt_read_slot = state.active_slot ^ 1u;
    before = state;
    check(&score, kfc_commit(&storage, key, 7u, 12u, digest_a, &state) ==
                    KFC_E_AUTH && memcmp(&before, &state, sizeof(state)) == 0,
          "adulterated readback prevents promotion after a nominal write");

    check(&score, kfc_load(&storage, key, 8u, &recovered) == KFC_E_VERSION,
          "a cursor from another registry version is rejected");

    printf("KNOWLEDGE FEED CURSOR: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
