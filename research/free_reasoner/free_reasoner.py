"""HERUS Free Symbolic Reasoning Engine — host-only research.

This module explores a stronger thesis than finite retrieval:
* terms can be generated, transformed, compared and synthesized;
* exact algebra supplies a small trusted semantic kernel;
* conjectures are generated from equivalence classes, then checked;
* program candidates are synthesized with semantic signatures and bounded search;
* every operation is bounded, deterministic and inert.

It does NOT claim universal AGI, natural-language grounding, or autonomous
authority. The goal is an auditable symbolic substrate for open-ended
combinatorial reasoning inside an explicit algebra.
"""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction
from itertools import product
from heapq import heappush, heappop
from typing import Iterable, Iterator, Sequence


class ReasoningError(Exception):
    pass


class BudgetExceeded(ReasoningError):
    pass


class NotProved(ReasoningError):
    pass


class InvalidTerm(ReasoningError):
    pass


class NoSynthesis(ReasoningError):
    pass


@dataclass(frozen=True)
class Term:
    op: str
    args: tuple["Term", ...] = ()

    def __str__(self) -> str:
        if not self.args:
            return self.op
        if self.op == "neg" and len(self.args) == 1:
            return f"-({self.args[0]})"
        if self.op in {"+", "-", "*"} and len(self.args) == 2:
            return f"({self.args[0]} {self.op} {self.args[1]})"
        return f"{self.op}({', '.join(map(str, self.args))})"

    @property
    def size(self) -> int:
        return 1 + sum(a.size for a in self.args)

    @property
    def depth(self) -> int:
        return 1 + max((a.depth for a in self.args), default=0)


def V(name: str) -> Term:
    if not name or any(ch.isspace() for ch in name) or name.startswith("#"):
        raise ValueError("invalid variable")
    return Term(name)


def C(value: int | Fraction) -> Term:
    value = Fraction(value)
    return Term(f"#{value.numerator}/{value.denominator}")


def Add(*xs: Term) -> Term:
    if not xs:
        return C(0)
    out = xs[0]
    for x in xs[1:]:
        out = Term("+", (out, x))
    return out


def Mul(*xs: Term) -> Term:
    if not xs:
        return C(1)
    out = xs[0]
    for x in xs[1:]:
        out = Term("*", (out, x))
    return out


def Neg(x: Term) -> Term:
    return Term("neg", (x,))


def Sub(a: Term, b: Term) -> Term:
    return Add(a, Neg(b))


def Pow(x: Term, n: int) -> Term:
    if n < 0:
        raise ValueError("negative exponent unsupported")
    out = C(1)
    for _ in range(n):
        out = Mul(out, x)
    return out


def is_const(t: Term) -> bool:
    return t.op.startswith("#") and not t.args


def const_value(t: Term) -> Fraction:
    if not is_const(t):
        raise ValueError("not a constant")
    p, q = t.op[1:].split("/")
    return Fraction(int(p), int(q))


@dataclass(frozen=True, order=True)
class Monomial:
    powers: tuple[tuple[str, int], ...] = ()

    def mul(self, other: "Monomial") -> "Monomial":
        d = dict(self.powers)
        for name, exp in other.powers:
            d[name] = d.get(name, 0) + exp
        return Monomial(tuple(sorted((k, v) for k, v in d.items() if v)))


@dataclass(frozen=True)
class Poly:
    terms: tuple[tuple[Monomial, Fraction], ...] = ()

    @staticmethod
    def zero() -> "Poly":
        return Poly(())

    @staticmethod
    def const(x: int | Fraction) -> "Poly":
        c = Fraction(x)
        return Poly(() if not c else ((Monomial(), c),))

    @staticmethod
    def var(name: str) -> "Poly":
        return Poly(((Monomial(((name, 1),)), Fraction(1)),))

    def as_dict(self) -> dict[Monomial, Fraction]:
        return dict(self.terms)

    @staticmethod
    def from_dict(d: dict[Monomial, Fraction]) -> "Poly":
        clean = {m: Fraction(c) for m, c in d.items() if c}
        return Poly(tuple(sorted(clean.items())))

    def add(self, other: "Poly") -> "Poly":
        d = self.as_dict()
        for m, c in other.terms:
            d[m] = d.get(m, 0) + c
        return Poly.from_dict(d)

    def neg(self) -> "Poly":
        return Poly(tuple((m, -c) for m, c in self.terms))

    def mul(self, other: "Poly") -> "Poly":
        d: dict[Monomial, Fraction] = {}
        for m1, c1 in self.terms:
            for m2, c2 in other.terms:
                m = m1.mul(m2)
                d[m] = d.get(m, 0) + c1 * c2
        return Poly.from_dict(d)

    def degree(self) -> int:
        return max((sum(e for _, e in m.powers) for m, _ in self.terms), default=0)

    def evaluate(self, env: dict[str, Fraction]) -> Fraction:
        total = Fraction(0)
        for mon, coeff in self.terms:
            x = coeff
            for name, exp in mon.powers:
                x *= env[name] ** exp
            total += x
        return total


