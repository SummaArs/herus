from __future__ import annotations
import json, sys

def main(payload):
    case = payload['case']; budget = payload['budget']
    if budget['probes'] <= 0 or budget['cost'] <= 0:
        return {'proposal':None,'status':'UNSUPPORTED_BY_CONTRACT','trace':[],'probes':0,'cost':0}
    observations = case['observations'][:budget['probes']]
    trace = [{'event':'probe','observation':value,'cost':1} for value in observations]
    return {'proposal':case['actions'],'status':'SAFE_BUT_UNPROVEN','trace':trace,'probes':len(trace),'cost':len(trace)}

if __name__ == '__main__': print(json.dumps(main(json.load(sys.stdin)), sort_keys=True))
