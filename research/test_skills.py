from __future__ import annotations

import unittest
from dataclasses import replace

from generative_lab.generalization_benchmark import run as run_generalization
from generative_lab.skill_ir_bridge import skill_to_proposal
from generative_lab.skill_memory import SkillMemory, utility_score
from generative_lab.skill_library import SkillLibrary, compose_sequential
from generative_lab.skill_planner import retrieve_or_compose
from generative_lab.skill_verifier import ArithmeticCase, verify_arithmetic_skill
from generative_lab.skill_wire import decode_skill_program, encode_verified_skill
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

    def test_composition_is_verifiable(self) -> None:
        double = candidate_skill(skill_id="double", input_type="Int", output_type="Int", program=("INPUT", "CONST:2", "MUL"), provenance={"kind": "test"}).with_state(SkillState.VERIFIED)
        add_three = candidate_skill(skill_id="add-three", input_type="Int", output_type="Int", program=("INPUT", "CONST:3", "ADD"), provenance={"kind": "test"}).with_state(SkillState.VERIFIED)
        composed = compose_sequential(double, add_three, skill_id="double-then-add-three")
        result = verify_arithmetic_skill(composed, [ArithmeticCase(0, 3), ArithmeticCase(2, 7)], [ArithmeticCase(-4, -5)])
        self.assertTrue(result.passed)
        self.assertEqual(composed.program, ("INPUT", "CONST:2", "MUL", "CONST:3", "ADD"))

    def test_embedded_wire_round_trip_and_refusal(self) -> None:
        skill = candidate_skill(skill_id="wire", input_type="Int", output_type="Int", program=("INPUT", "CONST:2", "MUL"), provenance={"kind": "test"}).with_state(SkillState.VERIFIED)
        wire = encode_verified_skill(skill)
        self.assertEqual(decode_skill_program(wire), skill.program)
        with self.assertRaises(ValueError):
            decode_skill_program(wire[:4] + bytes([2]) + wire[5:])
        effectful = replace(skill, allowed_effects=("EXECUTE_ACTUATOR",))
        with self.assertRaises(ValueError):
            encode_verified_skill(effectful)

    def test_skill_ir_bridge_is_proposal_only(self) -> None:
        proposal_skill = candidate_skill(skill_id="intent-proposal", input_type="HIR", output_type="IntentProposal", program=("PROPOSAL",), provenance={"kind": "test"}).with_state(SkillState.VERIFIED)
        proposal_skill = replace(proposal_skill, evidence_hash="evidence-bound")
        proposal, issues = skill_to_proposal(proposal_skill, event_kind="HELP")
        self.assertIsNotNone(proposal)
        self.assertFalse(issues)
        self.assertTrue(proposal.proposal_only)
        refused = candidate_skill(skill_id="raw-int", input_type="Int", output_type="Int", program=("INPUT",), provenance={"kind": "test"}).with_state(SkillState.VERIFIED)
        proposal, issues = skill_to_proposal(refused, event_kind="HELP")
        self.assertIsNone(proposal)
        self.assertIn("output_type_not_proposal", issues)
        unverified = candidate_skill(skill_id="unverified", input_type="HIR", output_type="IntentProposal", program=("PROPOSAL",), provenance={"kind": "test"})
        proposal, issues = skill_to_proposal(unverified, event_kind="HELP")
        self.assertIsNone(proposal)
        self.assertIn("skill_not_verified", issues)

    def test_skill_memory_records_use_and_archives_redundancy(self) -> None:
        library = SkillLibrary()
        first = replace(candidate_skill(skill_id="memory-a", input_type="Int", output_type="Int", program=("INPUT",), provenance={"kind": "test"}).with_state(SkillState.VERIFIED), reliability=1.0, generalization_score=1.0)
        second = replace(candidate_skill(skill_id="memory-b", input_type="Int", output_type="Int", program=("INPUT",), provenance={"kind": "test"}).with_state(SkillState.VERIFIED), reliability=1.0, generalization_score=1.0)
        library.add(first)
        library.add(second)
        memory = SkillMemory(library)
        memory.record_use("memory-a")
        self.assertGreater(utility_score(library.get("memory-a")), utility_score(library.get("memory-b")))
        archived = memory.archive_redundant()
        self.assertEqual(archived, ("memory-b@1",))
        self.assertEqual(library.get("memory-a").state, SkillState.VERIFIED)
        self.assertEqual(library.get("memory-b").state, SkillState.ARCHIVED)

    def test_multi_task_generalization_benchmark(self) -> None:
        result = run_generalization()
        self.assertTrue(result["all_tasks_verified"])
        self.assertEqual(len(result["tasks"]), 3)
        self.assertTrue(all(row["hidden_passed"] == 3 for row in result["tasks"]))

    def test_retrieve_first_composes_verified_skills(self) -> None:
        library = SkillLibrary()
        double = candidate_skill(skill_id="double-plan", input_type="Int", output_type="Int", program=("INPUT", "CONST:2", "MUL"), provenance={"kind": "test"}).with_state(SkillState.VERIFIED)
        add_three = candidate_skill(skill_id="add-three-plan", input_type="Int", output_type="Int", program=("INPUT", "CONST:3", "ADD"), provenance={"kind": "test"}).with_state(SkillState.VERIFIED)
        library.add(double)
        library.add(add_three)
        skill, metrics = retrieve_or_compose(
            library,
            input_type="Int",
            output_type="Token",
            visible=(ArithmeticCase(0, 3), ArithmeticCase(2, 7)),
            hidden=(ArithmeticCase(-4, -5),),
            composed_id="planned-affine",
        )
        self.assertIsNotNone(skill)
        self.assertEqual(metrics["verified_compositions"], 1)
        self.assertEqual(skill.program, ("INPUT", "CONST:2", "MUL", "CONST:3", "ADD"))

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
