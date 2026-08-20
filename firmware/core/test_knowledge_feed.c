#include "knowledge_feed.h"
#include <stdio.h>
#include <string.h>

typedef struct { int pass; int fail; } score_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static int trusted_link(const kf_packet_t *packet,
                        const uint8_t digest[KF_DIGEST_LEN], void *user)
{
    (void)digest;
    (void)user;
    return packet && packet->authn_status == KF_AUTH_VERIFIED_LINK;
}

static int hmac_link(const kf_packet_t *packet,
                     const uint8_t digest[KF_DIGEST_LEN], void *user)
{
    return kf_hmac_verify(packet, digest, user);
}

static kf_packet_t factory_packet(void)
{
    kf_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    packet.schema_version = KF_SCHEMA_VERSION;
    packet.source_kind = KF_SOURCE_CORE;
    packet.namespace_id = SRREG_NAMESPACE_FACTORY;
    packet.registry_version = 7u;
    packet.sequence = 1u;
    packet.ttl_seconds = 3600u;
    packet.producer_digest[0] = 0xa5u;
    packet.authn_status = KF_AUTH_VERIFIED_LINK;
    packet.fact_count = 1u;
    packet.facts[0] = (sr_fact_t){
        srreg_handle_make(SRREG_NAMESPACE_FACTORY, 7u, 1u),
        srreg_handle_make(SRREG_NAMESPACE_FACTORY, 7u, 2u),
        srreg_handle_make(SRREG_NAMESPACE_FACTORY, 7u, 3u),
        0u
    };
    kf_digest(&packet, packet.payload_digest);
    return packet;
}

