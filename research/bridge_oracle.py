from __future__ import annotations
import json, sys

def main(payload):
    case = payload['case']; runtime = payload['runtime']
    if runtime.get('proposal') is None: return {'verdict':'BLOCKED','reason':'NO_PROPOSAL'}
    if case.get('hidden_commit_effect'): return {'verdict':'BLOCKED','reason':'COMMIT_EFFECT_NOT_CLOSED'}
    if runtime.get('cost', 0) > 4: return {'verdict':'BLOCKED','reason':'COST_OVERRUN'}
    return {'verdict':'CANDIDATE','reason':'PRIVATE_TRANSITION_CLOSED'}

if __name__ == '__main__': print(json.dumps(main(json.load(sys.stdin)), sort_keys=True))
