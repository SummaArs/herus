#include "poisoning_guard.h"
#include <string.h>

static int valid_memory(const at_capsule_t *memory)
{
    return memory && memory->stage == AT_STAGE_MEMORY &&
           memory->provenance_id != 0u && memory->epoch != 0u &&
           (memory->authority & AT_AUTH_MEMORY) != 0u;
}

static uint32_t min_u32(uint32_t a, uint32_t b)
{
    if (a == 0u) return b;
    if (b == 0u) return a;
    return a < b ? a : b;
}

void pg_init(pg_bundle_t *bundle, uint32_t epoch)
{
    if (bundle) {
        memset(bundle, 0, sizeof(*bundle));
        bundle->epoch = epoch;
    }
}

int pg_add_memory(pg_bundle_t *bundle, const at_capsule_t *memory,
                  uint32_t current_epoch, uint32_t generation)
{
    uint8_t i;
    if (!bundle || !valid_memory(memory)) return PG_E_STAGE;
    if (current_epoch != bundle->epoch || memory->epoch != bundle->epoch)
        return PG_E_EPOCH;
    if (generation < memory->generation ||
        (memory->valid_until_generation != 0u &&
         generation > memory->valid_until_generation))
        return PG_E_EXPIRED;
    if (memory->conflict) return PG_E_CONFLICT;
    if (bundle->item_count >= PG_MAX_ITEMS) return PG_E_FULL;
    for (i = 0u; i < bundle->item_count; i++)
        if (bundle->provenance_id[i] == memory->provenance_id)
            return PG_NO_CHANGE;

    bundle->provenance_id[bundle->item_count++] = memory->provenance_id;
    bundle->authority_intersection =
        bundle->authority_intersection == 0u
            ? memory->authority
            : bundle->authority_intersection & memory->authority;
    if (bundle->source == 0u) bundle->source = memory->source;
    else if (bundle->source != memory->source)
        bundle->source = AT_SOURCE_CORE_KNOWLEDGE;
    bundle->generation = generation;
    bundle->valid_until_generation =
        min_u32(bundle->valid_until_generation,
                memory->valid_until_generation);
    return PG_OK;
}

int pg_trigger_context(const pg_bundle_t *bundle, uint32_t context_token,
                       uint32_t expected_context_token, uint32_t current_epoch,
                       uint32_t generation, at_capsule_t *out_offer)
{
    if (!bundle || !out_offer || bundle->item_count == 0u)
        return PG_E_ARG;
    if (context_token == 0u || context_token != expected_context_token)
        return PG_E_CONTEXT;
    if (bundle->epoch == 0u || current_epoch != bundle->epoch ||
        generation < bundle->generation ||
        (bundle->valid_until_generation != 0u &&
         generation > bundle->valid_until_generation))
        return PG_E_EXPIRED;
    if (bundle->conflict) return PG_E_CONFLICT;

    memset(out_offer, 0, sizeof(*out_offer));
    out_offer->stage = AT_STAGE_OFFER;
    out_offer->source = bundle->source;
    out_offer->provenance_id = bundle->provenance_id[0];
    out_offer->authority = bundle->authority_intersection;
    out_offer->generation = generation;
    out_offer->epoch = bundle->epoch;
    out_offer->valid_until_generation = bundle->valid_until_generation;
    /* Context activation cannot manufacture physical confirmation or action. */
    out_offer->physically_confirmed = 0u;
    out_offer->scope = 0u;
    return PG_OK;
}

int pg_grant_action(const pg_bundle_t *bundle, uint32_t current_epoch,
                    uint32_t local_scope, uint8_t physical_confirmation,
                    uint32_t generation, at_capsule_t *out_action)
{
    at_machine_t machine;
    at_capsule_t offer;
    int status;
    if (!bundle || !out_action) return PG_E_ARG;
    status = pg_trigger_context(bundle, bundle->provenance_id[0],
                                bundle->provenance_id[0], current_epoch,
                                generation, &offer);
    if (status != PG_OK) return status;
    at_init(&machine);
    machine.epoch = bundle->epoch;
    status = at_grant_local_action(&machine, &offer, local_scope,
                                   physical_confirmation, generation,
                                   out_action);
    return status == AT_OK ? PG_OK : PG_E_AUTH;
}
