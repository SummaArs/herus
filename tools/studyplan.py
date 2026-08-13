#!/usr/bin/env python3
"""studyplan.py — materialize the HERUS-A4-001 confirmatory trial order.

This generator does not create performance data. It turns the frozen manifest into
an operator worksheet, with source order counterbalanced by participant parity and
items shuffled reproducibly inside each participant/source block.

    python3 tools/studyplan.py --manifest research/interaction_study_manifest.json \
        --seed 20260813 --out research/generated/HERUS-A4-001-plan.csv
"""
import argparse
import csv
import json
import os
import random
import sys

HEADER = ["study_id", "participant_id", "block", "trial_id", "source", "item_id",
          "scenario", "expected", "kind"]


def load_manifest(path):
    with open(path) as f:
        m = json.load(f)
    required = {"study_id", "participants", "sources", "positive_items", "negative_templates"}
    missing = required - set(m)
    if missing or set(m["sources"]) != {"core", "nucleus"}:
        sys.exit("invalid manifest: missing %s or sources are not core/nucleus" % sorted(missing))
    if not m["participants"] or not m["positive_items"]:
        sys.exit("invalid manifest: no participants or positive items")
    return m


def block_items(manifest):
    items = []
    for x in manifest["positive_items"]:
        if x.get("expected") not in {"arrive", "help", "cancel"}:
            sys.exit("invalid positive expected intent")
        items.append(dict(item_id=x["item_id"], scenario=x["scenario"],
                          expected=x["expected"], kind="positive"))
    for x in manifest["negative_templates"]:
        repetitions = x.get("repetitions", 0)
        if repetitions <= 0:
            sys.exit("invalid negative repetitions")
        for n in range(repetitions):
            items.append(dict(item_id="%s-%02d" % (x["item_id"], n + 1),
                              scenario=x["scenario"], expected="none", kind="negative"))
    return items


def source_order(participant_index):
    return ("core", "nucleus") if participant_index % 2 == 0 else ("nucleus", "core")


def generate(manifest, seed):
    rows = []
    original = block_items(manifest)
    rng = random.Random(seed)
    for p_index, participant in enumerate(manifest["participants"]):
        for block, source in enumerate(source_order(p_index), start=1):
            items = list(original)
            rng.shuffle(items)
            for sequence, item in enumerate(items, start=1):
                rows.append({
                    "study_id": manifest["study_id"],
                    "participant_id": participant,
                    "block": str(block),
                    "trial_id": "%s-%s-%03d" % (participant, source, sequence),
                    "source": source,
                    "item_id": item["item_id"],
                    "scenario": item["scenario"],
                    "expected": item["expected"],
                    "kind": item["kind"],
                })
    return rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--manifest", required=True)
    ap.add_argument("--seed", required=True, type=int)
    ap.add_argument("--out", required=True)
    args = ap.parse_args()
    manifest = load_manifest(args.manifest)
    rows = generate(manifest, args.seed)
    parent = os.path.dirname(args.out)
    if parent:
        os.makedirs(parent, exist_ok=True)
    with open(args.out, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=HEADER)
        w.writeheader()
        w.writerows(rows)
    per_source = len(rows) // len(manifest["sources"])
    print("STUDY PLAN HOLD — %s: %d trials total, %d per source, seed %d."
          % (manifest["study_id"], len(rows), per_source, args.seed))


if __name__ == "__main__":
    main()
