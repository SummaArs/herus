/* region.c — see region.h. */
#include "region.h"
#include "../core/hcp.h"
#include <stdio.h>

/* ------------------------------------------------------------------------- *
 * P2, enforced by the compiler. If any of these fire, the fix is not to raise
 * the limit — the limit is a regulation. The fix is a lower SF or a shorter
 * frame, and the failing line tells you which frame is at fault.
 * ------------------------------------------------------------------------- */
_Static_assert(HERUS_AIRTIME_MEANING_US <= HZ_DWELL_LIMIT_US,
               "P2: the 34-byte meaning frame exceeds the 400 ms dwell limit");
_Static_assert(HERUS_AIRTIME_SKETCH_US <= HZ_DWELL_LIMIT_US,
               "P2: the Tier 0.5 sketch frame exceeds the 400 ms dwell limit");
_Static_assert(HERUS_AIRTIME_VOICE_US <= HZ_DWELL_LIMIT_US,
               "P2: a Tier 2 voice fragment exceeds the 400 ms dwell limit");
_Static_assert(HERUS_AIRTIME_SOS_US <= HZ_DWELL_LIMIT_US,
               "P2: the SOS beacon exceeds the 400 ms dwell limit");

/* P1, enforced by the compiler: equal AIRTIME, not equal length (erratum E-P1
 * in region.h). This is the assertion budget.py used to make about byte counts,
 * corrected to assert the property that is actually observable on air. */
_Static_assert(HERUS_AIRTIME_MEANING_US == HERUS_AIRTIME_SKETCH_US,
               "P1: Tier 0.5 airtime must equal Tier 0/1 airtime, or the tiers "
               "are separable by airtime alone and confidentiality is void");

/* The published figures, asserted so a formula edit cannot silently drift away
 * from the documents. Tolerance is 100 us: the docs round to 0.1 ms. */
_Static_assert(HERUS_AIRTIME_MEANING_US >= 246700u && HERUS_AIRTIME_MEANING_US <= 246900u,
               "34 B at SF9 must be 246.8 ms as published in 02-PROTOCOL.md");
_Static_assert(HERUS_AIRTIME_VOICE_US >= 286900u && HERUS_AIRTIME_VOICE_US <= 287100u,
               "178 B at SF7 must be 287.0 ms as published");
_Static_assert(HERUS_AIRTIME_SOS_US >= 256400u && HERUS_AIRTIME_SOS_US <= 256600u,
               "82 B at SF8 must be 256.5 ms as published");

/* SF10 at 34 B is the first illegal step, and proving that here is what makes
 * "SF9 is the ceiling" a fact in the build rather than a claim in a document. */
_Static_assert(HZ_AIRTIME_US(10u, HERUS_FRAME_LEN, 1u, 0u, HERUS_CR) > HZ_DWELL_LIMIT_US,
               "SF10 must be illegal at 34 B — if it is not, the formula is wrong");

static const hz_profile_t PROFILES[] = {
    /* BR915: ANATEL Ato 14448 mirrors FCC 15.247(a)(1)(iii). Two sub-bands, and
     * the gap 907.5-915 MHz is not ours to use. */
    { "BR915", 902000000u, 928000000u, 10u, 400000u, 0u,   30u },
    { "US915", 902000000u, 928000000u, 10u, 400000u, 0u,   30u },
    /* EU868 has no dwell rule but a 1% duty cycle, which at 246.8 ms is about
     * 145 frames/hour: ample for messaging, fatal for voice calls. */
    { "EU868", 863000000u, 870000000u, 10u, 0u,      10u,  14u },
    /* LAB exists so research is possible without shipping an illegal device. It
     * must be selected deliberately at provisioning and it is not a default. */
    { "LAB",   902000000u, 928000000u, 12u, 0u,     0u,   22u },
};

const hz_profile_t *hz_profile(hz_region_t r)
{
    if ((int)r < 0 || (int)r > 3) return &PROFILES[0];
    return &PROFILES[r];
}

