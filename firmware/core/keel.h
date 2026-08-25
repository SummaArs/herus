/*
 * HERUS keel — what keeps the brain on the wrist.
 *
 * docs/64-HERUS-SOBERANIA-ON-WRIST-E-FRONTEIRA-CORE.md states the product rule:
 * the wearable is the brain and the Core is a support station — charger, antenna
 * and an authorised feeder of new knowledge. A rule that lives only in a
 * document is a preference. Keel makes it a build failure.
 *
 * Two things are enforced here.
 *
 *   1. The budget. Every cognitive module declares its runtime footprint as the
 *      sizeof of the struct that actually exists, so the total is measured
 *      rather than asserted. If the cognitive path outgrows the wrist budget,
 *      keel_within_budget() returns zero and a suite fails.
 *   2. The Core role matrix. The Core has exactly three roles and a closed list
 *      of permitted actions. Executing, confirming, reading personal memory and
 *      writing personal memory are not on it, for any role, ever.
 *
 * Keel computes and refuses. It holds no state, no key and no personal data.
 */
#ifndef HERUS_KEEL_H
#define HERUS_KEEL_H

#include <stdint.h>

/* ESP32-S3 internal SRAM, from the silicon table in docs/00-HERUS-MASTER.md. */
#define KEEL_WRIST_SRAM_BYTES    524288u
/* What the whole HSCA cognitive path is allowed to occupy of it. */
#define KEEL_WRIST_BUDGET_BYTES   65536u

typedef enum {
    KEEL_CORE_ENERGY    = 1,   /* charge the wearable                          */
    KEEL_CORE_ANTENNA   = 2,   /* lend reach: relay, satellite uplink          */
    KEEL_CORE_KNOWLEDGE = 3    /* propose a versioned knowledge package        */
} keel_core_role_t;

typedef enum {
    KEEL_ACT_CHARGE            = 1,
    KEEL_ACT_RELAY_CIPHERTEXT  = 2,
    KEEL_ACT_SAT_UPLINK        = 3,
    KEEL_ACT_PROPOSE_KNOWLEDGE = 4,
    KEEL_ACT_EXECUTE           = 5,   /* never */
    KEEL_ACT_CONFIRM           = 6,   /* never */
    KEEL_ACT_READ_MEMORY       = 7,   /* never */
    KEEL_ACT_WRITE_MEMORY      = 8,   /* never */
    KEEL_ACT_COUNT             = 9
} keel_action_t;

typedef struct {
    const char *name;
    uint32_t    ram_bytes;      /* sizeof of the real runtime struct           */
    uint8_t     cognitive;      /* on the everyday thinking path               */
    uint8_t     core_required;  /* must be 0 for every cognitive module        */
} keel_module_t;

uint8_t                keel_module_count(void);
const keel_module_t   *keel_module(uint8_t index);

uint32_t keel_total_ram(void);
uint32_t keel_cognitive_ram(void);
int      keel_within_budget(void);

/* WRIST-01 / WRIST-08: the cognitive path is complete with the Core absent. */
int      keel_core_absent_complete(void);
/* WRIST-02 / WRIST-04 / WRIST-10: the Core role matrix. 1 permitted, 0 refused. */
int      keel_core_may(uint8_t role, uint8_t action);
/* WRIST-03 / WRIST-05 / WRIST-06: a knowledge package is admissible only when
 * fully identified, digest-checked, namespace-disjoint and locally confirmed. */
int      keel_knowledge_admissible(uint8_t producer_known, uint8_t registry_version_ok,
                                   uint8_t digest_ok, uint8_t namespace_personal,
                                   uint8_t locally_confirmed);

#endif /* HERUS_KEEL_H */
