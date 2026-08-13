#!/usr/bin/env python3
"""interactionlog.py — validate HERUS voice/runtime bench logs without audio.

The product log deliberately stores only scenario labels, configured intent labels,
timestamps, measured energy and terminal outcome. It never needs raw audio,
transcripts, embeddings, identity, location, keys or wire frames.

    python3 tools/interactionlog.py --csv lab.csv --strict

A strict run fails when a row violates state ordering or a product gate fails.
Use the CSV contract in docs/09-VALIDACAO-FISICA.md; do not edit failed rows.
"""
import argparse
import csv
import math
import statistics
import sys

REQUIRED = [
    "run_id", "trial_id", "scenario", "source", "expected", "observed",
    "button_ms", "draft_ms", "confirm_ms", "send_ms", "energy_uj", "outcome",
]
INTENTS = {"arrive", "help", "cancel", "none"}
SOURCES = {"core", "nucleus"}
OUTCOMES = {"sent", "cancelled", "rejected", "timed_out", "source_lost"}


def integer(row, key, errors):
    try:
        value = int(row[key])
        if value < 0:
            raise ValueError
        return value
    except (ValueError, TypeError, KeyError):
        errors.append("%s is not a non-negative integer" % key)
        return 0


def percentile(values, p):
    if not values:
        return None
    values = sorted(values)
    return values[max(0, math.ceil(p * len(values)) - 1)]


def validate_row(row, seen):
    errors = []
    ident = (row.get("run_id", ""), row.get("trial_id", ""))
    if not ident[0] or not ident[1]:
        errors.append("missing run_id or trial_id")
    elif ident in seen:
        errors.append("duplicate run_id/trial_id")
    seen.add(ident)

    if row.get("source") not in SOURCES:
        errors.append("source must be core or nucleus")
    if row.get("expected") not in INTENTS or row.get("observed") not in INTENTS:
        errors.append("expected/observed outside configured intent vocabulary")
    if row.get("outcome") not in OUTCOMES:
        errors.append("unknown outcome")

    button = integer(row, "button_ms", errors)
    draft = integer(row, "draft_ms", errors)
    confirm = integer(row, "confirm_ms", errors)
    sent = integer(row, "send_ms", errors)
    energy = integer(row, "energy_uj", errors)

    if draft and not button:
        errors.append("draft requires a physical button timestamp")
    if draft and draft < button:
        errors.append("draft precedes button")
    if confirm and (not draft or confirm < draft):
        errors.append("confirmation requires an earlier draft")
    if sent and (not confirm or sent < confirm):
        errors.append("send requires an earlier positive confirmation")

    outcome = row.get("outcome")
    expected = row.get("expected")
    observed = row.get("observed")
    if outcome == "sent":
        if not sent or not confirm or not draft:
            errors.append("sent outcome lacks a complete button/draft/confirm/send path")
        if expected in {"none", "cancel"} or observed in {"none", "cancel"}:
            errors.append("sent outcome is not a sendable semantic intent")
    elif sent:
        errors.append("only outcome=sent may carry send_ms")
    if expected == "none" and (draft or observed != "none"):
        errors.append("negative trial created a false draft or intent")
    if observed == "cancel" and outcome != "cancelled":
        errors.append("cancel intent must terminate locally as cancelled")
    if energy == 0 and outcome == "sent":
        errors.append("sent trial requires measured energy_uj")

    return dict(row=row, button=button, draft=draft, confirm=confirm,
                sent=sent, energy=energy, errors=errors)


def read_rows(path):
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        if reader.fieldnames != REQUIRED:
            got = ",".join(reader.fieldnames or [])
            sys.exit("invalid header; expected %s, got %s" % (",".join(REQUIRED), got))
        seen = set()
        rows = [validate_row(r, seen) for r in reader]
    if not rows:
        sys.exit("no data rows in %s" % path)
    return rows


def report(rows, min_accuracy, max_p95, min_positive):
    clean = [r for r in rows if not r["errors"]]
    positives = [r for r in clean if r["row"]["expected"] != "none"]
    negatives = [r for r in clean if r["row"]["expected"] == "none"]
    exact = [r for r in positives if r["row"]["expected"] == r["row"]["observed"]]
    false_drafts = [r for r in negatives if r["draft"] or r["row"]["observed"] != "none"]
    latencies = [r["draft"] - r["button"] for r in clean if r["draft"]]
    confirms = [r for r in clean if r["confirm"]]
    sends = [r for r in clean if r["sent"]]
    energy = [r["energy"] for r in clean if r["energy"]]

    accuracy = len(exact) / len(positives) if positives else 0.0
    handoff = len(sends) / len(confirms) if confirms else 0.0
    p95 = percentile(latencies, 0.95)

    print("HERUS interaction validation report")
    print("rows: %d valid / %d total" % (len(clean), len(rows)))
    print("positive exact intent: %d / %d (%.1f%%)" %
          (len(exact), len(positives), 100 * accuracy))
    print("negative false drafts: %d / %d" % (len(false_drafts), len(negatives)))
    print("button -> draft: p95 %s ms" % (str(p95) if p95 is not None else "n/a"))
    print("confirmed hand-offs: %d sends / %d confirmations (%.3f)" %
          (len(sends), len(confirms), handoff))
    if energy:
        print("measured energy: median %d uJ, mean %.0f uJ across %d sessions" %
              (statistics.median(energy), statistics.mean(energy), len(energy)))
    else:
        print("measured energy: no samples")

    gates = []
    gates.append((len(positives) >= min_positive,
                  "positive sample count >= %d" % min_positive))
    gates.append((accuracy >= min_accuracy,
                  "exact intent accuracy >= %.1f%%" % (100 * min_accuracy)))
    gates.append((not false_drafts, "zero false drafts in negative trials"))
    gates.append((p95 is not None and p95 <= max_p95,
                  "button->draft p95 <= %d ms" % max_p95))
    gates.append((bool(confirms) and handoff == 1.0,
                  "exactly one send hand-off per positive confirmation"))
    gates.append((all(not r["errors"] for r in rows), "all rows obey state/log invariants"))

    print("\ngates")
    for passed, name in gates:
        print("  %s  %s" % ("PASS" if passed else "FAIL", name))
    return all(passed for passed, _ in gates)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", required=True, help="normative interaction CSV")
    ap.add_argument("--strict", action="store_true", help="exit non-zero on failed gate")
    ap.add_argument("--min-accuracy", type=float, default=0.95)
    ap.add_argument("--max-p95-ms", type=int, default=2500)
    ap.add_argument("--min-positive", type=int, default=1)
    args = ap.parse_args()
    if not 0 < args.min_accuracy <= 1 or args.max_p95_ms <= 0 or args.min_positive <= 0:
        sys.exit("invalid gate threshold")
    rows = read_rows(args.csv)
    passed = report(rows, args.min_accuracy, args.max_p95_ms, args.min_positive)
    if args.strict and not passed:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
