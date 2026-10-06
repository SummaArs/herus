import unittest
from intent_normalization import normalize_surface


class IntentNormalizationTests(unittest.TestCase):
    def test_removes_only_known_leading_prefix(self):
        self.assertEqual(normalize_surface('Please book a flight'), 'book a flight')
        self.assertEqual(normalize_surface('IF YOU CAN book a flight'), 'book a flight')
        self.assertEqual(normalize_surface('thanks book a flight'), 'book a flight')

    def test_does_not_remove_internal_or_core_words(self):
        self.assertEqual(normalize_surface('book please a flight'), 'book please a flight')
        self.assertEqual(normalize_surface('book a flight'), 'book a flight')


if __name__ == '__main__':
    unittest.main()
