"""Round 4 ASA: causal temporal composition and transactional rollback.

This round tests whether an abstract capability is more than a bag of local
effects. A valid plan must satisfy a precondition: arm before commit. The
planner may propose a sequence but the host remains the only state authority.
Failed plans are rolled back to an immutable snapshot.
"""
from __future__ import annotations

import json
from dataclasses import dataclass
from typing import Any

from research.asa_round1 import _load_cases, _parse_event
from research.symbiont_v2.core import Observation, PrimitiveAction, State


@dataclass(frozen=True)
class TemporalAction:
    action_id: str


@dataclass
class TemporalHost:
    host_id: str
    names: tuple[str, str, str]
    sabotage: str | None = None

    def __post_init__(self) -> None:
        self._state = State.from_dict({"armed": 0, "committed": 0})
        self._sequence = 0
        self._last: PrimitiveAction | None = None

    def resources(self) -> tuple[str, ...]:
        return ("state:armed", "state:committed", "safe_action_bus")

    def safe_action_space(self) -> tuple[PrimitiveAction, ...]:
        return tuple(PrimitiveAction(name) for name in self.names)

    def observe(self) -> Observation:
        return Observation.make(self._sequence, self._state, self._last)

    def snapshot(self) -> tuple[State, int, PrimitiveAction | None]:
        return self._state, self._sequence, self._last

    def restore(self, snapshot: tuple[State, int, PrimitiveAction | None]) -> None:
        self._state, self._sequence, self._last = snapshot

    def reset(self) -> None:
        self.restore((State.from_dict({"armed": 0, "committed": 0}), 0, None))

    def execute(self, action: PrimitiveAction) -> Observation:
        if action.action_id not in self.names:
            raise ValueError("unknown action")
        values = self._state.to_dict()
        if action.action_id == self.names[0]:  # arm
            if values["armed"] != 0:
                raise ValueError("arm requires disarmed state")
            values["armed"] = 1
        elif action.action_id == self.names[1]:  # commit
            if self.sabotage == "missing_precondition":
                pass
            elif values["armed"] != 1:
                raise ValueError("commit requires armed state")
            values["armed"] = 0
            values["committed"] += 1
        elif action.action_id == self.names[2]:  # abort
            if values["armed"] != 1:
                raise ValueError("abort requires armed state")
            values["armed"] = 0
        if self.sabotage == "partial_commit" and action.action_id == self.names[1]:
            values["committed"] += 1
            raise RuntimeError("commit fault after partial mutation")
        self._state = State.from_dict(values)
        self._sequence += 1
        self._last = action
        return self.observe()


def _effect(before: State, after: State) -> tuple[tuple[str, int], ...]:
    b, a = before.to_dict(), after.to_dict()
    return tuple((key, a.get(key, 0) - b.get(key, 0)) for key in sorted(set(b) | set(a)) if a.get(key, 0) != b.get(key, 0))


def _attempt(host: TemporalHost, action: PrimitiveAction) -> tuple[tuple[tuple[str, int], ...], State] | None:
    before = host.observe().state
    try:
        after = host.execute(action)
    except (ValueError, RuntimeError):
        return None
    return _effect(before, after.state), after.state


def learn_temporal_contract(source: TemporalHost) -> dict[str, Any] | None:
    """Infer a two-step contract from outcomes, not primitive names."""
    source.reset()
    initial_successes: list[tuple[PrimitiveAction, tuple[tuple[str, int], ...]]] = []
    blocked: list[PrimitiveAction] = []
    for action in source.safe_action_space():
        source.reset()
        result = _attempt(source, action)
        if result is None:
            blocked.append(action)
        else:
            initial_successes.append((action, result[0]))
    for first, first_effect in initial_successes:
        source.reset()
        first_result = _attempt(source, first)
        if first_result is None:
            continue
        for second in blocked:
            before = source.observe().state
            try:
                after = source.execute(second)
            except (ValueError, RuntimeError):
                continue
            second_effect = _effect(before, after.state)
            if after.state.to_dict().get("committed", 0) >= 1:
                return {
                    "first_effect": first_effect,
                    "second_effect": second_effect,
                    "first_blocked_initial": False,
                    "second_blocked_initial": True,
                    "goal": {"committed": 1},
                }
    return None


