from __future__ import annotations
from dataclasses import dataclass, asdict
import hashlib, json, subprocess, sys

SCHEMA = 'herus-bridge-causal-v1'

def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(',', ':')).encode()).hexdigest()

@dataclass(frozen=True)
class Budget:
    probes: int = 4
    cost: int = 4

@dataclass(frozen=True)
class PublicCase:
    case_id: str
    actions: tuple[str, ...]
    observations: tuple[str, ...]
    hidden_commit_effect: bool
    expected_negative: bool

CASES = (
    PublicCase('opaque-a', ('select:1','select:2'), ('ready','ready'), False, False),
    PublicCase('opaque-b', ('select:1','select:2'), ('ready','ready'), True, True),
    PublicCase('order-a', ('prepare','confirm'), ('prepared','confirmed'), False, False),
    PublicCase('order-b', ('confirm','prepare'), ('prepared','confirmed'), True, True),
)

def _runtime(case, budget):
    if budget['probes'] <= 0 or budget['cost'] <= 0:
        return {'proposal': None, 'status': 'UNSUPPORTED_BY_CONTRACT', 'trace': [], 'probes': 0, 'cost': 0}
    trace = [{'event':'probe','observation':obs,'cost':1} for obs in case['observations'][:budget['probes']]]
    return {'proposal': list(case['actions']), 'status': 'SAFE_BUT_UNPROVEN', 'trace': trace, 'probes': len(trace), 'cost': len(trace)}

def _oracle(case, runtime):
    # The oracle sees private truth, but never receives expected_negative or reason labels.
    if runtime['proposal'] is None: return {'verdict':'BLOCKED','reason':'NO_PROPOSAL'}
    if case.hidden_commit_effect: return {'verdict':'BLOCKED','reason':'COMMIT_EFFECT_NOT_CLOSED'}
    if runtime['cost'] > 4: return {'verdict':'BLOCKED','reason':'COST_OVERRUN'}
    return {'verdict':'CANDIDATE','reason':'PRIVATE_TRANSITION_CLOSED'}

def run_case(case, budget=Budget()):
    payload = {'schema':SCHEMA, 'case':asdict(case), 'budget':asdict(budget)}
    runtime = json.loads(subprocess.check_output([sys.executable, '-m', 'research.bridge_runtime'], input=json.dumps(payload).encode()))
    oracle_input = {'schema':SCHEMA, 'case':{'case_id':case.case_id,'actions':list(case.actions),'observations':list(case.observations),'hidden_commit_effect':case.hidden_commit_effect}, 'runtime':runtime}
    oracle = json.loads(subprocess.check_output([sys.executable, '-m', 'research.bridge_oracle'], input=json.dumps(oracle_input).encode()))
    result = {'schema':SCHEMA, 'case_id':case.case_id, 'runtime':runtime, 'oracle':oracle, 'expected_negative':case.expected_negative, 'case_digest':digest(asdict(case))}
    result['result_digest'] = digest(result)
    return result

def campaign():
    return {'schema':SCHEMA, 'cases':[run_case(c) for c in CASES], 'runtime_digest':digest('research.bridge_runtime'), 'oracle_digest':digest('research.bridge_oracle')}

if __name__ == '__main__': print(json.dumps(campaign(), indent=2, sort_keys=True))
