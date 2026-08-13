#!/usr/bin/env python3
"""frontier.py — where the wall actually is.  stdlib only.

docs/ says "SF9 is the regulatory ceiling" and then "SF10 at 24 B is the
ceiling". Both were derived at BW 125 kHz, under one of the two regimes FCC
15.247 offers, and a claim that narrow is not a ceiling — it is a habit.

This enumerates EVERY (spreading factor, bandwidth) pair the SX1262 supports,
under BOTH regulatory regimes, and computes for each the best sensitivity
reachable with a payload large enough to carry an authenticated Herus frame.
The output is a Pareto frontier: the set of operating points where nothing else
is better in both sensitivity and payload at once.

The point of the exercise is falsifiability. "We are at the limit" is worth
nothing as an assertion and everything as a table someone can find a hole in.

    python3 tools/frontier.py
"""
import math

# ---------------------------------------------------------------- radio
# Sensitivity is not a lookup table, it is thermal noise plus a fixed receiver
# noise figure plus the demodulator's SNR limit:
#
#     S = -174 dBm/Hz + 10log10(BW) + NF + SNR_limit(SF)
#
# Both constants below are RECOVERED from the SX1262 datasheet's own BW-125
# column rather than assumed, and the recovery is checked at the bottom of this
# file against every published figure. That is what makes the model usable at
# bandwidths the datasheet does not tabulate.
NF_DB = 6.03
def snr_limit(sf):                       # -7.5 dB at SF7, -2.5 dB per step
    return -7.5 - 2.5 * (sf - 7)

def sensitivity(sf, bw_hz):
    return -174.0 + 10.0 * math.log10(bw_hz) + NF_DB + snr_limit(sf)

# Every bandwidth the SX1262 can actually be programmed to, in Hz.
BWS = [7810, 10420, 15630, 20830, 31250, 41670, 62500, 125000, 250000, 500000]
SFS = [7, 8, 9, 10, 11, 12]

def airtime_ms(pl, sf, bw, cr=1, crc=True, ih=False, npre=8):
    ldro = (2 ** sf) / bw > 0.016        # symbol longer than 16 ms
    de = 1 if ldro else 0
    ts = (2 ** sf) / bw
    num = 8 * pl - 4 * sf + 28 + (16 if crc else 0) - (20 if ih else 0)
    den = 4 * (sf - 2 * de)
    npay = 8 + max(math.ceil(num / den) * (cr + 4), 0)
    return ((npre + 4.25) + npay) * ts * 1000.0

# ------------------------------------------------- the axis that decides it
# LoRa tolerates a carrier frequency error of roughly +-25% of the bandwidth
# (Semtech AN1200.13). That is not a footnote at narrow bandwidths — it is the
# constraint that decides which of them exist at all.
#
# Two units each drift by the tolerance of their frequency reference, and the
# error that matters is the RELATIVE one, so it is twice the per-unit figure:
#
#     +-2 ppm TCXO   ->  3.7 kHz relative  ->  needs BW >= 14.6 kHz to be legal
#                                              at all, >= 29.3 kHz for 2x margin
#     +-10 ppm XO    -> 18.3 kHz           ->  BW >= 73 kHz / >= 146 kHz
#     +-20 ppm XO    -> 36.6 kHz           ->  BW >= 146 kHz / >= 293 kHz
#
# The last line is why every shipping LoRa board has a TCXO: with a plain
# crystal, BW 125 kHz — the setting the entire 915 MHz ecosystem uses — is
# already outside tolerance. It is also why the frontier below is not a property
# of the radio alone. It is a property of one line in the bill of materials.
FREQ_HZ    = 915e6
PPM        = 2.0            # TCXO, the reference hardware (LilyGO T3-S3) has one
TOL_FRAC   = 0.25           # receiver tolerance as a fraction of bandwidth
MARGIN     = 2.0            # how much of that tolerance we refuse to spend

def offset_hz(ppm=PPM):
    return 2.0 * ppm * 1e-6 * FREQ_HZ          # relative, both ends worst case

def bw_ok(bw, ppm=PPM, margin=MARGIN):
    return bw * TOL_FRAC >= offset_hz(ppm) * margin

def freq_budget_used(bw, ppm=PPM):
    return offset_hz(ppm) / (bw * TOL_FRAC)

# ---------------------------------------------------------------- regulation
DWELL_MS = 400.0            # 15.247(a)(1)(iii) and ANATEL 14448, per hop
PATIENCE_MS = 2000.0        # nobody waits longer than this for one glyph

def legal(sf, bw, pl, regime):
    """Is one frame of this shape legal under this regime?"""
    t = airtime_ms(pl, sf, bw)
    if regime == "FHSS":
        # Frequency hopping: any bandwidth, but a hard per-hop dwell limit.
        # 15.247(a)(1)(iii) also wants >= 50 channels below 250 kHz occupied
        # bandwidth, which 902-928 MHz affords at every BW in the table.
        return t <= DWELL_MS
    if regime == "DTS":
        # Digital transmission system, 15.247(b)(3): no dwell limit at all, but
        # the 6 dB bandwidth must be at least 500 kHz. LoRa's 6 dB bandwidth is
        # essentially its configured bandwidth, so only BW 500 qualifies.
        return bw >= 500000
    if regime == "EU868":
        # ETSI EN 300 220: no dwell rule, a 1% duty cycle instead. One frame is
        # always legal, so the binding constraint is human: a frame nobody will
        # wait for is not a frame. PATIENCE_MS, stated rather than smuggled in.
        return t <= PATIENCE_MS
    return False

