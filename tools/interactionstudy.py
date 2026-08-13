#!/usr/bin/env python3
"""interactionstudy.py — preregistered HERUS-A4-001 confirmatory analysis.

It evaluates only a complete data set that matches the frozen trial plan. No audio
or transcript is read. This program reports a product decision, not a claim about
general speech recognition beyond the declared study conditions.

    python3 tools/interactionstudy.py --manifest research/interaction_study_manifest.json \
        --plan research/generated/HERUS-A4-001-plan.csv --csv lab.csv --strict
"""
import argparse
import csv
import json
import math
import statistics
import sys
from statistics import NormalDist

PLAN_HEADER = ["study_id", "participant_id", "block", "trial_id", "source", "item_id",
               "scenario", "expected", "kind"]
DATA_HEADER = ["study_id", "participant_id", "trial_id", "source", "item_id", "scenario",
               "expected", "observed", "button_ms", "draft_ms", "confirm_ms", "send_ms",
               "energy_uj", "outcome", "firmware_rev", "device_id"]
INTENTS = {"arrive", "help", "cancel", "none"}
OUTCOMES = {"sent", "cancelled", "rejected", "timed_out", "source_lost"}
SOURCES = {"core", "nucleus"}


def die(message):
    sys.exit(message)


def read_csv(path, header):
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        if reader.fieldnames != header:
            die("invalid header in %s" % path)
        rows = list(reader)
    if not rows:
        die("no rows in %s" % path)
    return rows


def integer(row, key, errors):
    try:
        n = int(row[key])
        if n < 0:
            raise ValueError
        return n
    except (ValueError, TypeError, KeyError):
        errors.append("%s is not a non-negative integer" % key)
        return 0


def wilson(successes, total, confidence=0.95, side="lower"):
    if total <= 0:
        return None
    z = NormalDist().inv_cdf(confidence)
    p = successes / total
    denom = 1.0 + z * z / total
    center = (p + z * z / (2 * total)) / denom
    radius = z * math.sqrt(p * (1 - p) / total + z * z / (4 * total * total)) / denom
    return max(0.0, center - radius) if side == "lower" else min(1.0, center + radius)


def p95(values):
    if not values:
        return None
    values = sorted(values)
    return values[math.ceil(0.95 * len(values)) - 1]


def row_key(r):
    return (r["participant_id"], r["source"], r["trial_id"])


def validate_data(plan_rows, data_rows, study_id):
    plan = {row_key(r): r for r in plan_rows}
    if len(plan) != len(plan_rows):
        die("duplicate keys in generated plan")
    data = {}
    errors = []
    for r in data_rows:
        key = row_key(r)
        if key in data:
            errors.append("duplicate data key %s" % (key,))
            continue
        data[key] = r
        if r["study_id"] != study_id or r["source"] not in SOURCES:
            errors.append("invalid study/source for %s" % (key,))
        if r["observed"] not in INTENTS or r["expected"] not in INTENTS:
            errors.append("invalid intent for %s" % (key,))
        if r["outcome"] not in OUTCOMES:
            errors.append("invalid outcome for %s" % (key,))
        for forbidden in ("audio", "transcript", "embedding", "identity", "location", "key", "radio_address"):
            if forbidden in r:
                errors.append("forbidden privacy field %s" % forbidden)

        expected_plan = plan.get(key)
        if not expected_plan:
            errors.append("data row not in frozen plan: %s" % (key,))
            continue
        for field in ("item_id", "scenario", "expected"):
            if r[field] != expected_plan[field]:
                errors.append("plan mismatch %s for %s" % (field, key))

        button = integer(r, "button_ms", errors)
        draft = integer(r, "draft_ms", errors)
        confirm = integer(r, "confirm_ms", errors)
        sent = integer(r, "send_ms", errors)
        energy = integer(r, "energy_uj", errors)
        if not r["firmware_rev"] or not r["device_id"]:
            errors.append("missing firmware_rev or device_id for %s" % (key,))
        if draft and (not button or draft < button):
            errors.append("invalid draft timestamp for %s" % (key,))
        if confirm and (not draft or confirm < draft):
            errors.append("invalid confirmation timestamp for %s" % (key,))
        if sent and (not confirm or sent < confirm):
            errors.append("send without prior confirmation for %s" % (key,))
        if r["outcome"] == "sent":
            if not (draft and confirm and sent and energy):
                errors.append("sent row lacks complete measured path for %s" % (key,))
            if r["expected"] not in {"arrive", "help"} or r["observed"] != r["expected"]:
                errors.append("unsafe sent intent for %s" % (key,))
        elif sent:
            errors.append("non-sent outcome has send_ms for %s" % (key,))
        if r["expected"] == "none" and (draft or r["observed"] != "none"):
            errors.append("negative trial made a false draft for %s" % (key,))
        if r["expected"] == "cancel" and r["outcome"] != "cancelled":
            errors.append("cancel trial did not terminate locally for %s" % (key,))

    missing = set(plan) - set(data)
    if missing:
        errors.append("incomplete plan: %d planned rows are missing" % len(missing))
    return errors, list(data.values())


