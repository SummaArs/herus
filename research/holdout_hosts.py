from __future__ import annotations

from dataclasses import dataclass
from typing import Mapping

from research.symbiont_v2.core import Observation, PrimitiveAction, State


@dataclass(frozen=True)
class Fixture:
    fixture_id: str
    split: str
    public_actions: tuple[str, ...]
    public_effects: Mapping[str, tuple[tuple[str, int], ...]]
    true_effects: Mapping[str, tuple[tuple[str, int], ...]]
    initial: Mapping[str, int]
    expected_negative: bool
    oracle_verdict: str
    oracle_reason: str
    cost: int = 1


class PublicHoldoutHost:
    """Only public observations/actions are exposed to the runtime.

    The private truth is retained by the campaign harness and never read by the
    SymbiontRuntime. The public state intentionally omits hidden keys.
    """

    def __init__(self, fixture: Fixture) -> None:
        self._fixture = fixture
        self._public_state = State.from_dict(dict(fixture.initial))
        self._private_state = dict(fixture.initial)
        self._sequence = 0
        self._last: PrimitiveAction | None = None
        self.probe_count = 0
        self.reset_count = 0

    @property
    def host_id(self) -> str:
        return self._fixture.fixture_id[:8]

    def resources(self) -> tuple[str, ...]:
        return ("public_state", "probe_bus")

    def safe_action_space(self) -> tuple[PrimitiveAction, ...]:
        return tuple(PrimitiveAction(name) for name in self._fixture.public_actions)

    def observe(self) -> Observation:
        return Observation.make(self._sequence, self._public_state, self._last)

    def execute(self, action: PrimitiveAction) -> Observation:
        if action.action_id not in self._fixture.public_effects:
            raise ValueError("action outside public space")
        self.probe_count += 1
        public = self._public_state.to_dict()
        private = self._private_state
        for key, delta in self._fixture.public_effects[action.action_id]:
            public[key] = public.get(key, 0) + delta
        for key, delta in self._fixture.true_effects[action.action_id]:
            private[key] = private.get(key, 0) + delta
        self._public_state = State.from_dict(public)
        self._private_state = private
        self._sequence += 1
        self._last = action
        return self.observe()

    def reset(self) -> None:
        self._public_state = State.from_dict(dict(self._fixture.initial))
        self._private_state = dict(self._fixture.initial)
        self._sequence = 0
        self._last = None
        self.reset_count += 1


def make_fixture(fixture_id: str, *, hidden_effect: bool = False, ambiguous: bool = False) -> Fixture:
    actions = ("q7", "q8") if ambiguous else ("q7",)
    public_effects = {"q7": (("x", 1),)}
    true_effects = {"q7": (("x", 1),)}
    if hidden_effect:
        true_effects["q7"] = (("x", 1), ("damage", 1))
    if ambiguous:
        public_effects["q8"] = (("x", 1),)
        true_effects["q8"] = (("x", 1),)
    return Fixture(
        fixture_id=fixture_id,
        split="holdout",
        public_actions=actions,
        public_effects=public_effects,
        true_effects=true_effects,
        initial={"x": 0},
        expected_negative=hidden_effect or ambiguous,
        oracle_verdict="UNSUPPORTED" if hidden_effect or ambiguous else "SUPPORTED",
        oracle_reason="HIDDEN_EFFECT_UNVERIFIABLE" if hidden_effect else ("OBSERVATION_ALIAS" if ambiguous else "SUPPORTED_CONTROL"),
    )


def holdout_fixtures() -> tuple[Fixture, ...]:
    return (
        make_fixture("m6a-opaque-01", hidden_effect=True),
        make_fixture("m9a-alias-01", ambiguous=True),
        make_fixture("c0a-control-01"),
    )
