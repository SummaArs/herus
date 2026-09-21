"""Round 1 ASA evaluation: typed proposal transfer across hidden hosts.

This experiment deliberately separates two evidence classes:
- real local language/contract cases from semantic_ir_real_corpus.json;
- synthetic host adapters whose primitive names and ordering are hidden.

It does not claim production telemetry, AGI, or actuator authority. The ASA
under test only proposes a typed event; it never executes an external action.
"""
from __future__ import annotations

import json
import hashlib
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from research.symbiont_v2.core import Goal, SymbiontRuntime
from research.symbiont_v2.sim_hosts import ToyHost

CORPUS = Path(__file__).with_name("evidence") / "semantic_ir_real_corpus.json"

EVENT_EFFECTS: dict[str, tuple[tuple[str, int], ...]] = {
    "ARRIVE": (("arrive", 1),),
    "HELP": (("help", 1),),
    "CANCEL": (("cancel", 1),),
}


@dataclass(frozen=True)
class Proposal:
    case_id: str
    status: str
    event_kind: str | None
    requires_confirmation: bool
    source: str

    @property
    def safe(self) -> bool:
        return self.status in {"DRAFT", "CANCEL_LOCAL"} and self.event_kind is not None


def _load_cases() -> list[dict[str, Any]]:
    payload = json.loads(CORPUS.read_text(encoding="utf-8"))
    return list(payload["cases"])


def _target_host(seed: int = 0, variant: str = "equivalent") -> ToyHost:
    # Same semantic affordances, different names and order. Names are not
    # allowed to be used as a transfer key by the ASA.
    token = hashlib.sha256(f"round1:{seed}".encode()).hexdigest()[:8]
    names = (f"op_{token}_h", f"op_{token}_a", f"op_{token}_c")
    effects = {
        names[0]: EVENT_EFFECTS["HELP"],
        names[1]: EVENT_EFFECTS["ARRIVE"],
        names[2]: EVENT_EFFECTS["CANCEL"],
    }
    if variant == "incomplete":
        effects.pop(names[2])
        names = names[:2]
    elif variant == "altered":
        effects[names[1]] = (("arrive", 2),)
    elif variant == "ambiguous":
        effects[f"op_{token}_duplicate"] = EVENT_EFFECTS["ARRIVE"]
        names = names + (f"op_{token}_duplicate",)
    return ToyHost(
        f"round1-hidden-target-{seed}-{variant}", names, effects,
    )


def _source_host() -> ToyHost:
    return ToyHost(
        "round1-source",
        ("a1", "a2", "a3"),
        {
            "a1": EVENT_EFFECTS["ARRIVE"],
            "a2": EVENT_EFFECTS["HELP"],
            "a3": EVENT_EFFECTS["CANCEL"],
        },
    )


def _expected(case: dict[str, Any]) -> Proposal:
    value = case["expected"]
    return Proposal(
        case_id=case["id"],
        status=value["status"],
        event_kind=value["eventKind"],
        requires_confirmation=value["requiresConfirmation"],
        source="real-local-regression-corpus",
    )


def _proposal_for_case(case: dict[str, Any], event_kind: str | None) -> Proposal:
    if event_kind is None:
        return Proposal(case["id"], "ABSTAIN", None, False, "asa")
    status = "CANCEL_LOCAL" if event_kind == "CANCEL" else "DRAFT"
    return Proposal(case["id"], status, event_kind, event_kind != "CANCEL", "asa")


def _asa_event_map(target: ToyHost) -> dict[tuple[tuple[str, int], ...], str]:
    runtime = SymbiontRuntime("round1-asa")
    runtime.discover(_source_host())
    mapping: dict[tuple[tuple[str, int], ...], str] = {}
    for event, effect in EVENT_EFFECTS.items():
        skill = runtime.synthesize(Goal.from_dict(dict(effect)))
        if skill is not None:
            runtime.promote(skill)
    runtime.discover(target)
    assert runtime.host is not None
    effect_actions: dict[tuple[tuple[str, int], ...], list[str]] = {}
    for evidence in runtime.host.evidence:
        effect_actions.setdefault(evidence.effect.delta, []).append(evidence.action.action_id)
    for event, effect in EVENT_EFFECTS.items():
        if effect in effect_actions and len(effect_actions[effect]) == 1:
            mapping[effect] = event
    return mapping


