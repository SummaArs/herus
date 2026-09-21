from __future__ import annotations

import unittest

from api.herus_api.learning import learn_contract, make_record, split_records


class HerusLearningTests(unittest.TestCase):
    def test_redacts_secrets_before_digest(self) -> None:
        record = make_record(source="github", group_id="SummaArs/herus:main", payload={"body": "Bearer abc123 ghp_secret", "state": "open"}, provenance={"source": "real"})
        self.assertNotIn("abc123", record.payload["body"])
        self.assertNotIn("ghp_secret", record.payload["body"])
        self.assertEqual(record.provenance["payload_digest"], record.payload_digest)

    def test_deduplicates_exact_payloads(self) -> None:
        first = make_record(source="github", group_id="repo-a:main", payload={"sha": "1"}, provenance={})
        duplicate = make_record(source="github", group_id="repo-a:main", payload={"sha": "1"}, provenance={"retry": True})
        self.assertEqual(first.payload_digest, duplicate.payload_digest)
        self.assertEqual(sum(len(bucket) for bucket in split_records([first, duplicate]).values()), 1)

    def test_holdout_is_group_disjoint(self) -> None:
        records = [
            make_record(source="github", group_id="repo-a:main", payload={"sha": "1"}, provenance={}),
            make_record(source="github", group_id="repo-b:main", payload={"sha": "2"}, provenance={}),
        ]
        result = split_records(records, holdout_groups={"repo-b:main"})
        self.assertEqual({item["group_id"] for item in result["holdout"]}, {"repo-b:main"})
        self.assertFalse({item["group_id"] for item in result["train"]} & {"repo-b:main"})

    def test_learning_never_adds_authority(self) -> None:
        record = make_record(source="github", group_id="repo-a:main", payload={"sha": "1"}, provenance={})
        result = learn_contract([record])
        self.assertEqual(result["authority"], "PROPOSAL_ONLY")
        self.assertEqual(result["learned"]["action_authority"], "NONE")
        self.assertFalse(result["learned"]["external_effect"])
        self.assertEqual(result["metrics"]["oracle_inputs"], "0/1")


if __name__ == "__main__":
    unittest.main()
