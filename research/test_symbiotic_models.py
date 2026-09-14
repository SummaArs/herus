from __future__ import annotations
import unittest

from host_profile import HostProfile
from symbiotic_models import (
    PersistentIdentity,
    SelfModel,
    SymbioticState,
    WorldModel,
    WorldObservation,
)


class SymbioticModelTests(unittest.TestCase):
    def profile(self, host_id: str = "host-a", revision: str = "r1") -> HostProfile:
        return HostProfile(
            host_id=host_id,
            revision=revision,
            resources={"ram_bytes": 128},
            interfaces=frozenset({"button", "haptic", "radio"}),
            constraints={"max_steps": 3},
            representation_set=frozenset({"hir-v1"}),
            skill_budget={"bytes": 64, "steps": 3},
            evidence={"source": "test", "identity": revision},
        )

    def test_host_change_preserves_herus_identity(self) -> None:
        identity = PersistentIdentity("herus-001")
        first = identity.bind(self.profile("host-a"))
        second = first.unbind().bind(self.profile("host-b"))
        self.assertEqual(first.herus_id, second.herus_id)
        self.assertNotEqual(first.host_id, second.host_id)
        self.assertEqual(second.identity_revision, 1)
        self.assertEqual(len(second.continuity_events), 3)

    def test_attach_builds_three_models_with_consistent_binding(self) -> None:
        state = SymbioticState(identity=PersistentIdentity("herus-001")).attach(
            self.profile(), skills={"observe"}
        )
        self.assertEqual(state.validate(), ())
        self.assertEqual(state.propose("observe"), "PROPOSE")
        self.assertEqual(state.execute("observe"), "ABSTAIN")

    def test_host_digest_drift_invalidates_snapshot(self) -> None:
        state = SymbioticState(identity=PersistentIdentity("herus-001")).attach(self.profile())
        changed = self.profile(revision="r2")
        drifted = SymbioticState(
            identity=state.identity,
            host=changed,
            world=state.world,
            self_model=state.self_model,
        )
        self.assertIn("identity_host_mismatch", drifted.validate())
        self.assertIn("self_model_host_mismatch", drifted.validate())
        self.assertEqual(drifted.execute("anything"), "ABSTAIN")

    def test_unbound_state_cannot_propose_or_execute(self) -> None:
        state = SymbioticState(identity=PersistentIdentity("herus-001")).attach(
            self.profile(), skills={"observe"}
        ).detach()
        self.assertEqual(state.validate(), ())
        self.assertEqual(state.propose("observe"), "ABSTAIN")
        self.assertEqual(state.execute("observe"), "ABSTAIN")

    def test_rebind_clears_old_world_and_skill_scope(self) -> None:
        first = SymbioticState(identity=PersistentIdentity("herus-001")).attach(
            self.profile("host-a"), skills={"observe-a"}
        )
        first = SymbioticState(
            identity=first.identity,
            host=first.host,
            world=WorldModel().observe(WorldObservation(
                subject="host-a", predicate="ready", value="true", source="fixture"
            )),
            self_model=first.self_model,
        )
        rebound = first.rebind(self.profile("host-b"), skills={"observe-b"})
        self.assertEqual(rebound.validate(), ())
        self.assertEqual(rebound.identity.herus_id, "herus-001")
        self.assertEqual(rebound.host.host_id, "host-b")
        self.assertEqual(rebound.world.observations, ())
        self.assertEqual(rebound.propose("observe-a"), "ABSTAIN")
        self.assertEqual(rebound.propose("observe-b"), "PROPOSE")
        self.assertEqual(rebound.execute("observe-b"), "ABSTAIN")

    def test_world_model_rejects_unproven_confirmation(self) -> None:
        with self.assertRaises(ValueError):
            WorldModel().observe(WorldObservation(
                subject="door", predicate="open", value="true",
                source="sensor", state="CONFIRMED", confidence="SUPPORTED",
            ))

    def test_world_digest_changes_on_append_only_observation(self) -> None:
        world = WorldModel()
        first = world.observe(WorldObservation(
            subject="door", predicate="open", value="true", source="sensor"
        ))
        second = first.observe(WorldObservation(
            subject="door", predicate="open", value="false", source="button"
        ))
        self.assertNotEqual(first.digest(), second.digest())
        self.assertEqual(len(first.observations), 1)
        self.assertEqual(len(second.observations), 2)

    def test_invalid_self_model_is_fail_closed(self) -> None:
        model = SelfModel(herus_id="", host_digest="", authority="EXECUTE_ANYTHING")
        self.assertIn("self_identity_missing", model.validate())
        self.assertIn("self_host_binding_missing", model.validate())
        self.assertIn("self_authority_unknown", model.validate())
        self.assertFalse(model.can_execute("anything"))


if __name__ == "__main__":
    unittest.main()
