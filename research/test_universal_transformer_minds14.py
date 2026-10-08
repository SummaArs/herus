import json
import unittest
from pathlib import Path


class UniversalTransformerMinds14Tests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data=json.loads((Path(__file__).parent/'evidence/universal_transformer_minds14_v1.json').read_text())

    def test_transformer_is_present_but_not_admitted(self):
        self.assertIn('transformer',self.data['protocol']['candidate_paradigms'])
        paradigms=[p for p,_ in self.data['fit']['thresholds']]
        self.assertNotIn('transformer',paradigms)

    def test_universal_does_not_depend_on_unsafe_candidate(self):
        self.assertGreater(self.data['metrics']['universal']['accuracy'],self.data['metrics']['transformer']['accuracy'])
        self.assertEqual(self.data['metrics']['universal']['coverage'],1.0)

    def test_holdout_is_real_and_claim_is_bounded(self):
        self.assertEqual(self.data['dataset']['holdout'],98)
        self.assertIn('no SOTA claim',self.data['protocol']['claim_boundary'])


if __name__=='__main__': unittest.main()
