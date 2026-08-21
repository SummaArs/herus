#include "authority_transition.h"
#include <string.h>

static int valid_source(at_source_t source)
{
    return source == AT_SOURCE_LOCAL_OBSERVATION ||
           source == AT_SOURCE_CORE_KNOWLEDGE;
}

static int valid_capsule(const at_capsule_t *capsule)
{
    return capsule && capsule->stage != AT_STAGE_NONE &&
           valid_source(capsule->source) && capsule->provenance_id != 0u &&
           capsule->generation != 0u && capsule->epoch != 0u;
}

static int fail(at_machine_t *machine, int status)
{
    if (machine) machine->rejected++;
    return status;
}

void at_init(at_machine_t *machine)
{
    if (machine) {
        memset(machine, 0, sizeof(*machine));
        machine->epoch = 1u;
    }
}

int at_observe(at_machine_t *machine, at_source_t source,
               uint32_t provenance_id, uint32_t generation,
               uint32_t valid_until_generation, at_capsule_t *out)
{
    if (!machine || !out || !valid_source(source) || provenance_id == 0u ||
        generation == 0u ||
        (valid_until_generation != 0u && valid_until_generation < generation))
        return fail(machine, AT_E_FORMAT);
    memset(out, 0, sizeof(*out));
    out->stage = AT_STAGE_OBSERVATION;
    out->source = source;
    out->provenance_id = provenance_id;
    out->authority = AT_AUTH_OBSERVATION;
    out->generation = generation;
    out->valid_until_generation = valid_until_generation;
    out->epoch = machine->epoch;
    machine->observations++;
    return AT_OK;
}

int at_promote_memory(at_machine_t *machine, const at_capsule_t *candidate,
                      uint8_t physical_confirmation,
                      uint32_t generation, at_capsule_t *out)
{
    if (!machine || !out || !valid_capsule(candidate) ||
        candidate->stage != AT_STAGE_OBSERVATION ||
        candidate->epoch != machine->epoch)
        return fail(machine, AT_E_STAGE);
    if (candidate->conflict) return fail(machine, AT_E_CONFLICT);
    if (generation < candidate->generation ||
        (candidate->valid_until_generation != 0u &&
         generation > candidate->valid_until_generation))
        return fail(machine, AT_E_EXPIRED);
    /* A source is never allowed to promote itself. Physical confirmation is
     * separate evidence, and the source remains attached to the capsule. */
    if (physical_confirmation != 1u)
        return fail(machine, AT_E_AUTH);
    memset(out, 0, sizeof(*out));
    *out = *candidate;
    out->stage = AT_STAGE_MEMORY;
    out->authority = AT_AUTH_OBSERVATION | AT_AUTH_MEMORY;
    out->physically_confirmed = 1u;
    out->generation = generation;
    machine->memories++;
    return AT_OK;
}

int at_retrieve(at_machine_t *machine, const at_capsule_t *memory,
                uint32_t generation, at_capsule_t *out)
{
    if (!machine || !out || !valid_capsule(memory) ||
        memory->stage != AT_STAGE_MEMORY ||
        memory->epoch != machine->epoch)
        return fail(machine, AT_E_STAGE);
    if (memory->conflict) return fail(machine, AT_E_CONFLICT);
    if (generation < memory->generation ||
        (memory->valid_until_generation != 0u &&
         generation > memory->valid_until_generation))
        return fail(machine, AT_E_EXPIRED);
    memset(out, 0, sizeof(*out));
    *out = *memory;
    out->stage = AT_STAGE_RETRIEVAL;
    /* Retrieval cannot add authority. */
    out->authority = memory->authority;
    out->scope = 0u;
    out->generation = generation;
    machine->retrievals++;
    return AT_OK;
}

int at_offer(at_machine_t *machine, const at_capsule_t *retrieval,
             uint32_t generation, at_capsule_t *out)
{
    if (!machine || !out || !valid_capsule(retrieval) ||
        retrieval->stage != AT_STAGE_RETRIEVAL ||
        retrieval->epoch != machine->epoch)
        return fail(machine, AT_E_STAGE);
    if (retrieval->conflict) return fail(machine, AT_E_CONFLICT);
    if (generation < retrieval->generation ||
        (retrieval->valid_until_generation != 0u &&
         generation > retrieval->valid_until_generation))
        return fail(machine, AT_E_EXPIRED);
    memset(out, 0, sizeof(*out));
    *out = *retrieval;
    out->stage = AT_STAGE_OFFER;
    /* An offer is not an action and does not add authority. */
    out->authority = retrieval->authority;
    out->scope = 0u;
    out->generation = generation;
    machine->offers++;
    return AT_OK;
}

int at_grant_local_action(at_machine_t *machine, const at_capsule_t *offer,
                          uint32_t local_scope, uint8_t physical_confirmation,
                          uint32_t generation, at_capsule_t *out)
{
    const uint32_t allowed = AT_SCOPE_LOCAL_HAPTIC |
                             AT_SCOPE_LOCAL_DIALOGUE |
                             AT_SCOPE_LOCAL_RADIO;
    if (!machine || !out || !valid_capsule(offer) ||
        offer->stage != AT_STAGE_OFFER ||
        offer->epoch != machine->epoch)
        return fail(machine, AT_E_STAGE);
    if (offer->conflict) return fail(machine, AT_E_CONFLICT);
    if (local_scope == 0u || (local_scope & ~allowed) != 0u)
        return fail(machine, AT_E_SCOPE);
    if (generation < offer->generation ||
        (offer->valid_until_generation != 0u &&
         generation > offer->valid_until_generation))
        return fail(machine, AT_E_EXPIRED);
    if (physical_confirmation != 1u)
        return fail(machine, AT_E_AUTH);
    memset(out, 0, sizeof(*out));
    *out = *offer;
    out->stage = AT_STAGE_ACTION;
    out->authority = offer->authority | AT_AUTH_ACTION;
    out->scope = local_scope;
    out->physically_confirmed = 1u;
    out->generation = generation;
    machine->actions++;
    return AT_OK;
}

int at_execute_local(const at_capsule_t *action, uint32_t requested_scope)
{
    if (!valid_capsule(action) || action->stage != AT_STAGE_ACTION ||
        (action->authority & AT_AUTH_ACTION) == 0u ||
        requested_scope == 0u || (requested_scope & ~action->scope) != 0u)
        return AT_E_AUTH;
    if ((requested_scope & AT_SCOPE_CORE_EXECUTE) != 0u)
        return AT_E_SCOPE;
    return AT_OK;
}

void at_mark_conflict(at_capsule_t *capsule)
{
    if (capsule) capsule->conflict = 1u;
}

void at_reboot(at_machine_t *machine)
{
    if (machine) {
        /* Counters remain audit data; no in-flight authority exists in the
         * machine itself. A real semantic index performs its own scrub/floor
         * protocol, which this contract deliberately does not bypass. */
        if (++machine->epoch == 0u) machine->epoch = 1u;
        machine->observations = 0u;
        machine->memories = 0u;
        machine->retrievals = 0u;
        machine->offers = 0u;
        machine->actions = 0u;
    }
}
