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
        "structural_verdict": certificate.structural_verdict,
        "structural_reason": certificate.structural_reason,
        "inventory_verdict": certificate.inventory_verdict,
        "inventory_reason": certificate.inventory_reason,
        "assurance_scope": certificate.assurance_scope,
        "sink_audit_verdict": certificate.sink_audit_verdict,
        "sink_audit_reason": certificate.sink_audit_reason,
        "sink_audit_results": [
            {"sink_id": item.sink_id, "status": item.status, "source": item.source,
             "function": item.function, "detail": item.detail}
            for item in certificate.sink_audit_results
        ],
        "c11_structural_verdict": certificate.c11_structural_verdict,
        "c11_structural_reason": certificate.c11_structural_reason,
        "c11_structural_results": [
            {"sink_id": item.sink_id, "status": item.status, "source": item.source,
             "function": item.function, "detail": item.detail}
            for item in certificate.c11_structural_results
        ],
        "candidate_verdict": certificate.candidate_verdict,
        "candidate_reason": certificate.candidate_reason,
        "candidate_results": [
            {"source": item.source, "function": item.function, "operation": item.operation,
             "status": item.status, "detail": item.detail}
            for item in certificate.candidate_results
        ],
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