def _parse_event(case: dict[str, Any]) -> str | None:
    """Parse only the finite vocabulary, without consulting expected labels."""
    value = case["input"]
    if isinstance(value, dict):
        command = value.get("command")
        minutes = value.get("minutes")
        if command not in EVENT_EFFECTS:
            return None
        if command != "ARRIVE" and minutes is not None:
            return None
        if command == "ARRIVE" and minutes is not None and not 1 <= minutes <= 60:
            return None
        return command
    text = str(value).strip().lower()
    if "não" in text or "noventa" in text:
        return None
    mentions = [event for event, words in {
        "ARRIVE": ("chego", "chegando"),
        "HELP": ("socorro",),
        "CANCEL": ("cancelar",),
    }.items() if any(word in text for word in words)]
    if len(mentions) != 1:
        return None
    return mentions[0]


def _evaluate(cases: list[dict[str, Any]], mapped: dict[tuple[tuple[str, int], ...], str]) -> dict[str, Any]:
    rows: list[dict[str, Any]] = []
    for case in cases:
        expected = _expected(case)
        parsed = _parse_event(case)
        event = mapped.get(EVENT_EFFECTS[parsed]) if parsed is not None else None
        actual = _proposal_for_case(case, event)
        rows.append({
            "id": case["id"],
            "expected_status": expected.status,
            "actual_status": actual.status,
            "expected_event": expected.event_kind,
            "actual_event": actual.event_kind,
            "correct": (actual.status == expected.status and actual.event_kind == expected.event_kind),
            "abstained_correctly": expected.event_kind is None and actual.event_kind is None,
        })
    return {
        "cases": len(rows),
        "exact_contract_match": sum(row["correct"] for row in rows),
        "semantic_match": sum(row["actual_event"] == row["expected_event"] for row in rows),
        "safe_abstention": sum(row["abstained_correctly"] for row in rows),
        "unsafe_non_abstention": sum(
            row["expected_event"] is None and row["actual_event"] is not None
            for row in rows
        ),
        "rows": rows,
    }


def run() -> dict[str, Any]:
    cases = _load_cases()
    asa = _evaluate(cases, _asa_event_map(_target_host()))
    # Baseline 1: action names from the source host are assumed to remain
    # executable on the target. It cannot map hidden target affordances.
    name_baseline = _evaluate(cases, {})
    return {
        "schema": "herus-asa-round1-v2",
        "hypothesis": "effect-contract transfer preserves typed proposals across hidden host action names while retaining abstention",
        "data": {
            "real_local_cases": len(cases),
            "real_local_provenance": str(CORPUS.relative_to(CORPUS.parents[1])),
            "synthetic_hosts": ["round1-source", "100 equivalent hidden targets", "incomplete/altered/ambiguous controls"],
        },
        "metrics": {
            "asa": {k: v for k, v in asa.items() if k != "rows"},
            "name_baseline": {k: v for k, v in name_baseline.items() if k != "rows"},
        },
        "results": {"asa": asa["rows"], "name_baseline": name_baseline["rows"]},
        "authority_boundary": "proposal-only; no external execution",
        "interpretation": "A positive result supports bounded host-independent proposal transfer, not general intelligence.",
        "synthetic": _synthetic_campaign(cases),
    }


def _synthetic_campaign(cases: list[dict[str, Any]]) -> dict[str, Any]:
    valid_cases = [case for case in cases if _expected(case).event_kind is not None]
    equivalent = []
    for seed in range(100):
        result = _evaluate(valid_cases, _asa_event_map(_target_host(seed, "equivalent")))
        equivalent.append(result)
    controls = {
        variant: _evaluate(valid_cases, _asa_event_map(_target_host(100 + index, variant)))
        for index, variant in enumerate(("incomplete", "altered", "ambiguous"))
    }
    return {
        "equivalent_hosts": 100,
        "equivalent_semantic_matches": sum(item["semantic_match"] for item in equivalent),
        "equivalent_cases": sum(item["cases"] for item in equivalent),
        "control_abstentions": {
            key: value["cases"] - value["semantic_match"] for key, value in controls.items()
        },
        "control_unsafe_non_abstention": {
            key: value["unsafe_non_abstention"] for key, value in controls.items()
        },
    }


if __name__ == "__main__":
    print(json.dumps(run(), ensure_ascii=False, indent=2, sort_keys=True))
