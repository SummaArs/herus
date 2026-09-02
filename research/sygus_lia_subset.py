"""Fail-closed executable subset of SyGuS-IF for finite LIA experiments.

This module is deliberately not a general SyGuS parser or solver. It accepts one
synth-fun over Int, a grammar made only of variables, integer constants, + and -,
and equality constraints. Every candidate is checked over an explicit finite
input box. A bounded pass is reported as BOUNDED_VERIFIED, never as a proof over
unbounded integers.
"""
from __future__ import annotations

from dataclasses import dataclass
from itertools import product
import re
from typing import Any


class UnsupportedSyGuS(ValueError):
    pass


@dataclass(frozen=True)
class CandidateResult:
    status: str
    expression: str | None
    checked_points: int
    reason: str


@dataclass(frozen=True)
class Problem:
    name: str
    variables: tuple[str, ...]
    parameters: tuple[str, ...]
    function_arity: int
    constraints: tuple[Any, ...]
    productions: tuple[Any, ...]


def _tokens(text: str) -> list[str]:
    text = re.sub(r";[^\n]*", "", text)
    return re.findall(r"\(|\)|[^\s()]+", text)


def _parse_one(tokens: list[str], pos: int = 0) -> tuple[Any, int]:
    if pos >= len(tokens):
        raise UnsupportedSyGuS("unexpected_end")
    token = tokens[pos]
    if token != "(":
        if token == ")":
            raise UnsupportedSyGuS("unexpected_close")
        return token, pos + 1
    values: list[Any] = []
    pos += 1
    while pos < len(tokens) and tokens[pos] != ")":
        value, pos = _parse_one(tokens, pos)
        values.append(value)
    if pos >= len(tokens):
        raise UnsupportedSyGuS("unbalanced_parenthesis")
    return values, pos + 1


def _forms(text: str) -> list[Any]:
    tokens = _tokens(text)
    forms: list[Any] = []
    pos = 0
    while pos < len(tokens):
        form, pos = _parse_one(tokens, pos)
        forms.append(form)
    return forms


def _head(form: Any, name: str) -> bool:
    return isinstance(form, list) and bool(form) and form[0] == name


def parse_problem(text: str) -> Problem:
    forms = _forms(text)
    if [f for f in forms if _head(f, "set-logic")] != [["set-logic", "LIA"]]:
        raise UnsupportedSyGuS("logic_must_be_LIA")
    synth = [f for f in forms if _head(f, "synth-fun")]
    if len(synth) != 1:
        raise UnsupportedSyGuS("requires_one_synth_fun")
    if any(_head(f, n) for f in forms for n in ("synth-inv", "inv-constraint", "oracle", "weight", "define-fun")):
        raise UnsupportedSyGuS("unsupported_command")
    _, name, args, return_sort, *grammar = synth[0]
    if return_sort != "Int" or not isinstance(args, list):
        raise UnsupportedSyGuS("synth_fun_must_return_Int")
    variables: list[str] = []
    for arg in args:
        if not isinstance(arg, list) or len(arg) != 2 or arg[1] != "Int":
            raise UnsupportedSyGuS("arguments_must_be_Int")
        variables.append(arg[0])
    if len(grammar) == 1 and isinstance(grammar[0], list) and len(grammar[0]) == 1:
        production = grammar[0][0]
    elif len(grammar) == 2 and isinstance(grammar[1], list) and len(grammar[1]) == 1:
        production = grammar[1][0]
    else:
        raise UnsupportedSyGuS("explicit_single_sort_grammar_required")
    if not isinstance(production, list) or len(production) != 3 or production[1] != "Int":
        raise UnsupportedSyGuS("grammar_must_define_one_Int_sort")
    nonterminal = production[0]
    if not isinstance(production[2], list):
        raise UnsupportedSyGuS("invalid_grammar_body")
    constraints = tuple(f[1] for f in forms if _head(f, "constraint"))
    if not constraints:
        raise UnsupportedSyGuS("missing_constraints")
    declared = tuple(f[1] for f in forms if _head(f, "declare-var") and len(f) == 3 and f[2] == "Int")
    all_variables = tuple(dict.fromkeys((*variables, *declared)))
    if any(isinstance(item, str) and item not in all_variables and item != nonterminal and not re.fullmatch(r"-?[0-9]+", item) for item in production[2]):
        raise UnsupportedSyGuS("grammar_terminal_outside_declared_vocabulary")
    return Problem(name, all_variables, tuple(variables), len(variables), constraints, tuple(production[2]))


