"""Validate the pre-registered Gate 0 protocol without executing the study."""
from __future__ import annotations
import json
from pathlib import Path

PROTOCOL = Path(__file__).parent / "evidence" / "gate0_protocol_v1.json"


def validate(protocol: dict[str, object]) -> list[str]:
    errors: list[str] = []
    dataset = protocol.get("dataset", {})
    endpoint = protocol.get("primary_endpoint", {})
    analysis = protocol.get("analysis", {})
    obj = protocol.get("herus_object", {})
    if protocol.get("status") != "frozen_pending_execution":
        errors.append("protocol_status_not_frozen_pending_execution")
    if dataset.get("identity_key") != "example_id" or dataset.get("join_policy") != "exact_id_only_no_positional_join":
        errors.append("identity_join_policy_not_fail_closed")
    if dataset.get("holdout_rows_expected") != 386 or dataset.get("holdout_season") != "S06":
        errors.append("holdout_contract_incomplete")
    if obj.get("implementation_id") in (None, "", "TO_BE_FILLED_BY_GATE_1"):
        errors.append("herus_implementation_id_not_frozen")
    if endpoint.get("name") != "macro_f1_full_coverage" or endpoint.get("abstention_counts_as_non_correct") is not True:
        errors.append("primary_endpoint_is_ambiguous")
    if not isinstance(endpoint.get("practical_margin"), (int, float)) or endpoint.get("practical_margin", 0) <= 0:
        errors.append("practical_margin_missing")
    if not isinstance(protocol.get("seeds"), list) or len(protocol["seeds"]) < 3:
        errors.append("minimum_three_seeds_missing")
    if analysis.get("holdout_peeking") is not False or analysis.get("ledger_required") is not True:
        errors.append("holdout_or_ledger_control_missing")
    if len(protocol.get("comparators", [])) < 2:
        errors.append("named_comparators_missing")
    return errors


def run(path: Path = PROTOCOL) -> dict[str, object]:
    protocol = json.loads(path.read_text(encoding="utf-8"))
    errors = validate(protocol)
    protocol_only = errors and set(errors) == {"herus_implementation_id_not_frozen"}
    return {
        "decision": "PASS_FOR_PROTOCOL_ONLY" if not errors or protocol_only else "BLOCK",
        "execution_authorized": False,
        "errors": errors,
        "implementation_frozen": not any(e == "herus_implementation_id_not_frozen" for e in errors),
    }


if __name__ == "__main__":
    print(json.dumps(run(), indent=2, ensure_ascii=False, sort_keys=True))
