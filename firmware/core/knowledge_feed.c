#include "knowledge_feed.h"
#include "../net/crypto.h"
#include <string.h>

static void hash_u8(sha256_ctx *ctx, uint8_t value)
{
    sha256_update(ctx, &value, sizeof(value));
}

static void hash_u16(sha256_ctx *ctx, uint16_t value)
{
    uint8_t bytes[2];
    bytes[0] = (uint8_t)(value & 0xffu);
    bytes[1] = (uint8_t)(value >> 8);
    sha256_update(ctx, bytes, sizeof(bytes));
}

static void hash_u32(sha256_ctx *ctx, uint32_t value)
{
    uint8_t bytes[4];
    bytes[0] = (uint8_t)(value & 0xffu);
    bytes[1] = (uint8_t)((value >> 8) & 0xffu);
    bytes[2] = (uint8_t)((value >> 16) & 0xffu);
    bytes[3] = (uint8_t)(value >> 24);
    sha256_update(ctx, bytes, sizeof(bytes));
}

static void hash_symbol(sha256_ctx *ctx, sr_symbol_t symbol)
{
    hash_u32(ctx, symbol);
}

static void hash_term(sha256_ctx *ctx, const sr_term_t *term)
{
    hash_symbol(ctx, term->value);
    hash_u8(ctx, term->kind);
}

static void hash_pattern(sha256_ctx *ctx, const sr_pattern_t *pattern)
{
    hash_term(ctx, &pattern->subject);
    hash_term(ctx, &pattern->predicate);
    hash_term(ctx, &pattern->object);
    hash_u8(ctx, pattern->negated);
}

static void hash_fact(sha256_ctx *ctx, const sr_fact_t *fact)
{
    hash_symbol(ctx, fact->subject);
    hash_symbol(ctx, fact->predicate);
    hash_symbol(ctx, fact->object);
    hash_u8(ctx, fact->negated);
}

static void hash_rule(sha256_ctx *ctx, const sr_rule_t *rule)
{
    hash_u8(ctx, rule->id);
    hash_u8(ctx, rule->premise_count);
    for (uint8_t i = 0u; i < rule->premise_count; i++)
        hash_pattern(ctx, &rule->premise[i]);
    hash_pattern(ctx, &rule->conclusion);
    hash_u16(ctx, rule->cost);
}

kf_core_status_t kf_core_status(uint8_t core_link_present)
{
    return core_link_present == 1u ? KF_CORE_AVAILABLE : KF_CORE_UNAVAILABLE;
}

void kf_digest(const kf_packet_t *packet, uint8_t out[KF_DIGEST_LEN])
{
    sha256_ctx ctx;
    if (!out) return;
    if (!packet) {
        memset(out, 0, KF_DIGEST_LEN);
        return;
    }
    sha256_init(&ctx);
    hash_u8(&ctx, packet->schema_version);
    hash_u8(&ctx, packet->source_kind);
    hash_u8(&ctx, packet->namespace_id);
    hash_u8(&ctx, packet->registry_version);
    hash_u32(&ctx, packet->sequence);
    hash_u32(&ctx, packet->ttl_seconds);
    sha256_update(&ctx, packet->producer_digest, KF_DIGEST_LEN);
    hash_u16(&ctx, packet->fact_count);
    hash_u16(&ctx, packet->rule_count);
    for (uint16_t i = 0u; i < packet->fact_count; i++)
        hash_fact(&ctx, &packet->facts[i]);
    for (uint16_t i = 0u; i < packet->rule_count; i++)
        hash_rule(&ctx, &packet->rules[i]);
    sha256_final(&ctx, out);
}

static int valid_namespace(uint8_t namespace_id)
{
    return namespace_id == SRREG_NAMESPACE_FACTORY ||
           namespace_id == SRREG_NAMESPACE_PERSONAL;
}

static int valid_source(uint8_t source_kind)
{
    return source_kind == KF_SOURCE_CORE ||
           source_kind == KF_SOURCE_FACTORY_MEDIA ||
           source_kind == KF_SOURCE_USER_EXPORT;
}

static int valid_symbol(sr_symbol_t symbol, uint8_t namespace_id,
                        uint8_t registry_version)
{
    return symbol != 0u &&
           srreg_handle_namespace(symbol) == namespace_id &&
           srreg_handle_version(symbol) == registry_version &&
           srreg_handle_slot(symbol) != 0u;
}

static int valid_term(const sr_term_t *term, uint8_t namespace_id,
                      uint8_t registry_version)
{
    if (!term) return 0;
    if (term->kind == SR_TERM_VARIABLE)
        return term->value < SR_MAX_VARIABLES;
    if (term->kind != SR_TERM_CONSTANT) return 0;
    return valid_symbol(term->value, namespace_id, registry_version);
}

static int valid_pattern(const sr_pattern_t *pattern, uint8_t namespace_id,
                         uint8_t registry_version)
{
    return pattern && (pattern->negated == 0u || pattern->negated == 1u) &&
           valid_term(&pattern->subject, namespace_id, registry_version) &&
           valid_term(&pattern->predicate, namespace_id, registry_version) &&
           valid_term(&pattern->object, namespace_id, registry_version);
}

static int valid_fact(const sr_fact_t *fact, uint8_t namespace_id,
                      uint8_t registry_version)
{
    return fact && (fact->negated == 0u || fact->negated == 1u) &&
           valid_symbol(fact->subject, namespace_id, registry_version) &&
           valid_symbol(fact->predicate, namespace_id, registry_version) &&
           valid_symbol(fact->object, namespace_id, registry_version);
}

