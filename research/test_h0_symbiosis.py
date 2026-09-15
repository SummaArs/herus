from __future__ import annotations
import unittest

from h0_reference import H0Input, evaluate
from host_profile import HostProfile
from symbiotic_models import PersistentIdentity, SymbioticState, WorldModel, WorldObservation


class H0SymbiosisTests(unittest.TestCase):
    def profile(self, host_id: str) -> HostProfile:
        return HostProfile(
            host_id=host_id, revision="h0", resources={"ram_bytes": 4096},
            interfaces=frozenset({"button", "haptic", "radio"}), constraints={"max_steps": 12},
            representation_set=frozenset({"SIM-INT8"}), skill_budget={"bytes": 4096, "steps": 12},
            evidence={"identity": host_id},
        )

    def test_h0_enters_host_proposes_and_rebinds_without_leakage(self):
        state = SymbioticState(identity=PersistentIdentity("herus-h0")).attach(self.profile("computer"), skills={"observe"})
        decision = evaluate(H0Input("cycle", (4, 0, 0, 0), "sensor", "d" * 64, ("SIM-INT8",), 4096, 12))
        self.assertEqual(decision.proposal, "PROPOSE")
        self.assertEqual(decision.execution, "ABSTAIN")
        world = WorldModel().observe(WorldObservation("computer", "ready", "true", "fixture"))
        state = SymbioticState(state.identity, state.host, world, state.self_model)
        rebound = state.rebind(self.profile("second-computer"), skills={"observe-new"})
        self.assertEqual(rebound.identity.herus_id, "herus-h0")
        self.assertEqual(rebound.world.observations, ())
        self.assertEqual(rebound.propose("observe"), "ABSTAIN")
        self.assertEqual(rebound.propose("observe-new"), "PROPOSE")
        self.assertEqual(rebound.execute("observe-new"), "ABSTAIN")

    def test_h0_conflict_blocks_proposal_and_detach_clears_scope(self):
        state = SymbioticState(identity=PersistentIdentity("herus-h0")).attach(self.profile("computer"), skills={"observe"})
        world = WorldModel().observe(WorldObservation("door", "open", "true", "a")).observe(WorldObservation("door", "open", "false", "b"))
        conflicted = SymbioticState(state.identity, state.host, world, state.self_model)
        self.assertEqual(conflicted.propose("observe"), "ABSTAIN")
        detached = conflicted.detach()
        self.assertEqual(detached.propose("observe"), "ABSTAIN")
        self.assertEqual(detached.execute("observe"), "ABSTAIN")


if __name__ == "__main__":
    unittest.main()
