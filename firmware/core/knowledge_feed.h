/*
 * HERUS knowledge_feed — Core transport is a non-authoritative proposal.
 *
 * The packet contains typed symbolic facts/rules only. It does not contain raw
 * text, audio, embeddings, identity, location, keys, radio commands or actions.
 * Digest verification is implemented locally; signature verification is an
 * explicit dependency boundary owned by a secure-link/secure-element adapter.
 */
#ifndef HERUS_KNOWLEDGE_FEED_H
#define HERUS_KNOWLEDGE_FEED_H

#include "symbolic_dialogue.h"
#include <stddef.h>
#include <stdint.h>

#define KF_SCHEMA_VERSION       1u
#define KF_DIGEST_LEN           32u
#define KF_MAX_FEED_BYTES       8192u
#define KF_MAX_RECORDS          16u
#define KF_MAX_FACTS            16u
#define KF_MAX_RULES             8u
#define KF_MAX_NEW_SYMBOLS      32u
#define KF_MAX_NEW_PERSONAL      8u
#define KF_MAX_PREMISES          4u
#define KF_MAX_DERIVATION_COST 64u
#define KF_MAX_TTL          86400u

#define KF_SOURCE_CORE           1u
#define KF_SOURCE_FACTORY_MEDIA  2u
#define KF_SOURCE_USER_EXPORT    3u

typedef enum {
    KF_AUTH_UNVERIFIED = 0u,
    KF_AUTH_VERIFIED_LINK = 1u,
    KF_AUTH_VERIFIED_SIGNATURE = 2u
} kf_auth_status_t;

typedef enum {
    KF_CORE_AVAILABLE = 0u,
    KF_CORE_UNAVAILABLE = 1u
} kf_core_status_t;

typedef enum {
    KF_ACCEPTED = 0,
    KF_PROPOSED = 1,
    KF_REJECTED_VERSION = -1,
    KF_REJECTED_DIGEST = -2,
    KF_REJECTED_NAMESPACE = -3,
    KF_REJECTED_AUTHORITY = -4,
    KF_REJECTED_LIMIT = -5,
    KF_REJECTED_FORMAT = -6
} kf_status_t;

typedef struct {
    uint8_t schema_version;
    uint8_t source_kind;
    uint8_t namespace_id;
    uint8_t registry_version;
    uint32_t sequence;
    uint32_t ttl_seconds;
    uint8_t producer_digest[KF_DIGEST_LEN];
    uint8_t payload_digest[KF_DIGEST_LEN];
    uint8_t authn_status;
    uint16_t fact_count;
    uint16_t rule_count;
    sr_fact_t facts[KF_MAX_FACTS];
    sr_rule_t rules[KF_MAX_RULES];
} kf_packet_t;

typedef struct {
    uint8_t initialized;
    uint32_t last_sequence;
} kf_cursor_t;

typedef struct {
    uint8_t active_registry_version;
    uint8_t allow_factory;
    uint8_t allow_personal;
    uint8_t local_confirmation;
} kf_policy_t;

typedef struct {
    uint16_t facts_applied;
    uint16_t rules_applied;
    uint8_t committed;
    uint8_t proposal_only;
} kf_apply_result_t;

/* The verifier owns the trust anchor and must not expose key material here. */
typedef int (*kf_auth_verify_fn)(const kf_packet_t *packet,
                                 const uint8_t payload_digest[KF_DIGEST_LEN],
                                 void *user);

/* Compute the canonical digest over all semantic packet fields except digest. */
void kf_digest(const kf_packet_t *packet,
               uint8_t out[KF_DIGEST_LEN]);

/* Transport absence is a typed external status, never a reasoner failure. */
kf_core_status_t kf_core_status(uint8_t core_link_present);

/* Validate without mutating the packet or any dialogue state. */
kf_status_t kf_validate(const kf_packet_t *packet,
                        uint8_t active_registry_version,
                        const kf_cursor_t *cursor,
                        kf_auth_verify_fn verify_auth,
                        void *verify_user);

/* Apply is transactional: no dialogue mutation occurs unless every record is
 * valid and local_confirmation is explicit. Without confirmation it returns
 * KF_PROPOSED and leaves the dialogue byte-for-byte unchanged. */
kf_status_t kf_apply(const kf_packet_t *packet,
                     sd_dialogue_t *dialogue,
                     const kf_policy_t *policy,
                     kf_cursor_t *cursor,
                     kf_auth_verify_fn verify_auth,
                     void *verify_user,
                     kf_apply_result_t *out);

#endif /* HERUS_KNOWLEDGE_FEED_H */
