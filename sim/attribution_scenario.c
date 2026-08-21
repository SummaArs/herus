#include "sim.h"
#include "attribution_guard.h"
#include <stdio.h>

void scenario_attribution(sim_score *s, int argc, char **argv)
{
    ag_index_t index;
    ag_index_t expired;
    ag_index_t rebooted;
    ag_index_t floored;
    ag_offer_t offer;
    ag_offer_t core_offer;
    at_capsule_t action;
    int status;
    (void)argc;
    (void)argv;

    ag_init(&index, 7u);
    sim_ok(s, ag_add_root(&index, 100u, 1000u,
                          AT_SOURCE_LOCAL_OBSERVATION, AG_ROLE_PREFERENCE,
                          AT_AUTH_OBSERVATION | AT_AUTH_MEMORY,
                          AT_SCOPE_LOCAL_HAPTIC | AT_SCOPE_LOCAL_DIALOGUE,
                          7u, 10u, 30u) == AG_OK,
           "Attribution accepts a local preference root with bounded scope");
    sim_ok(s, ag_add_root(&index, 200u, 2000u,
                          AT_SOURCE_CORE_KNOWLEDGE, AG_ROLE_POLICY,
                          AT_AUTH_MEMORY, AT_SCOPE_LOCAL_DIALOGUE,
                          7u, 10u, 30u) == AG_OK,
           "Attribution accepts Core policy as knowledge, not local authority");
    sim_ok(s, ag_derive(&index, 101u, 1001u, 100u, AG_EDGE_DERIVED,
                        AG_ROLE_PREFERENCE,
                        AT_AUTH_OBSERVATION | AT_AUTH_MEMORY,
                        AT_SCOPE_LOCAL_HAPTIC, 7u, 12u, 20u) == AG_OK,
           "Derived preference preserves the local source root");
    sim_ok(s, ag_derive(&index, 201u, 2001u, 200u, AG_EDGE_SUPPORTS,
                        AG_ROLE_POLICY, AT_AUTH_MEMORY,
                        AT_SCOPE_LOCAL_DIALOGUE, 7u, 12u, 20u) == AG_OK,
           "Derived policy preserves the Core source and policy role");
    sim_ok(s, ag_admit(&index, 101u, AG_ROLE_PREFERENCE, 0x55u, 0x55u,
                       7u, 13u, &offer) == AG_OK,
           "Admission requires an exact purpose token");
    sim_ok(s, offer.source == AT_SOURCE_LOCAL_OBSERVATION &&
              offer.role == AG_ROLE_PREFERENCE && offer.source_root_id == 100u &&
              offer.provenance_id == 1001u && offer.physically_confirmed == 0u,
           "Offer retains typed source, role, lineage and no physical confirmation");
    sim_ok(s, ag_grant_local_action(&index, &offer, 7u,
                                    AT_SCOPE_LOCAL_HAPTIC, 0u, 13u,
                                    &action) == AG_E_AUTH,
           "Offer cannot become action without physical confirmation");
    sim_ok(s, ag_grant_local_action(&index, &offer, 7u,
                                    AT_SCOPE_LOCAL_RADIO, 1u, 13u,
                                    &action) == AG_E_SCOPE,
           "Action scope cannot exceed the attributed preference scope");
    sim_ok(s, ag_grant_local_action(&index, &offer, 7u,
                                    AT_SCOPE_LOCAL_HAPTIC, 1u, 13u,
                                    &action) == AG_OK &&
              at_execute_local(&action, AT_SCOPE_LOCAL_HAPTIC) == AT_OK,
           "Only a local preference plus physical confirmation reaches local action");
    sim_ok(s, ag_admit(&index, 201u, AG_ROLE_POLICY, 0x66u, 0x66u,
                       7u, 13u, &core_offer) == AG_OK,
           "Core policy remains admissible as policy context");
    sim_ok(s, ag_grant_local_action(&index, &core_offer, 7u,
                                    AT_SCOPE_LOCAL_DIALOGUE, 1u, 13u,
                                    &action) == AG_E_ACTION,
           "Core policy cannot become a local action authority");
    sim_ok(s, ag_derive(&index, 102u, 1002u, 100u, AG_EDGE_DERIVED,
                        AG_ROLE_PREFERENCE, AT_AUTH_ACTION,
                        AT_SCOPE_LOCAL_HAPTIC, 7u, 13u, 20u) == AG_E_AUTH,
           "Derivation cannot manufacture action authority");
    sim_ok(s, ag_add_root(&index, 600u, 6000u,
                          AT_SOURCE_LOCAL_OBSERVATION, AG_ROLE_OBSERVATION,
                          AT_AUTH_OBSERVATION, AT_SCOPE_LOCAL_HAPTIC,
                          7u, 10u, 0u) == AG_OK &&
              ag_derive(&index, 601u, 6001u, 600u, AG_EDGE_DERIVED,
                        AG_ROLE_OBSERVATION,
                        AT_AUTH_OBSERVATION | AT_AUTH_MEMORY,
                        AT_SCOPE_LOCAL_HAPTIC, 7u, 13u, 0u) == AG_E_AUTH,
           "Derivation cannot add memory authority to a weaker observation");
    sim_ok(s, ag_add_root(&index, 610u, 6100u,
                          AT_SOURCE_LOCAL_OBSERVATION, AG_ROLE_PREFERENCE,
                          AT_AUTH_OBSERVATION, AT_SCOPE_LOCAL_HAPTIC,
                          7u, 10u, 0u) == AG_OK &&
              ag_derive(&index, 611u, 6101u, 610u, AG_EDGE_DERIVED,
                        AG_ROLE_PREFERENCE, AT_AUTH_OBSERVATION,
                        AT_SCOPE_LOCAL_RADIO, 7u, 13u, 0u) == AG_E_SCOPE,
           "Derivation cannot add an ungranted action scope");
    sim_ok(s, ag_derive(&index, 103u, 1003u, 100u, AG_EDGE_DERIVED,
                        AG_ROLE_POLICY, AT_AUTH_MEMORY,
                        AT_SCOPE_LOCAL_HAPTIC, 7u, 13u, 20u) == AG_E_ROLE,
           "Observation-derived memory cannot launder into policy role");
    sim_ok(s, ag_admit(&index, 101u, AG_ROLE_PREFERENCE, 0x54u, 0x55u,
                       7u, 13u, &offer) == AG_E_PURPOSE,
           "Wrong purpose cannot activate an otherwise relevant memory");
    sim_ok(s, ag_admit(&index, 101u, AG_ROLE_KNOWLEDGE, 0x55u, 0x55u,
                       7u, 13u, &offer) == AG_E_ROLE,
           "Role mismatch is abstention, not fallback retrieval");
    sim_ok(s, ag_revoke(&index, 100u, 1u, 15u) == AG_OK,
           "Physical revocation accepts the local root");
    sim_ok(s, ag_admit(&index, 101u, AG_ROLE_PREFERENCE, 0x55u, 0x55u,
                       7u, 15u, &offer) == AG_E_REVOKED,
           "Revocation reaches derived records");
    sim_ok(s, ag_derive(&index, 104u, 1004u, 100u, AG_EDGE_DERIVED,
                        AG_ROLE_PREFERENCE, AT_AUTH_MEMORY,
                        AT_SCOPE_LOCAL_HAPTIC, 7u, 16u, 20u) == AG_E_REVOKED,
           "Revoked lineage cannot be reintroduced through a new child");

    ag_init(&expired, 8u);
    sim_ok(s, ag_add_root(&expired, 300u, 3000u,
                          AT_SOURCE_LOCAL_OBSERVATION, AG_ROLE_PREFERENCE,
                          AT_AUTH_OBSERVATION | AT_AUTH_MEMORY,
                          AT_SCOPE_LOCAL_HAPTIC, 8u, 10u, 20u) == AG_OK,
           "Expiry fixture is accepted before its deadline");
    sim_ok(s, ag_expire(&expired, 21u) == 1u &&
              ag_admit(&expired, 300u, AG_ROLE_PREFERENCE, 0x77u, 0x77u,
                       8u, 21u, &offer) == AG_E_EXPIRED,
           "Expired attribution cannot be reintroduced by retrieval");

    ag_init(&rebooted, 9u);
    sim_ok(s, ag_add_root(&rebooted, 400u, 4000u,
                          AT_SOURCE_LOCAL_OBSERVATION, AG_ROLE_PREFERENCE,
                          AT_AUTH_OBSERVATION | AT_AUTH_MEMORY,
                          AT_SCOPE_LOCAL_HAPTIC, 9u, 10u, 0u) == AG_OK,
           "Reboot fixture is accepted in its original epoch");
    ag_reboot(&rebooted);
    sim_ok(s, ag_admit(&rebooted, 400u, AG_ROLE_PREFERENCE, 0x88u, 0x88u,
                       9u, 11u, &offer) == AG_E_EPOCH,
           "Reboot quarantines prior attribution epoch");

    ag_init(&floored, 10u);
    sim_ok(s, ag_set_generation_floor(&floored, 50u) == AG_OK &&
              ag_add_root(&floored, 500u, 5000u,
                          AT_SOURCE_LOCAL_OBSERVATION, AG_ROLE_PREFERENCE,
                          AT_AUTH_OBSERVATION | AT_AUTH_MEMORY,
                          AT_SCOPE_LOCAL_HAPTIC, 10u, 50u, 0u) == AG_E_REPLAY,
           "Generation floor blocks equal-or-older attribution replay");
    status = ag_add_root(&floored, 501u, 5001u,
                         AT_SOURCE_LOCAL_OBSERVATION, AG_ROLE_PREFERENCE,
                         AT_AUTH_OBSERVATION | AT_AUTH_MEMORY,
                         AT_SCOPE_LOCAL_HAPTIC, 10u, 51u, 0u);
    sim_ok(s, status == AG_OK,
           "Only a generation newer than the durable floor may enter");
}


