import unittest

from benchmark_ir import (
    BenchmarkStatus,
    BenchmarkVocabulary,
    compile_slots,
    reject_to_herus,
)


class BenchmarkIRTests(unittest.TestCase):
    def setUp(self):
        self.vocabulary = BenchmarkVocabulary.create(
            actions=["activate", "deactivate"],
            objects=["lights", "music"],
            locations=["none", "kitchen"],
        )

    def test_known_external_slots_compile_to_finite_key(self):
        result = compile_slots(
            self.vocabulary,
            source="fluent_speech_commands",
            sample_id="audio/a.wav",
            action="activate",
            object="lights",
            location="none",
        )
        self.assertEqual(result.status, BenchmarkStatus.ACCEPTED)
        self.assertEqual(result.meaning_key, ("activate", "lights", "none"))
        self.assertFalse(result.operational_authority)

    def test_unknown_external_slot_stays_unknown(self):
        result = compile_slots(
            self.vocabulary,
            source="fluent_speech_commands",
            sample_id="audio/a.wav",
            action="activate",
            object="heater",
            location="none",
        )
        self.assertEqual(result.status, BenchmarkStatus.UNKNOWN)
        self.assertIsNone(result.meaning_key)

    def test_missing_provenance_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "missing_provenance"):
            compile_slots(
                self.vocabulary,
                source="",
                sample_id="audio/a.wav",
                action="activate",
                object="lights",
                location="none",
            )

    def test_external_benchmark_has_no_herus_authority(self):
        result = compile_slots(
            self.vocabulary,
            source="fluent_speech_commands",
            sample_id="audio/a.wav",
            action="activate",
            object="lights",
            location="none",
        )
        with self.assertRaisesRegex(ValueError, "benchmark_ir_not_herus"):
            reject_to_herus(result)

    def test_vocabulary_is_bounded(self):
        with self.assertRaisesRegex(ValueError, "invalid_finite_action_vocabulary"):
            BenchmarkVocabulary.create(
                actions=[str(index) for index in range(65)],
                objects=["lights"],
                locations=["none"],
            )


if __name__ == "__main__":
    unittest.main()
