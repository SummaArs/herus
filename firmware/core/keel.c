#include "keel.h"
#include "herald.h"
#include "hir.h"
#include "ladder.h"
#include "drift.h"
#include "aura.h"

/* Footprints are sizeof of the structs that exist, not estimates. A module that
 * grows changes this table automatically and the budget gate notices. */
static const keel_module_t MODULE[] = {
    { "hir",     (uint32_t)sizeof(hir_t),          1u, 0u },
    { "herald",  (uint32_t)sizeof(herald_unit_t),  1u, 0u },
    { "ladder",  (uint32_t)sizeof(ldr_plan_t) + (uint32_t)sizeof(ldr_facts_t), 1u, 0u },
    { "drift",   (uint32_t)sizeof(drift_store_t),  1u, 0u },
    { "aura",    (uint32_t)sizeof(aura_book_t),    1u, 0u }
};

#define MODULE_COUNT (sizeof MODULE / sizeof MODULE[0])

uint8_t keel_module_count(void) { return (uint8_t)MODULE_COUNT; }

const keel_module_t *keel_module(uint8_t index)
{
    if (index >= (uint8_t)MODULE_COUNT) return 0;
    return &MODULE[index];
}

uint32_t keel_total_ram(void)
{
    uint32_t total = 0;
    uint8_t i;
    for (i = 0; i < (uint8_t)MODULE_COUNT; i++) total += MODULE[i].ram_bytes;
    return total;
}

uint32_t keel_cognitive_ram(void)
{
    uint32_t total = 0;
    uint8_t i;
    for (i = 0; i < (uint8_t)MODULE_COUNT; i++)
        if (MODULE[i].cognitive) total += MODULE[i].ram_bytes;
    return total;
}

int keel_within_budget(void)
{
    return keel_total_ram() <= KEEL_WRIST_BUDGET_BYTES;
}

int keel_core_absent_complete(void)
{
    uint8_t i;
    for (i = 0; i < (uint8_t)MODULE_COUNT; i++)
        if (MODULE[i].cognitive && MODULE[i].core_required) return 0;
    return 1;
}

int keel_core_may(uint8_t role, uint8_t action)
{
    switch (action) {
    case KEEL_ACT_EXECUTE:
    case KEEL_ACT_CONFIRM:
    case KEEL_ACT_READ_MEMORY:
    case KEEL_ACT_WRITE_MEMORY:
        return 0;                       /* no role, ever */
    default:
        break;
    }
    switch (role) {
    case KEEL_CORE_ENERGY:
        return action == KEEL_ACT_CHARGE;
    case KEEL_CORE_ANTENNA:
        return action == KEEL_ACT_RELAY_CIPHERTEXT || action == KEEL_ACT_SAT_UPLINK;
    case KEEL_CORE_KNOWLEDGE:
        return action == KEEL_ACT_PROPOSE_KNOWLEDGE;
    default:
        return 0;
    }
}

int keel_knowledge_admissible(uint8_t producer_known, uint8_t registry_version_ok,
                              uint8_t digest_ok, uint8_t namespace_personal,
                              uint8_t locally_confirmed)
{
    if (!producer_known) return 0;
    if (!registry_version_ok) return 0;
    if (!digest_ok) return 0;
    /* WRIST-05: an external package may never land in the personal namespace. */
    if (namespace_personal) return 0;
    if (!locally_confirmed) return 0;
    return 1;
}
