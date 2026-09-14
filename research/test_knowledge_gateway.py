import unittest

from knowledge_gateway import (
    EvidenceRecord,
    EvidenceStatus,
    ResearchQuestion,
    digest_source,
    evaluate_evidence,
)


class KnowledgeGatewayTests(unittest.TestCase):
    def question(self, **overrides):
        values = dict(
            question_id="q-1",
            query="How does this host expose telemetry?",
            expected_vocabulary=frozenset({"telemetry"}),
            max_results=3,
        )
        values.update(overrides)
        return ResearchQuestion(**values)

    def record(self, **overrides):
        values = dict(
            source_url="https://example.org/spec",
            title="Host specification",
            content="The host exposes telemetry.",
            retrieved_by="notebook-1",
            retrieved_at="2026-09-13T00:00:00Z",
            source_digest="",
            claims=("telemetry",),
        )
        values.update(overrides)
        record = EvidenceRecord(**values)
        if not overrides.get("source_digest"):
            record = EvidenceRecord(
                record.source_url,
                record.title,
                record.content,
                record.retrieved_by,
                record.retrieved_at,
                digest_source(record),
                record.claims,
            )
        return record

    def test_verified_https_evidence_is_accepted_but_not_executable(self):
        decision = evaluate_evidence(self.question(), self.record())
        self.assertEqual(decision.status, EvidenceStatus.ACCEPTED)
        self.assertFalse(decision.executable)

    def test_http_source_is_rejected(self):
        decision = evaluate_evidence(self.question(), self.record(source_url="http://example.org/spec"))
        self.assertEqual(decision.status, EvidenceStatus.REJECTED)

    def test_tampered_digest_is_rejected(self):
        decision = evaluate_evidence(self.question(), self.record(source_digest="bad-digest"))
        self.assertEqual(decision.status, EvidenceStatus.REJECTED)
        self.assertEqual(decision.reason, "digest_mismatch")

    def test_missing_claims_needs_review(self):
        decision = evaluate_evidence(self.question(), self.record(claims=()))
        self.assertEqual(decision.status, EvidenceStatus.NEEDS_REVIEW)

    def test_missing_provenance_is_rejected(self):
        decision = evaluate_evidence(self.question(), self.record(retrieved_by=""))
        self.assertEqual(decision.status, EvidenceStatus.REJECTED)


if __name__ == "__main__":
    unittest.main()
