"""Adversarial host profiles for HERUS ASA adaptation tests."""
from __future__ import annotations

from typing import Any

from host_profile import discover_profile


def base_profile() -> dict[str, Any]:
    return {
        "host_id": "pulse-001",
        "revision": "t3s3-v1",
        "resources": {"ram_bytes": 512000, "energy_uj": 1000},
        "interfaces": ["button", "haptic", "radio"],
        "constraints": {"max_skill_bytes": 64, "max_steps": 3},
        "representation_set": ["herus-wire-v1"],
        "skill_budget": {"bytes": 64, "steps": 3, "depth": 2},
        "evidence": {"source": "adversarial-fixture", "revision_digest": "fixture"},
    }


def hostile_profiles() -> dict[str, dict[str, Any]]:
    profiles = {}
    missing_haptic = base_profile()
    missing_haptic["host_id"] = "pulse-no-haptic"
    missing_haptic["interfaces"] = ["button", "radio"]
    profiles["missing_haptic"] = missing_haptic

    contradiction = base_profile()
    contradiction["host_id"] = "contradictory"
    contradiction["resources"] = {"ram_bytes": -1}
    profiles["negative_resource"] = contradiction

    unknown_interface = base_profile()
    unknown_interface["host_id"] = "unknown-interface"
    unknown_interface["interfaces"] = ["button", "quantum_port"]
    profiles["unknown_interface"] = unknown_interface

    no_wire = base_profile()
    no_wire["host_id"] = "no-wire"
    no_wire["representation_set"] = []
    profiles["no_representation"] = no_wire
    return profiles


def evaluate_profiles() -> dict[str, str]:
    return {
        name: "ACCEPTED" if discover_profile(raw) is not None else "BLOCKED"
        for name, raw in hostile_profiles().items()
    }
