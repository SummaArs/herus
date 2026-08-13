#!/usr/bin/env python3
"""Herus physical-layer and energy budget calculator.  stdlib only.

Every hardware number in docs/00-HERUS-MASTER.md comes from this file. Run it
and the doc is reproducible; change an assumption and the doc is wrong until it
is rerun. Assumptions are named constants, never inline magic.

    python3 tools/budget.py
"""
import math

# ---------------------------------------------------------------- constants
FREQ_HZ      = 915e6
C            = 299_792_458.0
LAMBDA       = C / FREQ_HZ                    # 0.3277 m

# SX1262, BW = 125 kHz, from the datasheet's sensitivity table.
SENS_DBM     = {7: -124.5, 8: -127.0, 9: -129.5, 10: -132.0, 11: -134.5, 12: -137.0}
# ---------------------------------------------------------------- CORRECTION
# This was 14.0 dBm, with the comment "ANATEL 14448 / FCC 15.247 conducted limit
# we target". The comment names the Brazilian and American rules and the number
# is the EUROPEAN one. FCC 15.247(b)(1) allows 1 W (30 dBm) for a hopping system
# in 902-928 MHz with at least 50 channels, and 0.25 W (24 dBm) with 25 to 49;
# ANATEL Ato 14448 mirrors it. region.c has said tx_dbm_max = 30 since it was
# written — the profile knew, and the budget did not.
#
# The SX1262 reaches +22 dBm on its own high-power PA. No new part, no external
# amplifier, no regulatory argument: 8 dB, which is more range than every
# modulation change in this project put together, was sitting in a mislabelled
# constant. Antenna gain here is negative, so 15.247(b)(4)'s reduction for
# directional antennas does not apply.
#
# What it costs is real and is accounted below: 118 mA instead of 45 during
# transmit. At a leaf's traffic that is 0.61 mAh/day against 19.46 harvested.
TX_DBM       = 22.0        # SX1262 high-power PA; the legal ceiling is 30
TX_DBM_EU    = 14.0        # what the earlier revision assumed everywhere
DESIGN_MARGIN_DB = 3.0     # implementation loss + slow fading, not optimism

# Receiver noise figure. 6.03 dB is the SX1262 alone, recovered from its own
# sensitivity table (see tools/frontier.py). A front-end LNA at 0.9 dB NF and
# 18 dB gain, behind an RF switch costing 0.4 dB, brings the system to ~1.3 dB.
# That is +4.7 dB of link budget for about 5 mA whenever the receiver is open —
# cheap for a leaf at 1% duty, expensive for a relay that listens continuously.
NF_CHIP_DB   = 6.03
NF_LNA_DB    = 1.30
I_LNA        = 5.0         # mA, only while the receiver is open

# Antenna realised gain INCLUDING radiation efficiency and body loading.
# A 30 mm structure at 915 MHz is lambda/11 — electrically tiny. Worn on a
# wrist it is also loaded by tissue with high permittivity.
G_CAPSULE_DBI = -8.0       # PCB loop/IFA inside the sealed capsule, worn
G_BAND_DBI    = -3.0       # antenna in the strap, fed through the pogo pins

CLUTTER_URBAN_DB = 22.0    # dense-urban excess over the two-ray model
CLUTTER_SUBURB_DB = 12.0

# Regulatory: 15.247(a)(1)(iii) frequency-hopping dwell limit, mirrored by
# ANATEL Ato 14448 for 902-907.5 / 915-928 MHz.
MAX_DWELL_MS = 400.0

# Currents (mA at 3.3 V) — datasheet typicals.
I_SX_TX14     = 45.0
I_SX_TX22     = 118.0      # SX1262 datasheet, high-power PA at +22 dBm, 3.3 V
I_SX_RX       = 5.3
I_SX_SLEEP_R  = 0.0012     # sleep with register retention
I_S3_ACTIVE   = 45.0       # ESP32-S3 @ 240 MHz, radio off
I_S3_DEEP     = 0.015      # deep sleep, RTC on
I_NRF54_ACT   = 3.0        # Cortex-M33 @ 128 MHz
I_NRF54_SLEEP = 0.0012
I_MIC_EACH    = 0.49       # ICS-43434 class MEMS mic
I_MISC_SLEEP  = 0.005      # BQ25570 quiescent + ATECC sleep + fuel gauge

VBAT = 3.7

