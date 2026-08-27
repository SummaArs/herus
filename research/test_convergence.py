import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from convergence import (
    AlignmentClass,
    IdentityStatus,
    Observation,
    PairingBasis,
    compare,
    multimodal_rate,
)


class ConvergenceTests(unittest.TestCase):
    def paired(self, source="mintrec", sample_id="s1", modality="audio", key="k"):
        return Observation(source, sample_id, modality, AlignmentClass.PAIRED, key)

    def test_same_source_id_and_declared_paired_modalities_are_comparable(self):
        left = self.paired(modality="audio", key="arrive")
        right = self.paired(modality="video", key="arrive")
        result = compare(left, right)
        self.assertEqual(result.reason, "paired_same_source_id")
        self.assertTrue(result.allowed)
        self.assertTrue(result.multimodal)
        self.assertTrue(result.equal)
        self.assertEqual(multimodal_rate([(left, right)]), 1.0)

    def test_same_source_id_but_different_semantics_is_not_convergence(self):
        result = compare(
            self.paired(modality="audio", key="arrive"),
            self.paired(modality="video", key="help"),
        )
        self.assertTrue(result.allowed)
        self.assertFalse(result.equal)
        self.assertEqual(multimodal_rate([
            (self.paired(modality="audio", key="arrive"),
             self.paired(modality="video", key="help"))
        ]), 0.0)

    def test_different_sample_ids_are_refused(self):
        result = compare(
            self.paired(sample_id="s1"),
            self.paired(sample_id="s2", modality="video"),
        )
        self.assertFalse(result.allowed)
        self.assertIsNone(result.equal)
        self.assertEqual(result.reason, "sample_id_mismatch")

    def test_same_label_does_not_pair_different_sources(self):
        # This models the forbidden shortcut: MIntRec and WESAD both expose a
        # label-like field, but the labels do not establish a shared sample.
        mintrec = self.paired(source="mintrec", sample_id="label:stress", modality="audio")
        wesad = self.paired(source="wesad", sample_id="label:stress", modality="sensor")
        result = compare(mintrec, wesad)
        self.assertFalse(result.allowed)
        self.assertEqual(result.reason, "source_mismatch")
        with self.assertRaisesRegex(ValueError, "source_mismatch"):
            multimodal_rate([(mintrec, wesad)])

    def test_unpaired_observation_is_never_promoted(self):
        left = Observation("common_voice", "clip-1", "audio", AlignmentClass.INTRAMODAL, "x")
        right = Observation("common_voice", "clip-1", "transcript", AlignmentClass.UNPAIRED, "x")
        result = compare(left, right)
        self.assertFalse(result.allowed)
        self.assertEqual(result.reason, "unpaired_observation")

    def test_ambiguous_identity_is_refused(self):
        left = Observation(
            "minds14", "sample-1", "audio", AlignmentClass.PAIRED, "k",
            identity_status=IdentityStatus.AMBIGUOUS,
        )
        right = self.paired(source="minds14", sample_id="sample-1", modality="text")
        result = compare(left, right)
        self.assertFalse(result.allowed)
        self.assertEqual(result.reason, "identity_ambiguous")
        with self.assertRaisesRegex(ValueError, "identity_ambiguous"):
            multimodal_rate([(left, right)])

    def test_conflicting_identity_is_refused(self):
        left = Observation(
            "minds14", "sample-1", "audio", AlignmentClass.PAIRED, "k",
            identity_status=IdentityStatus.CONFLICT,
        )
        right = self.paired(source="minds14", sample_id="sample-1", modality="text")
        self.assertEqual(compare(left, right).reason, "identity_conflict")

    def test_label_order_and_basename_pairing_are_refused(self):
        for basis, reason in (
            (PairingBasis.LABEL, "label_only_pairing"),
            (PairingBasis.ORDER, "order_only_pairing"),
            (PairingBasis.PATH_BASENAME, "basename_only_pairing"),
        ):
            left = Observation("source", "same", "audio", AlignmentClass.PAIRED, "k", pairing_basis=basis)
            right = self.paired(source="source", sample_id="same", modality="text")
            self.assertEqual(compare(left, right).reason, reason)

    def test_timestamp_mismatch_is_refused(self):
        left = Observation("source", "same", "audio", AlignmentClass.PAIRED, "k", timestamp_ms=1)
        right = Observation("source", "same", "text", AlignmentClass.PAIRED, "k", timestamp_ms=2)
        self.assertEqual(compare(left, right).reason, "timestamp_mismatch")

    def test_duplicate_sample_id_is_refused_in_rate(self):
        left_a = self.paired(source="source", sample_id="same", modality="audio")
        right_a = self.paired(source="source", sample_id="same", modality="text")
        left_b = self.paired(source="source", sample_id="same", modality="video")
        right_b = self.paired(source="source", sample_id="same", modality="audio")
        with self.assertRaisesRegex(ValueError, "duplicate_sample_id"):
            multimodal_rate([(left_a, right_a), (left_b, right_b)])

    def test_intramodal_does_not_count_as_multimodal(self):
        left = Observation("common_voice", "clip-1", "audio", AlignmentClass.INTRAMODAL, "x")
        right = Observation("common_voice", "clip-1", "audio", AlignmentClass.INTRAMODAL, "x")
        result = compare(left, right)
        self.assertTrue(result.allowed)
        self.assertFalse(result.multimodal)
        with self.assertRaisesRegex(ValueError, "intramodal"):
            multimodal_rate([(left, right)])


if __name__ == "__main__":
    unittest.main()
