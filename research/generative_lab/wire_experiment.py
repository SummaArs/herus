"""Host-only embedded wire readiness experiment."""
from __future__ import annotations

import json

from .skill_wire import MAX_PROGRAM, MAX_WIRE_BYTES, decode_skill_program, encode_verified_skill
from .skills import SkillState, candidate_skill


def run() -> dict[str, object]:
    skill = candidate_skill(
        skill_id="embedded-affine",
        input_type="Int",
        output_type="Int",
        program=("INPUT", "CONST:2", "MUL", "CONST:3", "ADD"),
        provenance={"kind": "wire_fixture"},
    ).with_state(SkillState.VERIFIED)
    wire = encode_verified_skill(skill)
    decoded = decode_skill_program(wire)
    return {
        "schema": "herus.generative_lab.wire_experiment",
        "version": 1,
        "authority": "none",
        "program_length": len(skill.program),
        "max_program": MAX_PROGRAM,
        "wire_bytes": len(wire),
        "max_wire_bytes": MAX_WIRE_BYTES,
        "round_trip": decoded == skill.program,
        "contains_authority": False,
        "claims_not_supported": [
            "microcontroller timing",
            "flash durability",
            "radio transport",
            "power-loss recovery",
        ],
    }


if __name__ == "__main__":
    print(json.dumps(run(), indent=2, sort_keys=True))