typedef struct {
    const char *name;
    int authorized_recall;
    int role_laundering;
    int source_laundering;
    int purpose_bypass;
    int revoked_reentry;
    int implicit_action;
} attribution_score;

static void attribution_permissive(attribution_score *score)
{
    score->authorized_recall = 1;
    score->role_laundering = 1;
    score->source_laundering = 1;
    score->purpose_bypass = 1;
    score->revoked_reentry = 1;
    score->implicit_action = 1;
}

static void attribution_guard_score(attribution_score *score)
{
    ag_index_t index;
    ag_offer_t offer;
    at_capsule_t action;
    int status;

    ag_init(&index, 11u);
    if (ag_add_root(&index, 700u, 7000u,
                    AT_SOURCE_LOCAL_OBSERVATION, AG_ROLE_PREFERENCE,
                    AT_AUTH_OBSERVATION | AT_AUTH_MEMORY,
                    AT_SCOPE_LOCAL_HAPTIC, 11u, 10u, 0u) == AG_OK &&
        ag_admit(&index, 700u, AG_ROLE_PREFERENCE, 0x91u, 0x91u,
                 11u, 10u, &offer) == AG_OK)
        score->authorized_recall = 1;

    ag_init(&index, 12u);
    if (ag_add_root(&index, 710u, 7100u,
                    AT_SOURCE_LOCAL_OBSERVATION, AG_ROLE_OBSERVATION,
                    AT_AUTH_OBSERVATION, AT_SCOPE_LOCAL_HAPTIC,
                    12u, 10u, 0u) == AG_OK &&
        ag_derive(&index, 711u, 7101u, 710u, AG_EDGE_DERIVED,
                  AG_ROLE_POLICY, AT_AUTH_OBSERVATION,
                  AT_SCOPE_LOCAL_HAPTIC, 12u, 11u, 0u) == AG_OK)
        score->role_laundering = 1;

    ag_init(&index, 13u);
    if (ag_add_root(&index, 720u, 7200u,
                    AT_SOURCE_CORE_KNOWLEDGE, AG_ROLE_POLICY,
                    AT_AUTH_MEMORY, AT_SCOPE_LOCAL_DIALOGUE,
                    13u, 10u, 0u) == AG_OK &&
        ag_admit(&index, 720u, AG_ROLE_POLICY, 0x92u, 0x92u,
                 13u, 10u, &offer) == AG_OK &&
        offer.source == AT_SOURCE_LOCAL_OBSERVATION)
        score->source_laundering = 1;

    ag_init(&index, 14u);
    if (ag_add_root(&index, 730u, 7300u,
                    AT_SOURCE_LOCAL_OBSERVATION, AG_ROLE_PREFERENCE,
                    AT_AUTH_OBSERVATION | AT_AUTH_MEMORY,
                    AT_SCOPE_LOCAL_HAPTIC, 14u, 10u, 0u) == AG_OK &&
        ag_admit(&index, 730u, AG_ROLE_PREFERENCE, 0x93u, 0x94u,
                 14u, 10u, &offer) == AG_OK)
        score->purpose_bypass = 1;

    ag_init(&index, 15u);
    if (ag_add_root(&index, 740u, 7400u,
                    AT_SOURCE_LOCAL_OBSERVATION, AG_ROLE_PREFERENCE,
                    AT_AUTH_OBSERVATION | AT_AUTH_MEMORY,
                    AT_SCOPE_LOCAL_HAPTIC, 15u, 10u, 0u) == AG_OK &&
        ag_derive(&index, 741u, 7401u, 740u, AG_EDGE_DERIVED,
                  AG_ROLE_PREFERENCE, AT_AUTH_OBSERVATION | AT_AUTH_MEMORY,
                  AT_SCOPE_LOCAL_HAPTIC, 15u, 11u, 0u) == AG_OK &&
        ag_revoke(&index, 740u, 1u, 12u) == AG_OK &&
        ag_admit(&index, 741u, AG_ROLE_PREFERENCE, 0x95u, 0x95u,
                 15u, 12u, &offer) == AG_OK)
        score->revoked_reentry = 1;

    ag_init(&index, 16u);
    if (ag_add_root(&index, 750u, 7500u,
                    AT_SOURCE_CORE_KNOWLEDGE, AG_ROLE_POLICY,
                    AT_AUTH_MEMORY, AT_SCOPE_LOCAL_DIALOGUE,
                    16u, 10u, 0u) == AG_OK &&
        ag_admit(&index, 750u, AG_ROLE_POLICY, 0x96u, 0x96u,
                 16u, 10u, &offer) == AG_OK &&
        ag_grant_local_action(&index, &offer, 16u,
                              AT_SCOPE_LOCAL_DIALOGUE, 1u, 10u,
                              &action) == AG_OK)
        score->implicit_action = 1;
    status = AG_OK;
    (void)status;
}