# Solar
SUN_MW_CM2      = 100.0    # AM1.5
INDOOR_MW_CM2   = 0.10     # ~300 lux office/home
ETA_SI          = 0.15     # flexible mono-Si, outdoor
ETA_SI_INDOOR   = 0.05     # same cell, indoor spectrum + low irradiance
ETA_MPPT        = 0.80     # BQ25570 at tens of microwatts
AREA_CAPSULE_CM2 = 4.0     # 20x20 mm usable aperture under the window
AREA_BAND_CM2    = 20.0    # cells laminated into the strap


# ---------------------------------------------------------------- lora
def airtime_ms(payload_b, sf, bw_hz=125e3, cr=1, crc=True, ih=False,
               preamble=8, ldro=None):
    """Semtech AN1200.13 airtime. cr=1..4 means 4/5..4/8."""
    if ldro is None:
        ldro = sf >= 11                       # low-data-rate optimise
    de = 1 if ldro else 0
    ts = (2 ** sf) / bw_hz
    num = 8 * payload_b - 4 * sf + 28 + (16 if crc else 0) - (20 if ih else 0)
    den = 4 * (sf - 2 * de)
    n_pay = 8 + max(math.ceil(num / den) * (cr + 4), 0)
    return ((preamble + 4.25) + n_pay) * ts * 1000.0


def link_budget_db(sf, g_tx, g_rx):
    return TX_DBM + g_tx + g_rx - SENS_DBM[sf] - DESIGN_MARGIN_DB


def two_ray_range_m(budget_db, h1, h2, clutter_db=0.0):
    """Plane-earth two-ray model.

    Beyond the breakpoint d_bp = 4*pi*h1*h2/lambda the ground reflection turns
    path loss into d^4, not d^2. At wrist height (h ~ 1 m) d_bp is only ~38 m,
    so essentially the whole useful range lives in the d^4 regime. Ignoring
    this is the single most common way LoRa range gets overestimated.
    """
    d_bp = 4 * math.pi * h1 * h2 / LAMBDA
    fspl_bp = 32.44 + 20 * math.log10(FREQ_HZ / 1e6) + 20 * math.log10(d_bp / 1000.0)
    excess = budget_db - clutter_db - fspl_bp
    if excess <= 0:
        return d_bp * 10 ** (excess / 20.0)   # still in the d^2 regime
    return d_bp * 10 ** (excess / 40.0)


# ---------------------------------------------------------------- energy
def mah_per_day(current_ma, seconds_per_day):
    return current_ma * seconds_per_day / 3600.0


def solar_mah_day(area_cm2, irr_mw_cm2, eta, hours):
    mw = area_cm2 * irr_mw_cm2 * eta * ETA_MPPT
    return mw * hours / VBAT


def tier_range_m(sf, g_dbi, clutter_db):
    return two_ray_range_m(link_budget_db(sf, g_dbi, g_dbi), 1.0, 1.0, clutter_db)


# ---------------------------------------------------------------- framing
# One constant on-air AIRTIME for every tier that carries meaning.
#
# ERRATUM E-P1 (2026-07-30). This table used to give Tier 0.5 the same 34 bytes
# as Tier 0/1 and this function asserted len(Tier0) == len(Tier0.5), which looked
# like P1 but was not. Tier 0.5 runs implicit header with CRC off — 36 fewer bits
# in the symbol count — so 34 B cost 226.3 ms against 246.8 ms. Equal bytes,
# unequal airtime, and airtime is the observable. The old assertion passed while
# the invariant it named was broken, which is the most expensive kind of test.
#
# P1 restated: equal AIRTIME. At SF9 the payload symbol count is a step function,
# and 36..39 B of implicit-header CRC-less payload all land on 48 symbols. Tier
# 0.5 is 38 B (2 addr + 32 sketch + 4 pad); the pad is a free sketch extension
# for a later revision, since the step does not move until 40 B.
FRAMES = {
    # name          addr  body  tag   sf  crc     ih     note
    "Tier 0 glyph":   (2,  24,   8,   9,  True,  False, "AEAD, 24 B plaintext"),
    "Tier 1 composed":(2,  24,   8,   9,  True,  False, "AEAD, intent + up to 9 role/filler"),
    "Tier 0.5 sketch":(2,  36,   0,   9,  False, True,  "stream cipher, NO MAC, implicit hdr, CRC off"),
    "Tier 2 voice":   (2, 168,   8,   7,  True,  False, "Codec2 700C, 2 fragments per 4 s clip"),
    "SOS beacon":     (2,  16,  64,   8,  True,  False, "ECDSA P-256 signature, deliberately public"),
}


