#include "ladder.h"
#include <string.h>

/* Declared, not measured. Sources, in the order of the table:
 *   TOUCH      ISO/IEC 14443 tag payload, ST25DV dynamic tag mailbox
 *   GLYPH      QR version 3-L holds 53 bytes of binary data
 *   SOUND      near-ultrasonic acoustic coupling, ~100 bit/s conservative
 *   BLE        Bluetooth 5 LE, 1M PHY, extended advertising payload
 *   BLE_CODED  Bluetooth 5 LE Coded PHY S=8, ~4x range for ~1/8 rate
 *   ESPNOW     Espressif ESP-NOW, 250-byte connectionless frame
 *   WIFI       802.11 local link to Core / Paper-Core
 *   LORA       SX1262 SF9, 246.8 ms airtime, inside the 400 ms dwell rule
 *   LORA_MESH  Weave, three hops, Band antennas (docs/00-HERUS-MASTER.md sec.6)
 *   SAT        short-burst uplink; needs the Core antenna, never the wrist
 *   DRIFT      custody handed to a passing device; reach grows with time
 *
 * The energy column is a coarse per-meaning figure used only for ordering a
 * routine message. It is not a battery model and nothing in this file claims a
 * battery number. */
static const ldr_profile_t PROFILE[LDR_COUNT] = {
/*  name          maxB reach_m  lat_ms  cuAh  cls peer core los infra bcast meas */
  { "touch",       246,       0,    120,    2, LDR_CLASS_BULK,      0,0,0, 1,0, 0 },
  { "glyph",        53,       5,   1500,    5, LDR_CLASS_CARD,      0,0,1, 1,1, 0 },
  { "sound",        64,      10,   2800,   12, LDR_CLASS_ESSENTIAL, 0,0,0, 1,1, 0 },
  { "ble",         240,      30,     40,   18, LDR_CLASS_TELEMETRY, 1,0,0, 1,1, 0 },
  { "ble-coded",   240,     120,    160,   34, LDR_CLASS_TELEMETRY, 1,0,0, 1,1, 0 },
  { "esp-now",     250,     180,     20,   90, LDR_CLASS_TELEMETRY, 1,0,0, 1,1, 0 },
  { "wifi",       1400,      60,     15,  240, LDR_CLASS_BULK,      1,0,0, 0,0, 0 },
  { "lora",        222,     650,    247,   66, LDR_CLASS_CARD,      1,0,0, 1,1, 0 },
  { "lora-mesh",   222,    1900,    900,  198, LDR_CLASS_CARD,      1,0,0, 1,1, 0 },
  { "sat",         340, 4000000,  20000, 1500, LDR_CLASS_ESSENTIAL, 0,1,1, 1,0, 0 },
  { "drift",        34, 12000UL,3600000,   66, LDR_CLASS_ESSENTIAL, 0,0,0, 1,1, 0 }
};

const ldr_profile_t *ldr_profile(uint8_t carrier)
{
    if (carrier >= LDR_COUNT) return NULL;
    return &PROFILE[carrier];
}

const char *ldr_name(uint8_t carrier)
{
    const ldr_profile_t *p = ldr_profile(carrier);
    return p ? p->name : "?";
}

int ldr_carries(uint8_t carrier, uint16_t bytes, uint32_t max_latency_ms)
{
    const ldr_profile_t *p = ldr_profile(carrier);
    if (!p) return 0;
    if (bytes == 0u || bytes > p->max_payload_bytes) return 0;
    if (max_latency_ms != 0u && p->latency_ms > max_latency_ms) return 0;
    return 1;
}

uint8_t ldr_rungs_for(uint16_t bytes, uint32_t max_latency_ms)
{
    uint8_t i, n = 0;
    for (i = 0; i < LDR_COUNT; i++) if (ldr_carries(i, bytes, max_latency_ms)) n++;
    return n;
}

uint8_t ldr_unmeasured_count(void)
{
    uint8_t i, n = 0;
    for (i = 0; i < LDR_COUNT; i++) if (!PROFILE[i].measured) n++;
    return n;
}

