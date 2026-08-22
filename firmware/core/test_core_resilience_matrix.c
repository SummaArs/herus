#include "knowledge_feed.h"
#include "knowledge_feed_cursor.h"
#include "magic_trigger.h"
#include "memory_reasoning_bridge.h"
#include <stdio.h>
#include <string.h>

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static int functional(sr_symbol_t predicate, void *user)
{
    (void)user;
    return predicate == SR_SYMBOL_LEGACY(20u);
}

static memory_vault_card_t card(uint32_t id, uint32_t receipt)
{
    memory_vault_card_t out;
    memset(&out, 0, sizeof(out));
    out.card_id = id;
    out.review_receipt_id = receipt;
    return out;
}

typedef struct {
    uint8_t present[KFC_SLOT_COUNT];
    uint8_t record[KFC_SLOT_COUNT][KFC_RECORD_BYTES];
    int corrupt_read_slot;
} store_t;

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
    if (!store || slot >= KFC_SLOT_COUNT || !record) return -1;
    memcpy(store->record[slot], record, KFC_RECORD_BYTES);
    store->present[slot] = 1u;
    return 0;
}

static int trusted_link(const kf_packet_t *packet,
                        const uint8_t digest[KF_DIGEST_LEN], void *user)
{
    (void)digest;
    (void)user;
    return packet && packet->authn_status == KF_AUTH_VERIFIED_LINK;
}

static kf_packet_t packet(void)
{
    kf_packet_t out;
    memset(&out, 0, sizeof(out));
    out.schema_version = KF_SCHEMA_VERSION;
    out.source_kind = KF_SOURCE_CORE;
    out.namespace_id = SRREG_NAMESPACE_FACTORY;
    out.registry_version = 7u;
    out.sequence = 1u;
    out.ttl_seconds = 3600u;
    out.authn_status = KF_AUTH_VERIFIED_LINK;
    out.fact_count = 1u;
    out.facts[0] = (sr_fact_t){
        srreg_handle_make(SRREG_NAMESPACE_FACTORY, 7u, 1u),
        srreg_handle_make(SRREG_NAMESPACE_FACTORY, 7u, 2u),
        srreg_handle_make(SRREG_NAMESPACE_FACTORY, 7u, 3u),
        0u
    };
    kf_digest(&out, out.payload_digest);
    return out;
}

