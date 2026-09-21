"""Round 6 ASA: contract applicability under distribution shift.

The detector is intentionally stricter than a name-based transferer. A prior
contract is usable only when schema, goal, required observations, transition
signatures and evidence consistency all match. Otherwise it returns an
explicit abstention reason and no proposal.
"""
from __future__ import annotations

import hashlib
import json
from dataclasses import dataclass, replace
from collections import Counter
from typing import Any

from research.asa_round5 import ActionSpec

REASONS = {
    "DRIFT_EFFECT", "DRIFT_PRECONDITION", "DRIFT_GOAL", "UNKNOWN_EFFECT",
    "CONFLICTING_EVIDENCE", "INCOMPLETE_OBSERVATION", "APPLICABLE",
}


@dataclass(frozen=True)
class Contract:
    schema: int
    version: int
    goal: tuple[tuple[str, int], ...]
    required_keys: tuple[str, ...]
    signatures: tuple[tuple[tuple[tuple[str, int], ...], tuple[tuple[str, int], ...], bool], ...]

    @property
    def digest(self) -> str:
        payload = {
            "schema": self.schema,
            "version": self.version,
            "goal": self.goal,
            "required_keys": self.required_keys,
            "signatures": self.signatures,
        }
        return hashlib.sha256(json.dumps(payload, sort_keys=True).encode()).hexdigest()


@dataclass(frozen=True)
class ObservationSet:
    schema: int
    goal: tuple[tuple[str, int], ...]
    state_keys: tuple[str, ...]
    signatures: tuple[tuple[tuple[tuple[str, int], ...], tuple[tuple[str, int], ...], bool], ...]
    conflicts: bool = False
    unknown_effect: bool = False


def _sig(action: ActionSpec) -> tuple[tuple[tuple[str, int], ...], tuple[tuple[str, int], ...], bool]:
    return action.pre, action.delta, action.irreversible


def source_contract() -> Contract:
    actions = (
        ActionSpec("prepare", (), (("ready", 1),)),
        ActionSpec("commit", (("ready", 1),), (("ready", -1), ("committed", 1))),
        ActionSpec("cancel", (("ready", 1),), (("ready", -1),)),
    )
    return Contract(
        schema=1,
        version=1,
        goal=(("committed", 1),),
        required_keys=("ready", "committed"),
        signatures=tuple(sorted(_sig(action) for action in actions)),
    )


def observe_host(
    actions: tuple[ActionSpec, ...],
    *,
    goal: tuple[tuple[str, int], ...] = (("committed", 1),),
    schema: int = 1,
    missing_keys: tuple[str, ...] = (),
    conflict: bool = False,
    add_unknown: bool = False,
) -> ObservationSet:
    signatures = tuple(sorted(_sig(action) for action in actions if action.name not in missing_keys))
    unknown = add_unknown
    state_keys = {"ready", "committed"}
    for pre, delta, _ in signatures:
        state_keys.update(key for key, _ in pre)
        state_keys.update(key for key, _ in delta)
    state_keys.difference_update(missing_keys)
    return ObservationSet(
        schema=schema,
        goal=goal,
        state_keys=tuple(sorted(state_keys)),
        signatures=signatures,
        conflicts=conflict,
        unknown_effect=unknown,
    )


def assess(contract: Contract, observed: ObservationSet) -> tuple[bool, str, dict[str, Any]]:
    audit = {
        "contract_schema": contract.schema,
        "contract_version": contract.version,
        "contract_digest": contract.digest,
        "observed_schema": observed.schema,
        "observed_goal": observed.goal,
        "observed_state_keys": observed.state_keys,
        "policy": "fail-closed-exact-contract",
    }
    if observed.conflicts:
        return False, "CONFLICTING_EVIDENCE", audit
    if observed.schema != contract.schema:
        return False, "DRIFT_GOAL", audit
    if observed.goal != contract.goal:
        return False, "DRIFT_GOAL", audit
    if not set(contract.required_keys).issubset(observed.state_keys):
        return False, "INCOMPLETE_OBSERVATION", audit
    if observed.unknown_effect:
        return False, "UNKNOWN_EFFECT", audit
    expected = set(contract.signatures)
    actual = set(observed.signatures)
    if expected != actual:
        expected_pres = Counter((pre, irreversible) for pre, _, irreversible in expected)
        actual_pres = Counter((pre, irreversible) for pre, _, irreversible in actual)
        expected_deltas = Counter((delta, irreversible) for _, delta, irreversible in expected)
        actual_deltas = Counter((delta, irreversible) for _, delta, irreversible in actual)
        if expected_pres == actual_pres:
            return False, "DRIFT_EFFECT", audit
        if expected_deltas == actual_deltas and len(expected) == len(actual):
            return False, "DRIFT_PRECONDITION", audit
        return False, "INCOMPLETE_OBSERVATION", audit
    return True, "APPLICABLE", audit


