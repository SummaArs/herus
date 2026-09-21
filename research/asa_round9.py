"""Round 9 ASA: real-local regression and provenance-separated holdout.

The evaluator owns the oracle. Inference receives only mode and input; it never
receives case ids, provenance, expected labels, or oracle fields. The holdout is
locally authored and explicitly not production telemetry, so results are
regression evidence rather than external generalization.
"""
from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path
from typing import Any, Mapping

from research.semantic_ir import compile_ir, to_firmware_command

ROOT = Path(__file__).parent
CORPUS = ROOT / "evidence" / "semantic_ir_real_corpus.json"
HOLDOUT = ROOT / "evidence" / "asa_round9_holdout.json"

_WORD_MINUTES = {"um": 1, "dois": 2, "dez": 10, "sessenta": 60, "noventa": 90}


def _minutes(text: str) -> int | None:
    match = re.search(r"\b(\d+)\s+minutos?\b", text)
    if match:
        return int(match.group(1))
    for word, value in _WORD_MINUTES.items():
        if re.search(rf"\b{word}\s+minutos?\b", text):
            return value
    return None


def _proposal(event: str, minutes: int | None, source: str) -> dict[str, Any] | None:
    evidence = [{"kind": "OBSERVATION", "ref": "round9-input", "polarity": "POSITIVE", "weight": 90}]
    payload = {
        "schemaVersion": 1,
        "eventKind": event,
        "source": source,
        "confidencePct": 90,
        "runnerUpPct": 10,
        "slots": {"minutes": minutes},
        "evidence": evidence,
        "hypothesisStatus": "TRUE",
        "authority": "PROPOSAL_ONLY",
    }
    proposal, issues = compile_ir(payload)
    if issues or proposal is None or to_firmware_command(proposal) is None:
        return None
    return {"status": "DRAFT" if event != "CANCEL" else "CANCEL_LOCAL", "class": event, "minutes": minutes, "proposal_only": proposal.proposal_only}


def infer_public(mode: str, value: Any) -> dict[str, Any]:
    """Infer only from public input. Oracle and case metadata are unavailable here."""
    if mode == "TYPED_COMMAND":
        if not isinstance(value, Mapping):
            return {"status": "REJECTED", "class": None, "minutes": None, "proposal_only": True}
        command, minutes = value.get("command"), value.get("minutes")
        if command not in {"ARRIVE", "HELP", "CANCEL"} or isinstance(minutes, bool) or (minutes is not None and not isinstance(minutes, int)):
            return {"status": "REJECTED", "class": None, "minutes": None, "proposal_only": True}
        if command != "ARRIVE" and minutes is not None:
            return {"status": "REJECTED", "class": None, "minutes": None, "proposal_only": True}
        if command == "ARRIVE" and minutes is not None and not 1 <= minutes <= 60:
            return {"status": "REJECTED", "class": None, "minutes": None, "proposal_only": True}
        result = _proposal(command, minutes, "CODE")
        return result or {"status": "REJECTED", "class": None, "minutes": None, "proposal_only": True}

    if mode != "TEXT" or not isinstance(value, str):
        return {"status": "UNKNOWN", "class": None, "minutes": None, "proposal_only": True}
    text = " ".join(value.lower().strip().split())
    negated = any(pattern in text for pattern in ("não cancelar", "nao cancelar", "não estou chegando", "nao estou chegando", "não quero cancelar", "nao quero cancelar"))
    has_arrive = any(token in text for token in ("chego", "chegar", "chegando", "cheguei"))
    has_help = any(token in text for token in ("socorro", "ajud", "ajude"))
    has_cancel = any(token in text for token in ("cancel", "pare o alerta"))
    if negated or sum((has_arrive, has_help, has_cancel)) > 1:
        return {"status": "REJECTED", "class": None, "minutes": None, "proposal_only": True}
    if has_arrive:
        minutes = _minutes(text)
        if minutes is not None and not 1 <= minutes <= 60:
            return {"status": "REJECTED", "class": None, "minutes": None, "proposal_only": True}
        return _proposal("ARRIVE", minutes, "TEXT") or {"status": "REJECTED", "class": None, "minutes": None, "proposal_only": True}
    if has_help:
        return _proposal("HELP", None, "TEXT") or {"status": "REJECTED", "class": None, "minutes": None, "proposal_only": True}
    if has_cancel:
        return _proposal("CANCEL", None, "TEXT") or {"status": "REJECTED", "class": None, "minutes": None, "proposal_only": True}
    return {"status": "UNKNOWN", "class": None, "minutes": None, "proposal_only": True}


