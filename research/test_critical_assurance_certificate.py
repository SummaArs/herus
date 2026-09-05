import unittest

from critical_assurance_certificate import AssuranceVerdict, compose_assurance
from critical_call_path_audit import CallPathResult
from critical_state_verifier import PolicyRule, StateMachineSpec, Transition


class CriticalAssuranceCertificateTests(unittest.TestCase):
    def setUp(self) -> None:
        self.abstract = StateMachineSpec(
            initial_state="safe",
            states=frozenset({"safe", "hold"}),
            inputs=frozenset({"ok", "loss"}),
            actions=frozenset({"hold", "safe_hold"}),
            transitions=(
                Transition("safe", "ok", "hold", "safe"),
                Transition("safe", "loss", "safe_hold", "hold"),
                Transition("hold", "ok", "safe_hold", "hold"),
                Transition("hold", "loss", "safe_hold", "hold"),
            ),
            forbidden_states=frozenset(), forbidden_actions=frozenset(), max_steps=2,
        )
        self.concrete = StateMachineSpec(
            initial_state="c_safe",
            states=frozenset({"c_safe", "c_hold"}),
            inputs=frozenset({"packet_ok", "packet_loss"}),
            actions=frozenset({"retain", "safe_retain"}),
            transitions=(
                Transition("c_safe", "packet_ok", "retain", "c_safe"),
                Transition("c_safe", "packet_loss", "safe_retain", "c_hold"),
                Transition("c_hold", "packet_ok", "safe_retain", "c_hold"),
                Transition("c_hold", "packet_loss", "safe_retain", "c_hold"),
            ),
            forbidden_states=frozenset(), forbidden_actions=frozenset(), max_steps=2,
        )
        self.abstract_policy = (
            PolicyRule("safe", "ok", "hold"), PolicyRule("safe", "loss", "safe_hold"),
            PolicyRule("hold", "ok", "safe_hold"), PolicyRule("hold", "loss", "safe_hold"),
        )
        self.concrete_policy = (
            PolicyRule("c_safe", "packet_ok", "retain"), PolicyRule("c_safe", "packet_loss", "safe_retain"),
            PolicyRule("c_hold", "packet_ok", "safe_retain"), PolicyRule("c_hold", "packet_loss", "safe_retain"),
        )
        self.maps = (
            {"c_safe": "safe", "c_hold": "hold"},
            {"packet_ok": "ok", "packet_loss": "loss"},
            {"retain": "hold", "safe_retain": "safe_hold"},
        )
        self.covered = (CallPathResult("send", "COVERED", "interaction.c", "assured", "ok"),)

    def test_full_finite_chain_is_assured(self) -> None:
        result = compose_assurance(self.abstract, self.concrete, self.abstract_policy, self.concrete_policy, *self.maps, self.covered)
        self.assertEqual(result.verdict, AssuranceVerdict.ASSURED)
        self.assertEqual(len(result.evidence_digest), 64)

    def test_structural_extraction_failure_blocks_promotion(self) -> None:
        result = compose_assurance(
            self.abstract, self.concrete, self.abstract_policy, self.concrete_policy,
            *self.maps, self.covered,
            structural_verdict="DIVERGENCE",
            structural_reason="required_observation_missing",
        )
        self.assertEqual(result.verdict, AssuranceVerdict.BLOCKED)
        self.assertEqual(result.reason, "structural_extraction_not_promoted")

    def test_uncovered_call_path_blocks(self) -> None:
        result = compose_assurance(self.abstract, self.concrete, self.abstract_policy, self.concrete_policy, *self.maps, (CallPathResult("send", "UNCOVERED", "interaction.c", "bypass", "bad"),))
        self.assertEqual(result.verdict, AssuranceVerdict.BLOCKED)

    def test_policy_counterexample_is_not_assured(self) -> None:
        bad = self.concrete_policy[:-1] + (PolicyRule("c_hold", "packet_loss", "retain"),)
        executable_concrete = StateMachineSpec(
            initial_state=self.concrete.initial_state,
            states=self.concrete.states,
            inputs=self.concrete.inputs,
            actions=self.concrete.actions,
            transitions=self.concrete.transitions + (Transition("c_hold", "packet_loss", "retain", "c_hold"),),
            forbidden_states=self.concrete.forbidden_states,
            forbidden_actions=self.concrete.forbidden_actions,
            max_steps=self.concrete.max_steps,
        )
        result = compose_assurance(self.abstract, executable_concrete, self.abstract_policy, bad, *self.maps, self.covered)
        self.assertEqual(result.verdict, AssuranceVerdict.COUNTEREXAMPLE)
        self.assertEqual(result.reason, "policy_refinement_counterexample")

    def test_missing_verification_is_unknown(self) -> None:
        result = compose_assurance(self.abstract, self.concrete, self.abstract_policy[:-1], self.concrete_policy, *self.maps, self.covered)
        self.assertEqual(result.verdict, AssuranceVerdict.UNKNOWN)


if __name__ == "__main__":
    unittest.main()
