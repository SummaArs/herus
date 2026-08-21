#include "sim.h"
#include "authority_transition.h"
#include <string.h>

static void expect(sim_score *score, int condition, const char *what)
{
    sim_ok(score, condition, what);
}

void scenario_authority(sim_score *score, int argc, char **argv)
{
    at_machine_t machine;
    at_capsule_t observation;
    at_capsule_t memory;
    at_capsule_t retrieval;
    at_capsule_t offer;
    at_capsule_t action;
    at_capsule_t stale;
    (void)argc;
    (void)argv;

    at_init(&machine);
    memset(&observation, 0, sizeof(observation));
    memset(&memory, 0, sizeof(memory));
    memset(&retrieval, 0, sizeof(retrieval));
    memset(&offer, 0, sizeof(offer));
    memset(&action, 0, sizeof(action));
    memset(&stale, 0, sizeof(stale));

    expect(score, at_observe(&machine, AT_SOURCE_CORE_KNOWLEDGE,
                             100u, 1u, 0u, &observation) == AT_OK &&
                   observation.authority == AT_AUTH_OBSERVATION &&
                   observation.scope == 0u,
           "Core knowledge creates an observable candidate, never action authority");

    expect(score, at_promote_memory(&machine, &observation, 0u, 1u, &memory) ==
                       AT_E_AUTH,
           "external knowledge cannot promote itself without physical confirmation");

    expect(score, at_promote_memory(&machine, &observation, 1u, 1u, &memory) ==
                       AT_OK && memory.source == AT_SOURCE_CORE_KNOWLEDGE &&
                       memory.authority == (AT_AUTH_OBSERVATION | AT_AUTH_MEMORY),
           "confirmed memory preserves external provenance without amplifying authority");

    expect(score, at_retrieve(&machine, &memory, 2u, &retrieval) == AT_OK &&
                   retrieval.authority == memory.authority && retrieval.scope == 0u,
           "retrieval cannot add authority or action scope");

    expect(score, at_offer(&machine, &retrieval, 2u, &offer) == AT_OK &&
                   offer.authority == retrieval.authority && offer.scope == 0u,
           "an offer remains an offer and cannot become an action by presentation");

    expect(score, at_grant_local_action(&machine, &offer,
                                        AT_SCOPE_LOCAL_HAPTIC, 0u, 2u,
                                        &action) == AT_E_AUTH,
           "no physical confirmation means no local action grant");
    expect(score, at_grant_local_action(&machine, &offer,
                                        AT_SCOPE_CORE_EXECUTE, 1u, 2u,
                                        &action) == AT_E_SCOPE,
           "Core execution scope cannot be granted as a local action");

    expect(score, at_grant_local_action(&machine, &offer,
                                        AT_SCOPE_LOCAL_HAPTIC, 1u, 2u,
                                        &action) == AT_OK &&
                   (action.authority & AT_AUTH_ACTION) != 0u &&
                   action.scope == AT_SCOPE_LOCAL_HAPTIC,
           "physical confirmation grants only the requested local scope");

    expect(score, at_execute_local(&action, AT_SCOPE_LOCAL_HAPTIC) == AT_OK,
           "authorized local haptic action executes");
    expect(score, at_execute_local(&action, AT_SCOPE_LOCAL_DIALOGUE) == AT_E_AUTH,
           "an action cannot execute outside its granted local scope");
    expect(score, at_execute_local(&action, AT_SCOPE_CORE_EXECUTE) == AT_E_AUTH,
           "Core execution is not reachable through the local action scope");

    stale = observation;
    at_mark_conflict(&stale);
    expect(score, at_promote_memory(&machine, &stale, 1u, 1u, &memory) ==
                       AT_E_CONFLICT,
           "conflicting observation cannot be promoted into personal memory");

    expect(score, at_observe(&machine, AT_SOURCE_LOCAL_OBSERVATION,
                             101u, 2u, 2u, &observation) == AT_OK &&
                   at_promote_memory(&machine, &observation, 1u, 3u, &memory) ==
                       AT_E_EXPIRED,
           "expired observation cannot become current memory");

    expect(score, at_observe(&machine, AT_SOURCE_LOCAL_OBSERVATION,
                             102u, 4u, 0u, &observation) == AT_OK &&
                   at_promote_memory(&machine, &observation, 1u, 4u, &memory) ==
                       AT_OK && at_retrieve(&machine, &memory, 4u, &retrieval) ==
                       AT_OK && at_offer(&machine, &retrieval, 4u, &offer) == AT_OK,
           "a valid local chain reaches an offer without skipping states");

    stale = offer;
    at_mark_conflict(&stale);
    expect(score, at_grant_local_action(&machine, &stale,
                                        AT_SCOPE_LOCAL_DIALOGUE, 1u, 4u,
                                        &action) == AT_E_CONFLICT,
           "conflicting offer cannot reach local action");

    stale = offer;
    at_reboot(&machine);
    expect(score, at_grant_local_action(&machine, &stale,
                                        AT_SCOPE_LOCAL_DIALOGUE, 1u, 5u,
                                        &action) == AT_E_STAGE,
           "pre-reboot offer cannot regain authority after epoch change");

    expect(score, at_observe(&machine, AT_SOURCE_LOCAL_OBSERVATION,
                             103u, 5u, 0u, &observation) == AT_OK &&
                   at_promote_memory(&machine, &observation, 1u, 5u, &memory) ==
                       AT_OK && at_retrieve(&machine, &memory, 5u, &retrieval) ==
                       AT_OK,
           "post-reboot authority must be rebuilt by a fresh chain");

    expect(score, machine.rejected >= 5u,
           "the machine records rejected promotions and authority escalations");
}