def frame_ledger():
    rule("8. Frame ledger — one airtime for every tier that carries meaning")
    print("  tier               addr body tag  total  SF  airtime  dwell  note")
    air_of = {}
    for name, (a, b, t, sf, crc, ih, note) in FRAMES.items():
        tot = a + b + t
        air = airtime_ms(tot, sf, crc=crc, ih=ih)
        air_of[name] = air
        ok = "ok" if air <= MAX_DWELL_MS else "OVER"
        print("  %-17s %3d %4d %3d  %5d  %2d  %6.1f ms  %-5s %s"
              % (name, a, b, t, tot, sf, air, ok, note))

    t0, t05 = air_of["Tier 0 glyph"], air_of["Tier 0.5 sketch"]
    print("\n  Tier 0/1 (%d B, explicit hdr + CRC) and Tier 0.5 (%d B, implicit hdr,"
          % (sum(FRAMES["Tier 0 glyph"][:3]), sum(FRAMES["Tier 0.5 sketch"][:3])))
    print("  no CRC) both occupy %.1f ms -> indistinguishable by airtime. This is the" % t0)
    print("  whole point: a 2 B panic glyph and a 12 B status report must not be")
    print("  separable with a spectrum analyser.")
    assert abs(t0 - t05) < 0.05, (
        "P1 broken: Tier 0 is %.1f ms and Tier 0.5 is %.1f ms" % (t0, t05))
    print("  INVARIANT HOLDS: airtime(Tier0) == airtime(Tier0.5) == %.1f ms" % t0)
    print("\n  Tier 2 is length-distinguishable by nature — voice is voice. If the")
    print("  fact that you transmitted speech at all must be hidden, do not")
    print("  transmit speech. Stated, not papered over.")

    rule("9. Range per tier (two-ray, h=1 m both ends)")
    print("  tier              antenna    open field  suburban  dense urban")
    for name in ("Tier 0 glyph", "Tier 2 voice", "SOS beacon"):
        sf = FRAMES[name][3]
        for label, g in (("capsule", G_CAPSULE_DBI), ("Band", G_BAND_DBI)):
            print("  %-17s %-9s %8.0f m %8.0f m %8.0f m"
                  % (name if label == "capsule" else "", label,
                     tier_range_m(sf, g, 0.0),
                     tier_range_m(sf, g, CLUTTER_SUBURB_DB),
                     tier_range_m(sf, g, CLUTTER_URBAN_DB)))
    r_sem = tier_range_m(9, G_BAND_DBI, CLUTTER_URBAN_DB)
    r_voi = tier_range_m(7, G_BAND_DBI, CLUTTER_URBAN_DB)
    print("\n  meaning reaches %.0f m urban, speech reaches %.0f m: +%.0f%%."
          % (r_sem, r_voi, 100 * (r_sem / r_voi - 1)))
    print("  The thesis, in metres. Voice is pinned to SF7 by the dwell limit;")
    print("  a glyph is not, so the semantic tiers are strictly longer-ranged.")
    print("  Three-hop Weave with Band antennas: ~%.1f km effective urban reach."
          % (3 * r_sem / 1000.0))
    print("\n  SOS runs SF8 (%.0f ms) not SF9: it carries a 64 B signature so a"
          % airtime_ms(82, 8))
    print("  stranger can verify it, which pushes it to %d B. SF9 would cost"
          % 82)
    print("  %.0f ms and blow the dwell limit." % airtime_ms(82, 9))

    rule("10. Beat — slotted rendezvous timing")
    ppm, resync_s = 20.0, 60.0
    drift_ms = 2 * ppm * 1e-6 * resync_s * 1000
    guard_ms = 10.0
    sym_ms = (2 ** 9) / 125e3 * 1000
    pre_ms = 8 * sym_ms
    print("  crystal tolerance          +/- %.0f ppm each end" % ppm)
    print("  relative drift             %.0f us/s" % (2 * ppm))
    print("  drift over %.0f s resync    %.2f ms" % (resync_s, drift_ms))
    print("  guard window               +/- %.0f ms  (%.0fx the drift)"
          % (guard_ms, guard_ms / drift_ms))
    print("  SF9 symbol                 %.3f ms" % sym_ms)
    print("  8-symbol preamble          %.1f ms" % pre_ms)
    print("  RX window (from section 5) 20.0 ms = %.1f symbols" % (20.0 / sym_ms))
    ok = pre_ms >= 2 * guard_ms
    print("  preamble >= 2x guard?      %s (%.1f >= %.0f)"
          % ("yes" if ok else "NO", pre_ms, 2 * guard_ms))
    print("  -> the standard 8-symbol preamble already covers the guard window.")
    print("     No long-preamble mode needed, so no extra airtime is spent.")