def source_summary(rows, thresholds):
    positives = [r for r in rows if r["expected"] != "none"]
    negatives = [r for r in rows if r["expected"] == "none"]
    correct = [r for r in positives if r["observed"] == r["expected"]]
    false_drafts = [r for r in negatives if int(r["draft_ms"]) or r["observed"] != "none"]
    latencies = [int(r["draft_ms"]) - int(r["button_ms"]) for r in positives if int(r["draft_ms"])]
    confirms = [r for r in rows if int(r["confirm_ms"])]
    sends = [r for r in rows if int(r["send_ms"])]
    energies = [int(r["energy_uj"]) for r in rows if int(r["energy_uj"])]
    unsafe_sends = [r for r in rows if int(r["send_ms"]) and not int(r["confirm_ms"])]

    accuracy = len(correct) / len(positives) if positives else 0.0
    false_rate = len(false_drafts) / len(negatives) if negatives else 1.0
    handoff = len(sends) / len(confirms) if confirms else 0.0
    return {
        "positive_n": len(positives), "correct": len(correct), "accuracy": accuracy,
        "accuracy_lower": wilson(len(correct), len(positives), side="lower"),
        "negative_n": len(negatives), "false_drafts": len(false_drafts), "false_rate": false_rate,
        "false_upper": wilson(len(false_drafts), len(negatives), side="upper"),
        "p95_latency": p95(latencies), "confirmations": len(confirms), "sends": len(sends),
        "handoff": handoff, "energy_median": statistics.median(energies) if energies else None,
        "unsafe_sends": len(unsafe_sends),
        "h1": bool(positives) and wilson(len(correct), len(positives), side="lower") >= thresholds["intent_lower_wilson_95"],
        "h2": bool(negatives) and not false_drafts and wilson(len(false_drafts), len(negatives), side="upper") < thresholds["negative_upper_wilson_95"],
        "h3": bool(latencies) and p95(latencies) <= thresholds["max_p95_latency_ms"],
        "h4": bool(confirms) and handoff == 1.0 and not unsafe_sends,
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--manifest", required=True)
    ap.add_argument("--plan", required=True)
    ap.add_argument("--csv", required=True)
    ap.add_argument("--strict", action="store_true")
    args = ap.parse_args()

    with open(args.manifest) as f:
        manifest = json.load(f)
    plan_rows = read_csv(args.plan, PLAN_HEADER)
    data_rows = read_csv(args.csv, DATA_HEADER)
    errors, rows = validate_data(plan_rows, data_rows, manifest["study_id"])
    grouped = {source: [r for r in rows if r["source"] == source] for source in SOURCES}
    summaries = {source: source_summary(grouped[source], manifest["thresholds"]) for source in SOURCES}

    print("HERUS-A4-001 preregistered confirmatory report")
    print("study status: analysis only; no claim beyond recorded conditions")
    print("plan rows: %d; observed rows: %d; validation errors: %d" %
          (len(plan_rows), len(rows), len(errors)))
    for source in ("core", "nucleus"):
        s = summaries[source]
        print("\n%s" % source.upper())
        print("  exact intent: %d/%d = %.1f%%; Wilson 95%% lower %.1f%%" %
              (s["correct"], s["positive_n"], 100*s["accuracy"], 100*s["accuracy_lower"]))
        print("  false drafts: %d/%d; Wilson 95%% upper %.2f%%" %
              (s["false_drafts"], s["negative_n"], 100*s["false_upper"]))
        print("  button->draft p95: %s ms; handoff: %d/%d = %.3f" %
              (str(s["p95_latency"]), s["sends"], s["confirmations"], s["handoff"]))
        print("  median measured energy: %s uJ" % str(s["energy_median"]))
        print("  H1 %s  H2 %s  H3 %s  H4 %s" % tuple("PASS" if s[k] else "FAIL" for k in ("h1", "h2", "h3", "h4")))

    t = manifest["thresholds"]
    base_pass = not errors and all(all(s[k] for k in ("h1", "h2", "h3", "h4")) for s in summaries.values())
    core, nucleus = summaries["core"], summaries["nucleus"]
    h5 = (base_pass and core["p95_latency"] is not None and nucleus["p95_latency"] is not None and
          core["energy_median"] is not None and nucleus["energy_median"] is not None and
          nucleus["p95_latency"] <= core["p95_latency"] - t["nucleus_latency_advantage_ms"] and
          nucleus["energy_median"] <= core["energy_median"] * t["nucleus_energy_fraction_max"])
    print("\nH5 %s — %s" % ("PASS" if h5 else "FAIL",
          "NUCLEUS PREFERRED" if h5 else "CORE DEFAULT — no demonstrated Nucleus advantage"))
    if errors:
        print("\nvalidation deviations")
        for err in errors[:20]:
            print("  - %s" % err)
        if len(errors) > 20:
            print("  - ... %d additional deviations" % (len(errors) - 20))

    all_pass = base_pass and h5
    if args.strict and not all_pass:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
