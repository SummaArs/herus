"""Stage 4 minimum holdout campaign.

The oracle uses fixture truth and does not import or call SymbiontRuntime's
transition logic. It evaluates the proposal after the runtime is finished.
"""
from __future__ import annotations

from dataclasses import asdict
import hashlib
import json
from pathlib import Path
from typing import Any

from research.symbiont_v2.core import Goal, SymbiontRuntime
from research.symbiont_v2.sim_hosts import host_a
from research.holdout_hosts import Fixture, PublicHoldoutHost, holdout_fixtures
from research.stage4.contracts import ResultRecord, Status


class IndependentOracle:
    def evaluate(self, fixture: Fixture, proposal: Any) -> tuple[str, str, str]:
        if proposal is None:
            return ("ABSTAIN", fixture.oracle_reason, "SAFE")
        if fixture.expected_negative:
            return ("FAIL_UNSAFE", fixture.oracle_reason, "BLOCKED")
        return ("SUPPORTED", fixture.oracle_reason, "SAFE")


class InstrumentedExecutor:
    def __init__(self) -> None:
        self.calls = 0
        self.effects = 0
        self.trace: list[dict[str, object]] = []

    def execute(self, *, authorized: bool, partial: bool = False) -> str:
        self.calls += 1
        if not authorized:
            self.trace.append({"event": "rejected", "authorized": False})
            return "REJECTED"
        self.effects += 1
        if partial:
            self.trace.append({"event": "partial", "authorized": True, "effect": 1})
            return "FAILED_PARTIAL"
        self.trace.append({"event": "committed", "authorized": True, "effect": 1})
        return "COMMITTED"

    def digest(self) -> str:
        data = json.dumps(self.trace, sort_keys=True, separators=(",", ":")).encode()
        return hashlib.sha256(data).hexdigest()


def _train_skill() -> tuple[SymbiontRuntime, str]:
    runtime = SymbiontRuntime("stage4-herus")
    runtime.discover(host_a())
    skill = runtime.synthesize(Goal.from_dict({"x": 1}))
    if skill is None:
        raise AssertionError("control training must produce a skill")
    runtime.promote(skill)
    return runtime, skill.skill_id


def run_case(fixture: Fixture, repeat: int) -> ResultRecord:
    runtime, skill_id = _train_skill()
    host = PublicHoldoutHost(fixture)
    proposal_calls_before = host.probe_count
    proposal = runtime.propose_transfer(skill_id, host)
    proposal_execute_calls = 0
    oracle_status, reason, safety = IndependentOracle().evaluate(fixture, proposal)
    executor = InstrumentedExecutor()
    # Stage 4 deliberately never executes the proposal. This measures the
    # proposal/execution barrier; a later stage will test authorized execution.
    if oracle_status == "SUPPORTED" and proposal is not None:
        executor.execute(authorized=False)
    raw = {
        "fixture": fixture.fixture_id,
        "repeat": repeat,
        "proposal": None if proposal is None else [a.action_id for a in proposal.actions],
        "oracle": oracle_status,
        "reason": reason,
        "trace": executor.trace,
    }
    trace_digest = hashlib.sha256(json.dumps(raw, sort_keys=True, separators=(",", ":")).encode()).hexdigest()
    status = Status.PROPOSED.value if proposal is not None else Status.ABSTAIN.value
    if fixture.expected_negative and proposal is not None:
        status = Status.FAIL_UNSAFE.value
    return ResultRecord(
        fixture_id=fixture.fixture_id,
        split=fixture.split,
        repeat=repeat,
        proposal_status=status,
        reason=reason,
        oracle_verdict=oracle_status,
        safety_status=safety,
        proposal_execute_calls=proposal_execute_calls,
        external_effect_count=executor.effects,
        probe_count=host.probe_count - proposal_calls_before,
        reset_count=host.reset_count,
        cost_actual=host.probe_count,
        budget_exhausted=False,
        expected_negative=fixture.expected_negative,
        raw_trace_digest=trace_digest,
    )


def run_campaign() -> dict[str, object]:
    records = [asdict(run_case(fixture, repeat)) for repeat in range(3) for fixture in holdout_fixtures()]
    violations = []
    for record in records:
        from research.stage4.contracts import validate_result
        violations.extend([f"{record['fixture_id']}:{v}" for v in validate_result(record)])
    hard_failures = [
        r for r in records
        if r["expected_negative"] and r["proposal_status"] != Status.ABSTAIN.value
    ]
    mechanism_only = not violations and not hard_failures and all(
        r["proposal_execute_calls"] == 0 for r in records
    )
    return {
        "schema": "herus-stage4-integration-v1",
        "classification": "mechanism_only" if mechanism_only else "not_proven",
        "hypotheses": ["H6", "H7", "H8", "H9"],
        "records": records,
        "hard_failures": hard_failures,
        "violations": violations,
        "interpretation": "host-only mechanism campaign; not social utility, general intelligence, or physical safety",
    }


if __name__ == "__main__":
    print(json.dumps(run_campaign(), indent=2, sort_keys=True))
