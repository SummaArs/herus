#include "sim.h"
#include "adaptive_change.h"
#include <string.h>

#define AC_SUBJECT SR_SYMBOL_LEGACY(400u)
#define AC_PREFERENCE SR_SYMBOL_LEGACY(401u)
#define AC_GOAL SR_SYMBOL_LEGACY(402u)
#define AC_OLD_VALUE SR_SYMBOL_LEGACY(410u)
#define AC_NEW_VALUE SR_SYMBOL_LEGACY(411u)
#define AC_NOISE_VALUE SR_SYMBOL_LEGACY(412u)
#define AC_DERIVED_VALUE SR_SYMBOL_LEGACY(413u)
#define AC_EXTERNAL_VALUE SR_SYMBOL_LEGACY(414u)

static ac_observation_t observation(uint32_t card_id, uint32_t generation,
                                   sr_symbol_t object, at_source_t source,
                                   uint8_t confidence, uint32_t expiry,
                                   uint32_t derived_from,
                                   uint8_t physical, uint8_t change_confirmed,
                                   uint32_t epoch)
{
    ac_observation_t value;
    memset(&value, 0, sizeof(value));
    value.fact = (sr_fact_t){AC_SUBJECT, AC_PREFERENCE, object, 0u};
    value.card_id = card_id;
    value.review_receipt_id = 9000u + card_id;
    value.source = source;
    value.observed_generation = generation;
    value.valid_until_generation = expiry;
    value.derived_from_card_id = derived_from;
    value.epoch = epoch;
    value.confidence = confidence;
    value.physical_confirmation = physical;
    value.explicit_change_confirmation = change_confirmed;
    return value;
}

static sr_pattern_t preference_query(void)
{
    return (sr_pattern_t){SR_CONST(AC_SUBJECT), SR_CONST(AC_PREFERENCE),
                          SR_VAR(0u), 0u};
}

void scenario_adaptive_change(sim_score *score, int argc, char **argv)
{
    ac_index_t index;
    ac_observation_t old_preference;
    ac_observation_t noise;
    ac_observation_t new_preference;
    ac_observation_t derived;
    ac_observation_t external;
    ac_query_result_t result;
    sr_pattern_t query = preference_query();
    uint32_t card_id = 0u;
    (void)argc;
    (void)argv;

    ac_init(&index);
    old_preference = observation(1u, 1u, AC_OLD_VALUE,
                                 AT_SOURCE_LOCAL_OBSERVATION, 3u, 0u, 0u,
                                 1u, 1u, index.epoch);
    sim_ok(score, ac_apply_change(&index, &old_preference, 1u, &card_id) ==
                       AC_OK && card_id == 1u,
           "an initially confirmed preference becomes current identity state");
    sim_ok(score, ac_query(&index, &query, 1u, &result) == AC_OK &&
                   result.status == MSE_QUERY_MATCH &&
                   result.selected_card_id == 1u,
           "current preference is retrievable with its originating card");

    noise = observation(2u, 2u, AC_NOISE_VALUE,
                        AT_SOURCE_LOCAL_OBSERVATION, 2u, 0u, 0u,
                        1u, 0u, index.epoch);
    sim_ok(score, ac_apply_change(&index, &noise, 2u, &card_id) == AC_E_AUTH,
           "one unconfirmed preference observation cannot rewrite identity");

    noise = observation(8u, 2u, AC_NOISE_VALUE,
                        AT_SOURCE_LOCAL_OBSERVATION, 1u, 0u, 0u,
                        1u, 1u, index.epoch);
    sim_ok(score, ac_apply_change(&index, &noise, 2u, &card_id) ==
                       AC_E_CONFIDENCE,
           "low-confidence change cannot rewrite identity even with contact");

    new_preference = observation(3u, 3u, AC_NEW_VALUE,
                                 AT_SOURCE_LOCAL_OBSERVATION, 3u, 0u, 0u,
                                 1u, 1u, index.epoch);
    sim_ok(score, ac_apply_change(&index, &new_preference, 3u, &card_id) ==
                       AC_OK && card_id == 3u && index.supersessions == 1u,
           "an explicitly confirmed change supersedes the old preference");
    sim_ok(score, ac_query(&index, &query, 3u, &result) == AC_OK &&
                   result.status == MSE_QUERY_MATCH &&
                   result.selected_card_id == 3u &&
                   result.historical_matches == 1u,
           "superseded history remains auditable but cannot answer as current");

    derived = observation(4u, 4u, AC_DERIVED_VALUE,
                          AT_SOURCE_LOCAL_OBSERVATION, 3u, 0u, 3u,
                          1u, 1u, index.epoch);
    sim_ok(score, ac_apply_change(&index, &derived, 4u, &card_id) == AC_OK,
           "a derived update retains a link to the current predecessor");

    sim_ok(score, ac_revoke(&index, 3u, 0u, 5u) == AC_E_AUTH,
           "revocation without physical confirmation is rejected");
    sim_ok(score, ac_revoke(&index, 3u, 1u, 5u) == AC_OK &&
                   index.revocations == 2u,
           "physical revocation reaches the active fact and its derived memory");
    sim_ok(score, ac_query(&index, &query, 5u, &result) == AC_OK &&
                   result.status == MSE_QUERY_NO_MATCH &&
                   result.revoked_matches == 2u,
           "revoked memory and derived memory cannot be retrieved as current");

    external = observation(5u, 6u, AC_EXTERNAL_VALUE,
                           AT_SOURCE_CORE_KNOWLEDGE, 3u, 7u, 3u,
                           1u, 1u, index.epoch);
    sim_ok(score, ac_apply_change(&index, &external, 6u, &card_id) == AC_E_AUTH,
           "a source derived from revoked memory cannot re-enter the model");

    old_preference = observation(6u, 8u, AC_OLD_VALUE,
                                 AT_SOURCE_LOCAL_OBSERVATION, 3u, 8u, 0u,
                                 1u, 1u, index.epoch);
    sim_ok(score, ac_apply_change(&index, &old_preference, 8u, &card_id) ==
                       AC_OK && ac_expire(&index, 9u) == 1u,
           "a bounded preference expires without being reclassified as false");
    sim_ok(score, ac_query(&index, &query, 9u, &result) == AC_OK &&
                   result.status == MSE_QUERY_NO_MATCH &&
                   result.historical_matches >= 1u,
           "expired preference remains historical but not current");

    ac_reboot(&index);
    new_preference.card_id = 7u;
    new_preference.review_receipt_id = 9007u;
    new_preference.epoch = index.epoch - 1u;
    sim_ok(score, ac_apply_change(&index, &new_preference, 10u, &card_id) ==
                       AC_E_AUTH,
           "a pre-reboot observation cannot be replayed into the new epoch");
    sim_ok(score, ac_set_generation_floor(&index, 9u) == AC_E_FORMAT,
           "floor cannot be changed while the historical index is nonempty");
    sim_ok(score, index.epoch != 1u,
           "reboot advances epoch instead of silently retaining session authority");
}
