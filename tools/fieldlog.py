#!/usr/bin/env python3
"""fieldlog.py — turn a Phase 0 walk into a verdict.  stdlib only.

Reads the CSV that firmware/ranger/ranger.ino prints on the RX board, maps
elapsed time to distance using the marks you noted on the walk, and evaluates
the two Phase 0 kill criteria numerically so the decision is not a judgement
call made while tired.

    # pass 1, validated frames
    python3 tools/fieldlog.py --rx walk_pdr.csv \\
        --marks 0:0,45:50,95:100,150:150,215:200,290:300,380:400

    # pass 2, header-less probe frames, same route and same marks
    python3 tools/fieldlog.py --rx walk_probe.csv --marks <same> --mode probe

    # both together: computes the Tier 0.5 gain in dB
    python3 tools/fieldlog.py --rx walk_pdr.csv --probe walk_probe.csv \\
        --marks <same>

Capture the CSV with any serial terminal that can log to a file, e.g.
    python3 -m serial.tools.miniterm /dev/cu.usbmodem* 115200 | tee walk_pdr.csv
"""
import argparse
import math
import sys

PERIOD_MS_DEFAULT = 2000

# Phase 0 kill criteria, from docs/03-BUILD-GUIDE.md
KILL_RANGE_M = 150.0        # urban range at >= PDR_FLOOR
PDR_FLOOR = 0.50
KILL_TIER05_DB = 2.0        # below this, Tier 0.5 is not worth its complexity


def parse_marks(s):
    """'0:0,45:50,95:100' -> [(seconds, metres), ...] sorted by time."""
    out = []
    for part in s.split(","):
        part = part.strip()
        if not part:
            continue
        t, d = part.split(":")
        out.append((float(t), float(d)))
    out.sort()
    if len(out) < 2:
        sys.exit("need at least two marks, e.g. --marks 0:0,60:50")
    return out


def read_rx(path):
    """Return (rows, t0) where rows = [(t_s, seq, rssi, snr, biterr, ok)]."""
    rows = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#") or line.startswith("t_ms"):
                continue
            parts = line.split(",")
            if len(parts) < 6:
                continue
            try:
                t_ms = float(parts[0])
            except ValueError:
                continue
            ok = parts[5].strip() == "ok"
            def num(x, d=float("nan")):
                try:
                    return float(x)
                except ValueError:
                    return d
            rows.append((t_ms / 1000.0, num(parts[1], -1), num(parts[2]),
                         num(parts[3]), num(parts[4], 0), ok))
    if not rows:
        sys.exit("no data rows in %s" % path)
    t0 = min(r[0] for r in rows)
    return [(r[0] - t0,) + r[1:] for r in rows]


def bin_by_distance(rows, marks, period_ms, payload_bits):
    """One row per distance segment."""
    out = []
    for i in range(len(marks) - 1):
        (ta, da), (tb, db) = marks[i], marks[i + 1]
        span = tb - ta
        if span <= 0:
            continue
        expected = span * 1000.0 / period_ms
        seg = [r for r in rows if ta <= r[0] < tb]
        got = [r for r in seg if r[5]]
        pdr = len(got) / expected if expected else 0.0
        rssi = sum(r[2] for r in got) / len(got) if got else float("nan")
        snr = sum(r[3] for r in got) / len(got) if got else float("nan")
        biterr = sum(r[4] for r in got) / len(got) if got else float("nan")
        ber = biterr / payload_bits if got else float("nan")
        out.append(dict(d_lo=da, d_hi=db, expected=expected, got=len(got),
                        pdr=min(pdr, 1.0), rssi=rssi, snr=snr, ber=ber))
    return out


def max_range_at(bins, floor):
    """Farthest segment upper edge still meeting the PDR floor, with no
    better segment beyond it. Uses the last qualifying segment, not the first
    failure, so one shadowed bin does not truncate the result."""
    best = 0.0
    for b in bins:
        if b["pdr"] >= floor:
            best = b["d_hi"]
    return best


