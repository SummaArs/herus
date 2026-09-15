"""H0 Reference executor: canonical host-independent SIM decisions.

H0 prioritizes semantic correctness. It is the oracle for later H1 equivalence;
it is not a performance or energy baseline.
"""
from __future__ import annotations

from dataclasses import asdict, dataclass
import json
from typing import Any

from host_profile import HostProfile
from symbiotic_intelligence import Pattern, decide


@dataclass(frozen=True)
class H0Input:
    case_id: str
    features: tuple[int, ...]
    source: str
    digest: str
    representations: tuple[str, ...]
    budget_bytes: int
    budget_steps: int
    authority: str = "NONE"
    min_confidence_milli: int = 700


@dataclass(frozen=True)
class H0Output:
    case_id: str
    label: str
    representation: str
    confidence_milli: int
    proposal: str
    execution: str
    reason: str
    authority: str

    def canonical(self) -> dict[str, Any]:
        return asdict(self)


def evaluate(case: H0Input) -> H0Output:
    if not case.case_id:
        raise ValueError("h0_case_id_invalid")
    profile = HostProfile(
        host_id="h0-reference", revision="h0", resources={"ram_bytes": case.budget_bytes},
        interfaces=frozenset({"local"}), constraints={"max_steps": case.budget_steps},
        representation_set=frozenset(case.representations),
        skill_budget={"bytes": case.budget_bytes, "steps": case.budget_steps},
        evidence={"host": "h0"}, authority=case.authority,
    )
    result = decide(
        profile,
        Pattern(case.features, case.source, case.digest),
        required_bytes=4096,
        required_steps=12,
        min_confidence_milli=case.min_confidence_milli,
    )
    return H0Output(case.case_id, result.label, result.representation, result.neural_confidence_milli,
                    result.proposal, result.execution, result.reason, case.authority)


def evaluate_json(payload: str) -> str:
    raw = json.loads(payload)
    case = H0Input(
        case_id=str(raw["case_id"]), features=tuple(int(v) for v in raw["features"]),
        source=str(raw["source"]), digest=str(raw["digest"]),
        representations=tuple(str(v) for v in raw["representations"]),
        budget_bytes=int(raw["budget_bytes"]), budget_steps=int(raw["budget_steps"]),
        authority=str(raw.get("authority", "NONE")),
        min_confidence_milli=int(raw.get("min_confidence_milli", 700)),
    )
    return json.dumps(evaluate(case).canonical(), sort_keys=True, separators=(",", ":"))
