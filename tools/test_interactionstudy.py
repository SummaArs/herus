#!/usr/bin/env python3
"""Host proof for the HERUS-A4-001 plan and preregistered analyzer.

It uses constructed fixture rows only to test software logic. These rows are not
study evidence, are not committed as results, and must never be cited as hardware
performance.
"""
import csv
import os
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MANIFEST = os.path.join(ROOT, "research", "interaction_study_manifest.json")
PLAN_TOOL = os.path.join(ROOT, "tools", "studyplan.py")
ANALYZER = os.path.join(ROOT, "tools", "interactionstudy.py")
DATA_HEADER = ["study_id", "participant_id", "trial_id", "source", "item_id", "scenario",
               "expected", "observed", "button_ms", "draft_ms", "confirm_ms", "send_ms",
               "energy_uj", "outcome", "firmware_rev", "device_id"]


def write_fixture(plan_path, out_path, unsafe=False):
    with open(plan_path, newline="") as f:
        plan = list(csv.DictReader(f))
    assert len(plan) == 984, len(plan)
    with open(out_path, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=DATA_HEADER)
        w.writeheader()
        for row in plan:
            expected = row["expected"]
            source = row["source"]
            positive = expected != "none"
            sendable = expected in {"arrive", "help"}
            button = 1000
            draft = (1500 if source == "core" else 1200) if positive else 0
            confirm = (1700 if source == "core" else 1400) if sendable else 0
            send = (1710 if source == "core" else 1410) if sendable else 0
            if unsafe and row["trial_id"] == "p01-core-001":
                expected = "arrive"
                draft, confirm, send = 1500, 0, 1710
                positive, sendable = True, True
            w.writerow({
                "study_id": row["study_id"], "participant_id": row["participant_id"],
                "trial_id": row["trial_id"], "source": source, "item_id": row["item_id"],
                "scenario": row["scenario"], "expected": expected, "observed": expected,
                "button_ms": button, "draft_ms": draft, "confirm_ms": confirm, "send_ms": send,
                "energy_uj": 1000 if source == "core" else 700,
                "outcome": "sent" if sendable else ("cancelled" if expected == "cancel" else "rejected"),
                "firmware_rev": "test-fixture-only", "device_id": "fixture-%s" % source,
            })


def main():
    with tempfile.TemporaryDirectory() as tmp:
        plan = os.path.join(tmp, "plan.csv")
        good = os.path.join(tmp, "good.csv")
        bad = os.path.join(tmp, "bad.csv")
        subprocess.run([sys.executable, PLAN_TOOL, "--manifest", MANIFEST,
                        "--seed", "20260813", "--out", plan], check=True, stdout=subprocess.PIPE)
        write_fixture(plan, good)
        result = subprocess.run([sys.executable, ANALYZER, "--manifest", MANIFEST,
                                 "--plan", plan, "--csv", good, "--strict"],
                                text=True, capture_output=True)
        if result.returncode != 0 or "H5 PASS — NUCLEUS PREFERRED" not in result.stdout:
            sys.stderr.write(result.stdout + result.stderr)
            return 1
        write_fixture(plan, bad, unsafe=True)
        result = subprocess.run([sys.executable, ANALYZER, "--manifest", MANIFEST,
                                 "--plan", plan, "--csv", bad, "--strict"],
                                text=True, capture_output=True)
        if result.returncode == 0 or "send without prior confirmation" not in result.stdout:
            sys.stderr.write(result.stdout + result.stderr)
            return 1
    print("PREREGISTRATION INVARIANTS HOLD — frozen plan, gates and unsafe-send rejection verified.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
