import json
from programming_evaluator import ProgrammingTask, evaluate_python_candidate

RUNNER = '''
import importlib.util
spec = importlib.util.spec_from_file_location("candidate", __CANDIDATE__)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
assert module.solve(2, 3) == 5
assert module.solve(-1, 1) == 0
'''

def main():
    cases = (
        ("positive", ProgrammingTask("add.v1", "candidate.py", RUNNER), "def solve(a, b):\n    return a + b\n"),
        ("negative", ProgrammingTask("add.v1", "candidate.py", RUNNER), "def solve(a, b):\n    return a - b\n"),
        ("timeout", ProgrammingTask("loop.v1", "candidate.py", "while True: pass\n", 0.1), "def solve(a, b):\n    return a + b\n"),
    )
    results = []
    for case_kind, task, source in cases:
        result = evaluate_python_candidate(task, source)
        results.append({"case_kind": case_kind, **result.__dict__})
    print(json.dumps({"authority": "isolated-subprocess", "cases": results}, indent=2, sort_keys=True))

if __name__ == "__main__":
    main()
