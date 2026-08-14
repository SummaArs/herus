/* test_threat_model_stress.c — F3 deterministic hostile evidence-classifier campaign. */
#include "threat_model.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define F3_SEED 0xF3E1D3CEu
#define F3_CANONICAL_ROUNDS 50000u
#define F3_RAW_ROUNDS 10000u

static uint32_t RNG = F3_SEED;
static uint32_t HOST_SUCCESS = 0u;
static uint32_t BLOCKED = 0u;
static int FAILED = 0;

static void fail_once(const char *what)
{
    if (!FAILED) printf("  FAIL %s\n", what);
    FAILED = 1;
}

static void ok(int condition, const char *what)
{
    printf("  %-4s %s\n", condition ? "PASS" : "FAIL", what);
    if (!condition) FAILED = 1;
}

static uint32_t next_u32(void)
{
    uint32_t x = RNG;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    RNG = x;
    return x;
}

static void canonical_snapshot(threat_model_snapshot_t *s)
{
    size_t i;
    uint8_t *bytes = (uint8_t *)s;
    for (i = 0u; i < sizeof(*s); ++i)
        bytes[i] = (uint8_t)(next_u32() & 1u);
}

static void raw_snapshot(threat_model_snapshot_t *s)
{
    size_t i;
    uint8_t *bytes = (uint8_t *)s;
    for (i = 0u; i < sizeof(*s); ++i)
        bytes[i] = (uint8_t)next_u32();
}

static int decision_is_canonical(const threat_model_decision_t *d)
{
    return d && d->evidence >= THREAT_MODEL_MITIGATED_HOST &&
           d->evidence <= THREAT_MODEL_OUT_OF_SCOPE &&
           (d->host_mitigated == 0u || d->host_mitigated == 1u) &&
           (d->host_mitigated != 1u ||
            (d->evidence == THREAT_MODEL_MITIGATED_HOST &&
             d->failures == THREAT_MODEL_FAIL_NONE));
}

static void exercise(threat_model_threat_t threat, const threat_model_snapshot_t *source,
                     int raw)
{
    threat_model_snapshot_t s = *source;
    threat_model_snapshot_t before = s;
    threat_model_decision_t d;
    int rc = threat_model_assess(threat, &s, &d);

    if (memcmp(&s, &before, sizeof(s)) != 0) {
        fail_once("F3 threat classifier mutated caller-supplied evidence");
        return;
    }
    if (!decision_is_canonical(&d)) {
        fail_once("F3 threat classifier emitted noncanonical decision state");
        return;
    }
    if (raw || threat < THREAT_MODEL_RADIO_ACTIVE || threat >= THREAT_MODEL_COUNT) {
        if (rc != THREAT_MODEL_E_BLOCKED || d.host_mitigated != 0u ||
            d.evidence != THREAT_MODEL_PENDING_TARGET ||
            d.failures != THREAT_MODEL_FAIL_FORMAT) {
            fail_once("F3 invalid evidence or threat scope did not fail closed as format error");
            return;
        }
        BLOCKED++;
        return;
    }
    if (rc == THREAT_MODEL_OK) {
        if (d.host_mitigated != 1u || d.evidence != THREAT_MODEL_MITIGATED_HOST ||
            d.failures != THREAT_MODEL_FAIL_NONE) {
            fail_once("F3 host mitigation result was internally inconsistent");
            return;
        }
        HOST_SUCCESS++;
        return;
    }
    if (rc == THREAT_MODEL_E_BLOCKED && d.host_mitigated == 0u) {
        BLOCKED++;
        return;
    }
    fail_once("F3 classifier returned an unsupported result for canonical evidence");
}

static void mutate_all_one_template(void)
{
    threat_model_snapshot_t s;
    size_t byte_index;
    uint8_t bit;
    memset(&s, 1, sizeof(s));
    for (byte_index = 0u; byte_index < sizeof(s); ++byte_index) {
        for (bit = 0u; bit < 8u; ++bit) {
            threat_model_snapshot_t mutation = s;
            threat_model_threat_t threat;
            ((uint8_t *)&mutation)[byte_index] ^= (uint8_t)(1u << bit);
            for (threat = THREAT_MODEL_RADIO_ACTIVE;
                 threat < THREAT_MODEL_COUNT; ++threat)
                exercise(threat, &mutation, bit != 0u);
        }
    }
}

