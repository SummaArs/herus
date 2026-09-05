"""Bounded structural programs with reusable subterms.

The representation is an inert DAG over the existing exact Term kernel. Nodes
refer only to earlier nodes, so cycles, arbitrary code and hidden side effects
are impossible by construction.
"""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction

from .free_reasoner import Add, C, Mul, Neg, Term, V


class StructuralError(Exception):
    """Base error for malformed structural programs."""


class StructuralBudgetExceeded(StructuralError):
    """Raised when a program exceeds an explicit structural budget."""


@dataclass(frozen=True)
class StructuralBudget:
    max_nodes: int = 32
    max_depth: int = 12
    max_references: int = 2

    def validate(self) -> None:
        if self.max_nodes <= 0 or self.max_depth <= 0:
            raise ValueError("max_nodes and max_depth must be positive")
        if self.max_references <= 0:
            raise ValueError("max_references must be positive")


@dataclass(frozen=True)
class Node:
    op: str
    refs: tuple[int, ...] = ()
    atom: str | None = None
    constant: Fraction | None = None

    @staticmethod
    def variable(name: str) -> "Node":
        if not name or name.startswith("#") or any(ch.isspace() for ch in name):
            raise StructuralError("invalid variable")
        return Node("var", atom=name)

    @staticmethod
    def number(value: int | Fraction) -> "Node":
        return Node("const", constant=Fraction(value))

    @staticmethod
    def apply(op: str, *refs: int) -> "Node":
        if op not in {"add", "mul", "neg"}:
            raise StructuralError(f"unsupported structural operator: {op}")
        expected = 1 if op == "neg" else 2
        if len(refs) != expected:
            raise StructuralError(f"{op} expects {expected} references")
        return Node(op, tuple(refs))


@dataclass(frozen=True)
class Program:
    nodes: tuple[Node, ...]
    root: int
    budget: StructuralBudget = StructuralBudget()

    def __post_init__(self) -> None:
        self.budget.validate()
        if not self.nodes:
            raise StructuralError("program must contain at least one node")
        if len(self.nodes) > self.budget.max_nodes:
            raise StructuralBudgetExceeded("node budget exceeded")
        if self.root < 0 or self.root >= len(self.nodes):
            raise StructuralError("root outside node table")
        for index, node in enumerate(self.nodes):
            if len(node.refs) > self.budget.max_references:
                raise StructuralBudgetExceeded("reference budget exceeded")
            if node.op in {"var", "const"} and node.refs:
                raise StructuralError("leaf node cannot reference children")
            if node.op in {"var", "const"} and (node.atom is None) == (node.constant is None):
                raise StructuralError("leaf must contain exactly one payload")
            for ref in node.refs:
                if ref < 0 or ref >= index:
                    raise StructuralError("references must point to earlier nodes")
        if self.depth > self.budget.max_depth:
            raise StructuralBudgetExceeded("depth budget exceeded")

    @property
    def depth(self) -> int:
        memo: dict[int, int] = {}

        def visit(index: int) -> int:
            if index in memo:
                return memo[index]
            node = self.nodes[index]
            value = 1 + max((visit(ref) for ref in node.refs), default=0)
            memo[index] = value
            return value

        return visit(self.root)

    @property
    def reachable_nodes(self) -> tuple[int, ...]:
        seen: set[int] = set()

        def visit(index: int) -> None:
            if index in seen:
                return
            seen.add(index)
            for ref in self.nodes[index].refs:
                visit(ref)

        visit(self.root)
        return tuple(sorted(seen))

    def materialize(self) -> Term:
        memo: dict[int, Term] = {}

        def visit(index: int) -> Term:
            if index in memo:
                return memo[index]
            node = self.nodes[index]
            if node.op == "var":
                assert node.atom is not None
                term = V(node.atom)
            elif node.op == "const":
                assert node.constant is not None
                term = C(node.constant)
            elif node.op == "add":
                term = Add(visit(node.refs[0]), visit(node.refs[1]))
            elif node.op == "mul":
                term = Mul(visit(node.refs[0]), visit(node.refs[1]))
            elif node.op == "neg":
                term = Neg(visit(node.refs[0]))
            else:
                raise StructuralError(f"unsupported node operator: {node.op}")
            memo[index] = term
            return term

        return visit(self.root)

    def __str__(self) -> str:
        return str(self.materialize())

    def memory_words(self) -> int:
        return len(self.nodes) * 4 + len(self.reachable_nodes)


def append_node(program: Program | None, node: Node, budget: StructuralBudget | None = None) -> Program:
    chosen = budget or (program.budget if program else StructuralBudget())
    nodes = program.nodes + (node,) if program else (node,)
    return Program(nodes, len(nodes) - 1, chosen)


def seed_variable(name: str = "x", budget: StructuralBudget | None = None) -> Program:
    return append_node(None, Node.variable(name), budget)


def build_reusable_square_plus_square(budget: StructuralBudget | None = None) -> Program:
    chosen = budget or StructuralBudget()
    program = seed_variable("x", chosen)
    program = append_node(program, Node.apply("mul", 0, 0), chosen)
    return append_node(program, Node.apply("add", 1, 1), chosen)
