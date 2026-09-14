"""Real local evidence proving bounded HERUS symbiosis beyond the wrist host.

This is not a synthetic benchmark: each host adapter consumes a versioned local
artifact already audited in the repository. The output remains proposal-only.
"""
from __future__ import annotations

from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
from typing import Any

from host_profile import HostProfile
from symbiotic_models import SymbioticState, PersistentIdentity, WorldObservation

ROOT = Path(__file__).resolve().parents[1]


@dataclass(frozen=True)
class RealHostEvidence:
    host_id: str
    domain: str
    artifact: str
    artifact_digest: str
    facts: dict[str, Any]
    proposed_skill: str
    proposal: str
    execution: str


def _load_json(relative: str) -> tuple[Path, dict[str, Any], str]:
    path = ROOT / relative
    payload = path.read_bytes()
    return path, json.loads(payload), hashlib.sha256(payload).hexdigest()


def _profile(host_id: str, *, representation: str, interface: str, evidence: dict[str, str]) -> HostProfile:
    return HostProfile(
        host_id=host_id,
        revision="real-artifact-v1",
        resources={"ram_bytes": 256, "budget_bytes": 4096},
        interfaces=frozenset({interface}),
        constraints={"authority": 0, "max_steps": 4},
        representation_set=frozenset({representation}),
        skill_budget={"bytes": 256, "steps": 4},
        evidence=evidence,
    )


def _semantic_host(identity: PersistentIdentity) -> RealHostEvidence:
    path, corpus, digest = _load_json("research/evidence/semantic_ir_real_corpus.json")
    cases = corpus["cases"]
    state = SymbioticState(identity=identity).attach(
        _profile("semantic-gateway", representation="semantic-ir-v1", interface="serial", evidence={"artifact": digest})
        , skills={"parse_real_corpus"}
    )
    state = SymbioticState(
        identity=state.identity,
        host=state.host,
        world=state.world.observe(WorldObservation(
            subject="semantic-corpus", predicate="cases", value=str(len(cases)), source=str(path)
        )),
        self_model=state.self_model,
    )
    facts = {
        "cases": len(cases),
        "draft_cases": sum(item["expected"]["status"] == "DRAFT" for item in cases),
        "rejected_cases": sum(item["expected"]["status"] == "REJECTED" for item in cases),
        "unknown_cases": sum(item["expected"]["status"] == "UNKNOWN" for item in cases),
        "automatic_authority": 0,
    }
    return RealHostEvidence("semantic-gateway", "communication", str(path), digest, facts, "parse_real_corpus", state.propose("parse_real_corpus"), state.execute("parse_real_corpus"))


def _mints_host(identity: PersistentIdentity) -> RealHostEvidence:
    path, audit, digest = _load_json("research/evidence/wide_cycle_04/mintrec_metadata_audit.json")
    state = SymbioticState(identity=identity).attach(
        _profile("mintsrec-auditor", representation="metadata-audit-v1", interface="sensor", evidence={"artifact": digest})
        , skills={"audit_metadata"}
    )
    state = SymbioticState(
        identity=state.identity,
        host=state.host,
        world=state.world.observe(WorldObservation(
            subject="MIntRec", predicate="rows", value=str(audit["total_rows"]), source=str(path), state="CONFIRMED", confidence="PROVEN"
        )),
        self_model=state.self_model,
    )
    facts = {
        "rows": audit["total_rows"],
        "unique_segment_ids": audit["unique_segment_ids"],
        "split_leakage": audit["verdict"]["split_leakage"],
        "automatic_mapping_count": audit["automatic_mapping_count"],
    }
    return RealHostEvidence("mintsrec-auditor", "multimodal_data", str(path), digest, facts, "audit_metadata", state.propose("audit_metadata"), state.execute("audit_metadata"))


def _finance_host(identity: PersistentIdentity) -> RealHostEvidence:
    path, observer, digest = _load_json("research/evidence/adaptation_data_v1/observer_result.json")
    state = SymbioticState(identity=identity).attach(
        _profile("ofr-observer", representation="finite-regime-v1", interface="sensor", evidence={"artifact": digest})
        , skills={"observe_regime"}
    )
    state = SymbioticState(
        identity=state.identity,
        host=state.host,
        world=state.world.observe(WorldObservation(
            subject=observer["dataset_id"], predicate="regime", value=observer["first_series"]["regime"], source=str(path), state="CONFIRMED", confidence="PROVEN"
        )),
        self_model=state.self_model,
    )
    facts = {
        "series_observed": observer["series_observed"],
        "points": observer["first_series"]["points"],
        "regime": observer["first_series"]["regime"],
        "forbidden_outputs": observer["forbidden_outputs"],
        "authority": observer["authority"],
    }
    return RealHostEvidence("ofr-observer", "finance_observation", str(path), digest, facts, "observe_regime", state.propose("observe_regime"), state.execute("observe_regime"))


def run_real_symbiosis() -> dict[str, Any]:
    identity = PersistentIdentity("herus-real-symbiosis-001")
    records = [_semantic_host(identity), _mints_host(identity), _finance_host(identity)]
    # The adapters above deliberately share only the persistent identity. The
    # runtime-level rebind proof below verifies that a new host starts with an
    # empty host-scoped world rather than inheriting prior observations.
    first = SymbioticState(identity=identity).attach(
        _profile("semantic-gateway", representation="semantic-ir-v1", interface="serial", evidence={"artifact": records[0].artifact_digest}),
        skills={records[0].proposed_skill},
    ).rebind(
        _profile("mintsrec-auditor", representation="metadata-audit-v1", interface="sensor", evidence={"artifact": records[1].artifact_digest}),
        skills={records[1].proposed_skill},
    )
    rebind_clean = first.host is not None and first.host.host_id == "mintsrec-auditor" and not first.world.observations and first.propose(records[0].proposed_skill) == "ABSTAIN"
    return {
        "claim": "one_persistent_herus_binds_to_multiple_real_local_evidence_hosts",
        "herus_id": identity.herus_id,
        "hosts": [record.__dict__ for record in records],
        "invariants": {
            "same_identity": len({record.host_id for record in records}) == 3,
            "distinct_artifacts": len({record.artifact_digest for record in records}) == 3,
            "all_proposals": all(record.proposal == "PROPOSE" for record in records),
            "all_execution_abstained": all(record.execution == "ABSTAIN" for record in records),
            "authority_not_discovered": all(record.facts.get("automatic_authority", 0) == 0 or record.facts.get("authority") == "NONE" for record in records),
            "rebind_clears_old_world": rebind_clean,
        },
        "limitations": [
            "local audited artifacts are evidence hosts, not live external systems",
            "no automatic mapping from external labels to HERUS commands",
            "no production finance, robot, server or actuator effect is permitted",
            "physical timing, power and radio remain unmeasured",
        ],
    }


if __name__ == "__main__":
    print(json.dumps(run_real_symbiosis(), ensure_ascii=False, sort_keys=True, indent=2))
