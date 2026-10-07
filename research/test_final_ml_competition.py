import json
import unittest
from pathlib import Path
from final_ml_competition import build

class FinalCompetitionTests(unittest.TestCase):
    def test_real_evidence_contains_multiple_families_and_datasets(self):
        rows=build()
        self.assertGreaterEqual(len(rows),15)
        self.assertGreaterEqual(len({r['dataset'] for r in rows}),2)
        self.assertGreaterEqual(len({r['family'] for r in rows}),5)
    def test_scorecard_does_not_claim_universal_winner(self):
        p=Path(__file__).parent/'evidence'/'final_ml_competition_v1.json'
        d=json.loads(p.read_text())
        self.assertFalse(d['claims']['universal_winner'])
        self.assertIn('selective',d['claims']['herus_current_status'])
    def test_metrics_are_bounded(self):
        for r in build():
            for k in ('accuracy','macro_f1','coverage','selective_accuracy'):
                if r[k] is not None: self.assertGreaterEqual(r[k],0); self.assertLessEqual(r[k],1)

if __name__=='__main__': unittest.main()
