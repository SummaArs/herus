"""Small deterministic benchmark for Symbiotic Learning v2.

It compares name matching, effect matching and context-aware Symbiotic
Learning on the same black-box-style episodes. This is a research fixture,
not evidence of state-of-the-art performance.
"""
from __future__ import annotations
import json
from dataclasses import asdict, dataclass
from symbiotic_learning import Episode, SymbioticLearner

@dataclass(frozen=True)
class Case:
    name: str
    target: tuple[tuple[str, int], ...]
    candidates: tuple[Episode, ...]
    expected: str | None
    safe_to_propose: bool
    context: tuple[tuple[str, int], ...] = ()

def cases():
    return (
        Case('rename', (('mode', 1),), (Episode.from_maps({'mode': 0}, 'gesture', {'mode': 1}),), 'gesture', True),
        Case('ambiguous', (('mode', 1),), (Episode.from_maps({'mode': 0}, 'a', {'mode': 1}), Episode.from_maps({'mode': 0}, 'b', {'mode': 1})), None, False),
        Case('missing', (('volume', 1),), (Episode.from_maps({'mode': 0}, 'gesture', {'mode': 1}),), None, False),
        Case('wrong_context', (('mode', 1),), (Episode.from_maps({'mode': 0}, 'gesture', {'mode': 1}, context={'channel': 1}),), None, False, (('channel', 2),)),
        Case('stale', (('mode', 1),), (Episode.from_maps({'mode': 0}, 'gesture', {'mode': 1}, step=1), Episode.from_maps({'mode': 0}, 'gesture', {'mode': 1}, step=9)), None, False),
    )

def name_baseline(case):
    return case.candidates[0].action if case.candidates and case.candidates[0].effect == case.target else None

def effect_baseline(case):
    actions = {e.action for e in case.candidates if e.effect == case.target}
    return next(iter(actions)) if len(actions) == 1 else None

def run():
    results = []
    for case in cases():
        learner = SymbioticLearner(max_age=2)
        for episode in case.candidates: learner.observe(episode)
        proposal = learner.propose(case.target, case.candidates, context=case.context, current_step=9)
        results.append({'case': case.name, 'expected': case.expected, 'safe_to_propose': case.safe_to_propose, 'name': name_baseline(case), 'effect': effect_baseline(case), 'symbiotic': proposal.action, 'status': proposal.status, 'reason': proposal.reason})
    return {'algorithm': 'symbiotic-learning-v2', 'cases': results, 'safety_rule': 'unsafe cases must abstain'}

if __name__ == '__main__': print(json.dumps(run(), indent=2, sort_keys=True))
