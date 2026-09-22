"""Deterministic hosts used to test host-independent capability discovery.

The hosts intentionally expose different primitive names while preserving an
abstract semantic state.  This makes transfer measurable instead of relying on
matching names or a hand-written host profile.
"""
from __future__ import annotations

from dataclasses import dataclass, field

from .core import Observation, PrimitiveAction, State


@dataclass
class ToyHost:
    host_id: str
    action_names: tuple[str, ...]
    effects_by_name: dict[str, tuple[tuple[str, int], ...]]
    initial: dict[str, int] = field(default_factory=lambda: {"x": 0, "y": 0})
    _state: State = field(init=False)
    _sequence: int = field(default=0, init=False)
    _last: PrimitiveAction | None = field(default=None, init=False)

    def __post_init__(self) -> None:
        self._state = State.from_dict(self.initial)

    def resources(self) -> tuple[str, ...]:
        return ("state:x", "state:y", "safe_action_bus")

    def safe_action_space(self) -> tuple[PrimitiveAction, ...]:
        return tuple(PrimitiveAction(name) for name in self.action_names)

    def observe(self) -> Observation:
        return Observation.make(self._sequence, self._state, self._last)

    def execute(self, action: PrimitiveAction) -> Observation:
        if action.action_id not in self.effects_by_name:
            raise ValueError(f"unknown action: {action.action_id}")
        values = self._state.to_dict()
        for key, delta in self.effects_by_name[action.action_id]:
            values[key] = values.get(key, 0) + delta
        self._state = State.from_dict(values)
        self._sequence += 1
        self._last = action
        return self.observe()

    def reset(self) -> None:
        self._state = State.from_dict(self.initial)
        self._last = None
        self._sequence = 0


class BlackBoxHost:
    """Public-only wrapper used to catch accidental model/introspection access."""

    def __init__(self, inner: ToyHost) -> None:
        self._inner = inner
        self.execute_count = 0

    @property
    def host_id(self) -> str:
        return self._inner.host_id

    def resources(self) -> tuple[str, ...]:
        return self._inner.resources()

    def safe_action_space(self) -> tuple[PrimitiveAction, ...]:
        return self._inner.safe_action_space()

    def observe(self) -> Observation:
        return self._inner.observe()

    def execute(self, action: PrimitiveAction) -> Observation:
        self.execute_count += 1
        return self._inner.execute(action)

    def reset(self) -> None:
        self._inner.reset()

    @property
    def effects_by_name(self) -> dict[str, tuple[tuple[str, int], ...]]:
        raise AssertionError("black-box host internals must not be inspected")


def host_a() -> ToyHost:
    return ToyHost(
        "host-A",
        ("left", "right", "up"),
        {"left": (("x", 1),), "right": (("x", -1),), "up": (("y", 1),)},
    )


def host_b() -> ToyHost:
    return ToyHost(
        "host-B",
        ("servo7", "servo8", "pitch"),
        {"servo7": (("x", 1),), "servo8": (("x", -1),), "pitch": (("y", 1),)},
    )


def host_conflict() -> ToyHost:
    # Used with an injected mutable effect map to produce conflicting evidence.
    return ToyHost(
        "host-conflict",
        ("toggle",),
        {"toggle": (("x", 1),)},
    )
