import json
import unittest
from pathlib import Path


class CrossDomainShiftEvidenceTests(unittest.TestCase):
    def test_mixed_generalization_blocks_promotion(self):
        data=json.loads((Path(__file__).parent/'evidence/cross_domain_shift_v1.json').read_text())
        self.assertTrue(data['protocol']['calibration_is_clean_and_disjoint'])
        self.assertFalse(data['protocol']['holdout_labels_used'])
        mintrec=data['domains']['MIntRec']['attacks']['irrelevant_prefix']
        minds=data['domains']['MInDS-14']['attacks']['irrelevant_prefix']
        self.assertGreater(mintrec['attacked_acceptance'], mintrec['clean_acceptance'])
        self.assertGreater(minds['abstention_rate'], .9)
        self.assertIn('shift detection only', data['protocol']['claim_boundary'])


if __name__=='__main__': unittest.main()
