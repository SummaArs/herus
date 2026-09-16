from __future__ import annotations
import unittest

from h0_host_runtime import ExitBundle, H0ComputerResidence
from host_migration import HostMigration
from host_profile import HostProfile


class HostMigrationTests(unittest.TestCase):
    def profile(self, host_id: str, revision: str = "v1") -> HostProfile:
        return HostProfile(
            host_id=host_id, revision=revision,
            resources={"ram_bytes": 4096, "energy_budget": 100},
            interfaces=frozenset({"button", "haptic", "radio", "sensor", "actuator"}),
            constraints={"max_steps": 12, "max_latency_ms": 100},
            representation_set=frozenset({"SIM-INT8", "SIM-HDC8", "SIM-RULES"}),
            skill_budget={"bytes": 4096, "steps": 12}, evidence={"fixture": host_id},
        )

    def bundle(self) -> ExitBundle:
        runtime = H0ComputerResidence("herus-general")
        runtime.enter(self.profile("computer"), skills={"observe", "coordinate"})
        return runtime.prepare_exit()

    def test_identity_and_contracts_survive_five_declared_hosts(self):
        bundle = self.bundle()
        for kind in ("embedded", "wrist", "robot", "vehicle"):
            target = self.profile(kind + "-001")
            migration = HostMigration(bundle)
            state = migration.bind(target, host_kind=kind, skills=bundle.transferable_skills)
            self.assertEqual(migration.validate(), ())
            self.assertEqual(state.identity.herus_id, bundle.herus_id)
            self.assertEqual(state.self_model.authority, "NONE")
            self.assertEqual(state.world.observations, ())
            self.assertEqual(state.execute("coordinate"), "ABSTAIN")

    def test_source_target_collision_is_rejected(self):
        bundle = self.bundle()
        with self.assertRaisesRegex(ValueError, "migration_target_is_source"):
            HostMigration(bundle).bind(self.profile("computer"), host_kind="computer")

    def test_unknown_host_kind_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "host_kind_unknown"):
            HostMigration(self.bundle()).bind(self.profile("unknown"), host_kind="spaceship")

    def test_forged_authority_bundle_is_rejected(self):
        bundle = self.bundle()
        forged = ExitBundle(**{**bundle.canonical(), "authority": "EXECUTE"})
        with self.assertRaisesRegex(ValueError, "migration_authority_escalation"):
            HostMigration(forged).bind(self.profile("wrist-001"), host_kind="wrist")

    def test_context_transfer_is_rejected(self):
        bundle = self.bundle()
        forged = ExitBundle(**{**bundle.canonical(), "world_context_transferred": True})
        with self.assertRaisesRegex(ValueError, "migration_context_transfer_forbidden"):
            HostMigration(forged).bind(self.profile("robot-001"), host_kind="robot")


if __name__ == "__main__":
    unittest.main()
