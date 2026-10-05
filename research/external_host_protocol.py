"""External host protocol for process-isolated symbiosis tests."""
from __future__ import annotations
import json
import select
import subprocess
import sys
import time
from dataclasses import dataclass
from typing import Mapping
from symbiotic_learning import Episode

@dataclass(frozen=True)
class WireObservation:
    action: str
    before: dict[str, int]
    after: dict[str, int]
    context: dict[str, int]
    step: int
    risk: int

class ExternalHostClient:
    def __init__(self, *, timeout: float = 0.5, startup_timeout: float = 1.0) -> None:
        self.timeout = timeout
        self.proc = subprocess.Popen([sys.executable, __file__, '--server'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True, bufsize=1)
        old_timeout = self.timeout
        self.timeout = startup_timeout
        self._request({'op': 'ready'})
        self.timeout = old_timeout

    def _request(self, payload: dict[str, object]) -> dict[str, object]:
        assert self.proc.stdin and self.proc.stdout
        self.proc.stdin.write(json.dumps(payload, sort_keys=True) + '\n')
        self.proc.stdin.flush()
        ready, _, _ = select.select([self.proc.stdout], [], [], self.timeout)
        if not ready:
            self.proc.kill()
            raise TimeoutError('external_host_timeout')
        line = self.proc.stdout.readline()
        if not line:
            raise RuntimeError('external_host_closed')
        response = json.loads(line)
        if response.get('status') == 'ERROR':
            raise ValueError(str(response.get('reason', 'external_host_error')))
        return response

    def actions(self) -> tuple[str, ...]:
        return tuple(self._request({'op': 'describe'})['actions'])  # type: ignore[return-value]

    def probe(self, action: str) -> Episode:
        value = self._request({'op': 'probe', 'action': action})
        return Episode.from_maps(value['before'], action, value['after'], context=value['context'], step=int(value['step']), risk=int(value['risk']))  # type: ignore[arg-type]

    def reset_and_rotate(self) -> None:
        self._request({'op': 'reset'})
        self._request({'op': 'rotate'})

    def raw(self, line: str) -> str:
        assert self.proc.stdin and self.proc.stdout
        self.proc.stdin.write(line + '\n'); self.proc.stdin.flush()
        ready, _, _ = select.select([self.proc.stdout], [], [], self.timeout)
        if not ready:
            self.proc.kill()
            self.proc.wait(timeout=1)
            if self.proc.stdin: self.proc.stdin.close()
            if self.proc.stdout: self.proc.stdout.close()
            raise TimeoutError('external_host_timeout')
        return self.proc.stdout.readline().strip()

    def close(self) -> None:
        if self.proc.poll() is None:
            try: self._request({'op': 'quit'})
            except Exception: self.proc.kill()
        self.proc.wait(timeout=1)
        if self.proc.stdin: self.proc.stdin.close()
        if self.proc.stdout: self.proc.stdout.close()


def _server() -> None:
    actions = {'target_x': {'mode': 1}, 'target_y': {'level': 1}}
    public = {'mode': 0, 'level': 0}
    context = {'zone': 1}
    step = 0
    for line in sys.stdin:
        try:
            request = json.loads(line)
            op = request.get('op')
            if op == 'ready': response = {'status': 'OK'}
            elif op == 'describe': response = {'status': 'OK', 'actions': sorted(actions)}
            elif op == 'probe':
                action = request.get('action')
                before = dict(public); step += 1; risk = 0 if action in actions else 1
                for key, value in actions.get(action, {}).items(): public[key] = value
                response = {'status': 'OK', 'before': before, 'after': dict(public), 'context': dict(context), 'step': step, 'risk': risk}
            elif op == 'reset': public = {'mode': 0, 'level': 0}; step += 1; response = {'status': 'OK'}
            elif op == 'rotate': actions = {'target_new': {'mode': 1}, 'target_level': {'level': 1}}; step += 1; response = {'status': 'OK'}
            elif op == 'delay': time.sleep(float(request.get('seconds', 0))); response = {'status': 'OK'}
            elif op == 'quit': response = {'status': 'OK'}; print(json.dumps(response), flush=True); return
            else: response = {'status': 'ERROR', 'reason': 'unknown_operation'}
        except json.JSONDecodeError: response = {'status': 'ERROR', 'reason': 'invalid_json'}
        except Exception: response = {'status': 'ERROR', 'reason': 'malformed_request'}
        try:
            print(json.dumps(response, sort_keys=True), flush=True)
        except BrokenPipeError:
            return

if __name__ == '__main__':
    if '--server' in sys.argv: _server()
