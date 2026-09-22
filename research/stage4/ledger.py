from __future__ import annotations
from dataclasses import dataclass
from .execution_contracts import _digest

@dataclass
class AppendOnlyLedger:
    scope: str
    events: list[dict] | None = None
    def __post_init__(self): self.events = [] if self.events is None else self.events
    def append(self, event: dict) -> str:
        entry = {'seq': len(self.events) + 1, 'parent': self.digest(), 'event': dict(event)}
        self.events.append(entry)
        return self.digest()
    def digest(self) -> str: return _digest({'scope': self.scope, 'events': self.events})
    def snapshot(self): return {'scope': self.scope, 'events': list(self.events), 'digest': self.digest()}