int main(void)
{
    score_t score = { 0, 0 };

    {
        mse_index_t memory;
        sr_reasoner_t base;
        sr_reasoner_t scratch;
        sr_answer_t answer;
        mrb_meta_t meta;
        sr_symbol_t subject = 0x02070001u;
        sr_symbol_t predicate = SR_SYMBOL_LEGACY(20u);
        sr_pattern_t query = { SR_CONST(subject), SR_CONST(predicate), SR_VAR(0u), 0u };
        memory_vault_card_t a = card(100u, 900u);
        memory_vault_card_t b = card(101u, 901u);
        sr_fact_t fact_a = { subject, predicate, 0x02070011u, 0u };
        sr_fact_t fact_b = { subject, predicate, 0x02070012u, 0u };
        mse_init(&memory, functional, NULL);
        sr_init(&base);
        mse_add(&memory, &a, &fact_a, 1u, 0u);
        mse_add(&memory, &b, &fact_b, 2u, 0u);
        check(&score, kf_core_status(0u) == KF_CORE_UNAVAILABLE &&
                        mrb_query(&base, &memory, 2u, &query, 16u, &scratch,
                                  &answer, &meta) == MRB_CONTRADICTED &&
                        meta.selected_card_id == 0u,
              "Core absence plus conflicting local evidence produces local abstention, never a remote fallback");
    }

    {
        store_t store;
        kfc_storage_t storage;
        kfc_state_t state;
        kfc_state_t recovered;
        uint8_t key[32];
        uint8_t digest_a[KF_DIGEST_LEN];
        uint8_t digest_b[KF_DIGEST_LEN];
        memset(&store, 0, sizeof(store));
        store.corrupt_read_slot = -1;
        memset(key, 0x5au, sizeof(key));
        memset(digest_a, 0x11u, sizeof(digest_a));
        memset(digest_b, 0x22u, sizeof(digest_b));
        storage = (kfc_storage_t){ &store, store_read, store_write };
        memset(&state, 0, sizeof(state));
        check(&score, kfc_commit(&storage, key, 7u, 10u, digest_a, &state) == KFC_OK &&
                        kfc_commit(&storage, key, 7u, 11u, digest_b, &state) == KFC_OK,
              "two authenticated journal generations exist before the hostile reboot");
        store.corrupt_read_slot = (int)state.active_slot;
        memset(&recovered, 0xa5, sizeof(recovered));
        check(&score, kfc_load(&storage, key, 7u, &recovered) == KFC_E_AUTH &&
                        recovered.initialized == 0u,
              "reboot with an adulterated newest slot refuses stale-slot promotion");
    }

    {
        magic_trigger_t trigger;
        magic_context_t context = {
            { SR_CONST(SR_SYMBOL_LEGACY(1u)), SR_CONST(SR_SYMBOL_LEGACY(2u)),
              SR_VAR(0u), 0u },
            MAGIC_PRIVACY_ORDINARY, MAGIC_REQUEST_CONTEXTUAL, 1u, 1u
        };
        magic_policy_t policy;
        magic_proposal_t proposal;
        mse_index_t memory;
        sr_reasoner_t base;
        sr_reasoner_t scratch;
        mse_init(&memory, NULL, NULL);
        sr_init(&base);
        magic_policy_default(&policy);
        check(&score, magic_trigger_begin(&trigger, &context, 10u, 4u, 1u) ==
                        MAGIC_TRIGGER_OK,
              "a pending contextual proposal is bounded by an explicit attention window");
        trigger.context.proactive_consent = 0u;
        check(&score, magic_trigger_offer(&trigger, &base, &memory, 11u, &policy,
                                          &scratch, &proposal) == MAGIC_TRIGGER_SILENT &&
                        trigger.proposals_served == 0u,
              "revoking consent invalidates the pending proposal without consuming or presenting it");
    }

    {
        kf_packet_t incoming = packet();
        kf_cursor_t cursor = { 0u, 0u };
        kf_policy_t policy = { 7u, 1u, 0u, 0u };
        sd_dialogue_t dialogue;
        kf_apply_result_t result;
        sd_init(&dialogue);
        check(&score, kf_core_status(0u) == KF_CORE_UNAVAILABLE &&
                        kf_apply(&incoming, &dialogue, &policy, &cursor,
                                 trusted_link, NULL, &result) == KF_PROPOSED &&
                        result.committed == 0u && cursor.initialized == 0u,
              "a valid feed without an available Core remains a proposal and cannot gain local authority");
        policy.local_confirmation = 1u;
        incoming.sequence = 2u;
        kf_digest(&incoming, incoming.payload_digest);
        check(&score, kf_apply(&incoming, &dialogue, &policy, &cursor,
                               trusted_link, NULL, &result) == KF_ACCEPTED &&
                        result.committed == 1u && cursor.last_sequence == 2u,
              "only a new explicit local confirmation promotes a later feed");
    }

    {
        mse_index_t memory;
        memory_vault_card_t a = card(200u, 920u);
        sr_fact_t fact = { 0x02070001u, SR_SYMBOL_LEGACY(21u), 0x02070011u, 0u };
        uint16_t before;
        mse_init(&memory, NULL, NULL);
        for (uint32_t i = 0u; i < MSE_MAX_EVIDENCE; i++) {
            fact.subject = 0x02070001u + i;
            check(&score, mse_add(&memory, &a, &fact, i + 1u, 0u) == MSE_OK,
                  "bounded memory accepts only the next authorised evidence card");
        }
        before = memory.evidence_count;
        fact.subject = 0x020700f0u;
        check(&score, mse_add(&memory, &a, &fact, 100u, 0u) == MSE_E_FULL &&
                        memory.evidence_count == before && memory.rejected >= 1u,
              "memory exhaustion rejects new evidence without overwriting existing knowledge");
    }

    printf("CORE RESILIENCE MATRIX: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