static void check_all_one_contract(void)
{
    threat_model_snapshot_t s;
    threat_model_decision_t d;
    memset(&s, 1, sizeof(s));

    ok(threat_model_assess(THREAT_MODEL_RADIO_ACTIVE, &s, &d) == THREAT_MODEL_OK &&
       d.host_mitigated == 1u &&
       threat_model_assess(THREAT_MODEL_COMPANION_TRUST, &s, &d) == THREAT_MODEL_OK &&
       d.host_mitigated == 1u &&
       threat_model_assess(THREAT_MODEL_MEMORY_RETENTION, &s, &d) == THREAT_MODEL_OK &&
       d.host_mitigated == 1u &&
       threat_model_assess(THREAT_MODEL_MEMORY_RECOVERY, &s, &d) == THREAT_MODEL_OK &&
       d.host_mitigated == 1u &&
       threat_model_assess(THREAT_MODEL_MODEL_AGENCY, &s, &d) == THREAT_MODEL_OK &&
       d.host_mitigated == 1u &&
       threat_model_assess(THREAT_MODEL_TELEMETRY_PRIVACY, &s, &d) == THREAT_MODEL_OK &&
       d.host_mitigated == 1u,
       "F3 only complete canonical evidence mitigates supported host domains");

    ok(threat_model_assess(THREAT_MODEL_RADIO_METADATA, &s, &d) == THREAT_MODEL_E_BLOCKED &&
       d.evidence == THREAT_MODEL_OUT_OF_SCOPE && d.host_mitigated == 0u &&
       threat_model_assess(THREAT_MODEL_PHYSICAL_PLATFORM, &s, &d) == THREAT_MODEL_E_BLOCKED &&
       d.evidence == THREAT_MODEL_PENDING_TARGET && d.host_mitigated == 0u &&
       threat_model_assess(THREAT_MODEL_SUPPLY_CHAIN, &s, &d) == THREAT_MODEL_E_BLOCKED &&
       d.evidence == THREAT_MODEL_PENDING_TARGET && d.host_mitigated == 0u,
       "F3 unsupported metadata, target platform and supply chain never become host mitigation");
}

int main(void)
{
    threat_model_snapshot_t s;
    threat_model_decision_t d;
    uint32_t i;
    threat_model_threat_t threat;

    printf("\n== F3 deterministic hostile threat-model campaign ==\n");
    ok(threat_model_assess(THREAT_MODEL_MEMORY_RETENTION, 0, &d) == THREAT_MODEL_E_ARG &&
       threat_model_assess(THREAT_MODEL_MEMORY_RETENTION, &s, 0) == THREAT_MODEL_E_ARG,
       "F3 null threat-model inputs cannot produce classification");

    check_all_one_contract();
    mutate_all_one_template();
    for (i = 0u; i < F3_CANONICAL_ROUNDS; ++i) {
        canonical_snapshot(&s);
        for (threat = THREAT_MODEL_RADIO_ACTIVE;
             threat <= THREAT_MODEL_COUNT; ++threat)
            exercise(threat, &s, 0);
    }
    for (i = 0u; i < F3_RAW_ROUNDS; ++i) {
        raw_snapshot(&s);
        for (threat = THREAT_MODEL_RADIO_ACTIVE;
             threat <= THREAT_MODEL_COUNT; ++threat)
            exercise(threat, &s, 1);
    }

    ok(!FAILED && HOST_SUCCESS != 0u && BLOCKED != 0u,
       "F3 generated mitigated and blocked classifications without scope or format escalation");
    printf("  INFO seed=0x%08x canonical=%u raw=%u host_success=%u blocked=%u\n",
           F3_SEED, F3_CANONICAL_ROUNDS, F3_RAW_ROUNDS, HOST_SUCCESS, BLOCKED);
    if (FAILED) {
        printf("THREAT MODEL STRESS TESTS FAILED\n");
        return 1;
    }
    printf("THREAT MODEL STRESS INVARIANTS HOLD — hostile evidence never escalates model, privacy or target scope.\n");
    return 0;
}
