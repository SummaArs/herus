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

    printf("KNOWLEDGE FEED: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