def normalize(t: Term) -> Poly:
    """Exact normalization for the commutative-ring fragment (+, *, neg, rationals)."""
    if is_const(t):
        return Poly.const(const_value(t))
    if not t.args:
        return Poly.var(t.op)
    if t.op == "neg" and len(t.args) == 1:
        return normalize(t.args[0]).neg()
    if t.op == "+" and len(t.args) == 2:
        return normalize(t.args[0]).add(normalize(t.args[1]))
    if t.op == "*" and len(t.args) == 2:
        return normalize(t.args[0]).mul(normalize(t.args[1]))
    if t.op == "-" and len(t.args) == 2:
        return normalize(t.args[0]).add(normalize(t.args[1]).neg())
    raise InvalidTerm(f"unsupported operator {t.op!r}")


def polynomial_string(p: Poly) -> str:
    if not p.terms:
        return "0"
    chunks: list[str] = []
    for mon, coeff in sorted(p.terms, key=lambda x: x[0].powers):
        pieces: list[str] = []
        abs_c = abs(coeff)
        if mon.powers:
            if abs_c != 1:
                pieces.append(str(abs_c))
            for name, exp in mon.powers:
                pieces.append(name if exp == 1 else f"{name}^{exp}")
            body = "*".join(pieces)
        else:
            body = str(abs_c)
        if not chunks:
            chunks.append(("-" if coeff < 0 else "") + body)
        else:
            chunks.append(("- " if coeff < 0 else "+ ") + body)
    return " ".join(chunks)


def parse_simple(expr: str) -> Term:
    """Small expression parser for +,-,*, parentheses and exact numeric literals."""
    tokens: list[str] = []
    i = 0
    while i < len(expr):
        c = expr[i]
        if c.isspace():
            i += 1
            continue
        if c in "()+-*":
            tokens.append(c)
            i += 1
            continue
        j = i
        while j < len(expr) and (expr[j].isalnum() or expr[j] in "_./"):
            j += 1
        if j == i:
            raise ValueError(f"unexpected character {c!r}")
        tokens.append(expr[i:j])
        i = j
    pos = 0

    def peek() -> str | None:
        return tokens[pos] if pos < len(tokens) else None

    def eat(expected: str | None = None) -> str:
        nonlocal pos
        tok = peek()
        if tok is None:
            raise ValueError("unexpected end")
        if expected is not None and tok != expected:
            raise ValueError(f"expected {expected}, got {tok}")
        pos += 1
        return tok

    def factor() -> Term:
        tok = peek()
        if tok == "-":
            eat("-")
            return Neg(factor())
        if tok == "(":
            eat("(")
            out = additive()
            eat(")")
            return out
        token = eat()
        if token[0].isdigit():
            return C(Fraction(token))
        return V(token)

    def multiplicative() -> Term:
        out = factor()
        while peek() == "*":
            eat("*")
            out = Mul(out, factor())
        return out

    def additive() -> Term:
        out = multiplicative()
        while peek() in {"+", "-"}:
            op = eat()
            rhs = multiplicative()
            out = Add(out, rhs) if op == "+" else Sub(out, rhs)
        return out

    out = additive()
    if pos != len(tokens):
        raise ValueError("trailing tokens")
    return out


@dataclass(frozen=True)
class Rewrite:
    name: str
    lhs: Term
    rhs: Term
    bidirectional: bool = True


@dataclass(frozen=True)
class ProofStep:
    rule: str
    before: Term
    after: Term


