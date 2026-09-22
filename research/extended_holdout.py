from __future__ import annotations
from dataclasses import dataclass, asdict
import hashlib, json

@dataclass(frozen=True)
class ExtendedFixture:
    fixture_id: str
    kind: str
    public_trace: tuple[str, ...]
    expected_runtime: str
    oracle_verdict: str
    oracle_reason: str

FIXTURES = (
    ExtendedFixture('t-opaque-epoch-01','temporal',('epoch:1','x:+1','epoch:2'),'UNSUPPORTED_BY_CONTRACT','UNSUPPORTED','STALE_EVIDENCE'),
    ExtendedFixture('s-gated-order-01','structural',('precondition:missing','b:before:a'),'UNSUPPORTED_BY_CONTRACT','UNSUPPORTED','PRECONDITION_UNPROVEN'),
    ExtendedFixture('s-resource-floor-01','resource',('resource:0','consume:1'),'UNSUPPORTED_BY_CONTRACT','UNSUPPORTED','RESOURCE_UNAVAILABLE'),
)

def run_extended_holdout():
    records=[]
    for fixture in FIXTURES:
        for repeat in range(3):
            record={**asdict(fixture),'repeat':repeat,'proposal_status':'UNSUPPORTED_BY_CONTRACT','proposal_execute_calls':0,'external_effect_count':0}
            record['digest']=hashlib.sha256(json.dumps(record,sort_keys=True,separators=(',',':')).encode()).hexdigest()
            records.append(record)
    return {'schema':'herus-extended-holdout-v1','classification':'not_proven','records':records,'interpretation':'Extended families are reserved until typed host contracts and independent executor support them.'}

if __name__=='__main__': print(json.dumps(run_extended_holdout(),indent=2,sort_keys=True))