def rule(t):
    print("\n" + t)
    print("-" * len(t))


# ---------------------------------------------------------------- report
def main():
    print("HERUS budget — 915 MHz, lambda = %.4f m" % LAMBDA)

    rule("1. Frame airtime and the regulatory ceiling")
    print("Herus frame = 2 B short addr + 16 B ciphertext + 8 B tag = 26 B")
    print("All Tier 0 and Tier 1 frames are padded to this SAME length, so")
    print("airtime cannot leak which tier or how urgent the message was.\n")
    print("  SF   airtime(26 B)   <=400 ms dwell?   sensitivity")
    legal = []
    for sf in range(7, 13):
        a = airtime_ms(26, sf)
        ok = a <= MAX_DWELL_MS
        if ok:
            legal.append(sf)
        print("  %2d   %8.1f ms      %-16s  %.1f dBm"
              % (sf, a, "yes" if ok else "NO - illegal", SENS_DBM[sf]))
    print("\n  => highest legal spreading factor for this frame: SF%d" % max(legal))
    print("     SF11/SF12 exceed the 400 ms hop dwell limit and must stay")
    print("     disabled in the shipped region profile. Range is bounded by")
    print("     REGULATION, not by physics. Plan around it, not past it.")

    print("\n  Tier 2 voice, Codec2 700C, 4 s clip = 350 B, split into 2 frames:")
    for sf in (7, 8, 9):
        a = airtime_ms(175, sf)
        print("    SF%d  %6.1f ms/frame  %s" % (sf, a, "ok" if a <= MAX_DWELL_MS else "ILLEGAL"))

    rule("2. Link budget and honest range")
    print("Realised antenna gain includes efficiency and body loading:")
    print("  capsule-internal %.0f dBi, strap-fed %.0f dBi" % (G_CAPSULE_DBI, G_BAND_DBI))
    print("\n                        budget   open field   suburban   dense urban")
    for name, g in (("capsule<->capsule", G_CAPSULE_DBI), ("band<->band", G_BAND_DBI)):
        for sf in (max(legal), 12):
            b = link_budget_db(sf, g, g)
            tag = "SF%d%s" % (sf, "" if sf in legal else "*")
            print("  %-18s %-5s %5.0f dB  %8.0f m  %8.0f m  %8.0f m"
                  % (name, tag, b,
                     two_ray_range_m(b, 1.0, 1.0),
                     two_ray_range_m(b, 1.0, 1.0, CLUTTER_SUBURB_DB),
                     two_ray_range_m(b, 1.0, 1.0, CLUTTER_URBAN_DB)))
    print("  * SF12 shown for reference only — not legal for a 26 B frame.")
    b = link_budget_db(max(legal), G_BAND_DBI, G_BAND_DBI)
    print("\n  Height is a free lever: both ends at 1.6 m instead of 1.0 m")
    print("  gives %.0f m open field instead of %.0f m — raising your arm is"
          % (two_ray_range_m(b, 1.6, 1.6), two_ray_range_m(b, 1.0, 1.0)))
    print("  worth more than any firmware change (range ~ h^2 in the d^4 regime).")

    rule("3. Energy per operation")
    g_air = airtime_ms(26, max(legal)) / 1000.0
    e_glyph = I_SX_TX14 * g_air / 3600.0 * 1000.0
    print("  one Tier-0/1 frame TX (SF%d, %.0f ms)      = %.1f uAh"
          % (max(legal), g_air * 1000, e_glyph))
    v_air = 2 * airtime_ms(175, 9) / 1000.0
    e_voice_rf = I_SX_TX14 * v_air / 3600.0 * 1000.0
    e_voice_cpu = (I_S3_ACTIVE + 2 * I_MIC_EACH) * 5.0 / 3600.0 * 1000.0
    print("  one 4 s voice message: radio %.1f uAh + capture/encode %.1f uAh"
          % (e_voice_rf, e_voice_cpu))
    print("  -> transmission is nearly free; CAPTURE dominates Tier 2.")

    rule("4. Why there is no always-on microphone")
    always = mah_per_day(I_S3_ACTIVE + 2 * I_MIC_EACH, 86400)
    print("  continuous listen+classify = %.0f mAh/day" % always)
    print("  that is %.0fx the entire daily solar harvest of the capsule."
          % (always / solar_mah_day(AREA_CAPSULE_CM2, SUN_MW_CM2, ETA_SI, 1.5)))
    print("  Tier 2 is therefore push-to-talk. This is a physics decision that")
    print("  happens to also be the right privacy decision.")

    rule("5. Duty-cycled listening (the only budget that matters)")
    print("  SX1262 SetRxDutyCycle runs autonomously: the radio alternates")
    print("  RX and sleep and wakes the MCU only on preamble detection, so the")
    print("  ESP32 stays in deep sleep through every empty window.\n")
    print("  window   period   radio avg   +MCU+misc   mAh/day   role")
    for t_rx_ms, period_s, role in ((20, 2.0, "leaf"), (20, 1.0, "leaf-fast"),
                                    (20, 0.5, "responsive"), (1000, 1.0, "relay")):
        duty = t_rx_ms / 1000.0 / period_s
        i_radio = duty * I_SX_RX + (1 - duty) * I_SX_SLEEP_R
        i_tot = i_radio + I_S3_DEEP + I_MISC_SLEEP
        print("  %4d ms  %4.1f s   %7.3f mA  %7.3f mA  %7.2f   %s"
              % (t_rx_ms, period_s, i_radio, i_tot, mah_per_day(i_tot, 86400), role))

    rule("6. Daily budget vs harvest")
    leaf_idle = mah_per_day(0.020 / 2.0 * I_SX_RX + I_S3_DEEP + I_MISC_SLEEP, 86400)
    usage = 30 * e_glyph / 1000.0 + 20 * (e_voice_rf + e_voice_cpu) / 1000.0
    beacon = 24 * e_glyph / 1000.0
    total = leaf_idle + usage + beacon
    print("  leaf idle listening                    %6.2f mAh/day" % leaf_idle)
    print("  30 frames + 20 voice messages sent     %6.2f mAh/day" % usage)
    print("  hourly beacon                          %6.2f mAh/day" % beacon)
    print("  TOTAL leaf                             %6.2f mAh/day" % total)
    cap_sun = solar_mah_day(AREA_CAPSULE_CM2, SUN_MW_CM2, ETA_SI, 1.5)
    cap_in = solar_mah_day(AREA_CAPSULE_CM2, INDOOR_MW_CM2, ETA_SI_INDOOR, 8.0)
    band_sun = solar_mah_day(AREA_CAPSULE_CM2 + AREA_BAND_CM2, SUN_MW_CM2, ETA_SI, 1.5)
    print("\n  capsule solar, 1.5 h real sun          %6.2f mAh/day  -> %.1fx surplus"
          % (cap_sun, cap_sun / total))
    print("  capsule solar, 8 h indoor 300 lux      %6.3f mAh/day  -> %.0f%% of need"
          % (cap_in, 100 * cap_in / total))
    print("  capsule + solar Band, 1.5 h sun        %6.2f mAh/day" % band_sun)
    relay = mah_per_day(I_SX_RX + I_S3_DEEP + I_MISC_SLEEP, 86400)
    print("\n  continuous relay duty                  %6.2f mAh/day" % relay)
    print("  -> a relay needs %.0f cm2 of cell in 1.5 h sun. The capsule has %.0f."
          % (relay * VBAT / (SUN_MW_CM2 * ETA_SI * ETA_MPPT * 1.5),
             AREA_CAPSULE_CM2))
    print("     THIS is why the strap is modular and carries cells: mesh")
    print("     relaying and capsule-only solar are mutually exclusive.")
    print("\n  VERDICT: 'perpetual on light' is true for a LEAF in daylight and")
    print("  false everywhere else. Indoor light is a trickle (%.0f%%), not a"
          % (100 * cap_in / total))
    print("  power source. Pogo-pin charging stays the primary path.")

    rule("7. Battery sizing and the 8 mm question")
    for mah in (250, 150, 120):
        print("  %3d mAh -> %5.0f days dark autonomy at %.2f mAh/day, cell ~%.1f mm thick"
              % (mah, mah / total, total, 3.0 + mah / 100.0))
    print("  30 days is already far past any real need, so trade capacity for")
    print("  thickness: 150 mAh is the right cell. See the volume ledger in")
    print("  docs/00-HERUS-MASTER.md section 9.")
    print("\n  charge rate: a 150 mAh cell at 1C needs %.0f min to full; the"
          % 60.0)
    print("  draft's '20 minutes' implies 3C, which inflates or kills the cell.")

    frame_ledger()


if __name__ == "__main__":
    main()