@dataclass(frozen=True)
class Proof:
    theorem: str
    start: Term
    goal: Term
    semantic_left: Poly
    semantic_right: Poly
    steps: tuple[ProofStep, ...] = ()
    verified: bool = True

    @property
    def cost(self) -> int:
        return len(self.steps)

    def text(self) -> str:
        lines = [
            f"THEOREM: {self.theorem}",
            f"START: {self.start}",
            f"GOAL:  {self.goal}",
            f"CHECK: normalize(START) = {polynomial_string(self.semantic_left)}",
            f"CHECK: normalize(GOAL)  = {polynomial_string(self.semantic_right)}",
        ]
        for i, step in enumerate(self.steps, 1):
            lines.append(f"{i:02d}. [{step.rule}] {step.before} => {step.after}")
        if not self.steps:
            lines.append("00. [RING_NORMAL_FORM] exact canonical algebraic equality")
        return "\n".join(lines)


@dataclass(frozen=True)
class SearchBudget:
    max_steps: int = 24
    max_states: int = 8000
    max_depth: int = 10


def substitute(t: Term, env: dict[str, Term]) -> Term:
    if not t.args and not is_const(t):
        return env.get(t.op, t)
    return Term(t.op, tuple(substitute(a, env) for a in t.args))


def _match(pattern: Term, value: Term, env: dict[str, Term]) -> bool:
    if not pattern.args and not is_const(pattern):
        old = env.get(pattern.op)
        if old is None:
            env[pattern.op] = value
            return True
        return old == value
    if pattern.op != value.op or len(pattern.args) != len(value.args):
        return False
    return all(_match(a, b, env) for a, b in zip(pattern.args, value.args))


def rewrite_at_root(term: Term, rule: Rewrite) -> tuple[tuple[Term, str], ...]:
    out: list[tuple[Term, str]] = []
    orientations = [(rule.lhs, rule.rhs, rule.name)]
    if rule.bidirectional:
        orientations.append((rule.rhs, rule.lhs, rule.name + " [reverse]"))
    for lhs, rhs, label in orientations:
        env: dict[str, Term] = {}
        if _match(lhs, term, env):
            out.append((substitute(rhs, env), label))
    return tuple(out)


def rewrite_all(term: Term, rule: Rewrite) -> Iterable[tuple[Term, str]]:
    yield from rewrite_at_root(term, rule)
    for i, child in enumerate(term.args):
        for repl, label in rewrite_all(child, rule):
            args = list(term.args)
            args[i] = repl
            yield Term(term.op, tuple(args)), label


def default_rewrites() -> tuple[Rewrite, ...]:
    a, b, c = V("a"), V("b"), V("c")
    return (
        Rewrite("add-comm", Add(a, b), Add(b, a)),
        Rewrite("mul-comm", Mul(a, b), Mul(b, a)),
        Rewrite("add-assoc", Add(Add(a, b), c), Add(a, Add(b, c))),
        Rewrite("mul-assoc", Mul(Mul(a, b), c), Mul(a, Mul(b, c))),
        Rewrite("left-distrib", Mul(a, Add(b, c)), Add(Mul(a, b), Mul(a, c))),
        Rewrite("right-distrib", Mul(Add(a, b), c), Add(Mul(a, c), Mul(b, c))),
        Rewrite("neg-neg", Neg(Neg(a)), a),
        Rewrite("add-inverse", Add(a, Neg(a)), C(0)),
        Rewrite("zero-add", Add(a, C(0)), a),
        Rewrite("add-zero", Add(C(0), a), a),
        Rewrite("one-mul", Mul(a, C(1)), a),
        Rewrite("mul-one", Mul(C(1), a), a),
        Rewrite("zero-mul", Mul(a, C(0)), C(0)),
        Rewrite("mul-zero", Mul(C(0), a), C(0)),
    )


