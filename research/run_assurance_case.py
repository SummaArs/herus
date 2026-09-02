"""Reproduce and serialize a declarative HERUS assurance case."""
from __future__ import annotations

import argparse
import json
from pathlib import Path

from critical_assurance_case import run_case


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("case", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    certificate = run_case(args.case)
    result = {
        "verdict": certificate.verdict.value,
        "reason": certificate.reason,
        "evidence_digest": certificate.evidence_digest,
        "abstract_verification": certificate.abstract_verification.verdict.value,
        "concrete_verification": certificate.concrete_verification.verdict.value,
        "machine_refinement": certificate.machine_refinement.verdict.value,
        "policy_refinement": certificate.policy_refinement.verdict.value,
        "call_paths": [
            {"rule_id": item.rule_id, "status": item.status, "source": item.source,
             "caller": item.caller, "detail": item.detail}
            for item in certificate.call_path_results
        ],
    }
    rendered = json.dumps(result, sort_keys=True, indent=2) + "\n"
    if args.output:
        args.output.write_text(rendered, encoding="utf-8")
    else:
        print(rendered, end="")
    return 0 if certificate.verdict.value == "ASSURED" else 1


if __name__ == "__main__":
    raise SystemExit(main())
