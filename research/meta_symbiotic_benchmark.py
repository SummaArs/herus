"""Measure whether verified history improves candidate ordering safely."""
from __future__ import annotations
import json
from meta_symbiotic_learning import MetaSymbioticLearner, Problem
from symbiotic_learning import Episode

def run():
    problem = Problem.from_maps({'mode': 1}, context={'channel': 1}, host_kind='host-b')
    target = Episode.from_maps({'mode': 0}, 'gesture_double', {'mode': 1}, context={'channel': 1})
    decoys = [
        Episode.from_maps({'mode': 0}, 'menu', {'mode': 0}, context={'channel': 9}, cost=2),
        Episode.from_maps({'mode': 0}, 'button_1', {'mode': 2}, context={'channel': 1}, cost=2),
    ]
    candidates = decoys + [target]
    plain_first = candidates[0].action
    meta = MetaSymbioticLearner()
    source_problem = Problem.from_maps({'mode': 1}, context={'channel': 1}, host_kind='host-a')
    source = Episode.from_maps({'mode': 0}, 'gesture', {'mode': 1}, context={'channel': 1})
    source_proposal = meta.learner.propose(source_problem.goal_effect, [source], context=source_problem.context)
    meta.remember(source_problem, source_proposal, evidence_digest='independent-proof', verified=True)
    ranked = meta.rank_candidates(problem, candidates)
    result = {
        'algorithm': 'meta-symbiotic-learning-v1',
        'plain_first_action': plain_first,
        'guided_first_action': ranked[0].action,
        'candidate_count_before': len(candidates),
        'candidate_count_after': len(ranked),
        'fresh_evidence_required': True,
        'authority_granted': False,
        'improvement': ranked[0].action == target.action and len(ranked) == len(candidates),
    }
    return result

if __name__ == '__main__': print(json.dumps(run(), indent=2, sort_keys=True))
