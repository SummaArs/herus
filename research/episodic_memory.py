"""Bounded episodic memory for HERUS competence growth."""
from __future__ import annotations

from dataclasses import dataclass
from hashlib import sha256
import json
from typing import Iterable


@dataclass(frozen=True)
class Episode:
    episode_id: str
    domain: str
    host_digest: str
    goal: str
    observation: str
    outcome: str
    evidence_digest: str
    trusted: bool

    @staticmethod
    def create(domain: str, host_digest: str, goal: str, observation: str, outcome: str, evidence_digest: str) -> "Episode":
        payload = {"domain": domain, "host": host_digest, "goal": goal, "observation": observation, "outcome": outcome, "evidence": evidence_digest}
        episode_id = sha256(json.dumps(payload, sort_keys=True, separators=(",", ":")).encode()).hexdigest()
        return Episode(episode_id, domain, host_digest, goal, observation, outcome, evidence_digest, True)


@dataclass(frozen=True)
class Abstraction:
    key: str
    domains: tuple[str, ...]
    precondition: str
    consequence: str
    source_episodes: tuple[str, ...]
    confidence: float


def abstract_repeated(episodes: Iterable[Episode], minimum: int = 2) -> tuple[Abstraction, ...]:
    trusted = [episode for episode in episodes if episode.trusted and episode.evidence_digest]
    groups: dict[tuple[str, str], list[Episode]] = {}
    for episode in trusted:
        groups.setdefault((episode.observation, episode.outcome), []).append(episode)
    abstractions: list[Abstraction] = []
    for (observation, outcome), rows in groups.items():
        domains = tuple(sorted({row.domain for row in rows}))
        if len(rows) < minimum or len(domains) < 2:
            continue
        key = sha256(f"{observation}|{outcome}".encode()).hexdigest()[:16]
        confidence = min(1.0, len(rows) / (len(rows) + 1))
        abstractions.append(Abstraction(key, domains, observation, outcome, tuple(row.episode_id for row in rows), confidence))
    return tuple(abstractions)
