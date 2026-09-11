"""Active, iterative host discovery using the finite probe planner."""
from __future__ import annotations

from dataclasses import dataclass

from host_discovery import DiscoveryBudget, DiscoveryResult, HostHypothesis, discover_host
from host_discovery_lab import HostOracle


@dataclass(frozen=True)
class ActiveDiscoveryConfig:
    candidate_formats: tuple[str, ...]
    candidate_interfaces: tuple[str, ...]
    latency_targets: tuple[str, ...]
    budget: DiscoveryBudget


def run_active_discovery(oracle: HostOracle, config: ActiveDiscoveryConfig) -> DiscoveryResult:
    """Run the current bounded discovery protocol.

    The planner is exposed as a separate deterministic policy and is tested
    independently. This adapter keeps the existing evidence validator as the
    single authority for digest, sequence, budget and conflict handling.
    """
    return discover_host(
        oracle,
        candidate_formats=config.candidate_formats,
        candidate_interfaces=config.candidate_interfaces,
        latency_targets=config.latency_targets,
        budget=config.budget,
    )
