/* test_hsca_keel.c — the wearable is the brain; the Core is a support station.
 *
 * docs/64 states that rule. This file is what makes breaking it a build failure.
 */
#include "keel.h"
#include <stdio.h>
#include <string.h>

static int pass_count;
static int fail_count;

static void check(int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) pass_count++; else fail_count++;
}

static void run_budget(void)
{
    uint8_t i;
    uint32_t sum = 0;

    printf("  ---- measured cognitive footprint (sizeof of the real structs)\n");
    for (i = 0; i < keel_module_count(); i++) {
        const keel_module_t *m = keel_module(i);
        printf("       %-10s %6u B   cognitive=%u core_required=%u\n",
               m->name, (unsigned)m->ram_bytes, m->cognitive, m->core_required);
        sum += m->ram_bytes;
    }
    printf("       %-10s %6u B   of %u B budget, %u B of wrist SRAM\n",
           "total", (unsigned)sum, (unsigned)KEEL_WRIST_BUDGET_BYTES,
           (unsigned)KEEL_WRIST_SRAM_BYTES);

    check(keel_module_count() > 0u, "the module table is not empty");
    check(keel_total_ram() == sum, "the reported total is the sum of the parts");
    check(keel_within_budget(), "the whole cognitive path fits the wrist budget");
    check(keel_cognitive_ram() == keel_total_ram(),
          "every module on the table is on the everyday thinking path");
    check(keel_total_ram() * 50u < KEEL_WRIST_SRAM_BYTES,
          "the cognitive state is under two percent of the silicon's SRAM");
    check(keel_module(keel_module_count()) == NULL, "an out-of-range module has no record");
}

static void run_sovereignty(void)
{
    check(keel_core_absent_complete(),
          "WRIST-01/08: no cognitive module requires the Core to be present");
}

static void run_core_matrix(void)
{
    static const char *ROLE[] = { "?", "energy", "antenna", "knowledge" };
    static const char *ACT[] = { "?", "charge", "relay-ciphertext", "sat-uplink",
                                 "propose-knowledge", "execute", "confirm",
                                 "read-memory", "write-memory" };
    unsigned role, action, permitted = 0, refused = 0;
    int never_ok = 1, roles_exact = 1;

    printf("  ---- Core role matrix (rows: role, columns: action)\n");
    for (role = 1u; role <= 3u; role++) {
        printf("       %-10s", ROLE[role]);
        for (action = 1u; action < KEEL_ACT_COUNT; action++) {
            int may = keel_core_may((uint8_t)role, (uint8_t)action);
            printf(" %s=%d", ACT[action], may);
            if (may) permitted++; else refused++;
            if (may && (action == KEEL_ACT_EXECUTE || action == KEEL_ACT_CONFIRM ||
                        action == KEEL_ACT_READ_MEMORY || action == KEEL_ACT_WRITE_MEMORY))
                never_ok = 0;
        }
        printf("\n");
    }
    printf("       %u permitted, %u refused across the full cross product\n", permitted, refused);

    check(never_ok,
          "WRIST-02/10: no role, ever, may execute, confirm, read or write personal memory");
    check(permitted == 4u,
          "the Core has exactly four permitted actions: charge, relay, uplink, propose");

    roles_exact = keel_core_may(KEEL_CORE_ENERGY, KEEL_ACT_CHARGE) &&
                  !keel_core_may(KEEL_CORE_ENERGY, KEEL_ACT_RELAY_CIPHERTEXT) &&
                  keel_core_may(KEEL_CORE_ANTENNA, KEEL_ACT_RELAY_CIPHERTEXT) &&
                  keel_core_may(KEEL_CORE_ANTENNA, KEEL_ACT_SAT_UPLINK) &&
                  !keel_core_may(KEEL_CORE_ANTENNA, KEEL_ACT_PROPOSE_KNOWLEDGE) &&
                  keel_core_may(KEEL_CORE_KNOWLEDGE, KEEL_ACT_PROPOSE_KNOWLEDGE) &&
                  !keel_core_may(KEEL_CORE_KNOWLEDGE, KEEL_ACT_CHARGE);
    check(roles_exact, "a role does not leak into a neighbouring role's action");
    check(!keel_core_may(0u, KEEL_ACT_CHARGE) && !keel_core_may(99u, KEEL_ACT_CHARGE),
          "an unknown role may do nothing");
    check(!keel_core_may(KEEL_CORE_ENERGY, 0u) && !keel_core_may(KEEL_CORE_ENERGY, 250u),
          "an unknown action is refused rather than defaulted");
}

static void run_knowledge_gate(void)
{
    unsigned mask, admitted = 0;
    int only_full = 1;

    /* Exhaustive truth table: five independent conditions, thirty-two cases. */
    for (mask = 0; mask < 32u; mask++) {
        uint8_t producer  = (uint8_t)((mask >> 0) & 1u);
        uint8_t version   = (uint8_t)((mask >> 1) & 1u);
        uint8_t digest    = (uint8_t)((mask >> 2) & 1u);
        uint8_t personal  = (uint8_t)((mask >> 3) & 1u);
        uint8_t confirmed = (uint8_t)((mask >> 4) & 1u);
        int ok = keel_knowledge_admissible(producer, version, digest, personal, confirmed);
        if (ok) {
            admitted++;
            if (!(producer && version && digest && !personal && confirmed)) only_full = 0;
        }
    }
    printf("  ---- knowledge gate: %u of 32 combinations admitted\n", admitted);
    check(admitted == 1u, "exactly one combination is admissible out of thirty-two");
    check(only_full,
          "WRIST-03/05/06: identity, version, digest, disjoint namespace and local confirmation, all five");
    check(!keel_knowledge_admissible(1u, 1u, 1u, 1u, 1u),
          "WRIST-05: an external package never lands in the personal namespace");
    check(!keel_knowledge_admissible(1u, 1u, 1u, 0u, 0u),
          "WRIST-04: without a local confirmation the package stays a proposal");
}

int main(void)
{
    printf("--- keel: what keeps the brain on the wrist ---\n");
    run_budget();
    run_sovereignty();
    run_core_matrix();
    run_knowledge_gate();
    printf("HSCA KEEL: %d pass, %d fail\n", pass_count, fail_count);
    return fail_count ? 1 : 0;
}
