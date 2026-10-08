"""Fail-closed admission gate for an external LLM candidate.

The existing gpt-5.5 ledger is holdout-only. It must not enter the universal
router until an independent calibration ledger exists.
"""
from __future__ import annotations
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parent
EVIDENCE=ROOT/'evidence/llm_minds14_reference_v1.json'


def assess() -> dict:
    data=json.loads(EVIDENCE.read_text())
    protocol=data.get('protocol',{})
    reasons=[]
    if protocol.get('mode') != 'zero-shot': reasons.append('protocol_not_zero_shot')
    if protocol.get('training_on_dataset'): reasons.append('unexpected_dataset_training')
    if not data.get('ledger'): reasons.append('missing_holdout_ledger')
    if not (ROOT/'evidence/llm_minds14_calibration_v1.json').exists():
        reasons.append('missing_independent_calibration_ledger')
    return {'schema':'herus-universal-llm-admission-v1','candidate':'gpt-5.5','status':'BLOCKED' if reasons else 'ELIGIBLE_PENDING_THRESHOLD','reasons':reasons,'holdout_examples':len(data.get('ledger',[])),'router_admitted':False,'claim_boundary':'holdout-only LLM reference cannot be routed without calibration'}

if __name__=='__main__': print(json.dumps(assess(),indent=2))
