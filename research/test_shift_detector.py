import unittest
from shift_detector import LexicalShiftDetector


class ShiftDetectorTests(unittest.TestCase):
    def test_clean_calibration_is_accepted(self):
        detector = LexicalShiftDetector()
        detector.fit(["pay my bill", "check my account", "transfer money", "card payment"])
        self.assertTrue(detector.accept("check my bill"))

    def test_unfitted_detector_fails_closed(self):
        with self.assertRaises(RuntimeError):
            LexicalShiftDetector().accept("hello")

    def test_empty_calibration_is_rejected(self):
        with self.assertRaises(ValueError):
            LexicalShiftDetector().fit([])

    def test_shifted_unknown_text_can_be_rejected(self):
        detector = LexicalShiftDetector(quantile=.9)
        detector.fit(["pay my bill", "check my account", "transfer money", "card payment"] * 10)
        self.assertFalse(detector.accept("zxqv blorp unknownword"))


if __name__ == "__main__":
    unittest.main()
