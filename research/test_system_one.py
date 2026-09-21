from __future__ import annotations
import time
import unittest
from system_one import FiniteStateOracle, Question, QuestionKind, SystemOneEngine

class SystemOneTests(unittest.TestCase):
    def setUp(self):
        self.engine = SystemOneEngine(FiniteStateOracle(), min_confidence_milli=700)

    def test_parallel_typed_answers_and_no_authority(self):
        qs = (
            Question("urgent", QuestionKind.NOUL, "is it urgent"),
            Question("route", QuestionKind.CHOICE, "choose route", ("local", "remote")),
            Question("risk", QuestionKind.SCORE, "score risk", minimum=0, maximum=10),
        )
        batch = self.engine.decide({"evidence": {"urgent": .99, "route": {"local": 9, "remote": 1}, "risk": 2}}, qs)
        self.assertEqual(len(batch.answers), 3)
        self.assertFalse(batch.executable)
        self.assertEqual(batch.answers[0].value, "true")
        self.assertEqual(sum(v for _, v in batch.answers[1].probabilities), 1000)
        self.assertFalse(batch.answers[0].abstained)

    def test_low_confidence_abstains(self):
        q = Question("route", QuestionKind.CHOICE, "choose route", ("a", "b"))
        answer = self.engine.decide({"evidence": {"route": {"a": 1, "b": 1}}}, (q,)).answers[0]
        self.assertTrue(answer.abstained)
        self.assertEqual(answer.reason, "confidence_below_gate")

    def test_unknown_finite_evidence_abstains(self):
        q = Question("urgent", QuestionKind.NOUL, "is urgent")
        answer = self.engine.decide({"evidence": {}}, (q,)).answers[0]
        self.assertTrue(answer.abstained)

    def test_invalid_questions_fail_closed(self):
        with self.assertRaises(ValueError): Question("x", QuestionKind.CHOICE, "bad", ("only",))
        with self.assertRaises(ValueError): self.engine.decide({}, ())
        with self.assertRaises(ValueError): self.engine.decide({}, (Question("x", QuestionKind.NOUL, "x"), Question("x", QuestionKind.NOUL, "x")))

    def test_parallelism_does_not_change_semantics(self):
        def slow_oracle(state, q):
            time.sleep(.02)
            return state["values"][q.name]
        engine = SystemOneEngine(slow_oracle, max_workers=4, min_confidence_milli=0)
        qs = tuple(Question(str(i), QuestionKind.NOUL, "q") for i in range(4))
        start = time.perf_counter(); batch = engine.decide({"values": {str(i): 1 for i in range(4)}}, qs); elapsed = time.perf_counter() - start
        self.assertLess(elapsed, .12)
        self.assertTrue(all(not a.abstained for a in batch.answers))

if __name__ == "__main__": unittest.main()
