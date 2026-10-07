import unittest
from herus_symbiotic.data_science import DataScienceSkill

class DataScienceSkillTests(unittest.TestCase):
    def setUp(self): self.skill=DataScienceSkill()
    def test_empty_abstains(self):
        self.assertEqual(self.skill.analyze(()).status,'ABSTAIN')
    def test_missing_label_blocks(self):
        plan=self.skill.analyze(({'text':'a'},{'text':'b'}))
        self.assertEqual(plan.status,'ABSTAIN')
        self.assertEqual(plan.profile.issues[0].code,'MISSING_LABEL')
    def test_realistic_profile_proposes_protocol(self):
        rows=tuple({'text':x,'label':y} for x,y in [('a','x'),('b','y'),('c','x'),('d','y')])
        plan=self.skill.analyze(rows,objective='classify intent')
        self.assertEqual(plan.status,'PROPOSE')
        self.assertIn('TF-IDF + linear SVM',plan.baselines)
        self.assertIn('coverage-risk curve',plan.metrics)
        self.assertEqual(plan.authority,'none')
    def test_duplicates_and_imbalance_are_visible(self):
        rows=({'text':'same','label':'x'},)*4 + ({'text':'other','label':'y'},)
        plan=self.skill.analyze(rows)
        codes={i.code for i in plan.profile.issues}
        self.assertIn('DUPLICATES',codes)
        self.assertIn('CLASS_IMBALANCE',codes)

if __name__=='__main__': unittest.main()
