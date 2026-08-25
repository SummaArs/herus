/*
 * HERUS ladder — the carrier ladder: one meaning, every channel.
 *
 * transport_selector.{h,c} answers "which of the four radios should carry this
 * envelope?". The ladder answers a larger question that only becomes askable
 * once the unit of communication is a 34-byte meaning: *which channels in the
 * world are wide enough?* The answer is all of them, and that is the point.
 *
 * A HIR meaning on air is HIR_ONAIR_BYTES = 34 bytes. Thirty-four bytes fit in
 * an NFC tap, in a printed or displayed glyph, in a near-ultrasonic chirp, in a
 * BLE advertisement, in an ESP-NOW frame, in one legal SF9 LoRa dwell, in a
 * satellite short burst, and in a bundle handed to a stranger walking the other
 * way. Four seconds of compressed speech do not fit in most of them. The ladder
 * makes that difference executable: LADDER_INV_MEANING_FITS_EVERY_RUNG is
 * asserted over the whole table, not asserted in a document.
 *
 * This module is advice. It never opens a session, touches a key, serializes a
 * frame, transmits, or turns a recommendation into permission. ldr_plan() may be
 * called before confirmation precisely so a person can see where their meaning
 * would go; only ldr_commit() looks at the confirmation, and it refuses to
 * commit a route whose payload class differs from the one that was confirmed.
 *
 * Every reach, latency and energy figure in the table below is DECLARED from a
 * datasheet or a specification, not measured on Herus hardware. `measured` is
 * zero for every rung in this revision and ldr_unmeasured_count() exists so that
 * no document can quietly forget it.
 */
#ifndef HERUS_LADDER_H
#define HERUS_LADDER_H

#include <stdint.h>
#include "hir.h"

typedef enum {
    LDR_TOUCH = 0,     /* NFC tap: centimetres, no session, no radio budget    */
    LDR_GLYPH,         /* optical: a rendered symbol read by any camera        */
    LDR_SOUND,         /* near-ultrasonic acoustic coupling                    */
    LDR_BLE,           /* BLE 1M PHY, connectable or advertisement             */
    LDR_BLE_CODED,     /* BLE LE Coded PHY S=8                                 */
    LDR_ESPNOW,        /* connectionless Wi-Fi between provisioned peers       */
    LDR_WIFI,          /* local Wi-Fi to Core / Paper-Core                     */
    LDR_LORA,          /* SX1262, SF9, direct, inside the legal dwell window   */
    LDR_LORA_MESH,     /* Weave, three hops                                    */
    LDR_SAT,           /* short-burst uplink through the Core antenna          */
    LDR_DRIFT,         /* delay-tolerant custody: reach as a function of time  */
    LDR_COUNT
} ldr_carrier_t;

/* Payload classes, ordered. A carrier declares the highest class it may carry. */
typedef enum {
    LDR_CLASS_ESSENTIAL = 1,  /* a bare meaning: state, arrival, help          */
    LDR_CLASS_CARD      = 2,  /* a minimal semantic card                       */
    LDR_CLASS_TELEMETRY = 3,  /* consented derived personal metrics            */
    LDR_CLASS_BULK      = 4   /* versioned local packages, vault, knowledge     */
} ldr_class_t;

typedef enum {
    LDR_OK            = 0,
    LDR_E_ARG         = 1,
    LDR_E_NO_ROUTE    = 2,
    LDR_E_UNCONFIRMED = 3,
    LDR_E_CLASS_DRIFT = 4,   /* the committed class is not the confirmed one   */
    LDR_E_EMPTY_PLAN  = 5
} ldr_status_t;

typedef struct {
    const char *name;
    uint16_t max_payload_bytes;
    uint32_t reach_m;             /* order of magnitude, declared              */
    uint32_t latency_ms;          /* one way, one meaning, declared            */
    uint16_t energy_cuah;         /* hundredths of uAh per meaning, declared   */
    uint8_t  max_class;           /* highest ldr_class_t it may carry          */
    uint8_t  needs_paired_peer;
    uint8_t  needs_core;
    uint8_t  needs_line_of_sight;
    uint8_t  works_without_infrastructure;
    uint8_t  broadcast_capable;
    uint8_t  measured;            /* 0 = never measured on Herus hardware      */
} ldr_profile_t;

typedef struct {
    uint8_t  payload_class;       /* ldr_class_t                               */
    uint16_t payload_bytes;
    /* How far the recipient is, in metres. Zero means "in front of me", which
     * is the only condition under which a tap or a glyph is a real route. A
     * rung whose declared reach is shorter than this is not eligible: a plan
     * that names NFC for someone four kilometres away is not a plan. */
    uint32_t peer_distance_m;
    uint8_t  urgency;             /* hir_urgency_t                             */
    uint8_t  peer_paired;
    uint8_t  core_present;
    uint8_t  line_of_sight;
    uint8_t  available[LDR_COUNT];
} ldr_facts_t;

typedef struct {
    uint8_t count;
    uint8_t carrier[LDR_COUNT];   /* ordered, best rung first                  */
    uint8_t payload_class;        /* copied from the facts, never rewritten    */
    uint8_t widest_reach_dropped; /* a wider rung existed but was not eligible */
} ldr_plan_t;

const ldr_profile_t *ldr_profile(uint8_t carrier);
const char          *ldr_name(uint8_t carrier);

/* True when `bytes` fit this carrier and arrive within `max_latency_ms`
 * (0 = no latency requirement). */
int  ldr_carries(uint8_t carrier, uint16_t bytes, uint32_t max_latency_ms);

/* How many rungs can carry a full meaning; how many can carry `bytes`. */
uint8_t ldr_rungs_for(uint16_t bytes, uint32_t max_latency_ms);
uint8_t ldr_unmeasured_count(void);

ldr_status_t ldr_plan(const ldr_facts_t *facts, ldr_plan_t *out);

/* The only function that looks at confirmation. `confirmed_class` is the class
 * the person actually saw and confirmed. */
ldr_status_t ldr_commit(const ldr_plan_t *plan, uint8_t confirmed,
                        uint8_t confirmed_class, uint8_t *chosen_carrier);

#endif /* HERUS_LADDER_H */
