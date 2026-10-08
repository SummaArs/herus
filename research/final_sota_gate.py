"""Final SOTA gate: a valid benchmark is not enough for a general SOTA claim."""
from __future__ import annotations
import json
from pathlib import Path


def decide(evidence: dict) -> dict:
    blockers = []
    if not evidence.get('independent_dataset_evidence'):
        blockers.append('independent_dataset_missing')
    if not evidence.get('current_state_of_art_baseline'):
        blockers.append('current_sota_baseline_missing')
    if evidence.get('adapter_only'):
        blockers.append('herus_core_not_isolated')
    if evidence.get('matched_risk_coverage') is not True:
        blockers.append('matched_risk_coverage_incomplete')
    if evidence.get('bootstrap_complete') is not True:
        blockers.append('paired_bootstrap_incomplete')
    return {'schema': 'herus-final-sota-decision-v1', 'status': 'BLOCKED' if blockers else 'ELIGIBLE_FOR_INDEPENDENT_REVIEW', 'claim_allowed': False, 'scientific_claim': 'not_proven' if blockers else 'not_yet_accepted', 'blockers': blockers}


if __name__ == '__main__':
    path = Path('research/evidence/paired_risk_coverage_mintrec_v1.json')
    print(json.dumps(decide(json.loads(path.read_text())), indent=2, sort_keys=True))
