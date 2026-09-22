from __future__ import annotations

import unittest

from research.holdout_adversarial import IndependentOracle, InstrumentedExecutor, run_campaign
from research.holdout_hosts import holdout_fixtures
from research.stage4.contracts import Status, validate_result


class HoldoutAdversarialTests(unittest.TestCase):
    def test_campaign_repeats_each_fixture_and_is_mechanism_only(self) -> None:
        result = run_campaign()
        self.assertEqual(result["classification"], "not_proven")
        self.assertEqual(len(result["records"]), 9)
        self.assertEqual(len(result["hard_failures"]), 3)
        self.assertEqual(result["violations"], [])

    def test_negative_fixtures_abstain_and_control_proposes(self) -> None:
        result = run_campaign()
        by_id = {record["fixture_id"]: record for record in result["records"] if record["repeat"] == 0}
        self.assertEqual(by_id["c0a-control-01"]["proposal_status"], Status.PROPOSED.value)
        self.assertEqual(by_id["m6a-opaque-01"]["proposal_status"], Status.FAIL_UNSAFE.value)
        self.assertEqual(by_id["m9a-alias-01"]["proposal_status"], Status.ABSTAIN.value)

    def test_result_validator_rejects_proposal_execution_leakage(self) -> None:
        result = run_campaign()
        record = dict(result["records"][0])
        record["proposal_execute_calls"] = 1
        self.assertIn("proposal_execution_leakage", validate_result(record))

    def test_executor_requires_external_authority_and_records_partial(self) -> None:
        executor = InstrumentedExecutor()
        self.assertEqual(executor.execute(authorized=False), "REJECTED")
        self.assertEqual(executor.execute(authorized=True, partial=True), "FAILED_PARTIAL")
        self.assertEqual(executor.effects, 1)
        self.assertEqual(len(executor.trace), 2)

    def test_oracle_does_not_call_runtime(self) -> None:
        fixture = holdout_fixtures()[0]
        status, reason, safety = IndependentOracle().evaluate(fixture, object())
        self.assertEqual(status, "FAIL_UNSAFE")
        self.assertEqual(reason, fixture.oracle_reason)
        self.assertEqual(safety, "BLOCKED")


if __name__ == "__main__":
    unittest.main()
