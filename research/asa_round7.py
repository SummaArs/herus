"""Round 7 ASA: temporal drift monitoring with hysteresis.

A single bad observation must not cause needless deactivation, but persistent
incompatibility must stop proposals. Recovery requires multiple consecutive
good observations. The monitor is proposal-only and deterministic.
"""
from __future__ import annotations

import json
from dataclasses import dataclass, field
from typing import Any


@dataclass
class DriftMonitor:
    bad_limit: int = 2
    good_limit: int = 3
    state: str = "ACTIVE"
    bad_streak: int = 0
    good_streak: int = 0
    history: list[dict[str, Any]] = field(default_factory=list)

    def observe(self, sequence: int, compatible: bool, quality: str = "fresh") -> bool:
        valid = compatible and quality == "fresh"
        before = self.state
        if self.state == "ACTIVE":
            self.good_streak = 0
            if valid:
                self.bad_streak = 0
            else:
                self.bad_streak += 1
                if self.bad_streak >= self.bad_limit:
                    self.state = "ABSTAIN"
                    self.bad_streak = 0
        else:
            self.bad_streak = 0
            if valid:
                self.good_streak += 1
                if self.good_streak >= self.good_limit:
                    self.state = "ACTIVE"
                    self.good_streak = 0
            else:
                self.good_streak = 0
        proposal_allowed = self.state == "ACTIVE" and valid
        self.history.append({
            "sequence": sequence,
            "compatible": compatible,
            "quality": quality,
            "valid": valid,
            "state_before": before,
            "state_after": self.state,
            "proposal_allowed": proposal_allowed,
        })
        return proposal_allowed


def _run_stream(name: str, stream: list[tuple[bool, str]]) -> dict[str, Any]:
    monitor = DriftMonitor()
    blind_proposals = 0
    proposals = 0
    for sequence, (compatible, quality) in enumerate(stream):
        if compatible:
            blind_proposals += 1
        if monitor.observe(sequence, compatible, quality):
            proposals += 1
    abstain_indices = [row["sequence"] for row in monitor.history if row["state_after"] == "ABSTAIN"]
    recover_indices = [row["sequence"] for row in monitor.history if row["state_before"] == "ABSTAIN" and row["state_after"] == "ACTIVE"]
    return {
        "name": name,
        "samples": len(stream),
        "proposals": proposals,
        "blind_proposals": blind_proposals,
        "abstain_started": min(abstain_indices) if abstain_indices else None,
        "recovered_at": min(recover_indices) if recover_indices else None,
        "final_state": monitor.state,
        "false_alarm": name in {"clean", "single_transient"} and bool(abstain_indices),
        "unsafe_non_abstention": sum(row["proposal_allowed"] and not row["valid"] for row in monitor.history),
        "history": monitor.history,
    }


def run() -> dict[str, Any]:
    scenarios = {
        "clean": [(True, "fresh")] * 20,
        "single_transient": [(True, "fresh")] * 5 + [(False, "fresh")] + [(True, "fresh")] * 14,
        "persistent_drift": [(True, "fresh")] * 6 + [(False, "fresh")] * 8 + [(True, "fresh")] * 6,
        "stale_memory": [(True, "fresh")] * 5 + [(True, "stale")] * 3 + [(True, "fresh")] * 12,
        "reordered_frames": [(True, "fresh")] * 4 + [(True, "reordered")] + [(True, "fresh")] * 15,
        "dropped_frame_burst": [(True, "fresh")] * 5 + [(False, "missing")] * 2 + [(True, "fresh")] * 13,
        "drift_and_recovery": [(True, "fresh")] * 5 + [(False, "fresh")] * 3 + [(True, "fresh")] * 12,
    }
    results = {name: _run_stream(name, stream) for name, stream in scenarios.items()}
    summary = {
        name: {key: value for key, value in result.items() if key != "history"}
        for name, result in results.items()
    }
    return {
        "schema": "herus-asa-round7-v1",
        "hypothesis": "persistent temporal incompatibility triggers abstention, while isolated quality faults do not cause false alarms and recovery requires stable evidence",
        "policy": {"bad_limit": 2, "good_limit": 3, "states": ["ACTIVE", "ABSTAIN"]},
        "scenarios": summary,
        "audit_traces": results,
        "baselines": {
            "blind_temporal_reuse": {
                "persistent_drift_proposals": results["persistent_drift"]["blind_proposals"],
                "stale_proposals": results["stale_memory"]["blind_proposals"],
            },
            "always_abstain": {"proposals": 0},
        },
        "authority_boundary": "proposal-only; temporal drift disables proposals",
        "interpretation": "Hysteresis should reject persistent drift without turning one isolated fault into a false alarm.",
    }


if __name__ == "__main__":
    print(json.dumps(run(), ensure_ascii=False, indent=2, sort_keys=True))