uint32_t hz_airtime_us(uint8_t sf, uint16_t payload_len, int crc_on,
                       int implicit_header, uint8_t cr)
{
    uint32_t tsym = HZ_TSYM_US(sf);
    int32_t num = (int32_t)(8u * payload_len) + 28 + (crc_on ? 16 : 0)
                  - (int32_t)(4u * sf) - (implicit_header ? 20 : 0);
    int32_t den = (int32_t)(4u * sf);
    int32_t nsym;
    if (num <= 0) nsym = 0;
    else          nsym = ((num + den - 1) / den) * (int32_t)(cr + 4u);
    return (49u * tsym) / 4u + (uint32_t)(8 + nsym) * tsym;
}

int hz_tx_permitted(hz_region_t r, uint8_t sf, uint16_t payload_len, int crc_on,
                    int implicit_header, uint8_t cr, uint8_t dbm)
{
    const hz_profile_t *p = hz_profile(r);
    if (sf > p->max_sf) return 0;
    if (dbm > p->tx_dbm_max) return 0;
    if (p->dwell_us) {
        uint32_t t = hz_airtime_us(sf, payload_len, crc_on, implicit_header, cr);
        if (t > p->dwell_us) return 0;
    }
    return 1;
}

void hz_print_ledger(void)
{
    struct { const char *n; uint8_t sf; uint16_t pl; int crc, ih; } rows[] = {
        { "Tier 0 glyph   ", HERUS_SF_MEANING, HERUS_FRAME_LEN, 1, 0 },
        { "Tier 1 composed", HERUS_SF_MEANING, HERUS_FRAME_LEN, 1, 0 },
        { "Tier 0.5 sketch", HERUS_SF_MEANING, HERUS_SKETCH_FRAME_LEN, 0, 1 },
        { "Tier 2 voice   ", HERUS_SF_VOICE,   178u,            1, 0 },
        { "SOS beacon     ", HERUS_SF_SOS,     82u,             1, 0 },
    };
    printf("  %-16s %5s %4s %10s %s\n", "tier", "bytes", "SF", "airtime", "dwell");
    int over = 0;
    for (unsigned i = 0; i < sizeof rows / sizeof rows[0]; i++) {
        uint32_t t = hz_airtime_us(rows[i].sf, rows[i].pl, rows[i].crc, rows[i].ih, HERUS_CR);
        int bad = t > HZ_DWELL_LIMIT_US;
        over |= bad;
        printf("  %-16s %5u %4u %8.1f ms %s\n", rows[i].n, rows[i].pl, rows[i].sf,
               t / 1000.0, bad ? "OVER" : "ok");
    }
    printf("  P1 constant airtime: %s (Tier 0 %u B and Tier 0.5 %u B both %.1f ms)\n",
           HERUS_AIRTIME_MEANING_US == HERUS_AIRTIME_SKETCH_US
               ? "INVARIANT HOLDS" : "VIOLATED",
           HERUS_FRAME_LEN, HERUS_SKETCH_FRAME_LEN, HERUS_AIRTIME_MEANING_US / 1000.0);
    printf("  P2 dwell           : %s\n", over ? "VIOLATED" : "every frame within 400 ms");
}

/* The two shipping link profiles. See the derivation and the three static
 * asserts in region.h — every figure here is forced by the dwell limit. */
static const hz_link_profile_t LINKS[] = {
    { "Rich",  HERUS_SF_MEANING, (uint8_t)HERUS_FRAME_LEN,       (uint8_t)HCP_PT_RICH,
      8u, 9u, HERUS_AIRTIME_MEANING_US, -259 },
    { "Reach", HERUS_SF_REACH,   (uint8_t)HERUS_REACH_FRAME_LEN, (uint8_t)HERUS_REACH_PT_LEN,
      8u, 4u, HERUS_AIRTIME_REACH_US,   -264 },
};

const hz_link_profile_t *hz_link(hz_link_t p)
{
    return &LINKS[(p == HZ_LINK_REACH) ? 1 : 0];
}
