from __future__ import annotations
import json
from pathlib import Path

ROOT=Path(__file__).parent

def load_protocol():
    return json.loads((ROOT/'utility_protocol_v1.json').read_text())

def validate_protocol(protocol):
    required={'CB_CONVENTIONAL_BASELINE','NS_NO_SYMBIONT','H_HERUS_CHECKED'}
    errors=[]
    if protocol.get('status') != 'protocol_ready_no_human_data': errors.append('human_data_must_not_be_fabricated')
    if set(protocol.get('conditions',())) != required: errors.append('conditions_incomplete')
    for metric in ('task_error','time_or_effort','host_or_interface_dependence'):
        if metric not in protocol.get('primary_metrics',()): errors.append('missing_primary_metric:'+metric)
    return tuple(errors)

def main():
    protocol=load_protocol(); errors=validate_protocol(protocol)
    return {'schema':protocol['schema'],'status':'READY_FOR_EXTERNAL_REVIEW' if not errors else 'BLOCKED','errors':errors,'human_results':None}

if __name__=='__main__': print(json.dumps(main(),indent=2,sort_keys=True))
