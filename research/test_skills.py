from __future__ import annotations

import unittest

from generative_lab.skill_library import SkillLibrary, compose_sequential
from generative_lab.skill_verifier import ArithmeticCase, verify_arithmetic_skill
from generative_lab.skill_synthesis import synthesize_arithmetic_skill
from generative_lab.skills import SkillState, candidate_skill


class SkillContractTests(unittest.TestCase):
    def test_candidate_cannot_start_authorized(self) -> None:
        with self.assertRaises(ValueError):
            candidate_skill(
                skill_id="bad",
                input_type="Int",
                output_type="Int",
                program=("INPUT",),
                provenance={"kind": "test"},
            ).with_state(SkillState.AUTHORIZED)

    def test_library_requires_explicit_quarantine_transition(self) -> None:
        skill = candidate_skill(skill_id="id", input_type="Int", output_type="Int", program=("INPUT",), provenance={"kind": "test"})
        library = SkillLibrary()
        library.add(skill)
        with self.assertRaises(ValueError):
            library.transition("id", 1, SkillState.VERIFIED)
        library.transition("id", 1, SkillState.QUARANTINED)
        library.transition("id", 1, SkillState.TESTED)
        library.transition("id", 1, SkillState.VERIFIED)
        self.assertEqual(library.get("id").state, SkillState.VERIFIED)

    def test_composition_requires_matching_types(self) -> None:
        first = candidate_skill(skill_id="a", input_type="Int", output_type="Token", program=("INPUT",), provenance={"kind": "test"}).with_state(SkillState.TESTED)
        second = candidate_skill(skill_id="b", input_type="Token", output_type="Int", program=("INPUT",), provenance={"kind": "test"}).with_state(SkillState.TESTED)
        composed = compose_sequential(first, second, skill_id="ab")
        self.assertEqual(composed.input_type, "Int")
        self.assertEqual(composed.output_type, "Int")
        self.assertEqual(composed.state, SkillState.CANDIDATE)

    def test_attestation_is_bound_to_skill_content(self) -> None:
        skill = candidate_skill(skill_id="bound", input_type="Int", output_type="Int", program=("INPUT",), provenance={"kind": "test"})
        library = SkillLibrary()
        library.add(skill)
        library.transition("bound", 1, SkillState.QUARANTINED)
        library.transition("bound", 1, SkillState.TESTED)
        with self.assertRaises(ValueError):
            library.attest("bound", 1, evidence_hash="forged:3/3:3/3", generalization_score=1.0, reliability=1.0)

    def test_verifier_rejects_budget_exhaustion(self) -> None:
        skill = candidate_skill(skill_id="budget", input_type="Int", output_type="Int", program=("INPUT", "CONST:2", "MUL"), provenance={"kind": "test"}, resource_budget={"steps": 2, "memory": 2})
        result = verify_arithmetic_skill(skill, [ArithmeticCase(2, 4)], [ArithmeticCase(3, 6)])
        self.assertFalse(result.passed)
        self.assertTrue(result.counterexamples)

    def test_verifier_rejects_unknown_opcode_and_effect(self) -> None:
        unknown = candidate_skill(skill_id="unknown", input_type="Int", output_type="Int", program=("INPUT", "MAGIC"), provenance={"kind": "test"})
        result = verify_arithmetic_skill(unknown, [ArithmeticCase(1, 1)], [ArithmeticCase(2, 2)])
        self.assertFalse(result.passed)
        self.assertEqual(result.state, SkillState.TESTED)
        effectful = candidate_skill(skill_id="effect", input_type="Int", output_type="Int", program=("INPUT",), provenance={"kind": "test"})
        object.__setattr__(effectful, "allowed_effects", ("EXECUTE_ACTUATOR",))
        result = verify_arithmetic_skill(effectful, [ArithmeticCase(1, 1)], [ArithmeticCase(2, 2)])
        self.assertFalse(result.passed)
        self.assertEqual(result.reason, "effect_policy_violation")

    def test_synthesis_generalizes_to_hidden_cases(self) -> None:
        visible = [ArithmeticCase(0, 3), ArithmeticCase(1, 5), ArithmeticCase(2, 7)]
        hidden = [ArithmeticCase(-3, -3), ArithmeticCase(4, 11), ArithmeticCase(8, 19)]
        skill, metrics = synthesize_arithmetic_skill(skill_id="affine-2x-plus-3", visible=visible, hidden=hidden, max_program_length=7)
        self.assertIsNotNone(skill)
        self.assertEqual(skill.state, SkillState.CANDIDATE)
        self.assertGreater(metrics["rejected"], 0)

    def test_synthesis_rejects_memorization_without_hidden_generalization(self) -> None:
        visible = [ArithmeticCase(0, 3), ArithmeticCase(1, 5)]
        hidden = [ArithmeticCase(2, 999)]
        skill, _ = synthesize_arithmetic_skill(skill_id="must-fail", visible=visible, hidden=hidden, max_program_length=5)
        self.assertIsNone(skill)


if __name__ == "__main__":
    unittest.main()
