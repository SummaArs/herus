"""Typed symbolic terms for the HERUS generative reasoning laboratory.

This module is deliberately host-only. It has no natural-language, firmware,
transport, persistence, or execution interfaces.
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from typing import Iterable, Mapping


class Status(str, Enum):
    OK = "OK"
    TYPE_ERROR = "TYPE_ERROR"
    UNKNOWN_SYMBOL = "UNKNOWN_SYMBOL"
    ARITY_ERROR = "ARITY_ERROR"
    CONFLICT = "CONFLICT"
    UNKNOWN = "UNKNOWN"
    BUDGET_EXCEEDED = "BUDGET_EXCEEDED"


@dataclass(frozen=True)
class Budget:
    max_depth: int = 8
    max_nodes: int = 128
    max_steps: int = 256
    max_terms: int = 512

    def valid(self) -> bool:
        return (
            self.max_depth >= 0
            and self.max_nodes >= 1
            and self.max_steps >= 0
            and self.max_terms >= 1
        )


@dataclass(frozen=True)
class Symbol:
    name: str
    output: str
    inputs: tuple[str, ...] = ()

    @property
    def arity(self) -> int:
        return len(self.inputs)


@dataclass(frozen=True)
class Term:
    symbol: str
    output: str
    args: tuple["Term", ...] = ()

    @property
    def arity(self) -> int:
        return len(self.args)

    @property
    def depth(self) -> int:
        if not self.args:
            return 0
        return 1 + max(arg.depth for arg in self.args)

    @property
    def nodes(self) -> int:
        return 1 + sum(arg.nodes for arg in self.args)

    def key(self) -> tuple:
        return (self.symbol, tuple(arg.key() for arg in self.args))

    def __str__(self) -> str:
        if not self.args:
            return self.symbol
        return f"{self.symbol}({', '.join(map(str, self.args))})"


@dataclass(frozen=True)
class BuildResult:
    status: Status
    term: Term | None = None
    reason: str = ""

    @property
    def ok(self) -> bool:
        return self.status is Status.OK and self.term is not None


class Signature:
    """Closed symbol signature and deterministic term constructor."""

    def __init__(self, symbols: Iterable[Symbol] = ()) -> None:
        self._symbols: dict[str, Symbol] = {}
        for symbol in symbols:
            self.add(symbol)

    def add(self, symbol: Symbol) -> None:
        if not symbol.name or symbol.name in self._symbols:
            raise ValueError("symbol names must be nonempty and unique")
        if any(not item for item in (symbol.name, symbol.output, *symbol.inputs)):
            raise ValueError("symbol types and names must be nonempty")
        self._symbols[symbol.name] = symbol

    def symbols(self) -> Mapping[str, Symbol]:
        return dict(self._symbols)

    def get(self, name: str) -> Symbol | None:
        return self._symbols.get(name)

    def validate(self, term: Term, budget: Budget | None = None) -> BuildResult:
        budget = budget or Budget()
        if not budget.valid():
            return BuildResult(Status.BUDGET_EXCEEDED, reason="invalid budget")
        symbol = self.get(term.symbol)
        if symbol is None:
            return BuildResult(Status.UNKNOWN_SYMBOL, reason=term.symbol)
        if term.output != symbol.output:
            return BuildResult(Status.TYPE_ERROR, reason=f"{term.symbol}: output type mismatch")
        if len(term.args) != symbol.arity:
            return BuildResult(Status.ARITY_ERROR, reason=f"{term.symbol}:{len(term.args)}!={symbol.arity}")
        for position, (expected, argument) in enumerate(zip(symbol.inputs, term.args)):
            child = self.validate(argument, budget)
            if not child.ok:
                return child
            if argument.output != expected:
                return BuildResult(
                    Status.TYPE_ERROR,
                    reason=f"{term.symbol}[{position}]: expected {expected}, got {argument.output}",
                )
        if term.depth > budget.max_depth or term.nodes > budget.max_nodes:
            return BuildResult(Status.BUDGET_EXCEEDED, reason=f"depth={term.depth},nodes={term.nodes}")
        return BuildResult(Status.OK, term=term)

    def build(self, name: str, args: Iterable[Term] = (), budget: Budget | None = None) -> BuildResult:
        budget = budget or Budget()
        if not budget.valid():
            return BuildResult(Status.BUDGET_EXCEEDED, reason="invalid budget")
        symbol = self.get(name)
        if symbol is None:
            return BuildResult(Status.UNKNOWN_SYMBOL, reason=name)
        arguments = tuple(args)
        if len(arguments) != symbol.arity:
            return BuildResult(Status.ARITY_ERROR, reason=f"{name}:{len(arguments)}!={symbol.arity}")
        for position, (expected, actual) in enumerate(zip(symbol.inputs, arguments)):
            if expected != actual.output:
                return BuildResult(
                    Status.TYPE_ERROR,
                    reason=f"{name}[{position}]: expected {expected}, got {actual.output}",
                )
        term = Term(symbol=name, output=symbol.output, args=arguments)
        if term.depth > budget.max_depth or term.nodes > budget.max_nodes:
            return BuildResult(Status.BUDGET_EXCEEDED, reason=f"depth={term.depth},nodes={term.nodes}")
        return BuildResult(Status.OK, term=term)

    def atom(self, name: str, budget: Budget | None = None) -> BuildResult:
        return self.build(name, (), budget)

    def __contains__(self, name: str) -> bool:
        return name in self._symbols
