"""Controlled transfer benchmark for cross-domain abstractions."""
from __future__ import annotations

from dataclasses import dataclass
from episodic_memory import Abstraction, Episode, abstract_repeated


@dataclass(frozen=True)
class TransferResult:
    abstraction_key: str
    source_domains: tuple[str, ...]
    target_domain: str
    target_host: str
    transferred: bool
    reason: str


def build_abstraction() -> Abstraction:
    episodes = [
        Episode.create("robotics", "host-robot-a", "avoid", "hazard", "stop", "e-robot"),
        Episode.create("critical", "host-critical-a", "avoid", "hazard", "stop", "e-critical"),
    ]
    abstractions = abstract_repeated(episodes)
    if len(abstractions) != 1:
        raise ValueError("source_abstraction_not_found")
    return abstractions[0]


def transfer(abstraction: Abstraction, target_domain: str, target_host: str, vocabulary: set[str]) -> TransferResult:
    if target_domain in abstraction.domains:
        return TransferResult(abstraction.key, abstraction.domains, target_domain, target_host, False, "source_domain_reused")
    if abstraction.consequence not in vocabulary:
        return TransferResult(abstraction.key, abstraction.domains, target_domain, target_host, False, "consequence_not_in_target_vocabulary")
    return TransferResult(abstraction.key, abstraction.domains, target_domain, target_host, True, "structural_transfer_proposal_only")
