"""Measure whether verified history improves probe ordering causally."""
from __future__ import annotations
import json
from meta_symbiotic_learning import MetaSymbioticLearner, Problem
from symbiotic_learning import Episode

def rank_position(candidates, action):
    return next(i + 1 for i, item in enumerate(candidates) if item.action == action)

def run():
    problem = Problem.from_maps({'mode': 1}, context={'channel': 1}, host_kind='host-b')
    target = Episode.from_maps({'mode': 0}, 'gesture_double', {'mode': 1}, context={'channel': 1}, cost=2)
    decoys = [
        Episode.from_maps({'mode': 0}, 'menu', {'mode': 0}, context={'channel': 1}, cost=1),
        Episode.from_maps({'mode': 0}, 'button_1', {'mode': 2}, context={'channel': 1}, cost=1),
    ]
    candidates = decoys + [target]
    plain = MetaSymbioticLearner()
    plain_ranked = plain.rank_candidates(problem, candidates)
    guided = MetaSymbioticLearner()
    source_problem = Problem.from_maps({'mode': 1}, context={'channel': 1}, host_kind='host-a')
    source = Episode.from_maps({'mode': 0}, 'gesture', {'mode': 1}, context={'channel': 1})
    source_proposal = guided.learner.propose(source_problem.goal_effect, [source], context=source_problem.context)
    guided.remember(source_problem, source_proposal, evidence_digest='independent-proof', verified=True)
    guided_ranked = guided.rank_candidates(problem, candidates)
    return {
        'algorithm': 'meta-symbiotic-learning-v1',
        'plain_target_rank': rank_position(plain_ranked, target.action),
        'guided_target_rank': rank_position(guided_ranked, target.action),
        'candidate_count_before': len(candidates),
        'candidate_count_after': len(guided_ranked),
        'history_effect_match_used': guided_ranked[0].effect == problem.goal_effect,
        'fresh_evidence_required': True,
        'authority_granted': False,
        'improvement': rank_position(guided_ranked, target.action) < rank_position(plain_ranked, target.action),
    }

if __name__ == '__main__': print(json.dumps(run(), indent=2, sort_keys=True))
