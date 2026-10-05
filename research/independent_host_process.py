#!/usr/bin/env python3
"""Standalone reference host for the frozen HERUS JSONL protocol.

This file intentionally imports only Python's standard library. It does not
import the learner, Episode, Problem, or any HERUS research module.
"""
import json
import sys
import time

ACTIONS = {'target_x': {'mode': 1}, 'target_y': {'level': 1}}
PUBLIC = {'mode': 0, 'level': 0}
CONTEXT = {'zone': 1}
STEP = 0

def reply(value):
    try:
        print(json.dumps(value, sort_keys=True), flush=True)
    except BrokenPipeError:
        raise SystemExit(0)

def main():
    global ACTIONS, PUBLIC, STEP
    for line in sys.stdin:
        try:
            request = json.loads(line)
            op = request.get('op')
            if op == 'ready':
                response = {'status': 'OK', 'protocol': 'herus-host-jsonl-v1'}
            elif op == 'describe':
                response = {'status': 'OK', 'actions': sorted(ACTIONS), 'protocol': 'herus-host-jsonl-v1'}
            elif op == 'probe':
                action = request.get('action')
                before = dict(PUBLIC); STEP += 1
                risk = 0 if action in ACTIONS else 1
                for key, value in ACTIONS.get(action, {}).items():
                    if key in PUBLIC: PUBLIC[key] = value
                response = {'status': 'OK', 'before': before, 'after': dict(PUBLIC), 'context': dict(CONTEXT), 'step': STEP, 'risk': risk}
            elif op == 'reset':
                PUBLIC = {'mode': 0, 'level': 0}; STEP += 1; response = {'status': 'OK'}
            elif op == 'rotate':
                ACTIONS = {'target_new': {'mode': 1}, 'target_level': {'level': 1}}; STEP += 1; response = {'status': 'OK'}
            elif op == 'delay':
                time.sleep(float(request.get('seconds', 0))); response = {'status': 'OK'}
            elif op == 'quit':
                reply({'status': 'OK'}); return
            else:
                response = {'status': 'ERROR', 'reason': 'unknown_operation'}
        except json.JSONDecodeError:
            response = {'status': 'ERROR', 'reason': 'invalid_json'}
        except Exception:
            response = {'status': 'ERROR', 'reason': 'malformed_request'}
        reply(response)

if __name__ == '__main__': main()
