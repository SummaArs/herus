"""Fail-closed checks for the frozen useful-symbiosis research contract."""
from __future__ import annotations

import json
from pathlib import Path
from typing import Any, Mapping

CONTRACT_PATH = Path(__file__).with_name("symbiosis_utility_contract.json")


def load_contract() -> dict[str, Any]:
    return json.loads(CONTRACT_PATH.read_text(encoding="utf-8"))


def missing_fields(report: Mapping[str, Any], fields: list[str]) -> tuple[str, ...]:
    return tuple(field for field in fields if not report.get(field))


def validate_report(report: Mapping[str, Any], contract: Mapping[str, Any] | None = None) -> tuple[str, ...]:
    """Return deterministic violations; an empty tuple means protocol-complete.

    This checks completeness, not truth. Human benefit still requires actual
    observations against the frozen baseline; no self-reported success passes
    as evidence merely because it has the right keys.
    """
    contract = contract or load_contract()
    violations: list[str] = []
    for dimension in contract["required_dimensions"]:
        if not isinstance(report.get(dimension), Mapping):
            violations.append(f"missing_dimension:{dimension}")

    mechanism = report.get("mechanism", {})
    for metric in contract["mechanism"]["required_metrics"]:
        if metric not in mechanism:
            violations.append(f"missing_mechanism_metric:{metric}")
    for metric, limit in contract["mechanism"]["hard_limits"].items():
        if metric not in mechanism:
            violations.append(f"missing_mechanism_metric:{metric}")
        elif mechanism.get(metric) != limit:
            violations.append(f"mechanism_hard_limit:{metric}")

    human = report.get("human_value", {})
    for field in contract["human_value"]["required_fields"]:
        if not human.get(field):
            violations.append(f"missing_human_field:{field}")
    for baseline in contract["human_value"]["required_comparisons"]:
        if not human.get(baseline):
            violations.append(f"missing_comparison:{baseline}")
    for metric in contract["human_value"]["benefit_must_be_measured_in"]:
        if metric not in human.get("measured_benefit", {}):
            violations.append(f"missing_benefit_metric:{metric}")
    if human.get("improves_over_frozen_baseline") is not True:
        violations.append("human_value_not_proven")

    baseline = report.get("baseline", {})
    for field in ("frozen", "failures_reported"):
        if baseline.get(field) is not True:
            violations.append(f"baseline:{field}")

    safety = report.get("safety", {})
    for control in contract["safety"]["required_controls"]:
        if safety.get(control) is not True:
            violations.append(f"missing_safety_control:{control}")
    for metric, limit in contract["safety"]["hard_limits"].items():
        if metric not in safety:
            violations.append(f"missing_safety_metric:{metric}")
        elif safety.get(metric) != limit:
            violations.append(f"safety_hard_limit:{metric}")

    privacy = report.get("privacy_accessibility", {})
    for field in contract["privacy_accessibility"]["required_fields"]:
        if not privacy.get(field):
            violations.append(f"missing_privacy_accessibility:{field}")
    for metric, limit in contract["privacy_accessibility"]["hard_limits"].items():
        if metric not in privacy:
            violations.append(f"missing_privacy_metric:{metric}")
        elif privacy.get(metric) != limit:
            violations.append(f"privacy_hard_limit:{metric}")

    reproducibility = report.get("reproducibility", {})
    for field in contract["reproducibility"]["required_fields"]:
        if not reproducibility.get(field):
            violations.append(f"missing_reproducibility:{field}")
    return tuple(violations)


def classify(report: Mapping[str, Any]) -> str:
    violations = validate_report(report)
    if not violations:
        return "useful_symbiosis"
    if any("hard_limit" in violation for violation in violations):
        return "not_proven"
    mechanism = report.get("mechanism", {})
    mechanism_complete = all(
        metric in mechanism for metric in load_contract()["mechanism"]["required_metrics"]
    )
    return "mechanism_only" if mechanism_complete else "not_proven"