def _expr_key(expr: Any) -> str:
    if isinstance(expr, tuple):
        return "(" + " ".join((_expr_key(x) for x in expr)) + ")"
    return str(expr)


def _eval(expr: Any, env: dict[str, int]) -> int:
    if isinstance(expr, str):
        if expr in env:
            return env[expr]
        return int(expr)
    if isinstance(expr, tuple):
        expr = list(expr)
    if not isinstance(expr, list) or not expr:
        raise UnsupportedSyGuS("invalid_term")
    op = expr[0]
    if op not in ("+", "-") or len(expr) != 3:
        raise UnsupportedSyGuS("term_operator_outside_plus_minus")
    left, right = _eval(expr[1], env), _eval(expr[2], env)
    return left + right if op == "+" else left - right


def _check_constraint(expr: Any, env: dict[str, int], function_name: str, parameters: tuple[str, ...], candidate: Any) -> bool:
    if not isinstance(expr, list) or len(expr) != 3 or expr[0] != "=":
        raise UnsupportedSyGuS("constraints_must_be_equalities")
    left, right = expr[1], expr[2]

    def eval_side(side: Any) -> int:
        if isinstance(side, list) and side and side[0] == function_name:
            if len(side) != 1 + len(parameters):
                raise UnsupportedSyGuS("function_arity_mismatch")
            call_env = dict(env)
            for formal, actual in zip(parameters, side[1:]):
                call_env[formal] = _eval(actual, env)
            return _eval(candidate, call_env)
        return _eval(side, env)

    return eval_side(left) == eval_side(right)


def _generate(productions: tuple[Any, ...], variables: tuple[str, ...], max_depth: int) -> tuple[Any, ...]:
    by_depth: list[set[Any]] = [set() for _ in range(max_depth + 1)]
    for item in productions:
        if isinstance(item, str) and (item in variables or re.fullmatch(r"-?[0-9]+", item)):
            by_depth[0].add(item)
        elif isinstance(item, list) and item and item[0] in ("+", "-"):
            if len(item) != 3 or not isinstance(item[1], str) or not isinstance(item[2], str):
                raise UnsupportedSyGuS("grammar_operator_shape")
        else:
            raise UnsupportedSyGuS("grammar_terminal_or_plus_minus_only")
    all_terms: set[Any] = set(by_depth[0])
    for depth in range(1, max_depth + 1):
        for op in ("+", "-"):
            for left_depth in range(depth):
                right_depth = depth - 1 - left_depth
                for left, right in product(by_depth[left_depth], by_depth[right_depth]):
                    all_terms.add((op, left, right))
                    by_depth[depth].add((op, left, right))
    return tuple(sorted(all_terms, key=lambda x: (len(_expr_key(x)), _expr_key(x))))


def synthesize_bounded(text: str, *, lower: int, upper: int, max_depth: int = 2, max_candidates: int = 1000) -> CandidateResult:
    if lower > upper or max_depth < 0 or max_candidates <= 0:
        return CandidateResult("UNKNOWN", None, 0, "invalid_budget_or_domain")
    try:
        problem = parse_problem(text)
        candidates = _generate(problem.productions, problem.variables, max_depth)
        points = tuple(dict(zip(problem.variables, values)) for values in product(range(lower, upper + 1), repeat=len(problem.variables)))
        checked = 0
        for candidate in candidates:
            checked += 1
            if checked > max_candidates:
                return CandidateResult("UNKNOWN", None, len(points) * max_candidates, "candidate_budget_exhausted")
            if all(all(_check_constraint(c, env, problem.name, problem.parameters, candidate) for c in problem.constraints) for env in points):
                return CandidateResult("BOUNDED_VERIFIED", _expr_key(candidate), checked * len(points), "finite_domain_only")
        return CandidateResult("COUNTEREXAMPLE", None, checked * len(points), "no_candidate_satisfies_finite_domain")
    except (UnsupportedSyGuS, ValueError) as exc:
        return CandidateResult("UNKNOWN", None, 0, str(exc))
