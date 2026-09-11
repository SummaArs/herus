from __future__ import annotations

import unittest

from domain_readiness import Evidence, Readiness, highest_allowed, production_is_forbidden


class DomainReadinessTests(unittest.TestCase):
    def full(self, **changes):
        values = dict(data_provenance=True, hidden_tests=True, adversarial_tests=True, drift_policy=True, rollback=True, human_review=True, external_authority=True, physical_validation=True)
        values.update(changes)
        return Evidence(**values)

    def test_finance_stays_in_simulation(self) -> None:
        self.assertEqual(highest_allowed("finance_sandbox", self.full()), Readiness.SIMULATE)

    def test_critical_stays_in_shadow(self) -> None:
        self.assertEqual(highest_allowed("critical", self.full()), Readiness.SHADOW)

    def test_pulse_requires_physical_validation(self) -> None:
        self.assertEqual(highest_allowed("pulse", self.full(physical_validation=False)), Readiness.SIMULATE)
        self.assertEqual(highest_allowed("pulse", self.full()), Readiness.HUMAN_BOUND)

    def test_missing_hidden_tests_falls_to_proposal(self) -> None:
        self.assertEqual(highest_allowed("server", self.full(hidden_tests=False)), Readiness.PROPOSAL)

    def test_production_needs_external_authority_and_human_review(self) -> None:
        self.assertTrue(production_is_forbidden(self.full(external_authority=False)))
        self.assertFalse(production_is_forbidden(self.full()))


if __name__ == "__main__":
    unittest.main()
