import unittest
from shortcut_probe import transformed, flip_rate


class ShortcutProbeTests(unittest.TestCase):
    def test_transformation_preserves_rows_and_labels(self):
        rows = [{'text': 'book a flight', 'label': 'travel', 'season': 'S06'}]
        changed = transformed(rows, 'please')
        self.assertEqual(changed[0]['label'], rows[0]['label'])
        self.assertIn('please', changed[0]['text'])

    def test_flip_rate_is_zero_for_identical_predictions(self):
        self.assertEqual(flip_rate(['A', None], ['A', 'B']), 0.0)


if __name__ == '__main__':
    unittest.main()
