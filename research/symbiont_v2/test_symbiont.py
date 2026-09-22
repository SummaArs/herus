from __future__ import annotations

import unittest

from research.symbiont_v2.core import Effect, Evidence, Goal, State, SymbiontRuntime
from research.symbiont_v2.sim_hosts import BlackBoxHost, ToyHost, host_a, host_b


class SymbiontV2Tests(unittest.TestCase):
    def test_discovery_is_evidence_backed(self) -> None:
        runtime = SymbiontRuntime("herus-test")
        evidence = runtime.discover(host_a())
        self.assertEqual({e.action.action_id for e in evidence}, {"left", "right", "up"})
        self.assertEqual(evidence[0].effect.delta, (("x", 1),))
        self.assertTrue(all(e.before.to_dict() == {"x": 0, "y": 0} for e in evidence))

    def test_skill_must_verify_before_promotion(self) -> None:
        runtime = SymbiontRuntime("herus-test")
        runtime.discover(host_a())
        skill = runtime.synthesize(Goal.from_dict({"x": 1}))
        self.assertIsNotNone(skill)
        assert skill is not None
        self.assertEqual(skill.status, "VERIFIED")
        promoted = runtime.promote(skill)
        self.assertIn(promoted.skill_id, runtime.memory.verified_skills)

    def test_identity_survives_rebind_but_host_context_does_not(self) -> None:
        runtime = SymbiontRuntime("herus-test")
        runtime.discover(host_a())
        skill = runtime.synthesize(Goal.from_dict({"x": 1}))
        assert skill is not None
        runtime.promote(skill)
        old_epoch = runtime.host.model.epoch if runtime.host else 0
        runtime.bind(host_b())
        self.assertEqual(runtime.herus_id, "herus-test")
        assert runtime.host is not None
        self.assertGreater(runtime.host.model.epoch, old_epoch)
        self.assertEqual(runtime.host.evidence, [])
        self.assertIn(skill.skill_id, runtime.memory.verified_skills)

    def test_transfer_uses_effect_contract_not_action_names(self) -> None:
        runtime = SymbiontRuntime("herus-test")
        runtime.discover(host_a())
        skill = runtime.synthesize(Goal.from_dict({"x": 1}))
        assert skill is not None
        runtime.promote(skill)
        self.assertTrue(runtime.transfer(skill.skill_id, host_b()))

    def test_black_box_host_is_sufficient_for_discovery_and_transfer(self) -> None:
        runtime = SymbiontRuntime("herus-test")
        source = BlackBoxHost(host_a())
        runtime.discover(source)
        skill = runtime.synthesize(Goal.from_dict({"x": 1}))
        assert skill is not None
        runtime.promote(skill)
        target = BlackBoxHost(host_b())
        proposal = runtime.propose_transfer(skill.skill_id, target)
        self.assertIsNotNone(proposal)
        self.assertEqual(proposal.host_id if proposal else None, "host-B")
        self.assertEqual(target.execute_count, len(target.safe_action_space()))

    def test_transfer_proposal_never_executes_the_target_plan(self) -> None:
        runtime = SymbiontRuntime("herus-test")
        runtime.discover(host_a())
        skill = runtime.synthesize(Goal.from_dict({"x": 1}))
        assert skill is not None
        runtime.promote(skill)
        target = BlackBoxHost(host_b())
        proposal = runtime.propose_transfer(skill.skill_id, target)
        self.assertIsNotNone(proposal)
        # Only discovery probes execute; the proposed action is not sent.
        self.assertEqual(target.execute_count, 3)

    def test_ambiguous_effect_match_abstains(self) -> None:
        runtime = SymbiontRuntime("herus-test")
        runtime.discover(host_a())
        skill = runtime.synthesize(Goal.from_dict({"x": 1}))
        assert skill is not None
        runtime.promote(skill)
        ambiguous = ToyHost(
            "host-ambiguous",
            ("a", "b"),
            {"a": (("x", 1),), "b": (("x", 1),)},
        )
        self.assertIsNone(runtime.propose_transfer(skill.skill_id, BlackBoxHost(ambiguous)))

    def test_transfer_does_not_reuse_old_host_evidence(self) -> None:
        runtime = SymbiontRuntime("herus-test")
        runtime.discover(host_a())
        skill = runtime.synthesize(Goal.from_dict({"x": 1}))
        assert skill is not None
        runtime.promote(skill)
        empty_host = ToyHost("host-empty", (), {})
        self.assertFalse(runtime.transfer(skill.skill_id, empty_host))
        assert runtime.host is not None
        self.assertEqual(runtime.host.model.host_id, "host-empty")
        self.assertEqual(runtime.host.evidence, [])

    def test_conflicting_evidence_is_quarantined(self) -> None:
        runtime = SymbiontRuntime("herus-test")
        runtime.discover(host_a())
        assert runtime.host is not None
        first = runtime.host.evidence[0]
        conflicting = Evidence(
            episode_id="conflict",
            action=first.action,
            before=first.before,
            after=State.from_dict({"x": 2, "y": 0}),
            effect=Effect((("x", 2),)),
            provenance="adversarial",
        )
        runtime.host.evidence.append(conflicting)
        runtime.host.world.learn(conflicting)
        self.assertIsNone(runtime.host.world.predict(first.action.action_id, first.before))


if __name__ == "__main__":
    unittest.main()