int main(void)
{
    score_t score = { 0, 0 };
    kf_packet_t packet = factory_packet();
    kf_packet_t tampered;
    kf_packet_t rollback;
    kf_packet_t wrong_namespace;
    kf_packet_t too_many;
    kf_cursor_t cursor = { 0u, 0u };
    sd_dialogue_t dialogue;
    sd_reply_t reply;
    kf_apply_result_t applied;
    kf_policy_t policy = { 7u, 1u, 0u, 0u };
    sr_pattern_t query;

    sd_init(&dialogue);
    check(&score, kf_validate(&packet, 7u, &cursor, trusted_link, NULL) ==
                    KF_ACCEPTED,
          "a complete Core packet passes digest, version, namespace and auth gates");

    {
        uint8_t auth_key[32] = { 0u };
        kf_packet_t signed_packet = packet;
        kf_cursor_t signed_cursor = { 0u, 0u };
        signed_packet.authn_status = KF_AUTH_VERIFIED_SIGNATURE;
        for (unsigned i = 0u; i < sizeof(auth_key); i++)
            auth_key[i] = (uint8_t)(0xa0u + i);
        kf_hmac_tag(auth_key, signed_packet.payload_digest,
                    signed_packet.auth_tag);
        check(&score, kf_validate(&signed_packet, 7u, &signed_cursor,
                                  hmac_link, auth_key) == KF_ACCEPTED,
              "a paired Core HMAC tag authenticates the canonical payload digest");
        signed_packet.auth_tag[0] ^= 1u;
        check(&score, kf_validate(&signed_packet, 7u, &signed_cursor,
                                  hmac_link, auth_key) == KF_REJECTED_AUTHORITY,
              "a tampered HMAC tag is rejected before any proposal or insertion");
    }

    tampered = packet;
    tampered.facts[0].object ^= 1u;
    check(&score, kf_validate(&tampered, 7u, &cursor, trusted_link, NULL) ==
                    KF_REJECTED_DIGEST && sr_fact_count(&dialogue.reasoner) == 0u,
          "payload alteration is rejected before any local insertion");

    rollback = packet;
    cursor.initialized = 1u;
    cursor.last_sequence = 1u;
    check(&score, kf_validate(&rollback, 7u, &cursor, trusted_link, NULL) ==
                    KF_REJECTED_VERSION,
          "a repeated sequence is rejected as rollback before authentication side effects");
    cursor.initialized = 0u;
    cursor.last_sequence = 0u;

    wrong_namespace = packet;
    wrong_namespace.namespace_id = SRREG_NAMESPACE_PERSONAL;
    kf_digest(&wrong_namespace, wrong_namespace.payload_digest);
    check(&score, kf_validate(&wrong_namespace, 7u, &cursor, trusted_link, NULL) ==
                    KF_REJECTED_NAMESPACE,
          "a factory-handle payload cannot be relabelled as personal knowledge");

    too_many = packet;
    too_many.rule_count = KF_MAX_RULES + 1u;
    check(&score, kf_validate(&too_many, 7u, &cursor, trusted_link, NULL) ==
                    KF_REJECTED_LIMIT,
          "record limits are rejected before reading outside bounded arrays");

    policy.local_confirmation = 0u;
    check(&score, kf_apply(&packet, &dialogue, &policy, &cursor,
                           trusted_link, NULL, &applied) == KF_PROPOSED &&
                    applied.proposal_only == 1u && applied.committed == 0u &&
                    sr_fact_count(&dialogue.reasoner) == 0u && cursor.initialized == 0u,
          "a valid Core feed remains a proposal without local confirmation");

    policy.local_confirmation = 1u;
    check(&score, kf_apply(&packet, &dialogue, &policy, &cursor,
                           trusted_link, NULL, &applied) == KF_ACCEPTED &&
                    applied.committed == 1u && applied.facts_applied == 1u &&
                    sr_fact_count(&dialogue.reasoner) == 1u &&
                    cursor.initialized == 1u && cursor.last_sequence == 1u,
          "local confirmation commits the proposal and advances the anti-rollback cursor");

    query = (sr_pattern_t){
        SR_CONST(packet.facts[0].subject),
        SR_CONST(packet.facts[0].predicate),
        SR_CONST(packet.facts[0].object),
        0u
    };
    check(&score, sd_ask(&dialogue, &query, SD_MAX_DERIVATION_STEPS, &reply) ==
                    SD_OK && reply.answer.kind == SR_ANSWER_DIRECT,
          "knowledge accepted from the Core is ordinary local evidence and is queryable offline");

    check(&score, kf_apply(&packet, &dialogue, &policy, &cursor,
                           trusted_link, NULL, &applied) == KF_REJECTED_VERSION &&
                    sr_fact_count(&dialogue.reasoner) == 1u,
          "replaying an accepted feed cannot duplicate or mutate local memory");

    check(&score, sd_ask(&dialogue, &query, SD_MAX_DERIVATION_STEPS, &reply) ==
                    SD_OK && reply.answer.kind == SR_ANSWER_DIRECT,
          "absence of a new Core feed does not disable the local reasoner");

    {
        kf_packet_t personal_packet = packet;
        kf_cursor_t personal_cursor = { 0u, 0u };
        kf_policy_t personal_policy = { 7u, 0u, 1u, 0u };
        sd_dialogue_t personal_dialogue;
        personal_packet.namespace_id = SRREG_NAMESPACE_PERSONAL;
        personal_packet.sequence = 1u;
        personal_packet.facts[0] = (sr_fact_t){
            srreg_handle_make(SRREG_NAMESPACE_PERSONAL, 7u, 11u),
            srreg_handle_make(SRREG_NAMESPACE_PERSONAL, 7u, 12u),
            srreg_handle_make(SRREG_NAMESPACE_PERSONAL, 7u, 13u),
            0u
        };
        kf_digest(&personal_packet, personal_packet.payload_digest);
        sd_init(&personal_dialogue);
        check(&score, kf_apply(&personal_packet, &personal_dialogue,
                               &personal_policy, &personal_cursor,
                               trusted_link, NULL, &applied) == KF_PROPOSED &&
                        sr_fact_count(&personal_dialogue.reasoner) == 0u,
              "Core cannot create personal memory without local confirmation");
        personal_policy.local_confirmation = 1u;
        check(&score, kf_apply(&personal_packet, &personal_dialogue,
                               &personal_policy, &personal_cursor,
                               trusted_link, NULL, &applied) == KF_ACCEPTED &&
                        sr_fact_count(&personal_dialogue.reasoner) == 1u,
              "personal feed knowledge is promoted only after explicit local confirmation");
    }

    {
        kf_packet_t unauthenticated = packet;
        kf_cursor_t unsigned_cursor = { 0u, 0u };
        unauthenticated.authn_status = KF_AUTH_UNVERIFIED;
        check(&score, kf_validate(&unauthenticated, 7u, &unsigned_cursor,
                                  trusted_link, NULL) == KF_REJECTED_AUTHORITY,
              "an unsigned Core packet cannot enter even the proposal verifier");
    }

    check(&score, kf_core_status(0u) == KF_CORE_UNAVAILABLE &&
                    kf_core_status(1u) == KF_CORE_AVAILABLE &&
                    sd_ask(&dialogue, &query, SD_MAX_DERIVATION_STEPS, &reply) ==
                        SD_OK && reply.answer.kind == SR_ANSWER_DIRECT,
          "Core absence is typed as unavailable while local intelligence remains usable");

    printf("KNOWLEDGE FEED: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