class Prover:
    def __init__(self, rules: Sequence[Rewrite] | None = None, budget: SearchBudget | None = None):
        self.rules = tuple(rules or default_rewrites())
        self.budget = budget or SearchBudget()

    def prove(self, left: Term, right: Term, theorem: str = "anonymous") -> Proof:
        nl, nr = normalize(left), normalize(right)
        if nl != nr:
            raise NotProved("canonical algebraic forms differ")
        if left == right:
            return Proof(theorem, left, right, nl, nr, ())
        path = self._search(left, right)
        # Even when a short rewrite path is not found, the exact ring-kernel
        # equality is still a sound certificate for this algebraic fragment.
        return Proof(theorem, left, right, nl, nr, tuple(path))

    def _search(self, left: Term, right: Term) -> list[ProofStep]:
        target = normalize(right)
        heap: list[tuple[int, int, Term, tuple[ProofStep, ...]]] = []
        seq = 0
        heappush(heap, (0, seq, left, ()))
        best = {left: 0}
        expanded = 0
        while heap:
            cost, _, current, path = heappop(heap)
            expanded += 1
            if normalize(current) == target:
                return list(path)
            if expanded > self.budget.max_states:
                return []
            if cost >= self.budget.max_steps or current.depth > self.budget.max_depth:
                continue
            for rule in self.rules:
                for nxt, label in rewrite_all(current, rule):
                    if nxt.depth > self.budget.max_depth or nxt.size > self.budget.max_states:
                        continue
                    new_cost = cost + 1
                    if best.get(nxt, 10**9) <= new_cost:
                        continue
                    best[nxt] = new_cost
                    seq += 1
                    heappush(heap, (new_cost, seq, nxt, path + (ProofStep(label, current, nxt),)))
        return []


@dataclass(frozen=True)
class Candidate:
    left: Term
    right: Term
    semantic: Poly
    novelty: float
    complexity: int

    def statement(self) -> str:
        return f"{self.left} == {self.right}"


@dataclass(frozen=True)
class DiscoveryBudget:
    max_depth: int = 2
    max_terms: int = 180
    max_pairs: int = 4000


def enumerate_terms(variables: Sequence[str], max_depth: int, max_terms: int) -> tuple[Term, ...]:
    atoms = tuple(V(x) for x in variables)
    terms = set(atoms)
    if len(terms) > max_terms:
        raise BudgetExceeded("term budget")
    for _ in range(max_depth):
        current = tuple(sorted(terms, key=lambda t: (t.size, str(t))))
        additions: set[Term] = set()
        for x, y in product(current, repeat=2):
            additions.add(Add(x, y))
            additions.add(Mul(x, y))
        additions.update(Neg(x) for x in current)
        for t in sorted(additions, key=lambda z: (z.size, str(z))):
            if t in terms:
                continue
            terms.add(t)
            if len(terms) >= max_terms:
                return tuple(sorted(terms, key=lambda t: (t.size, str(t))))
    return tuple(sorted(terms, key=lambda t: (t.size, str(t))))


def variables(t: Term) -> frozenset[str]:
    out: set[str] = set()
    if not t.args and not is_const(t):
        out.add(t.op)
    for arg in t.args:
        out.update(variables(arg))
    return frozenset(out)


def discover_conjectures(
    budget: DiscoveryBudget | None = None,
    variables_set: Sequence[str] = ("a", "b", "c"),
) -> tuple[Candidate, ...]:
    b = budget or DiscoveryBudget()
    terms = enumerate_terms(variables_set, b.max_depth, b.max_terms)
    buckets: dict[Poly, list[Term]] = {}
    for term in terms:
        buckets.setdefault(normalize(term), []).append(term)
    candidates: list[Candidate] = []
    pairs = 0
    for poly, group in buckets.items():
        group = sorted(group, key=lambda t: (t.size, str(t)))
        for i in range(len(group)):
            for j in range(i + 1, len(group)):
                pairs += 1
                if pairs > b.max_pairs:
                    raise BudgetExceeded("candidate pair budget")
                left, right = group[i], group[j]
                if not variables(left) or not variables(right):
                    continue
                if left.size + right.size < 7:
                    continue
                candidates.append(
                    Candidate(
                        left,
                        right,
                        poly,
                        float(len(str(left)) + len(str(right))),
                        left.size + right.size,
                    )
                )
    candidates.sort(key=lambda c: (-c.novelty, c.complexity, c.statement()))
    unique: list[Candidate] = []
    seen: set[tuple[str, str]] = set()
    for cand in candidates:
        key = (str(cand.left), str(cand.right))
        if key in seen:
            continue
        seen.add(key)
        unique.append(cand)
    return tuple(unique)


