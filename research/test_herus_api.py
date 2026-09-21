from __future__ import annotations

import unittest
import importlib
from unittest.mock import patch
from fastapi.testclient import TestClient

from api.herus_api.app import app


IR = {
    "schemaVersion": 1,
    "eventKind": "HELP",
    "source": "TEXT",
    "confidencePct": 90,
    "runnerUpPct": 10,
    "slots": {"minutes": None},
    "evidence": [{"kind": "OBSERVATION", "ref": "test-1", "polarity": "POSITIVE", "weight": 90}],
    "hypothesisStatus": "TRUE",
    "authority": "PROPOSAL_ONLY",
}


class HerusApiTests(unittest.TestCase):
    def setUp(self) -> None:
        module_app = importlib.import_module("api.herus_api.app")
        module_app._PROPOSALS.clear()
        module_app._OBSERVATIONS.clear()
        module_app._LEARNING_RECORDS.clear()
        module_app._IDEMPOTENCY.clear()
        self.client = TestClient(app)

    def test_health_is_proposal_only(self) -> None:
        response = self.client.get("/api/v1/health")
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.json()["mode"], "proposal-only")

    def test_invalid_semantic_ir_is_fail_closed(self) -> None:
        invalid = {**IR, "confidencePct": 10, "runnerUpPct": 90}
        response = self.client.post("/api/v1/proposals", json={"ir": invalid}, headers={"Idempotency-Key": "bad-1"})
        self.assertEqual(response.status_code, 422)

    def test_proposal_requires_idempotency_and_replays(self) -> None:
        missing = self.client.post("/api/v1/proposals", json={"ir": IR})
        self.assertEqual(missing.status_code, 400)
        first = self.client.post("/api/v1/proposals", json={"ir": IR}, headers={"Idempotency-Key": "proposal-1"})
        second = self.client.post("/api/v1/proposals", json={"ir": IR}, headers={"Idempotency-Key": "proposal-1"})
        self.assertEqual(first.status_code, 201)
        self.assertEqual(second.status_code, 200)
        self.assertEqual(second.headers["Idempotency-Replayed"], "true")
        self.assertEqual(first.json()["proposal_id"], second.json()["proposal_id"])

    def test_idempotency_conflict_is_rejected(self) -> None:
        self.client.post("/api/v1/proposals", json={"ir": IR}, headers={"Idempotency-Key": "proposal-2"})
        changed = {**IR, "eventKind": "ARRIVE"}
        response = self.client.post("/api/v1/proposals", json={"ir": changed}, headers={"Idempotency-Key": "proposal-2"})
        self.assertEqual(response.status_code, 422)
        self.assertEqual(response.json()["code"], "IDEMPOTENCY_CONFLICT")

    def test_github_observation_is_allowlisted_and_real_read_only(self) -> None:
        response = self.client.post("/api/v1/connectors/github/observe", json={"repository": "other/repo", "ref": "main"}, headers={"Idempotency-Key": "github-1"})
        self.assertEqual(response.status_code, 422)
        self.assertEqual(response.json()["code"], "VALIDATION_ERROR")

    def test_remote_execution_is_disabled(self) -> None:
        response = self.client.post("/api/v1/executions", json={})
        self.assertEqual(response.status_code, 501)
        self.assertEqual(response.json()["code"], "EXECUTION_DISABLED")

    def test_dry_run_declares_not_implemented_until_adapter_exists(self) -> None:
        response = self.client.post("/api/v1/executions/dry-run", json={"operation": "merge"}, headers={"Idempotency-Key": "dry-1"})
        self.assertEqual(response.status_code, 501)
        self.assertEqual(response.json()["code"], "DRY_RUN_NOT_IMPLEMENTED")

    @patch("api.herus_api.app.GitHubObserver.observe")
    def test_learning_endpoint_builds_bounded_contract_without_authority(self, observe) -> None:
        observe.return_value = {"snapshot": {"repository_id": 1, "full_name": "SummaArs/herus", "ref": "main", "commit_sha": "abc", "default_branch": "main", "visibility": "public", "archived": False, "fork": False}, "snapshot_digest": "digest-1"}
        response = self.client.post("/api/v1/learning/github", json={"repository": "SummaArs/herus", "ref": "main"}, headers={"Idempotency-Key": "learn-1"})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.json()["authority"], "PROPOSAL_ONLY")
        self.assertEqual(response.json()["learned"]["action_authority"], "NONE")
        self.assertEqual(response.json()["metrics"]["authority_escalations"], "0/1")
        self.assertEqual(response.json()["mode"], "observation")


if __name__ == "__main__":
    unittest.main()
