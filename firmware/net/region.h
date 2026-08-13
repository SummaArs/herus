/* region.h — LoRa airtime, and the regulatory ceiling made a compile error.
 *
 * P2 says no frame may exceed 400 ms of dwell. That is not a guideline you check
 * at certification, it is an arithmetic property of (SF, payload, CR, CRC) that
 * can be evaluated at COMPILE TIME — so it is, below, with _Static_assert. A
 * build that would transmit an illegal frame does not link.
 *
 * The airtime formula is the Semtech one (AN1200.13 / SX1276 datasheet §4.1.1.7,
 * unchanged for SX126x):
 *
 *   Tsym      = 2^SF / BW
 *   Tpreamble = (n_pre + 4.25) * Tsym
 *   n_payload = 8 + max(0, ceil((8*PL - 4*SF + 28 + 16*CRC - 20*IH)
 *                               / (4*(SF - 2*DE))) * (CR + 4))
 *   Tair      = Tpreamble + n_payload * Tsym
 *
 * Cross-checked three ways against docs/02-PROTOCOL.md §2, which was produced
 * independently by tools/budget.py:
 *   34 B, SF9, CR4/5, CRC on, explicit header -> 246.8 ms   (matches)
 *   178 B, SF7                                -> 287.0 ms   (matches)
 *   82 B, SF8                                 -> 256.5 ms   (matches)
 * Three independent agreements is why these numbers are usable as a constraint
 * rather than as decoration.
 */
#ifndef HERUS_REGION_H
#define HERUS_REGION_H

#include <stdint.h>

/* All Herus profiles run BW 125 kHz, so Tsym is exactly (2^SF) * 8 us and the
 * whole calculation stays in integers — no floating point in a constraint that
 * has to hold at compile time. */
#define HZ_TSYM_US(sf)          ((1u << (sf)) * 8u)
#define HZ_CEILDIV(a, b)        (((a) + (b) - 1) / (b))

/* Low data-rate optimisation is off for SF < 11, and SF 10-12 are absent from
 * every shipping profile, so DE = 0 everywhere. Stated rather than hidden in a
 * conditional, because if SF ever changes this line is the one that must move. */
#define HZ_PAYLOAD_SYM(sf, pl, crc, ih, cr)                                    \
    (8u + HZ_CEILDIV((8u * (pl)) + 28u + (16u * (crc)) - (4u * (sf))           \
                     - (20u * (ih)), 4u * (sf)) * ((cr) + 4u))

/* n_pre = 8 symbols; (8 + 4.25) * 4 = 49 quarters. */
#define HZ_AIRTIME_US(sf, pl, crc, ih, cr)                                     \
    ((49u * HZ_TSYM_US(sf)) / 4u + HZ_PAYLOAD_SYM(sf, pl, crc, ih, cr) * HZ_TSYM_US(sf))

#define HZ_DWELL_LIMIT_US   400000u

/* ---------------- the frames Herus actually transmits ----------------
 *
 * ERRATUM E-P1 (found by this file, 2026-07-30). Revision 0.2 of the protocol
 * gave Tier 0.5 the same 34 BYTES as Tier 0/1 and concluded the tiers were
 * indistinguishable. They are not. Tier 0.5 runs implicit header with CRC off,
 * which removes 20 + 16 bits from the symbol count, so 34 B costs 226.3 ms
 * against 246.8 ms — a 20.5 ms difference, visible to exactly the spectrum
 * analyser P1 exists to defeat, and 8x larger than the Beat guard window.
 *
 * P1 is a statement about AIRTIME, and bytes were only ever a proxy for it. The
 * fix is to equalise the thing that is actually observable: at SF9 the payload
 * symbol count is a step function, and 36..39 bytes of implicit-header CRC-less
 * payload all land on exactly 48 symbols = 246.8 ms. Tier 0.5 is therefore 38
 * bytes: 2 address + 32 sketch + 4 pad. The 4 pad bytes are not waste — they are
 * a sketch extension to 288 or 320 bits that a later revision can spend at ZERO
 * airtime cost, because the step does not move until 40 bytes.
 *
 * Restated as the rule an implementation must hold: equal AIRTIME across
 * meaning tiers, not equal length. The static assert below is the enforcement.
 */
#define HERUS_FRAME_LEN         34u  /* Tier 0/1: 2 addr + 24 ct + 8 tag */
#define HERUS_SKETCH_FRAME_LEN  38u  /* Tier 0.5: 2 addr + 32 sketch + 4 pad */
#define HERUS_SKETCH_BYTES      32u
#define HERUS_SKETCH_PAD        4u
#define HERUS_SF_MEANING      9u     /* the ceiling for a 34 B frame, not a preference */
#define HERUS_SF_VOICE        7u
#define HERUS_SF_SOS          8u
#define HERUS_CR              1u     /* 4/5 */

#define HERUS_AIRTIME_MEANING_US  HZ_AIRTIME_US(HERUS_SF_MEANING, HERUS_FRAME_LEN, 1u, 0u, HERUS_CR)
#define HERUS_AIRTIME_SKETCH_US   HZ_AIRTIME_US(HERUS_SF_MEANING, HERUS_SKETCH_FRAME_LEN, 0u, 1u, HERUS_CR)
#define HERUS_AIRTIME_VOICE_US    HZ_AIRTIME_US(HERUS_SF_VOICE, 178u, 1u, 0u, HERUS_CR)
#define HERUS_AIRTIME_SOS_US      HZ_AIRTIME_US(HERUS_SF_SOS, 82u, 1u, 0u, HERUS_CR)

