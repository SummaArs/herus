"""Client for the frozen HERUS Host JSONL v1 protocol.

The host implementation is intentionally elsewhere in
``independent_host_process.py`` and is launched as a separate process.
"""
from __future__ import annotations
import json
import select
import subprocess
import sys
from pathlib import Path
from symbiotic_learning import Episode

class ExternalHostClient:
    def __init__(self, *, timeout: float = 0.5, startup_timeout: float = 1.0, executable: str | None = None) -> None:
        self.timeout = timeout
        host_path = Path(__file__).with_name('independent_host_process.py')
        command = [executable] if executable else [sys.executable, str(host_path)]
        self.proc = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True, bufsize=1)
        old_timeout = self.timeout
        self.timeout = startup_timeout
        ready = self._request({'op': 'ready'})
        if ready.get('protocol') != 'herus-host-jsonl-v1':
            self.proc.kill()
            raise RuntimeError('protocol_version_mismatch')
        self.timeout = old_timeout

    def _request(self, payload: dict[str, object]) -> dict[str, object]:
        assert self.proc.stdin and self.proc.stdout
        self.proc.stdin.write(json.dumps(payload, sort_keys=True) + '\n')
        self.proc.stdin.flush()
        ready, _, _ = select.select([self.proc.stdout], [], [], self.timeout)
        if not ready:
            self.proc.kill()
            self.proc.wait(timeout=1)
            if self.proc.stdin: self.proc.stdin.close()
            if self.proc.stdout: self.proc.stdout.close()
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
        self.proc.stdin.write(line + '\n')
        self.proc.stdin.flush()
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