static int eligible(uint8_t c, const ldr_facts_t *f)
{
    const ldr_profile_t *p = &PROFILE[c];
    if (!f->available[c]) return 0;
    if (f->payload_bytes == 0u || f->payload_bytes > p->max_payload_bytes) return 0;
    if (f->payload_class > p->max_class) return 0;
    if (p->needs_paired_peer && !f->peer_paired) return 0;
    if (p->needs_core && !f->core_present) return 0;
    if (p->needs_line_of_sight && !f->line_of_sight) return 0;
    if (p->reach_m < f->peer_distance_m) return 0;
    return 1;
}

/* Ordering key. Urgent traffic is ordered by latency, routine traffic by energy;
 * reach breaks every tie so that, all else equal, the meaning goes further.
 * DRIFT is never ranked: it is appended last, because handing custody costs a
 * person nothing and is the only rung whose reach grows after the send. */
static int better(uint8_t a, uint8_t b, uint8_t urgency)
{
    const ldr_profile_t *pa = &PROFILE[a], *pb = &PROFILE[b];
    if (urgency >= (uint8_t)HIR_URG_URGENTE) {
        if (pa->latency_ms != pb->latency_ms) return pa->latency_ms < pb->latency_ms;
    } else {
        if (pa->energy_cuah != pb->energy_cuah) return pa->energy_cuah < pb->energy_cuah;
    }
    if (pa->reach_m != pb->reach_m) return pa->reach_m > pb->reach_m;
    return a < b;
}

ldr_status_t ldr_plan(const ldr_facts_t *facts, ldr_plan_t *out)
{
    uint8_t cand[LDR_COUNT];
    uint8_t n = 0, i, j;
    uint32_t best_possible_reach = 0, best_eligible_reach = 0;

    if (!facts || !out) return LDR_E_ARG;
    memset(out, 0, sizeof *out);
    if (facts->payload_class < LDR_CLASS_ESSENTIAL || facts->payload_class > LDR_CLASS_BULK)
        return LDR_E_ARG;
    if (facts->urgency > (uint8_t)HIR_URG_SOCORRO) return LDR_E_ARG;
    out->payload_class = facts->payload_class;

    for (i = 0; i < LDR_COUNT; i++) {
        if (PROFILE[i].reach_m > best_possible_reach &&
            facts->payload_bytes <= PROFILE[i].max_payload_bytes &&
            facts->payload_class <= PROFILE[i].max_class &&
            PROFILE[i].reach_m >= facts->peer_distance_m) {
            best_possible_reach = PROFILE[i].reach_m;
        }
        if (i == LDR_DRIFT) continue;
        if (!eligible(i, facts)) continue;
        cand[n++] = i;
        if (PROFILE[i].reach_m > best_eligible_reach) best_eligible_reach = PROFILE[i].reach_m;
    }

    for (i = 1; i < n; i++) {
        uint8_t key = cand[i];
        j = i;
        while (j > 0 && better(key, cand[j-1], facts->urgency)) {
            cand[j] = cand[j-1];
            j--;
        }
        cand[j] = key;
    }
    for (i = 0; i < n; i++) out->carrier[out->count++] = cand[i];

    if (eligible(LDR_DRIFT, facts)) {
        out->carrier[out->count++] = LDR_DRIFT;
        if (PROFILE[LDR_DRIFT].reach_m > best_eligible_reach)
            best_eligible_reach = PROFILE[LDR_DRIFT].reach_m;
    }

    if (out->count == 0u) return LDR_E_NO_ROUTE;
    out->widest_reach_dropped = (best_eligible_reach < best_possible_reach) ? 1u : 0u;
    return LDR_OK;
}

ldr_status_t ldr_commit(const ldr_plan_t *plan, uint8_t confirmed,
                        uint8_t confirmed_class, uint8_t *chosen_carrier)
{
    if (!plan || !chosen_carrier) return LDR_E_ARG;
    *chosen_carrier = LDR_COUNT;
    if (plan->count == 0u) return LDR_E_EMPTY_PLAN;
    if (!confirmed) return LDR_E_UNCONFIRMED;
    /* Changing rung must never change what the person agreed to send. */
    if (confirmed_class != plan->payload_class) return LDR_E_CLASS_DRIFT;
    if (plan->carrier[0] >= LDR_COUNT) return LDR_E_ARG;
    *chosen_carrier = plan->carrier[0];
    return LDR_OK;
}