/* ---------------- THE REACH PROFILE, AND THE PROOF THAT IT IS THE CEILING ----
 *
 * Range here is bounded by regulation, not by the radio, and the dwell budget is
 * a currency: 400 ms buys either bytes or spreading factor, never both. Rev 1
 * spent all of it on bytes — 34 B at SF9, with 25 B of dwell headroom left
 * unspent. Spending that headroom on SF instead buys 2.5 dB, which is +15.5% of
 * range in the d^4 regime a wrist device actually lives in.
 *
 * The three asserts below are the whole argument, and they are asserts rather
 * than a paragraph because a paragraph cannot fail a build:
 *
 *   1. 24 B at SF10 is legal.
 *   2. 25 B at SF10 is NOT. So 24 is maximal, not chosen.
 *   3. SF11 cannot carry even a single byte. So SF10 is the last rung.
 *
 * Together they say something stronger than "we improved the range": they say
 * there is nothing left to take without changing the band or the law. Anyone who
 * later proposes SF11 has to delete assert 3 first, and will then see why.
 *
 * What it costs, stated where the gain is stated: 370.7 ms of airtime against
 * 246.8 ms (+50% transmit energy per frame) and 4 slots against 9. Reach is
 * further. It is not cheaper and it is not richer. */
#define HERUS_SF_REACH           10u
#define HERUS_REACH_FRAME_LEN    24u     /* 2 addr + 14 ct + 8 tag */
#define HERUS_REACH_PT_LEN       14u
#define HERUS_AIRTIME_REACH_US   HZ_AIRTIME_US(HERUS_SF_REACH, HERUS_REACH_FRAME_LEN, 1u, 0u, HERUS_CR)

_Static_assert(HERUS_AIRTIME_REACH_US < HZ_DWELL_LIMIT_US,
               "P2: the Reach frame must fit the 400 ms dwell limit");
_Static_assert(HZ_AIRTIME_US(HERUS_SF_REACH, HERUS_REACH_FRAME_LEN + 1u, 1u, 0u, HERUS_CR)
                   > HZ_DWELL_LIMIT_US,
               "24 B is the CEILING at SF10, not a preference: 25 B is illegal");
_Static_assert(HZ_AIRTIME_US(11u, 1u, 1u, 0u, HERUS_CR) > HZ_DWELL_LIMIT_US,
               "SF11 cannot carry one single byte, so SF10 is the last rung");

/* Which of the two profiles a group is provisioned for. Per GROUP, never per
 * message: mixing them inside one group would publish which messages mattered,
 * which is exactly what P1 exists to prevent. */
typedef enum { HZ_LINK_RICH = 0, HZ_LINK_REACH } hz_link_t;

typedef struct {
    const char *name;
    uint8_t     sf;
    uint8_t     frame_len;
    uint8_t     pt_len;
    uint8_t     tag_len;
    uint8_t     max_slot;
    uint32_t    airtime_us;
    int16_t     sens_dbm_x2;      /* sensitivity in half-dBm; -129.5 dBm needs 16 bits */
} hz_link_profile_t;

const hz_link_profile_t *hz_link(hz_link_t p);

typedef enum { HZ_REGION_BR915 = 0, HZ_REGION_US915, HZ_REGION_EU868, HZ_REGION_LAB }
        hz_region_t;

typedef struct {
    const char *name;
    uint32_t    f_lo_hz, f_hi_hz;
    /* A HARD ceiling on spreading factor, independent of payload. It used to be
     * documented as "for a 34 B frame", which made it a restatement of the dwell
     * rule in a place that could not see the payload — and when the Reach
     * profile arrived it silently refused every legal SF10 frame. A guard that
     * encodes an assumption instead of a rule fails exactly once the assumption
     * changes, and fails silently, which is the worst way.
     *
     * The payload-dependent work belongs to the dwell check below, which is the
     * actual regulation. This field now says only what no payload can buy back:
     * SF11 cannot carry one byte inside 400 ms (asserted above), so 10. */
    uint8_t     max_sf;
    uint32_t    dwell_us;           /* 0 = no dwell rule (duty cycle instead) */
    uint16_t    duty_permille;      /* 0 = no duty rule */
    uint8_t     tx_dbm_max;
} hz_profile_t;

/* Compiled in, selected at provisioning, never inferred at runtime from
 * anything an attacker can influence (footgun #10). */
const hz_profile_t *hz_profile(hz_region_t r);

/* Runtime airtime, for the scheduler and the field log. */
uint32_t hz_airtime_us(uint8_t sf, uint16_t payload_len, int crc_on,
                       int implicit_header, uint8_t cr);

/* Returns 1 if this frame is legal in this profile. The transmit path calls it
 * and refuses; that refusal is the last line of defence behind the static
 * asserts, for payload lengths only known at runtime (Tier 2 fragments). */
int hz_tx_permitted(hz_region_t r, uint8_t sf, uint16_t payload_len, int crc_on,
                    int implicit_header, uint8_t cr, uint8_t dbm);

/* Prints the ledger that docs/02-PROTOCOL.md §2 asserts. Used by test_net. */
void hz_print_ledger(void);

#endif /* HERUS_REGION_H */
