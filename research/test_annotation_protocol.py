import json
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from annotation_protocol import (
    AnnotationLabel,
    PROTOCOL_VERSION,
    annotation_is_non_operational,
    annotation_to_operational_command,
    classify_disagreement,
    create_annotation,
    external_label_to_annotation,
    protocol_schema,
)


class AnnotationProtocolTests(unittest.TestCase):
    def test_published_json_schema_matches_protocol(self):
        schema_path = Path(__file__).resolve().parent / "evidence" / "wide_cycle_06" / "herus_annotation_protocol_v1.json"
        published = json.loads(schema_path.read_text(encoding="utf-8"))
        self.assertEqual(published, protocol_schema() | {"cycle_status": "protocol_only_no_real_HERUS_annotations_claimed"})

    def test_schema_is_finite_and_declares_no_bridge_path(self):
        schema = protocol_schema()
        self.assertEqual(schema["protocol_version"], PROTOCOL_VERSION)
        self.assertEqual(
            schema["labels"],
            ["ARRIVE", "HELP", "CANCEL", "OTHER", "AMBIGUOUS", "CONFLICT"],
        )
        self.assertEqual(schema["bridge_path"], "none")
        self.assertEqual(schema["external_label_mapping"], "forbidden")
        self.assertTrue(schema["rules"]["annotation_is_not_confirmation"])

    def test_missing_evidence_is_ambiguous(self):
        self.assertEqual(classify_disagreement([]), AnnotationLabel.AMBIGUOUS)

    def test_disagreement_is_conflict_without_majority_vote(self):
        self.assertEqual(
            classify_disagreement([AnnotationLabel.ARRIVE, AnnotationLabel.HELP]),
            AnnotationLabel.CONFLICT,
        )

    def test_non_operational_labels_cannot_cross_boundary(self):
        for label in (
            AnnotationLabel.OTHER,
            AnnotationLabel.AMBIGUOUS,
            AnnotationLabel.CONFLICT,
        ):
            annotation = create_annotation(
                evidence_ref="evidence-ref",
                annotator_ref="annotator-ref",
                label=label,
            )
            self.assertTrue(annotation_is_non_operational(annotation))
            with self.assertRaisesRegex(ValueError, "annotation_has_no_operational_authority"):
                annotation_to_operational_command(annotation)

    def test_operational_label_is_still_not_confirmation(self):
        annotation = create_annotation(
            evidence_ref="evidence-ref",
            annotator_ref="annotator-ref",
            label=AnnotationLabel.HELP,
        )
        self.assertFalse(annotation_is_non_operational(annotation))
        with self.assertRaisesRegex(ValueError, "annotation_has_no_operational_authority"):
            annotation_to_operational_command(annotation)

    def test_external_label_mapping_is_forbidden(self):
        with self.assertRaisesRegex(ValueError, "external_label_mapping_forbidden"):
            external_label_to_annotation({"intent_class": 0})

    def test_tampered_annotation_authority_fails_closed(self):
        annotation = create_annotation(
            evidence_ref="evidence-ref",
            annotator_ref="annotator-ref",
            label=AnnotationLabel.ARRIVE,
        )
        tampered = annotation.__class__(
            evidence_ref=annotation.evidence_ref,
            annotator_ref=annotation.annotator_ref,
            label=annotation.label,
            operational_authority=True,
        )
        self.assertTrue(annotation_is_non_operational(tampered))


if __name__ == "__main__":
    unittest.main()
