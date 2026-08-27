import unittest

from finite_reasoner import (
    EGraph,
    Evidence,
    ParaconsistentKB,
    Sort,
    Symbol,
    Term,
    build_context_signature,
    generate_terms,
)


class FiniteReasonerTests(unittest.TestCase):
    def setUp(self):
        self.sig = build_context_signature()
        self.alice = Term.atom_term(Symbol("alice", Sort.PERSON, "context"))
        self.trail = Term.atom_term(Symbol("trail", Sort.PLACE, "context"))
        self.friday = Term.atom_term(Symbol("friday", Sort.TIME, "context"))

    def test_typed_composition(self):
        arrival = self.sig.compose("arrive", (self.alice, self.trail))
        self.assertEqual(arrival.sort, Sort.STATUS)
        with self.assertRaises(TypeError):
            self.sig.compose("arrive", (self.trail, self.alice))

    def test_colored_operations_reject_foreign_color(self):
        foreign = Term.atom_term(Symbol("alice", Sort.PERSON, "foreign"))
        with self.assertRaises(TypeError):
            self.sig.compose("arrive", (foreign, self.trail))

    def test_finite_generation_is_combinatorial_not_open_ended(self):
        terms = generate_terms(
            self.sig,
            (
                Symbol("alice", Sort.PERSON, "context"),
                Symbol("trail", Sort.PLACE, "context"),
                Symbol("friday", Sort.TIME, "context"),
            ),
            max_depth=2,
            max_terms=32,
        )
        self.assertTrue(any(term.op == "arrive" for term in terms))
        self.assertTrue(any(term.op == "meet" for term in terms))
        self.assertTrue(all(term.color in {"context", "card"} for term in terms))
        with self.assertRaises(OverflowError):
            generate_terms(
                self.sig,
                (
                    Symbol("alice", Sort.PERSON, "context"),
                    Symbol("trail", Sort.PLACE, "context"),
                    Symbol("friday", Sort.TIME, "context"),
                ),
                max_depth=3,
                max_terms=3,
            )

    def test_bounded_equality_saturation(self):
        arrival = self.sig.compose("arrive", (self.alice, self.trail))
        alias = self.sig.compose("arrive_alias", (self.alice, self.trail))
        graph = EGraph(max_nodes=16)
        graph.add(arrival)
        graph.add(alias)
        graph.rules.append(lambda term: (arrival,) if term == alias else ())
        steps = graph.saturate(fuel=8)
        self.assertLessEqual(steps, 8)
        self.assertTrue(graph.equivalent(arrival, alias))
        self.assertLessEqual(len(graph.parent), 16)

    def test_budget_is_fail_closed_and_atomic(self):
        graph = EGraph(max_nodes=1)
        alice = Term.atom_term(Symbol("alice", Sort.PERSON, "context"))
        graph.add(alice)
        before = dict(graph.parent)
        with self.assertRaises(OverflowError):
            graph.add(Term.atom_term(Symbol("trail", Sort.PLACE, "context")))
        self.assertEqual(graph.parent, before)

        nested = self.sig.compose("arrive", (alice, self.trail))
        with self.assertRaises(OverflowError):
            graph.add(nested)
        self.assertEqual(graph.parent, before)

    def test_paraconsistency_keeps_both_without_explosion(self):
        kb = ParaconsistentKB({"arrival_confirmed", "help_needed"})
        evidence = Evidence("arrival_confirmed", "button", 100)
        kb.add(evidence, True)
        kb.add(Evidence("arrival_confirmed", "radio", 55), False)
        self.assertEqual(kb.status("arrival_confirmed"), "BOTH")
        self.assertEqual(kb.status("help_needed"), "NEITHER")
        self.assertEqual(kb.status("unknown"), "UNKNOWN_SYMBOL")

    def test_unknown_proposition_is_rejected(self):
        kb = ParaconsistentKB({"arrival_confirmed"})
        with self.assertRaises(ValueError):
            kb.add(Evidence("free_text", "model", 1), True)


if __name__ == "__main__":
    unittest.main()
