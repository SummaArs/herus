#include "sim.h"
#include "poisoning_guard.h"
#include <string.h>

static int observe_memory(at_machine_t *machine, at_source_t source,
                          uint32_t provenance, uint32_t generation,
                          uint32_t expiry, uint8_t physical,
                          at_capsule_t *out)
{
    at_capsule_t observation;
    int status = at_observe(machine, source, provenance, generation,
                            expiry, &observation);
    if (status != AT_OK) return status;
    return at_promote_memory(machine, &observation, physical,
                             generation, out);
}

void scenario_poisoning(sim_score *score, int argc, char **argv)
{
    at_machine_t machine;
    at_capsule_t poisoned_observation;
    at_capsule_t local_memory;
    at_capsule_t core_memory;
    at_capsule_t offer;
    at_capsule_t action;
    pg_bundle_t bundle;
    int status;
    (void)argc;
    (void)argv;

    at_init(&machine);
    status = at_observe(&machine, AT_SOURCE_CORE_KNOWLEDGE, 7001u, 1u, 0u,
                       &poisoned_observation);
    sim_ok(score, status == AT_OK &&
                   at_promote_memory(&machine, &poisoned_observation, 0u, 1u,
                                     &core_memory) == AT_E_AUTH,
           "L1 external record cannot self-promote without physical confirmation");

    sim_ok(score, observe_memory(&machine, AT_SOURCE_LOCAL_OBSERVATION, 7002u,
                                 2u, 12u, 1u, &local_memory) == AT_OK,
           "a locally confirmed memory can enter a compositional bundle");
    sim_ok(score, observe_memory(&machine, AT_SOURCE_CORE_KNOWLEDGE, 7003u,
                                 2u, 12u, 1u, &core_memory) == AT_OK,
           "Core memory remains source-labelled after explicit review");
    /* A reviewed derivative may carry less authority than its predecessor.
     * Composition must preserve the lower mask, never reconstruct the higher one. */
    core_memory.authority = AT_AUTH_MEMORY;

    pg_init(&bundle, machine.epoch);
    sim_ok(score, pg_add_memory(&bundle, &local_memory, machine.epoch, 2u) == PG_OK &&
                   pg_add_memory(&bundle, &core_memory, machine.epoch, 2u) == PG_OK,
           "L2 accepts bounded memories without merging their provenance");
    sim_ok(score, bundle.authority_intersection == AT_AUTH_MEMORY &&
                   bundle.item_count == 2u,
           "L2 composition intersects authority instead of adding authority");

    sim_ok(score, pg_trigger_context(&bundle, 77u, 77u, machine.epoch, 3u,
                                     &offer) == PG_OK &&
                   (offer.authority & AT_AUTH_ACTION) == 0u &&
                   offer.physically_confirmed == 0u,
           "L2 composition yields only a non-action offer");
    sim_ok(score, at_execute_local(&offer, AT_SCOPE_LOCAL_RADIO) == AT_E_AUTH,
           "a composed offer cannot execute without a fresh local action stage");
    sim_ok(score, at_grant_local_action(&machine, &offer, AT_SCOPE_LOCAL_RADIO,
                                        0u, 3u, &action) == AT_E_AUTH,
           "L2 action still requires separate physical confirmation");
    sim_ok(score, at_grant_local_action(&machine, &offer, AT_SCOPE_LOCAL_RADIO,
                                        1u, 3u, &action) == AT_OK &&
                   at_execute_local(&action, AT_SCOPE_LOCAL_RADIO) == AT_OK,
           "authorized local action is possible only after explicit contact");

    bundle.conflict = 1u;
    sim_ok(score, pg_trigger_context(&bundle, 77u, 77u, machine.epoch, 4u,
                                     &offer) == PG_E_CONFLICT,
           "compositional conflict blocks contextual activation");
    bundle.conflict = 0u;
    sim_ok(score, pg_trigger_context(&bundle, 78u, 77u, machine.epoch, 4u,
                                     &offer) == PG_E_CONTEXT,
           "wrong dormant trigger cannot activate the bundle");
    sim_ok(score, pg_trigger_context(&bundle, 77u, 77u, machine.epoch, 13u,
                                     &offer) == PG_E_EXPIRED,
           "dormant activation after validity expiry is rejected");

    at_reboot(&machine);
    sim_ok(score, pg_trigger_context(&bundle, 77u, 77u, machine.epoch, 5u,
                                     &offer) == PG_E_EXPIRED,
           "L3 dormant bundle cannot cross a session epoch");
    sim_ok(score, pg_add_memory(&bundle, &local_memory, machine.epoch, 5u) == PG_E_EPOCH,
           "replayed memory cannot re-enter a bundle after reboot");
    sim_ok(score, pg_grant_action(&bundle, machine.epoch, AT_SCOPE_LOCAL_HAPTIC, 1u,
                                  5u, &action) == PG_E_EXPIRED,
           "replay cannot manufacture action through the convenience path");
}
