"""Execution boundary for evaluating programming proposals.

This evaluator is deliberately outside ``herus_symbiotic``. The public
library proposes; this research harness executes only an explicit task in a
temporary directory and returns evidence suitable for the programming ledger.
It is not a general sandbox or a security boundary.
"""
from __future__ import annotations
from dataclasses import dataclass
import hashlib
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Tuple

@dataclass(frozen=True)
class ProgrammingTask:
    task_id: str
    candidate_filename: str
    runner_source: str
    timeout_seconds: float = 2.0

@dataclass(frozen=True)
class EvaluationResult:
    task_id: str
    status: str
    authority: str
    exit_code: int | None
    stdout_digest: str
    stderr_digest: str
    detail: str


def evaluate_python_candidate(task: ProgrammingTask, candidate_source: str) -> EvaluationResult:
    if not task.task_id or not task.candidate_filename.endswith('.py'):
        return _result(task, 'invalid_request', None, '', 'invalid task contract')
    if task.timeout_seconds <= 0 or task.timeout_seconds > 30:
        return _result(task, 'invalid_request', None, '', 'timeout outside bounded range')
    try:
        compile(candidate_source, task.candidate_filename, 'exec')
        compile(task.runner_source, 'runner.py', 'exec')
    except SyntaxError as error:
        return _result(task, 'candidate_syntax_error', None, '', str(error))
    with tempfile.TemporaryDirectory(prefix='herus-program-eval-') as directory:
        root = Path(directory)
        candidate = root / task.candidate_filename
        runner = root / 'runner.py'
        candidate.write_text(candidate_source, encoding='utf-8')
        runner.write_text(task.runner_source.replace('__CANDIDATE__', repr(str(candidate))), encoding='utf-8')
        try:
            completed = subprocess.run(
                [sys.executable, '-I', str(runner)],
                cwd=root,
                stdin=subprocess.DEVNULL,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                timeout=task.timeout_seconds,
                env={},
                check=False,
            )
        except subprocess.TimeoutExpired as error:
            return _result(task, 'timeout', None, _text(error.stdout), 'deadline exceeded')
        status = 'pass' if completed.returncode == 0 else 'fail'
        detail = 'all runner assertions passed' if status == 'pass' else 'runner returned non-zero'
        return _result(task, status, completed.returncode, completed.stdout, completed.stderr or detail)


def _result(task: ProgrammingTask, status: str, exit_code: int | None, stdout: str, stderr: str) -> EvaluationResult:
    digest = lambda value: hashlib.sha256(value.encode('utf-8', errors='replace')).hexdigest()
    return EvaluationResult(task.task_id, status, 'isolated-subprocess', exit_code, digest(stdout), digest(stderr), stderr[:240])


def _text(value: object) -> str:
    if isinstance(value, bytes):
        return value.decode('utf-8', errors='replace')
    return str(value or '')
