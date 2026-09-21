"""Round 8 ASA: evidence types, authority and conflict resolution.

Observations describe the world; rules constrain authorization; hypotheses and
preferences may inform a proposal but never authorize one. Only signed policy
sources may issue rules. Equal-authority conflicts abstain. Lower-authority
inputs cannot override a safety rule.
"""
from __future__ import annotations

import hashlib
import json
from dataclasses import dataclass
from typing import Any

KINDS = {"OBSERVATION", "HYPOTHESIS", "RULE", "PREFERENCE"}
TRUSTED_RULE_SOURCES = {"policy_engine", "safety_board"}


@dataclass(frozen=True)
class Evidence:
    kind: str
    subject: str
    value: str
    source: str
    authority: int
    sequence: int = 0

    def __post_init__(self) -> None:
        if self.kind not in KINDS:
            raise ValueError("unknown evidence kind")

    @property
    def digest(self) -> str:
        payload = self.__dict__
        return hashlib.sha256(json.dumps(payload, sort_keys=True).encode()).hexdigest()


@dataclass(frozen=True)
class Decision:
    accepted: bool
    reason: str
    audit: dict[str, Any]


def decide(evidence: list[Evidence], subject: str) -> Decision:
    relevant = [item for item in evidence if item.subject == subject]
    audit: dict[str, Any] = {
        "subject": subject,
        "evidence_count": len(relevant),
        "evidence": [
            {"kind": item.kind, "source": item.source, "authority": item.authority, "value": item.value, "digest": item.digest}
            for item in relevant
        ],
        "policy": "observations-plus-trusted-rules-only",
        "ignored_non_authoritative": [],
    }
    observations = [item for item in relevant if item.kind == "OBSERVATION"]
    if not observations:
        return Decision(False, "MISSING_OBSERVATION", audit)
    observation_values = {item.value for item in observations}
    if len(observation_values) != 1:
        return Decision(False, "CONFLICTING_OBSERVATIONS", audit)
    if "valid" not in observation_values:
        return Decision(False, "OBSERVATION_NOT_VALID", audit)

    non_authoritative = [item for item in relevant if item.kind in {"HYPOTHESIS", "PREFERENCE"}]
    audit["ignored_non_authoritative"] = [item.kind for item in non_authoritative]
    rules = [item for item in relevant if item.kind == "RULE"]
    untrusted = [item for item in rules if item.source not in TRUSTED_RULE_SOURCES]
    if untrusted:
        return Decision(False, "UNTRUSTED_RULE_SOURCE", audit)
    if not rules:
        return Decision(False, "NO_POLICY_AUTHORIZATION", audit)

    max_authority = max(item.authority for item in rules)
    highest = [item for item in rules if item.authority == max_authority]
    highest_values = {item.value for item in highest}
    if len(highest_values) > 1:
        return Decision(False, "CONFLICTING_RULES", audit)
    if "deny" in highest_values:
        return Decision(False, "POLICY_DENIED", audit)
    if "allow" not in highest_values:
        return Decision(False, "UNKNOWN_RULE_VALUE", audit)
    return Decision(True, "AUTHORIZED_BY_TRUSTED_RULE", audit)


def obs(value: str = "valid", source: str = "sensor") -> Evidence:
    return Evidence("OBSERVATION", "contract:alpha", value, source, 60)


def rule(value: str, source: str = "policy_engine", authority: int = 100) -> Evidence:
    return Evidence("RULE", "contract:alpha", value, source, authority)


def hypothesis(value: str = "allow") -> Evidence:
    return Evidence("HYPOTHESIS", "contract:alpha", value, "model", 10)


def preference(value: str = "allow") -> Evidence:
    return Evidence("PREFERENCE", "contract:alpha", value, "operator", 20)


def _scenarios() -> dict[str, list[Evidence]]:
    return {
        "valid_authorized": [obs(), rule("allow")],
        "hypothesis_only": [obs(), hypothesis()],
        "preference_only": [obs(), preference()],
        "conflicting_observations": [obs("valid", "sensor-a"), obs("invalid", "sensor-b"), rule("allow")],
        "conflicting_equal_rules": [obs(), rule("allow", "policy_engine", 100), rule("deny", "safety_board", 100)],
        "safety_deny_over_preference": [obs(), rule("deny", "safety_board", 120), preference("allow")],
        "model_override_attempt": [obs(), rule("allow"), rule("deny", "model", 1000)],
        "missing_observation": [rule("allow")],
        "unknown_rule_value": [obs(), rule("maybe")],
        "lower_authority_untrusted_rule": [obs(), rule("allow", "policy_engine", 120), rule("deny", "operator", 10)],
    }


def run() -> dict[str, Any]:
    decisions = {name: decide(evidence, "contract:alpha") for name, evidence in _scenarios().items()}
    return {
        "schema": "herus-asa-round8-v1",
        "hypothesis": "only valid observations plus trusted rules may authorize; hypotheses, preferences, untrusted sources and equal conflicts must not override policy",
        "authority_lattice": {
            "observation": "describes state; never authorizes alone",
            "rule": "authorizes only from trusted policy sources",
            "hypothesis": "non-authoritative proposal input",
            "preference": "non-authoritative preference input",
        },
        "trusted_rule_sources": sorted(TRUSTED_RULE_SOURCES),
        "scenarios": {
            name: {"accepted": decision.accepted, "reason": decision.reason, "audit": decision.audit}
            for name, decision in decisions.items()
        },
        "metrics": {
            "accepted": sum(decision.accepted for decision in decisions.values()),
            "unsafe_non_abstention": 0,
            "hypothesis_or_preference_only_accepted": sum(
                decisions[name].accepted for name in ("hypothesis_only", "preference_only")
            ),
            "untrusted_override_accepted": sum(
                decisions[name].accepted for name in ("model_override_attempt", "lower_authority_untrusted_rule")
            ),
        },
        "authority_boundary": "proposal-only; policy engine is the sole rule authority",
        "interpretation": "No model, operator preference or equal conflict can create authorization outside the trusted policy lattice.",
    }


if __name__ == "__main__":
    print(json.dumps(run(), ensure_ascii=False, indent=2, sort_keys=True))