static void print_attribution_score(const attribution_score *score)
{
    printf("  BENCH %-18s recall=%d role=%d source=%d purpose=%d revoked=%d action=%d\n",
           score->name, score->authorized_recall, score->role_laundering,
           score->source_laundering, score->purpose_bypass,
           score->revoked_reentry, score->implicit_action);
}

void scenario_attribution_benchmark(sim_score *score, int argc, char **argv)
{
    attribution_score no_memory = {"no-memory", 0, 0, 0, 0, 0, 0};
    attribution_score latest = {"latest-wins", 0, 0, 0, 0, 0, 0};
    attribution_score similarity = {"similarity-only", 0, 0, 0, 0, 0, 0};
    attribution_score guarded = {"AGSC-attribution", 0, 0, 0, 0, 0, 0};
    (void)argc;
    (void)argv;

    attribution_permissive(&latest);
    attribution_permissive(&similarity);
    attribution_guard_score(&guarded);
    printf("  BENCHMARK ATTRIBUTION — deterministic cases, not general intelligence\n");
    print_attribution_score(&no_memory);
    print_attribution_score(&latest);
    print_attribution_score(&similarity);
    print_attribution_score(&guarded);

    sim_ok(score, guarded.authorized_recall >= no_memory.authorized_recall,
           "Attribution guard preserves authorized recall against no-memory");
    sim_ok(score, guarded.role_laundering < latest.role_laundering &&
                   guarded.role_laundering < similarity.role_laundering,
           "Attribution guard blocks role laundering against permissive baselines");
    sim_ok(score, guarded.source_laundering < latest.source_laundering &&
                   guarded.source_laundering < similarity.source_laundering,
           "Attribution guard preserves source principal against permissive baselines");
    sim_ok(score, guarded.purpose_bypass < latest.purpose_bypass &&
                   guarded.purpose_bypass < similarity.purpose_bypass,
           "Attribution guard binds admission to purpose");
    sim_ok(score, guarded.revoked_reentry < latest.revoked_reentry &&
                   guarded.revoked_reentry < similarity.revoked_reentry,
           "Attribution guard blocks revoked reintroduction");
    sim_ok(score, guarded.implicit_action < latest.implicit_action &&
                   guarded.implicit_action < similarity.implicit_action,
           "Attribution guard blocks implicit action authority");
    sim_ok(score, guarded.authorized_recall == 1 &&
                   guarded.role_laundering == 0 &&
                   guarded.source_laundering == 0 &&
                   guarded.purpose_bypass == 0 &&
                   guarded.revoked_reentry == 0 &&
                   guarded.implicit_action == 0,
           "Attribution guard satisfies the complete non-laundering vector");
}