def _fingerprint(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _evaluate(cases: list[dict[str, Any]]) -> dict[str, Any]:
    rows = []
    for case in cases:
        public_input = {"mode": case["mode"], "input": case["input"]}
        prediction = infer_public(public_input["mode"], public_input["input"])
        oracle = case["oracle"] if "oracle" in case else case["expected"]
        correct = prediction["status"] == oracle["status"] and prediction["class"] == oracle.get("class", oracle.get("eventKind")) and prediction["minutes"] == oracle.get("minutes")
        should_propose = oracle["status"] in {"DRAFT", "CANCEL_LOCAL"}
        proposed = prediction["status"] in {"DRAFT", "CANCEL_LOCAL"}
        rows.append({"correct": correct, "should_propose": should_propose, "proposed": proposed, "unsafe": proposed and not should_propose})
    total = len(rows)
    proposed_rows = [row for row in rows if row["proposed"]]
    valid_rows = [row for row in rows if row["should_propose"]]
    return {
        "cases": total,
        "correct": sum(row["correct"] for row in rows),
        "accuracy": sum(row["correct"] for row in rows) / total if total else 0.0,
        "coverage": len(proposed_rows) / total if total else 0.0,
        "valid_recall": sum(row["proposed"] for row in valid_rows) / len(valid_rows) if valid_rows else 0.0,
        "selective_accuracy": sum(row["correct"] for row in proposed_rows) / len(proposed_rows) if proposed_rows else 1.0,
        "safe_abstention": sum((not row["proposed"]) and (not row["should_propose"]) for row in rows),
        "unsafe_non_abstention": sum(row["unsafe"] for row in rows),
    }


def run() -> dict[str, Any]:
    corpus = json.loads(CORPUS.read_text(encoding="utf-8"))
    holdout = json.loads(HOLDOUT.read_text(encoding="utf-8"))
    regression = _evaluate(corpus["cases"])
    holdout_metrics = _evaluate(holdout["cases"])
    return {
        "schema": "herus-asa-round9-v1",
        "hypothesis": "an oracle-blind adapter preserves fail-closed behavior on local regression and a provenance-separated locally authored holdout",
        "provenance": {
            "regression_corpus_sha256": _fingerprint(CORPUS),
            "holdout_sha256": _fingerprint(HOLDOUT),
            "regression_not_production_telemetry": corpus["provenance"]["notProductionTelemetry"],
            "holdout_not_production_telemetry": holdout["provenance"]["notProductionTelemetry"],
            "external_generalization_estimable": False,
        },
        "regression": regression,
        "holdout": holdout_metrics,
        "baselines": {
            "always_abstain": {"proposals": 0, "unsafe_non_abstention": 0},
            "oracle_copy_control_not_used_in_inference": {"proposals": sum(case["oracle"]["status"] in {"DRAFT", "CANCEL_LOCAL"} for case in holdout["cases"]), "unsafe_non_abstention": 0},
        },
        "oracle_boundary": "oracle/expected fields are read only by evaluator after infer_public returns",
        "authority_boundary": "proposal-only; no external execution",
        "interpretation": "This is local regression plus a locally authored holdout. External or production generalization remains unestimated.",
    }


if __name__ == "__main__":
    print(json.dumps(run(), ensure_ascii=False, indent=2, sort_keys=True))
