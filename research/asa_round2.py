"""Round 2 ASA: partial observability and noisy evidence.

The experiment is proposal-only. A host wrapper corrupts observations by
masking state keys and adding deterministic bounded noise. The independent
host specification remains the evaluator's ground truth; the ASA sees only
observations returned by the wrapper.
"""
from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from research.asa_round1 import EVENT_EFFECTS, _load_cases, _parse_event
from research.symbiont_v2.core import Goal, Observation, PrimitiveAction, State, SymbiontRuntime
from research.symbiont_v2.sim_hosts import ToyHost

CORPUS = Path(__file__).with_name("evidence") / "semantic_ir_real_corpus.json"


@dataclass
class PartialNoisyHost:
    """Host adapter exposing only a projection of the real state."""

    inner: ToyHost
    visible_keys: tuple[str, ...]
    noise_period: int = 0
    noise_delta: int = 0

    @property
    def host_id(self) -> str:
        return f"{self.inner.host_id}:visible={','.join(self.visible_keys)}:noise={self.noise_period}"

    def resources(self) -> tuple[str, ...]:
        return self.inner.resources()

    def safe_action_space(self) -> tuple[PrimitiveAction, ...]:
        return self.inner.safe_action_space()

    def _project(self, observation: Observation) -> Observation:
        values = observation.state.to_dict()
        projected: dict[str, int] = {}
        for key in self.visible_keys:
            if key in values:
                value = values[key]
                if self.noise_period and observation.sequence and observation.sequence % self.noise_period == 0:
                    value += self.noise_delta
                projected[key] = value
        return Observation.make(observation.sequence, State.from_dict(projected), observation.action)

    def observe(self) -> Observation:
        return self._project(self.inner.observe())

    def execute(self, action: PrimitiveAction) -> Observation:
        self.inner.execute(action)
        return self.observe()

    def reset(self) -> None:
        self.inner.reset()


def _source() -> ToyHost:
    return ToyHost("r2-source", ("src_a", "src_b", "src_c"), {
        "src_a": EVENT_EFFECTS["ARRIVE"],
        "src_b": EVENT_EFFECTS["HELP"],
        "src_c": EVENT_EFFECTS["CANCEL"],
    }, initial={"arrive": 0, "help": 0, "cancel": 0})


def _target(seed: int) -> ToyHost:
    # This specification is independent of the wrapper's projection logic.
    names = (f"t{seed}_arr", f"t{seed}_help", f"t{seed}_cancel")
    return ToyHost(f"r2-target-{seed}", names, {
        names[0]: EVENT_EFFECTS["ARRIVE"],
        names[1]: EVENT_EFFECTS["HELP"],
        names[2]: EVENT_EFFECTS["CANCEL"],
    }, initial={"arrive": 0, "help": 0, "cancel": 0})


def _runtime_with_skills() -> SymbiontRuntime:
    runtime = SymbiontRuntime("r2-asa")
    runtime.discover(_source())
    for effect in EVENT_EFFECTS.values():
        skill = runtime.synthesize(Goal.from_dict(dict(effect)))
        if skill is not None:
            runtime.promote(skill)
    return runtime


def _effect_map(host: PartialNoisyHost) -> dict[tuple[tuple[str, int], ...], str]:
    runtime = _runtime_with_skills()
    runtime.discover(host)
    assert runtime.host is not None
    observed: dict[tuple[tuple[str, int], ...], set[str]] = {}
    for evidence in runtime.host.evidence:
        observed.setdefault(evidence.effect.delta, set()).add(evidence.action.action_id)
    # Strict one-to-one matching. Missing, noisy or colliding evidence cannot
    # authorize a semantic mapping.
    mapping: dict[tuple[tuple[str, int], ...], str] = {}
    for event, effect in EVENT_EFFECTS.items():
        actions = observed.get(effect, set())
        if len(actions) == 1:
            mapping[effect] = event
    return mapping


def _expected_event(case: dict[str, Any]) -> str | None:
    expected = case["expected"]
    return expected["eventKind"]


def _evaluate(cases: list[dict[str, Any]], mapping: dict[tuple[tuple[str, int], ...], str]) -> dict[str, Any]:
    rows = []
    for case in cases:
        parsed = _parse_event(case)
        predicted = mapping.get(EVENT_EFFECTS[parsed]) if parsed is not None else None
        expected = _expected_event(case)
        rows.append({"id": case["id"], "expected": expected, "predicted": predicted})
    valid = [row for row in rows if row["expected"] is not None]
    negatives = [row for row in rows if row["expected"] is None]
    selected = [row for row in rows if row["predicted"] is not None]
    return {
        "cases": len(rows),
        "coverage": len(selected) / len(rows),
        "valid_recall": sum(row["predicted"] == row["expected"] for row in valid) / len(valid),
        "selective_accuracy": (sum(row["predicted"] == row["expected"] for row in selected) / len(selected)) if selected else 1.0,
        "safe_abstention": sum(row["predicted"] is None and row["expected"] is None for row in negatives),
        "unsafe_non_abstention": sum(row["predicted"] is not None and row["expected"] is None for row in negatives),
        "rows": rows,
    }


def _scenario(seed: int, visible: tuple[str, ...], noise_period: int = 0, noise_delta: int = 0) -> dict[str, Any]:
    host = PartialNoisyHost(_target(seed), visible, noise_period, noise_delta)
    result = _evaluate(_load_cases(), _effect_map(host))
    return {
        "seed": seed,
        "visible_keys": list(visible),
        "noise_period": noise_period,
        "noise_delta": noise_delta,
        "metrics": {key: value for key, value in result.items() if key != "rows"},
    }


def run() -> dict[str, Any]:
    cases = _load_cases()
    scenarios = {
        "full_clean": [_scenario(seed, ("arrive", "help", "cancel")) for seed in range(10)],
        "partial_one_key": [_scenario(seed, ("arrive",)) for seed in range(10)],
        "partial_two_keys": [_scenario(seed, ("arrive", "help")) for seed in range(10)],
        "noisy_all_keys": [_scenario(seed, ("arrive", "help", "cancel"), noise_period=1, noise_delta=1) for seed in range(10)],
    }
    summary: dict[str, Any] = {}
    for name, rows in scenarios.items():
        summary[name] = {
            "runs": len(rows),
            "mean_coverage": sum(row["metrics"]["coverage"] for row in rows) / len(rows),
            "mean_valid_recall": sum(row["metrics"]["valid_recall"] for row in rows) / len(rows),
            "mean_selective_accuracy": sum(row["metrics"]["selective_accuracy"] for row in rows) / len(rows),
            "unsafe_non_abstention": sum(row["metrics"]["unsafe_non_abstention"] for row in rows),
            "safe_abstention": sum(row["metrics"]["safe_abstention"] for row in rows),
        }
    return {
        "schema": "herus-asa-round2-v1",
        "hypothesis": "under partial or corrupted observation, strict effect-contract transfer trades coverage for abstention and keeps unsafe non-abstention at zero",
        "real_local_cases": len(cases),
        "scenarios": summary,
        "authority_boundary": "proposal-only; no external execution",
        "interpretation": "Positive evidence requires zero unsafe non-abstention; reduced coverage under missing or noisy evidence is expected and safer than guessing.",
    }


if __name__ == "__main__":
    print(json.dumps(run(), ensure_ascii=False, indent=2, sort_keys=True))
