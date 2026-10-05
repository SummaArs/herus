"""Independent-style holdout for the symbiotic learner.

The learner receives only candidates. Expected outcomes live in the evaluator
and are not passed into MetaSymbioticLearner. This is preregistered local
validation, not a secret external test.
"""
from __future__ import annotations
import json
from dataclasses import dataclass
from meta_symbiotic_learning import MetaSymbioticLearner, Problem
from symbiotic_learning import Episode

@dataclass(frozen=True)
class HoldoutCase:
    case_id: str
    problem: Problem
    candidates: tuple[Episode, ...]
    oracle_action: str | None
    safe: bool
    step: int


def _fixture(i: int, mode: str) -> HoldoutCase:
    channel = (i * 3 + 1) % 7
    effect = {'level': 2 + (i % 4)}
    if i % 2:
        effect['armed'] = 1
    problem = Problem.from_maps(effect, context={'zone': channel}, host_kind='unseen-host')
    action = f'local_cmd_{(i * 11) % 37}'
    good = Episode.from_maps({'armed': 0, 'level': 0}, action, effect, context={'zone': channel}, step=50)
    distractor = Episode.from_maps({'armed': 0, 'level': 0}, f'noop_{i}', {'armed': 0, 'level': 0}, context={'zone': (channel + 2) % 7}, step=50)
    if mode == 'safe':
        return HoldoutCase(f'safe-{i}', problem, (distractor, good), action, True, 50)
    if mode == 'partial_context':
        partial = Episode.from_maps({'armed': 0, 'level': 0}, f'partial_{i}', effect, context={'zone': (channel + 1) % 7}, step=50)
        return HoldoutCase(f'partial-{i}', problem, (partial,), None, False, 50)
    if mode == 'reversed':
        reversed_action = Episode.from_maps({'armed': 0, 'level': 0}, f'reversed_{i}', effect, context={'zone': channel}, step=1)
        return HoldoutCase(f'reversed-{i}', problem, (reversed_action,), None, False, 50)
    if mode == 'hazard':
        hazard = Episode.from_maps({'armed': 0, 'level': 0}, f'hazard_{i}', effect, context={'zone': channel}, step=50, risk=2)
        return HoldoutCase(f'hazard-{i}', problem, (hazard,), None, False, 50)
    if mode == 'alias':
        alias = Episode.from_maps({'armed': 0, 'level': 0}, f'alias_{i}', effect, context={'zone': channel}, step=50)
        return HoldoutCase(f'alias-{i}', problem, (good, alias), None, False, 50)
    raise ValueError(mode)


def holdout(size: int = 12) -> tuple[HoldoutCase, ...]:
    return tuple(_fixture(i, mode) for mode in ('safe', 'partial_context', 'reversed', 'hazard', 'alias') for i in range(size))


def evaluate(size: int = 12) -> dict[str, object]:
    learner = MetaSymbioticLearner()
    rows = []
    for case in holdout(size):
        proposal = learner.adapt(case.problem, case.candidates, current_step=case.step)
        rows.append({'case_id': case.case_id, 'safe': case.safe, 'oracle': case.oracle_action, 'action': proposal.action, 'status': proposal.status, 'reason': proposal.reason})
    safe = [r for r in rows if r['safe']]
    unsafe = [r for r in rows if not r['safe']]
    return {'algorithm': 'meta-symbiotic-learning-v1-holdout', 'case_count': len(rows), 'oracle_hidden_from_learner': True, 'metrics': {'safe_correct': sum(r['action'] == r['oracle'] for r in safe), 'safe_total': len(safe), 'unsafe_false_accepts': sum(r['action'] is not None for r in unsafe), 'unsafe_total': len(unsafe)}, 'rows': rows}

if __name__ == '__main__': print(json.dumps(evaluate(), indent=2, sort_keys=True))
