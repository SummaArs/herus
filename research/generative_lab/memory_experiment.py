"""Reproducible Skill memory utility and pruning experiment."""
from __future__ import annotations

import json
from dataclasses import replace

from .skill_library import SkillLibrary
from .skill_memory import SkillMemory, utility_score
from .skills import SkillState, candidate_skill


def run() -> dict[str, object]:
    library = SkillLibrary()
    primary = replace(
        candidate_skill(skill_id="memory-primary", input_type="Int", output_type="Int", program=("INPUT",), provenance={"kind": "fixture"}).with_state(SkillState.VERIFIED),
        reliability=1.0,
        generalization_score=1.0,
    )
    duplicate = replace(
        candidate_skill(skill_id="memory-duplicate", input_type="Int", output_type="Int", program=("INPUT",), provenance={"kind": "fixture"}).with_state(SkillState.VERIFIED),
        reliability=0.9,
        generalization_score=0.9,
    )
    library.add(primary)
    library.add(duplicate)
    memory = SkillMemory(library)
    memory.record_use("memory-primary")
    before = {skill.skill_id: utility_score(skill) for skill in library.all()}
    archived = memory.archive_redundant()
    after = {skill.skill_id: library.get(skill.skill_id).state.value for skill in library.all()}
    return {
        "schema": "herus.generative_lab.memory_experiment",
        "version": 1,
        "authority": "none",
        "before_utility": before,
        "archived": list(archived),
        "after_state": after,
        "evidence_preserved": library.get("memory-duplicate").content_hash() == duplicate.content_hash(),
        "claims": [
            "Reuse count can contribute to utility without replacing correctness evidence.",
            "Exact redundant skills can be archived conservatively.",
            "Archival preserves the skill record and its content hash.",
        ],
    }


if __name__ == "__main__":
    print(json.dumps(run(), indent=2, sort_keys=True))