static int fact_has_namespace_mismatch(const sr_fact_t *fact,
                                        uint8_t namespace_id,
                                        uint8_t registry_version)
{
    return fact &&
           ((fact->subject != 0u &&
             (srreg_handle_namespace(fact->subject) != namespace_id ||
              srreg_handle_version(fact->subject) != registry_version)) ||
            (fact->predicate != 0u &&
             (srreg_handle_namespace(fact->predicate) != namespace_id ||
              srreg_handle_version(fact->predicate) != registry_version)) ||
            (fact->object != 0u &&
             (srreg_handle_namespace(fact->object) != namespace_id ||
              srreg_handle_version(fact->object) != registry_version)));
}

static int valid_rule(const sr_rule_t *rule, uint8_t namespace_id,
                      uint8_t registry_version)
{
    if (!rule || rule->premise_count == 0u ||
        rule->premise_count > KF_MAX_PREMISES || rule->cost == 0u ||
        rule->cost > KF_MAX_DERIVATION_COST)
        return 0;
    for (uint8_t i = 0u; i < rule->premise_count; i++) {
        if (!valid_pattern(&rule->premise[i], namespace_id, registry_version))
            return 0;
    }
    return valid_pattern(&rule->conclusion, namespace_id, registry_version);
}

kf_status_t kf_validate(const kf_packet_t *packet,
                        uint8_t active_registry_version,
                        const kf_cursor_t *cursor,
                        kf_auth_verify_fn verify_auth,
                        void *verify_user)
{
    uint8_t digest[KF_DIGEST_LEN];
    sr_reasoner_t scratch;
    if (!packet || active_registry_version == 0u || !verify_auth)
        return KF_REJECTED_FORMAT;
    if (packet->schema_version != KF_SCHEMA_VERSION ||
        packet->registry_version != active_registry_version ||
        packet->ttl_seconds == 0u || packet->ttl_seconds > KF_MAX_TTL)
        return KF_REJECTED_VERSION;
    if (!valid_source(packet->source_kind) || !valid_namespace(packet->namespace_id))
        return KF_REJECTED_NAMESPACE;
    if (packet->fact_count > KF_MAX_FACTS || packet->rule_count > KF_MAX_RULES ||
        packet->fact_count + packet->rule_count > KF_MAX_RECORDS)
        return KF_REJECTED_LIMIT;
    if (cursor && cursor->initialized && packet->sequence <= cursor->last_sequence)
        return KF_REJECTED_VERSION;
    kf_digest(packet, digest);
    if (!ct_eq(packet->payload_digest, digest, KF_DIGEST_LEN))
        return KF_REJECTED_DIGEST;
    if (packet->authn_status != KF_AUTH_VERIFIED_LINK &&
        packet->authn_status != KF_AUTH_VERIFIED_SIGNATURE)
        return KF_REJECTED_AUTHORITY;
    if (verify_auth(packet, digest, verify_user) != 1)
        return KF_REJECTED_AUTHORITY;
    sr_init(&scratch);
    for (uint16_t i = 0u; i < packet->fact_count; i++) {
        if (!valid_fact(&packet->facts[i], packet->namespace_id,
                        packet->registry_version)) {
            if (fact_has_namespace_mismatch(&packet->facts[i],
                                            packet->namespace_id,
                                            packet->registry_version))
                return KF_REJECTED_NAMESPACE;
            return KF_REJECTED_FORMAT;
        }
        if (sr_add_fact(&scratch, packet->facts[i]) == SR_E_FULL)
            return KF_REJECTED_LIMIT;
    }
    for (uint16_t i = 0u; i < packet->rule_count; i++) {
        if (!valid_rule(&packet->rules[i], packet->namespace_id,
                        packet->registry_version) ||
            sr_add_rule(&scratch, &packet->rules[i]) == SR_E_FULL)
            return KF_REJECTED_FORMAT;
    }
    return KF_ACCEPTED;
}

kf_status_t kf_apply(const kf_packet_t *packet,
                     sd_dialogue_t *dialogue,
                     const kf_policy_t *policy,
                     kf_cursor_t *cursor,
                     kf_auth_verify_fn verify_auth,
                     void *verify_user,
                     kf_apply_result_t *out)
{
    sd_dialogue_t staged;
    kf_status_t status;
    if (out) memset(out, 0, sizeof(*out));
    if (!packet || !dialogue || !policy || !out)
        return KF_REJECTED_FORMAT;
    status = kf_validate(packet, policy->active_registry_version, cursor,
                          verify_auth, verify_user);
    if (status != KF_ACCEPTED) return status;
    if ((packet->namespace_id == SRREG_NAMESPACE_FACTORY &&
         policy->allow_factory != 1u) ||
        (packet->namespace_id == SRREG_NAMESPACE_PERSONAL &&
         policy->allow_personal != 1u))
        return KF_REJECTED_AUTHORITY;
    if (policy->local_confirmation != 1u) {
        out->proposal_only = 1u;
        return KF_PROPOSED;
    }
    staged = *dialogue;
    for (uint16_t i = 0u; i < packet->fact_count; i++) {
        if (sd_add_personal_fact(&staged, packet->facts[i], 1u) != SD_OK)
            return KF_REJECTED_LIMIT;
        out->facts_applied++;
    }
    for (uint16_t i = 0u; i < packet->rule_count; i++) {
        if (sd_add_rule(&staged, &packet->rules[i]) != SD_OK)
            return KF_REJECTED_LIMIT;
        out->rules_applied++;
    }
    *dialogue = staged;
    if (cursor) {
        cursor->initialized = 1u;
        cursor->last_sequence = packet->sequence;
    }
    out->committed = 1u;
    return KF_ACCEPTED;
}
