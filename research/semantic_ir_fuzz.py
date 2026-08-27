"""Deterministic adversarial campaign for Semantic IR v0.1."""
from __future__ import annotations

import copy
import json
import random
from collections import Counter
from typing import Any, Callable

from semantic_ir import compile_ir, validate_ir


Mutator = Callable[[dict[str, Any]], None]


def valid_case(rng: random.Random) -> dict[str, Any]:
    event = rng.choice(["ARRIVE", "HELP", "CANCEL"])
    minutes = rng.randint(1, 60) if event == "ARRIVE" else None
    source = rng.choice(["TEXT", "VOICE", "BUTTON", "SENSOR", "VISION", "CODE", "ENVIRONMENT"])
    return {
        "schemaVersion": 1,
        "eventKind": event,
        "source": source,
        "confidencePct": rng.randint(80, 100),
        "runnerUpPct": rng.randint(0, 60),
        "slots": {"minutes": minutes},
        "evidence": [
            {
                "kind": rng.choice(["OBSERVATION", "RULE", "MODEL", "USER"]),
                "ref": f"case.{rng.randint(0, 9999)}",
                "polarity": "POSITIVE",
                "weight": rng.randint(0, 100),
            }
        ],
        "hypothesisStatus": "TRUE",
        "authority": "PROPOSAL_ONLY",
    }


def mutators() -> dict[str, Mutator]:
    return {
        "unknown_top_level_key": lambda x: x.update(freeText="send now"),
        "wrong_schema_version": lambda x: x.update(schemaVersion=2),
        "schema_version_string": lambda x: x.update(schemaVersion="1"),
        "wrong_authority": lambda x: x.update(authority="EXECUTE"),
        "unknown_event": lambda x: x.update(eventKind="DANCE"),
        "unknown_source": lambda x: x.update(source="DREAM"),
        "confidence_bool": lambda x: x.update(confidencePct=True),
        "confidence_over": lambda x: x.update(confidencePct=101),
        "runner_up_under": lambda x: x.update(runnerUpPct=-1),
        "slots_unknown_key": lambda x: x.update(slots={"minutes": None, "seconds": 10}),
        "slots_missing_minutes": lambda x: x.update(slots={}),
        "minutes_zero": lambda x: x.update(eventKind="ARRIVE", slots={"minutes": 0}),
        "minutes_over": lambda x: x.update(eventKind="ARRIVE", slots={"minutes": 61}),
        "minutes_bool": lambda x: x.update(eventKind="ARRIVE", slots={"minutes": False}),
        "minutes_string": lambda x: x.update(eventKind="ARRIVE", slots={"minutes": "15"}),
        "minutes_on_help": lambda x: x.update(eventKind="HELP", slots={"minutes": 15}),
        "evidence_empty": lambda x: x.update(evidence=[]),
        "evidence_too_many": lambda x: x.update(evidence=x["evidence"] * 9),
        "evidence_not_object": lambda x: x.update(evidence=["bad"]),
        "evidence_unknown_key": lambda x: x.update(evidence=[{**x["evidence"][0], "extra": 1}]),
        "evidence_missing_field": lambda x: x.update(evidence=[{"kind": "OBSERVATION", "ref": "case.1", "polarity": "POSITIVE"}]),
        "evidence_kind": lambda x: x.update(evidence=[{**x["evidence"][0], "kind": "HUNCH"}]),
        "evidence_unicode_ref": lambda x: x.update(evidence=[{**x["evidence"][0], "ref": "ação"}]),
        "evidence_bad_ref": lambda x: x.update(evidence=[{**x["evidence"][0], "ref": "bad ref"}]),
        "evidence_bad_polarity": lambda x: x.update(evidence=[{**x["evidence"][0], "polarity": "MAYBE"}]),
        "evidence_bad_weight": lambda x: x.update(evidence=[{**x["evidence"][0], "weight": 101}]),
        "hypothesis_unknown": lambda x: x.update(hypothesisStatus="MAYBE"),
    }


def run_campaign(cases_per_mutator: int = 100) -> dict[str, Any]:
    if cases_per_mutator < 1:
        raise ValueError("cases_per_mutator must be positive")
    rng = random.Random(0x48525553)
    valid_total = 0
    valid_failures: list[str] = []
    mutated_total = 0
    mutation_failures: list[str] = []
    codes: Counter[str] = Counter()
    operators = mutators()

    for case_index in range(cases_per_mutator):
        base = valid_case(rng)
        proposal, issues = compile_ir(base)
        valid_total += 1
        if proposal is None or issues:
            valid_failures.append(f"valid_case[{case_index}]")
        for name, mutate in operators.items():
            candidate = copy.deepcopy(base)
            mutate(candidate)
            mutated_total += 1
            if validate_ir(candidate):
                continue
            mutation_failures.append(f"{name}[{case_index}]")
            compiled, _issues = compile_ir(candidate)
            if compiled is not None:
                mutation_failures.append(f"{name}[{case_index}]:compiled")
            else:
                mutation_failures.append(f"{name}[{case_index}]:no-issues")
            continue
        # Revalidate each valid base after all mutations to detect accidental aliasing.
        if validate_ir(base) != ():
            valid_failures.append(f"base-mutated[{case_index}]")

    # Collect one representative issue from each mutator for a readable raw report.
    for name, mutate in operators.items():
        candidate = valid_case(random.Random(7))
        mutate(candidate)
        issues = validate_ir(candidate)
        if issues:
            codes[f"{name}:{issues[0].code}"] += 1

    return {
        "seed": "0x48525553",
        "cases_per_mutator": cases_per_mutator,
        "mutator_count": len(operators),
        "valid_cases": valid_total,
        "mutated_cases": mutated_total,
        "valid_failures": valid_failures,
        "mutation_failures": mutation_failures,
        "issue_examples": dict(codes),
        "pass": not valid_failures and not mutation_failures,
    }


if __name__ == "__main__":
    report = run_campaign()
    print(json.dumps(report, indent=2, sort_keys=True, ensure_ascii=False))
    raise SystemExit(0 if report["pass"] else 1)