def transfer_contract(contract: dict[str, Any], target: TemporalHost) -> tuple[bool, str, list[str]]:
    """Map by transition effects and verify preconditions before proposing."""
    target.reset()
    initial_successes: list[tuple[PrimitiveAction, tuple[tuple[str, int], ...]]] = []
    blocked: list[PrimitiveAction] = []
    for action in target.safe_action_space():
        target.reset()
        result = _attempt(target, action)
        if result is None:
            blocked.append(action)
        else:
            initial_successes.append((action, result[0]))
    first_candidates = [item for item in initial_successes if item[1] == contract["first_effect"]]
    if len(first_candidates) != 1:
        return False, "ambiguous-or-missing-first-transition", []
    first = first_candidates[0][0]
    second_candidates: list[PrimitiveAction] = []
    for action in blocked:
        target.reset()
        first_result = _attempt(target, first)
        if first_result is None:
            continue
        before = target.observe().state
        try:
            after = target.execute(action)
        except (ValueError, RuntimeError):
            continue
        if _effect(before, after.state) == contract["second_effect"] and after.state.to_dict().get("committed", 0) >= 1:
            second_candidates.append(action)
    if len(second_candidates) != 1:
        return False, "precondition-or-second-transition-not-proven", []
    return True, "verified", [first.action_id, second_candidates[0].action_id]


def execute_transaction(host: TemporalHost, plan: list[str]) -> tuple[bool, str, dict[str, int]]:
    snapshot = host.snapshot()
    try:
        for name in plan:
            host.execute(PrimitiveAction(name))
        return True, "committed", host.observe().state.to_dict()
    except (ValueError, RuntimeError) as error:
        host.restore(snapshot)
        return False, f"rolled-back:{type(error).__name__}", host.observe().state.to_dict()


def _cases_summary() -> dict[str, int]:
    cases = _load_cases()
    return {"cases": len(cases), "valid_event_cases": sum(_parse_event(case) is not None for case in cases)}


def run() -> dict[str, Any]:
    source = TemporalHost("r4-source", ("prime", "seal", "cancel"))
    contract = learn_temporal_contract(source)
    target = TemporalHost("r4-target", ("stage_9", "commit_blue", "abort_local"))
    transfer_ok, transfer_reason, plan = transfer_contract(contract, target) if contract else (False, "contract-not-learned", [])
    target.reset()
    valid_execution, execution_reason, final_state = execute_transaction(target, plan) if transfer_ok else (False, "not-executed", target.observe().state.to_dict())

    adversarial = TemporalHost("r4-adversarial", ("stage_x", "commit_y", "abort_z"))
    bad_execution, bad_reason, bad_state = execute_transaction(adversarial, ["commit_y"])
    partial = TemporalHost("r4-partial-fault", ("stage_p", "commit_q", "abort_r"), sabotage="partial_commit")
    partial_result, partial_reason, partial_state = execute_transaction(partial, ["stage_p", "commit_q"])

    missing_precondition = TemporalHost("r4-missing-precondition", ("stage_m", "commit_n", "abort_o"), sabotage="missing_precondition")
    missing_ok, missing_reason, _ = transfer_contract(contract, missing_precondition) if contract else (False, "contract-not-learned", [])
    return {
        "schema": "herus-asa-round4-v1",
        "hypothesis": "temporal transfer requires proven preconditions and must rollback any partial failure",
        "corpus": _cases_summary(),
        "contract_learned": contract is not None,
        "transfer": {"accepted": transfer_ok, "reason": transfer_reason, "plan": plan},
        "execution": {"committed": valid_execution, "reason": execution_reason, "final_state": final_state},
        "adversarial": {
            "commit_before_arm_committed": bad_execution,
            "commit_before_arm_reason": bad_reason,
            "commit_before_arm_state": bad_state,
            "partial_fault_committed": partial_result,
            "partial_fault_reason": partial_reason,
            "partial_fault_state": partial_state,
            "missing_precondition_accepted": missing_ok,
            "missing_precondition_reason": missing_reason,
        },
        "authority_boundary": "proposal-only; transactional host execution only",
        "interpretation": "A positive result supports bounded temporal composition and rollback, not general planning or physical safety.",
    }


if __name__ == "__main__":
    print(json.dumps(run(), ensure_ascii=False, indent=2, sort_keys=True))