@dataclass(frozen=True)
class SynthesisBudget:
    max_depth: int = 3
    max_terms: int = 5000
    max_candidates: int = 10000


def eval_term(term: Term, env: dict[str, int | Fraction]) -> Fraction:
    if is_const(term):
        return const_value(term)
    if not term.args:
        return Fraction(env[term.op])
    vals = [eval_term(arg, env) for arg in term.args]
    if term.op == "+":
        return vals[0] + vals[1]
    if term.op == "-":
        return vals[0] - vals[1]
    if term.op == "*":
        return vals[0] * vals[1]
    if term.op == "neg":
        return -vals[0]
    raise InvalidTerm(term.op)


def synthesize_linear_or_polynomial(
    variables: Sequence[str],
    examples: Sequence[tuple[dict[str, int], int]],
    budget: SynthesisBudget | None = None,
    operators: Sequence[str] = ("+", "-", "*"),
) -> Term:
    """Bounded enumerative synthesis with semantic-signature pruning."""
    b = budget or SynthesisBudget()
    if not examples:
        raise ValueError("examples required")
    atoms = [V(v) for v in variables] + [C(0), C(1), C(-1)]
    seen = set(atoms)
    signature_owner: dict[tuple[Fraction, ...], Term] = {}

    def signature(term: Term) -> tuple[Fraction, ...] | None:
        try:
            return tuple(eval_term(term, ex) for ex, _ in examples)
        except (KeyError, ZeroDivisionError):
            return None

    target = tuple(Fraction(y) for _, y in examples)
    for term in sorted(seen, key=lambda t: (t.size, str(t))):
        sig = signature(term)
        if sig is not None:
            signature_owner.setdefault(sig, term)
    if target in signature_owner:
        return signature_owner[target]

    candidates = 0
    for _ in range(b.max_depth):
        prev = tuple(sorted(seen, key=lambda t: (t.size, str(t))))
        additions: set[Term] = set()
        for x, y in product(prev, repeat=2):
            if "+" in operators:
                additions.add(Add(x, y))
            if "-" in operators:
                additions.add(Sub(x, y))
            if "*" in operators:
                additions.add(Mul(x, y))
        for term in sorted(additions, key=lambda z: (z.size, str(z))):
            if term in seen:
                continue
            seen.add(term)
            candidates += 1
            if len(seen) > b.max_terms or candidates > b.max_candidates:
                raise BudgetExceeded("synthesis budget")
            sig = signature(term)
            if sig is None:
                continue
            if sig in signature_owner:
                continue
            signature_owner[sig] = term
            if sig == target:
                return term
    raise NoSynthesis("no expression fits examples within budget")


def counterexample_grid(
    variables: Sequence[str], lo: int = -3, hi: int = 3
) -> Iterator[dict[str, int]]:
    for vals in product(range(lo, hi + 1), repeat=len(variables)):
        yield dict(zip(variables, vals))


def verify_equation_on_grid(
    left: Term,
    right: Term,
    variables: Sequence[str],
    lo: int = -3,
    hi: int = 3,
) -> tuple[bool, dict[str, int] | None]:
    for env in counterexample_grid(variables, lo, hi):
        if eval_term(left, env) != eval_term(right, env):
            return False, env
    return True, None


def demo() -> dict[str, object]:
    left = parse_simple("(a+b)*(c+d)")
    right = parse_simple("a*c+a*d+b*c+b*d")
    proof = Prover(
        budget=SearchBudget(max_steps=30, max_states=25000, max_depth=10)
    ).prove(left, right, "distributivity generates four-term expansion")
    discovered = discover_conjectures(
        DiscoveryBudget(max_depth=2, max_terms=120, max_pairs=3000)
    )
    examples = tuple(({"x": x}, x * x) for x in range(-3, 4))
    synthesized = synthesize_linear_or_polynomial(
        ("x",), examples, SynthesisBudget(max_depth=2, max_terms=1000)
    )
    return {"proof": proof, "generated": discovered[:12], "synthesized": synthesized}


if __name__ == "__main__":
    result = demo()
    print(result["proof"].text())
    print("\nGENERATED CONJECTURES")
    for candidate in result["generated"]:
        print("-", candidate.statement())
    print("\nSYNTHESIZED FUNCTION:", result["synthesized"])
