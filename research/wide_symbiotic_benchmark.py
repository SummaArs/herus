"""Wide deterministic campaign for Symbiotic Learning.

The campaign is host-only and synthetic, but separates positive utility from
negative safety cases. It is a regression instrument, not external evidence.
"""
from __future__ import annotations
import json
from dataclasses import asdict, dataclass
from meta_symbiotic_learning import MetaSymbioticLearner, Problem
from symbiotic_learning import Episode

@dataclass(frozen=True)
class CampaignCase:
    case_id: str
    category: str
    problem: Problem
    candidates: tuple[Episode, ...]
    expected_action: str | None
    safe_to_propose: bool
    current_step: int = 0


def _case(i: int, category: str) -> CampaignCase:
    channel = i % 5
    effect = {'mode': 1 + (i % 3)}
    problem = Problem.from_maps(effect, context={'channel': channel}, host_kind='target')
    target = Episode.from_maps({'mode': 0}, f'gesture_{i}', effect, context={'channel': channel}, step=10)
    decoy = Episode.from_maps({'mode': 0}, f'decoy_{i}', {'mode': 0}, context={'channel': (channel + 1) % 5}, cost=2, step=10)
    if category == 'safe':
        return CampaignCase(f'{category}-{i}', category, problem, (decoy, target), target.action, True, 10)
    if category == 'ambiguous':
        alias = Episode.from_maps({'mode': 0}, f'alias_{i}', effect, context={'channel': channel}, step=10)
        return CampaignCase(f'{category}-{i}', category, problem, (target, alias), None, False, 10)
    if category == 'context_mismatch':
        wrong = Episode.from_maps({'mode': 0}, f'wrong_context_{i}', effect, context={'channel': (channel + 1) % 5}, step=10)
        return CampaignCase(f'{category}-{i}', category, problem, (wrong,), None, False, 10)
    if category == 'stale':
        old = Episode.from_maps({'mode': 0}, f'stale_{i}', effect, context={'channel': channel}, step=1)
        return CampaignCase(f'{category}-{i}', category, problem, (old,), None, False, 10)
    if category == 'risky':
        risky = Episode.from_maps({'mode': 0}, f'risky_{i}', effect, context={'channel': channel}, step=10, risk=1)
        return CampaignCase(f'{category}-{i}', category, problem, (risky,), None, False, 10)
    raise ValueError(category)


def campaign(size_per_category: int = 20) -> tuple[CampaignCase, ...]:
    categories = ('safe', 'ambiguous', 'context_mismatch', 'stale', 'risky')
    return tuple(_case(i, category) for category in categories for i in range(size_per_category))


def fixed_name(case: CampaignCase) -> str | None:
    return case.candidates[0].action if case.candidates else None


def effect_only(case: CampaignCase) -> str | None:
    matches = [e.action for e in case.candidates if e.effect == case.problem.goal_effect]
    return matches[0] if len(set(matches)) == 1 else None


def evaluate(size_per_category: int = 20) -> dict[str, object]:
    cases = campaign(size_per_category)
    meta = MetaSymbioticLearner()
    # Seed only a verified reference pattern; target candidates still need fresh evidence.
    seed_problem = Problem.from_maps({'mode': 1}, context={'channel': 0}, host_kind='source')
    seed = Episode.from_maps({'mode': 0}, 'old_gesture', {'mode': 1}, context={'channel': 0}, step=1)
    seed_proposal = meta.learner.propose(seed_problem.goal_effect, [seed], context=seed_problem.context)
    meta.remember(seed_problem, seed_proposal, evidence_digest='independent-seed-proof', verified=True)
    rows = []
    for case in cases:
        proposal = meta.adapt(case.problem, case.candidates, current_step=case.current_step)
        rows.append({
            'case_id': case.case_id,
            'category': case.category,
            'expected': case.expected_action,
            'fixed_name': fixed_name(case),
            'effect_only': effect_only(case),
            'meta_action': proposal.action,
            'meta_status': proposal.status,
            'meta_reason': proposal.reason,
            'safe_to_propose': case.safe_to_propose,
        })
    metrics = {}
    for key in ('fixed_name', 'effect_only', 'meta_action'):
        safe = [r for r in rows if r['safe_to_propose']]
        unsafe = [r for r in rows if not r['safe_to_propose']]
        metrics[key] = {
            'safe_correct': sum(r[key] == r['expected'] for r in safe),
            'safe_total': len(safe),
            'unsafe_false_accepts': sum(r[key] is not None for r in unsafe),
            'unsafe_total': len(unsafe),
            'safe_accuracy': sum(r[key] == r['expected'] for r in safe) / len(safe),
            'false_accept_rate': sum(r[key] is not None for r in unsafe) / len(unsafe),
        }
    return {'algorithm': 'meta-symbiotic-learning-v1-wide-campaign', 'case_count': len(rows), 'metrics': metrics, 'rows': rows}

if __name__ == '__main__': print(json.dumps(evaluate(), indent=2, sort_keys=True))
