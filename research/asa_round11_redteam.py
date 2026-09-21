"""Deterministic bounded red-team campaign for the Round 10 protocol."""
from __future__ import annotations

from dataclasses import dataclass, replace
import json
from typing import Callable

from research.asa_round10 import ActionSignature, Evidence, Simulator, SimulatorExecutor, Verifier, capability_for, sample_contract


@dataclass(frozen=True)
class Case:
    case_id: str
    family: str
    expected: str
    run: Callable[[], str]


def _fixture():
    contract = sample_contract()
    evidence = (
        Evidence.issue(contract=contract, sequence=1, action_id="arm", argument="none", before={"armed": 0, "committed": 0}, after={"armed": 1, "committed": 0}),
        Evidence.issue(contract=contract, sequence=2, action_id="commit", argument="none", before={"armed": 1, "committed": 0}, after={"armed": 1, "committed": 1}),
    )
    verification = Verifier().verify(contract, evidence, now=1)
    return contract, evidence, verification, capability_for(contract)


def cases() -> tuple[Case, ...]:
    c, e, v, token = _fixture()
    assert v.attestation is not None
    valid = lambda: "ACCEPTED" if v.accepted else v.reason
    return (
        Case("R11-001", "positive", "ACCEPTED", valid),
        Case("R11-002", "expiry", "EXPIRED", lambda: Verifier().verify(c, e, now=100).reason),
        Case("R11-003", "expiry-extension", "SCOPE", lambda: SimulatorExecutor().run(c, replace(v.attestation, expires_at=1000), token, Simulator({"armed": 0, "committed": 0}), 101).code),
        Case("R11-004", "forged-evidence", "EVIDENCE_SIGNATURE", lambda: Verifier().verify(c, (replace(e[0], after=(("armed", 2), ("committed", 0))), e[1]), 1).reason),
        Case("R11-005", "replay", "REPLAY", lambda: _replay(c, v.attestation, token)),
        Case("R11-006", "forged-token", "CAPABILITY_PROOF", lambda: SimulatorExecutor().run(c, v.attestation, replace(token, proof="forged"), Simulator({"armed": 0, "committed": 0}), 1).code),
        Case("R11-007", "mixed-epoch", "IDENTITY_SEQUENCE", lambda: Verifier().verify(c, (replace(e[0], epoch=99), e[1]), 1).reason),
        Case("R11-008", "cross-contract", "IDENTITY_SEQUENCE", lambda: Verifier().verify(replace(c, host_digest="other"), e, 1).reason),
        Case("R11-009", "unknown-state", "UNKNOWN_STATE_KEY", lambda: Verifier().verify(replace(c, actions=(ActionSignature("bad", "none", (("unknown", 0),), (), ()),), max_steps=1), (), 1).reason),
        Case("R11-010", "external-effect", "EXTERNAL_EFFECT_BLOCKED", lambda: Verifier().verify(replace(c, actions=(ActionSignature("radio", "none", (), (), (), external_effect=True),), max_steps=1), (), 1).reason),
        Case("R11-011", "attacker-verifier", "VERIFIER_NOT_ALLOWED", lambda: SimulatorExecutor().run(c, replace(v.attestation, verifier_id="attacker"), token, Simulator({"armed": 0, "committed": 0}), 1).code),
        Case("R11-012", "bad-now", "NOW_TYPE", lambda: SimulatorExecutor().run(c, v.attestation, token, Simulator({"armed": 0, "committed": 0}), float("nan")).code),
        Case("R11-013", "bad-simulator", "SIMULATOR_TYPE", lambda: SimulatorExecutor().run(c, v.attestation, token, object(), 1).code),
        Case("R11-014", "state-digest", "STATE_DIGEST", lambda: SimulatorExecutor().run(c, v.attestation, token, Simulator({"armed": 1, "committed": 0}), 1).code),
        Case("R11-015", "action-collision", "ACTION_COLLISION", lambda: Verifier().verify(replace(c, actions=c.actions + (ActionSignature("arm", "other", (), (), ()),), max_steps=3), e, 1).reason),
    )


def _replay(contract, attestation, token) -> str:
    executor = SimulatorExecutor()
    executor.run(contract, attestation, token, Simulator({"armed": 0, "committed": 0}), 1)
    return executor.run(contract, attestation, token, Simulator({"armed": 0, "committed": 0}), 1).code


def run() -> dict:
    rows = []
    unhandled = 0
    for item in cases():
        try:
            observed = item.run()
        except Exception as error:  # campaign records unexpected failures instead of hiding them
            observed = f"UNHANDLED:{type(error).__name__}"
            unhandled += 1
        rows.append({"case_id": item.case_id, "family": item.family, "expected": item.expected, "observed": observed, "passed": observed == item.expected})
    positives = [row for row in rows if row["family"] == "positive"]
    negatives = [row for row in rows if row["family"] != "positive"]
    return {"schema": "herus-asa-round11-redteam-v1", "seed": "fixed-no-randomness", "cases": rows, "metrics": {"total": len(rows), "passed": sum(row["passed"] for row in rows), "positive_acceptance": f"{sum(row['observed'] == 'ACCEPTED' for row in positives)}/{len(positives)}", "negative_safe_abstention": f"{sum(row['passed'] for row in negatives)}/{len(negatives)}", "unsafe_decisions": f"{sum(row['observed'] == 'ACCEPTED' for row in negatives)}/{len(negatives)}", "unhandled_exceptions": unhandled}}


if __name__ == "__main__":
    print(json.dumps(run(), sort_keys=True, indent=2))