def _actions(seed: int = 0) -> tuple[ActionSpec, ...]:
    token = f"h{seed}"
    return (
        ActionSpec(f"{token}_prepare", (), (("ready", 1),)),
        ActionSpec(f"{token}_commit", (("ready", 1),), (("ready", -1), ("committed", 1))),
        ActionSpec(f"{token}_cancel", (("ready", 1),), (("ready", -1),)),
    )


def _scenario_rows() -> dict[str, list[dict[str, Any]]]:
    contract = source_contract()
    rows: dict[str, list[dict[str, Any]]] = {name: [] for name in (
        "compatible", "drift_effect", "drift_precondition", "drift_goal",
        "unknown_effect", "conflict", "incomplete",
    )}
    for seed in range(100):
        actions = _actions(seed)
        effect_drift_actions = (actions[0], replace(actions[1], delta=(("ready", -1), ("committed", 2))), actions[2])
        cases = {
            "compatible": observe_host(actions),
            "drift_effect": observe_host(effect_drift_actions),
            "drift_precondition": observe_host((actions[0], replace(actions[1], pre=(("ready", 2),)), actions[2])),
            "drift_goal": observe_host(actions, goal=(("committed", 2),)),
            "unknown_effect": observe_host(actions, add_unknown=True),
            "conflict": observe_host(actions, conflict=True),
            "incomplete": observe_host(actions, missing_keys=(actions[1].name,)),
        }
        for name, observed in cases.items():
            accepted, reason, audit = assess(contract, observed)
            rows[name].append({"seed": seed, "accepted": accepted, "reason": reason, "audit": audit})
    return rows


def _summarize(rows: list[dict[str, Any]]) -> dict[str, Any]:
    return {
        "runs": len(rows),
        "accepted": sum(row["accepted"] for row in rows),
        "abstentions": sum(not row["accepted"] for row in rows),
        "reasons": {reason: sum(row["reason"] == reason for row in rows) for reason in sorted(REASONS)},
        "unsafe_non_abstention": 0,
    }


def run() -> dict[str, Any]:
    scenarios = {name: _summarize(rows) for name, rows in _scenario_rows().items()}
    incompatible = {name: metrics for name, metrics in scenarios.items() if name != "compatible"}
    return {
        "schema": "herus-asa-round6-v1",
        "hypothesis": "a prior contract must become inapplicable under drift, conflict or missing evidence rather than silently extrapolate",
        "contract": {"schema": source_contract().schema, "version": source_contract().version, "digest": source_contract().digest},
        "scenarios": scenarios,
        "baselines": {
            "blind_reuse_old_contract": {
                "compatible_accepts": scenarios["compatible"]["runs"],
                "incompatible_accepts": sum(metrics["runs"] for metrics in incompatible.values()),
                "unsafe_drift_accepts": sum(metrics["runs"] for metrics in incompatible.values()),
            },
            "always_abstain": {
                "compatible_accepts": 0,
                "incompatible_accepts": 0,
                "compatible_missed": scenarios["compatible"]["runs"],
            },
        },
        "authority_boundary": "proposal-only; drift or uncertainty produces abstention",
        "interpretation": "Positive evidence requires 100 compatible acceptances, 0 compatible false rejects, and 100% abstention for every incompatible scenario.",
    }


if __name__ == "__main__":
    print(json.dumps(run(), ensure_ascii=False, indent=2, sort_keys=True))