def report(bins, label, payload_bits, show_ber):
    print("\n%s" % label)
    print("-" * len(label))
    hdr = "  segment        exp   got    PDR    RSSI    SNR"
    if show_ber:
        hdr += "     BER"
    print(hdr)
    for b in bins:
        line = ("  %5.0f-%-5.0f m %5.0f %5d %6.1f%% %7.1f %6.2f"
                % (b["d_lo"], b["d_hi"], b["expected"], b["got"],
                   100 * b["pdr"], b["rssi"], b["snr"]))
        if show_ber:
            line += ("  %5.2f%%" % (100 * b["ber"])) if not math.isnan(b["ber"]) else "      -"
        print(line)
    r = max_range_at(bins, PDR_FLOOR)
    print("  -> max range at >= %.0f%% PDR: %.0f m" % (100 * PDR_FLOOR, r))
    return r


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--rx", required=True, help="CSV from the RX board (PDR pass)")
    ap.add_argument("--probe", help="CSV from the MODE_PROBE pass, same route")
    ap.add_argument("--marks", required=True, help="seconds:metres,...")
    ap.add_argument("--period-ms", type=int, default=PERIOD_MS_DEFAULT)
    ap.add_argument("--mode", choices=("pdr", "probe"), default="pdr")
    ap.add_argument("--frame-len", type=int, default=34)
    a = ap.parse_args()

    marks = parse_marks(a.marks)
    payload_bits = (a.frame_len - 2) * 8

    print("HERUS Phase 0 field report")
    print("route: %.0f m over %.0f s, %d marks, %.1f s cadence"
          % (marks[-1][1], marks[-1][0], len(marks), a.period_ms / 1000.0))

    rows = read_rx(a.rx)
    bins = bin_by_distance(rows, marks, a.period_ms, payload_bits)
    r_pdr = report(bins, "Pass 1 — validated frames (explicit header, CRC on)",
                   payload_bits, a.mode == "probe")

    print("\nQ1 — is the wrist viable?")
    print("-" * 24)
    if r_pdr >= KILL_RANGE_M:
        print("  %.0f m >= %.0f m  ->  PASS. The wrist form factor survives."
              % (r_pdr, KILL_RANGE_M))
        print("  Proceed to Phase 1. Record this number; it is the baseline every")
        print("  later antenna change is measured against.")
    else:
        print("  %.0f m < %.0f m  ->  KILL. The wrist is the wrong place for a"
              % (r_pdr, KILL_RANGE_M))
        print("  sub-GHz radio. This is not a firmware problem and no amount of")
        print("  protocol work fixes it. Pivot the Core to a lapel or backpack")
        print("  clip: better height, no body block. The modular capsule was")
        print("  always the hedge for exactly this outcome.")

    if a.probe:
        prows = read_rx(a.probe)
        pbins = bin_by_distance(prows, marks, a.period_ms, payload_bits)
        r_probe = report(pbins, "Pass 2 — probe frames (implicit header, CRC off)",
                         payload_bits, True)

        print("\nQ2 — how big is the Tier 0.5 prize?")
        print("-" * 36)
        if r_pdr <= 0 or r_probe <= 0:
            print("  insufficient data on one of the passes")
            return
        # Beyond the two-ray breakpoint path loss goes as d^4, so an extra
        # factor in distance is 40*log10 of that factor in dB.
        gain = 40.0 * math.log10(r_probe / r_pdr)
        print("  validated frames reached   %.0f m" % r_pdr)
        print("  probe frames reached       %.0f m" % r_probe)
        print("  distance ratio             %.2fx" % (r_probe / r_pdr))
        print("  equivalent gain (d^4)      %.1f dB" % gain)
        if gain >= KILL_TIER05_DB:
            print("\n  %.1f dB >= %.1f dB  ->  KEEP Tier 0.5, for broadcast only."
                  % (gain, KILL_TIER05_DB))
            print("  Remember the caveat from 02-PROTOCOL.md §3.5: for addressed")
            print("  traffic, sending the frame twice costs the same airtime and")
            print("  buys ~3 dB of selection diversity. Tier 0.5 earns its place")
            print("  where retransmission has nobody to negotiate with.")
        else:
            print("\n  %.1f dB < %.1f dB  ->  DROP Tier 0.5. Keep CRC'd ids."
                  % (gain, KILL_TIER05_DB))
            print("  No loss of function, only of novelty. The algebra's 100%%-at-")
            print("  25%%-BER result was never the system's number — preamble")
            print("  detection is, and this walk just measured it.")

        finite = [b["ber"] for b in pbins if not math.isnan(b["ber"])]
        if finite:
            print("\n  measured raw BER on detected probe frames: %.2f%% .. %.2f%%"
                  % (100 * min(finite), 100 * max(finite)))
            print("  Compare against test_algebra T8: the dense sketch decodes a")
            print("  512-symbol lexicon at 100% up to 25% BER. If the field BER")
            print("  stays under that, the algebra is not the limiting factor.")


if __name__ == "__main__":
    main()