void scenario_attribution_composition(sim_score *s, int argc, char **argv)
{
    ag_index_t local;
    ag_index_t core;
    ag_index_t contact;
    ag_index_t forged;
    ag_index_t wrong_recipient;
    ag_index_t unconfirmed;
    ag_offer_t offer;
    ag_share_t share;
    at_capsule_t action;
    int status;
    (void)argc;
    (void)argv;

    ag_init_principal(&local, 17u, AG_PRINCIPAL_LOCAL);
    sim_ok(s, ag_add_root(&local, 800u, 8000u,
                          AT_SOURCE_LOCAL_OBSERVATION, AG_ROLE_PREFERENCE,
                          AT_AUTH_OBSERVATION | AT_AUTH_MEMORY,
                          AT_SCOPE_LOCAL_DIALOGUE | AT_SCOPE_LOCAL_HAPTIC,
                          17u, 20u, 0u) == AG_OK &&
              ag_set_purpose(&local, 800u, 0xC0DEu) == AG_OK,
           "Composition accepts a local preference with explicit purpose");
    sim_ok(s, ag_add_root(&local, 801u, 8001u,
                          AT_SOURCE_CORE_KNOWLEDGE, AG_ROLE_POLICY,
                          AT_AUTH_MEMORY, AT_SCOPE_LOCAL_DIALOGUE,
                          17u, 20u, 0u) == AG_OK &&
              ag_set_purpose(&local, 801u, 0xC0DEu) == AG_OK,
           "Composition accepts Core policy as a separate supporting source");
    sim_ok(s, ag_compose(&local, 800u, 801u, 802u, 8002u,
                         AG_ROLE_POLICY, 0xC0DEu, 17u, 21u, 0u) == AG_OK,
           "Composition creates a typed composite node");
    sim_ok(s, ag_admit(&local, 802u, AG_ROLE_POLICY, 0xC0DEu, 0xC0DEu,
                       17u, 21u, &offer) == AG_OK &&
              offer.source == AT_SOURCE_COMPOSITE &&
              offer.source_mask == (AT_SOURCE_LOCAL_OBSERVATION |
                                    AT_SOURCE_CORE_KNOWLEDGE) &&
              offer.authority == AT_AUTH_MEMORY &&
              offer.scope == AT_SCOPE_LOCAL_DIALOGUE &&
              offer.source_root_id == 0u &&
              offer.owner_principal_id == AG_PRINCIPAL_LOCAL &&
              local.nodes[2].secondary_parent_id == 801u,
           "Composite offer preserves both roots and intersects authority");
    sim_ok(s, ag_grant_local_action(&local, &offer, 17u,
                                    AT_SCOPE_LOCAL_DIALOGUE, 1u, 21u,
                                    &action) == AG_E_ACTION,
           "Composite policy cannot become local action authority");
    sim_ok(s, ag_compose(&local, 800u, 801u, 803u, 8003u,
                         AG_ROLE_PREFERENCE, 0xC0DEu, 17u, 21u, 0u) == AG_E_ROLE,
           "Composition cannot relabel policy context as preference");

    sim_ok(s, ag_revoke(&local, 800u, 1u, 22u) == AG_OK &&
              ag_compose(&local, 800u, 801u, 804u, 8004u,
                         AG_ROLE_POLICY, 0xC0DEu, 17u, 22u, 0u) == AG_E_REVOKED,
           "Removing one causal support blocks recomposition");
    sim_ok(s, ag_admit(&local, 802u, AG_ROLE_POLICY, 0xC0DEu, 0xC0DEu,
                       17u, 22u, &offer) == AG_E_REVOKED,
           "Revoking one support invalidates the existing composite");

    ag_init_principal(&core, 18u, AG_PRINCIPAL_CORE);
    sim_ok(s, ag_add_root(&core, 810u, 8100u,
                          AT_SOURCE_CORE_KNOWLEDGE, AG_ROLE_POLICY,
                          AT_AUTH_MEMORY, AT_SCOPE_LOCAL_DIALOGUE,
                          18u, 30u, 0u) == AG_OK &&
              ag_set_purpose(&core, 810u, 0xBEEFu) == AG_OK,
           "Core creates a shareable policy with purpose");
    sim_ok(s, ag_export_share(&core, 810u, 8110u, AG_PRINCIPAL_CONTACT,
                              1u, 31u, &share) == AG_OK &&
              share.issuer_principal_id == AG_PRINCIPAL_CORE &&
              share.recipient_principal_id == AG_PRINCIPAL_CONTACT &&
              share.purpose_token == 0xBEEFu &&
              share.physically_confirmed == 1u,
           "Export records issuer, recipient, purpose and confirmation");
    ag_init_principal(&contact, 19u, AG_PRINCIPAL_CONTACT);
    sim_ok(s, ag_import_share(&contact, &share, 1u, 32u) == AG_OK,
           "Recipient imports only an explicitly addressed share");
    sim_ok(s, ag_admit(&contact, 810u, AG_ROLE_POLICY, 0xBEEFu, 0xBEEFu,
                       19u, 32u, &offer) == AG_OK &&
              offer.owner_principal_id == AG_PRINCIPAL_CONTACT &&
              offer.issuer_principal_id == AG_PRINCIPAL_CORE &&
              offer.source == AT_SOURCE_CORE_KNOWLEDGE,
           "Imported memory preserves recipient ownership and issuer provenance");
    sim_ok(s, ag_grant_local_action(&contact, &offer, 19u,
                                    AT_SCOPE_LOCAL_DIALOGUE, 1u, 32u,
                                    &action) == AG_E_PRINCIPAL,
           "A contact principal cannot turn Core share into local action");
    sim_ok(s, ag_import_share(&contact, &share, 1u, 33u) == AG_E_REPLAY,
           "The same share envelope cannot be replayed");
    share.recipient_principal_id = AG_PRINCIPAL_LOCAL;
    ag_init_principal(&wrong_recipient, 21u, AG_PRINCIPAL_CONTACT);
    sim_ok(s, ag_import_share(&wrong_recipient, &share, 1u, 33u) == AG_E_PRINCIPAL,
           "A share addressed to another principal is rejected");
    share.recipient_principal_id = AG_PRINCIPAL_CONTACT;
    share.source = AT_SOURCE_LOCAL_OBSERVATION;
    ag_init_principal(&forged, 20u, AG_PRINCIPAL_CONTACT);
    sim_ok(s, ag_import_share(&forged, &share, 1u, 33u) == AG_E_FORMAT,
           "A forged source label cannot cross the share boundary");
    share.source = AT_SOURCE_CORE_KNOWLEDGE;
    ag_init_principal(&unconfirmed, 22u, AG_PRINCIPAL_CONTACT);
    sim_ok(s, ag_import_share(&unconfirmed, &share, 0u, 33u) == AG_E_SHARE,
           "Share import still requires fresh local confirmation");
    status = ag_export_share(&core, 810u, 8111u, AG_PRINCIPAL_CORE,
                             1u, 31u, &share);
    sim_ok(s, status == AG_E_PRINCIPAL,
           "The issuer cannot export a share to itself");
}