# A Herus frame must carry, at minimum, 2 bytes of address, an 8-byte tag, and
# enough plaintext to be a message at all: 6 fixed bytes plus one 2-byte slot.
MIN_FRAME = 2 + 8 + 6 + 2       # 18 bytes

def best_payload(sf, bw, regime):
    best = 0
    for pl in range(1, 256):
        if legal(sf, bw, pl, regime):
            best = pl
    return best

def survey(regime, ppm=PPM, margin=MARGIN):
    rows = []
    for sf in SFS:
        for bw in BWS:
            if not bw_ok(bw, ppm, margin):
                continue
            pl = best_payload(sf, bw, regime)
            if pl < MIN_FRAME:
                continue
            rows.append({
                "sf": sf, "bw": bw, "pl": pl,
                "sens": sensitivity(sf, bw),
                "air": airtime_ms(pl, sf, bw),
                "slots": (pl - 2 - 8 - 6) // 2,      # addr, tag, fixed header
                "fbudget": freq_budget_used(bw),
            })
    return rows

def pareto(rows):
    """Points nothing else beats on BOTH sensitivity and payload."""
    out = []
    for a in rows:
        dominated = any(
            b is not a and b["sens"] <= a["sens"] and b["pl"] >= a["pl"]
            and (b["sens"] < a["sens"] or b["pl"] > a["pl"])
            for b in rows)
        if not dominated:
            out.append(a)
    return sorted(out, key=lambda r: r["sens"])

def rng(gain_db):
    """Range multiplier for a link-budget gain, in the d^4 regime a wrist
    device lives in past its 38 m two-ray breakpoint."""
    return 10 ** (gain_db / 40.0)

def main():
    # -- the model must reproduce the datasheet before it is used anywhere else
    published = {7: -124.5, 8: -127.0, 9: -129.5, 10: -132.0, 11: -134.5, 12: -137.0}
    print("model check against the SX1262 datasheet, BW 125 kHz")
    worst = 0.0
    for sf, s in published.items():
        got = sensitivity(sf, 125000)
        worst = max(worst, abs(got - s))
        print(f"  SF{sf:<3} datasheet {s:7.1f}   model {got:7.2f}   error {got-s:+.2f} dB")
    print(f"  worst error {worst:.2f} dB — the model is the datasheet, extended\n")
    assert worst < 0.1, "the sensitivity model does not reproduce the datasheet"

    base = sensitivity(9, 125000)     # Rich, the profile we started from

    for regime, note in (("FHSS", "hopping, 400 ms dwell per hop (BR/US)"),
                         ("DTS",  "digital modulation, no dwell, BW >= 500 kHz (US)"),
                         ("EU868","1% duty cycle, no dwell (EU)")):
        rows = survey(regime)
        front = pareto(rows)
        print(f"=== {regime} — {note} ===")
        if not front:
            print("  no legal operating point carries a Herus frame\n")
            continue
        print("   SF   BW kHz   max payload   airtime     sens      vs SF9/125   range x")
        for r in front:
            g = base - r["sens"]
            print(f"   {r['sf']:2d}   {r['bw']/1000:6.1f}   {r['pl']:9d} B   {r['air']:7.1f} ms  "
                  f"{r['sens']:7.2f}   {g:+8.2f} dB   {rng(g):5.3f}")
        print()

    # -- the decisive comparison
    print("=== the decision ===")
    fh = pareto(survey("FHSS"))
    usable = [r for r in fh if r["pl"] >= 24]
    bestu = min(usable, key=lambda r: r["sens"]) if usable else None
    besta = min(fh, key=lambda r: r["sens"])
    print(f"  best sensitivity at >= 24 B payload : SF{bestu['sf']} / BW {bestu['bw']/1000:.1f} kHz"
          f" -> {bestu['sens']:.2f} dBm" if bestu else "  none")
    print(f"  best sensitivity at any payload     : SF{besta['sf']} / BW {besta['bw']/1000:.1f} kHz"
          f" -> {besta['sens']:.2f} dBm at {besta['pl']} B")
    d = bestu["sens"] - besta["sens"]
    print(f"  the whole remaining headroom under a dwell rule is {d:.2f} dB,")
    print(f"  which is {100*(rng(d)-1):.1f}% of range, and it costs "
          f"{bestu['pl'] - besta['pl']} bytes of frame.")
    eu = pareto(survey("EU868"))
    bestE = min(eu, key=lambda r: r["sens"])
    gE = base - bestE["sens"]
    print(f"\n  Where there is no dwell rule the wall moves: EU868 allows "
          f"SF{bestE['sf']} / BW {bestE['bw']/1000:.0f} kHz")
    print(f"  at {bestE['sens']:.2f} dBm — {gE:+.1f} dB on Rich, x{rng(gE):.3f} range, "
          f"{bestE['air']/1000:.2f} s per frame.")
    duty = 0.01
    print(f"  The 1% duty cycle then allows {duty*3600/(bestE['air']/1000):.0f} frames/hour,")
    print("  so the EU ceiling is higher and much slower. It is a different device.")

if __name__ == "__main__":
    main()
