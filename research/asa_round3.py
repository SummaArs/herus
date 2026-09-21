"""Round 3 ASA: redundant observations and robust effect calibration.

The ASA aggregates independent host observations using a coordinate-wise
median. Symmetric bounded noise should recover the true effect; persistent
biased noise or disagreement must yield abstention. The output is still a
proposal-only semantic mapping and never an external action.
"""
from __future__ import annotations

import json
from dataclasses import dataclass
from statistics import median
from typing import Any

from research.asa_round1 import EVENT_EFFECTS, _load_cases, _parse_event
from research.asa_round2 import PartialNoisyHost, _evaluate, _target
from research.symbiont_v2.core import Goal, SymbiontRuntime
from research.symbiont_v2.sim_hosts import ToyHost


@dataclass(frozen=True)
class CalibratedMapping:
    mapping: dict[tuple[tuple[str, int], ...], str]
    support: dict[tuple[tuple[str, int], ...], float]
    replicas: int


def _source() -> ToyHost:
    return ToyHost("r3-source", ("src_a", "src_b", "src_c"), {
        "src_a": EVENT_EFFECTS["ARRIVE"],
        "src_b": EVENT_EFFECTS["HELP"],
        "src_c": EVENT_EFFECTS["CANCEL"],
    }, initial={"arrive": 0, "help": 0, "cancel": 0})


def _runtime_with_skills() -> SymbiontRuntime:
    runtime = SymbiontRuntime("r3-asa")
    runtime.discover(_source())
    for effect in EVENT_EFFECTS.values():
        skill = runtime.synthesize(Goal.from_dict(dict(effect)))
        if skill is not None:
            runtime.promote(skill)
    return runtime


def _replica_effects(host: PartialNoisyHost) -> dict[str, tuple[tuple[str, int], ...]]:
    runtime = _runtime_with_skills()
    evidence = runtime.discover(host)
    return {item.action.action_id: item.effect.delta for item in evidence}


def _calibrated_map(seed: int, deltas: tuple[int, ...]) -> CalibratedMapping:
    observations: dict[str, list[tuple[tuple[str, int], ...]]] = {}
    for delta in deltas:
        replica = PartialNoisyHost(
            _target(seed),
            ("arrive", "help", "cancel"),
            noise_period=1,
            noise_delta=delta,
        )
        for action, effect in _replica_effects(replica).items():
            observations.setdefault(action, []).append(effect)

    action_medians: dict[str, tuple[tuple[str, int], ...]] = {}
    action_support: dict[str, float] = {}
    for action, effects in observations.items():
        keys = sorted({key for effect in effects for key, _ in effect})
        aggregate = tuple((key, int(median([dict(effect).get(key, 0) for effect in effects]))) for key in keys)
        action_medians[action] = tuple((key, value) for key, value in aggregate if value != 0)
        center = dict(action_medians[action])
        inlier_count = sum(
            all(abs(dict(effect).get(key, 0) - center.get(key, 0)) <= 1 for key in keys)
            for effect in effects
        )
        action_support[action] = inlier_count / len(effects)

    mapping: dict[tuple[tuple[str, int], ...], str] = {}
    support: dict[tuple[tuple[str, int], ...], float] = {}
    for event, expected_effect in EVENT_EFFECTS.items():
        candidates = [action for action, effect in action_medians.items() if effect == expected_effect]
        # Require unique semantic identification and at least a majority of
        # exact replicas. Persistent disagreement is not authorization.
        candidates = [action for action in candidates if action_support[action] >= 0.6]
        if len(candidates) == 1:
            mapping[expected_effect] = event
            support[expected_effect] = action_support[candidates[0]]
    return CalibratedMapping(mapping, support, len(deltas))


def _scenario(name: str, deltas: tuple[int, ...]) -> dict[str, Any]:
    cases = _load_cases()
    runs = []
    for seed in range(10):
        calibration = _calibrated_map(seed, deltas)
        result = _evaluate(cases, calibration.mapping)
        runs.append(result)
    return {
        "replicas": len(deltas),
        "noise_deltas": list(deltas),
        "mean_coverage": sum(item["coverage"] for item in runs) / len(runs),
        "mean_valid_recall": sum(item["valid_recall"] for item in runs) / len(runs),
        "mean_selective_accuracy": sum(item["selective_accuracy"] for item in runs) / len(runs),
        "safe_abstention": sum(item["safe_abstention"] for item in runs),
        "unsafe_non_abstention": sum(item["unsafe_non_abstention"] for item in runs),
    }


def run() -> dict[str, Any]:
    scenarios = {
        "clean_replicates": _scenario("clean_replicates", (0, 0, 0, 0, 0)),
        "symmetric_bounded_noise": _scenario("symmetric_bounded_noise", (-1, 0, 1, 0, -1)),
        "one_bad_replica": _scenario("one_bad_replica", (0, 0, 0, 0, 2)),
        "persistent_positive_bias": _scenario("persistent_positive_bias", (1, 1, 1, 1, 1)),
        "persistent_negative_bias": _scenario("persistent_negative_bias", (-1, -1, -1, -1, -1)),
    }
    return {
        "schema": "herus-asa-round3-v1",
        "hypothesis": "median aggregation restores coverage under symmetric bounded noise while persistent bias remains fail-closed",
        "replicas_per_scenario": 5,
        "scenarios": scenarios,
        "authority_boundary": "proposal-only; no external execution",
        "interpretation": "Consensus can recover bounded symmetric noise, but no amount of repetition authorizes a persistently biased contract.",
    }


if __name__ == "__main__":
    print(json.dumps(run(), ensure_ascii=False, indent=2, sort_keys=True))
