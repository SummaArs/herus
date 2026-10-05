"""Black-box host harness for the meta-symbiotic learner.

The host owns its private state and action map. The learner sees only public
observations returned by probes; it cannot read or mutate the private model.
"""
from __future__ import annotations
import json
from dataclasses import dataclass
from typing import Mapping, Sequence
from meta_symbiotic_learning import MetaSymbioticLearner, Problem
from symbiotic_learning import Episode

@dataclass(frozen=True)
class Observation:
    action: str
    before: tuple[tuple[str, int], ...]
    after: tuple[tuple[str, int], ...]
    context: tuple[tuple[str, int], ...]
    step: int
    risk: int

class BlackBoxHost:
    def __init__(self, *, host_kind: str, action_map: Mapping[str, Mapping[str, int]], hidden: Mapping[str, int] | None = None) -> None:
        self.host_kind = host_kind
        self._action_map = {str(k): dict(v) for k, v in action_map.items()}
        self._hidden = dict(hidden or {'battery': 97, 'internal_mode': 4})
        self._public = {'mode': 0, 'level': 0}
        self._step = 0
        self._context = {'zone': 1}

    def public_state(self) -> tuple[tuple[str, int], ...]:
        return tuple(sorted(self._public.items()))

    def action_names(self) -> tuple[str, ...]:
        return tuple(sorted(self._action_map))

    def rotate_interface(self, action_map: Mapping[str, Mapping[str, int]]) -> None:
        self._action_map = {str(k): dict(v) for k, v in action_map.items()}
        self._step += 1

    def reset_task(self) -> None:
        self._public = {'mode': 0, 'level': 0}
        self._step += 1

    def probe(self, action: str) -> Observation:
        before = self.public_state()
        self._step += 1
        delta = self._action_map.get(action, {})
        for key, value in delta.items():
            if key in self._public:
                self._public[key] = value
        return Observation(action, before, self.public_state(), tuple(sorted(self._context.items())), self._step, 0 if action in self._action_map else 1)


def as_episode(observation: Observation) -> Episode:
    return Episode(observation.before, observation.action, observation.after, context=observation.context, step=observation.step, risk=observation.risk)


def probe_host(host: BlackBoxHost) -> tuple[Episode, ...]:
    return tuple(as_episode(host.probe(action)) for action in host.action_names())


def run() -> dict[str, object]:
    source = BlackBoxHost(host_kind='source', action_map={'source_a': {'mode': 1}, 'source_b': {'level': 1}})
    source_episodes = probe_host(source)
    target = BlackBoxHost(host_kind='target', action_map={'target_x': {'mode': 1}, 'target_y': {'level': 1}})
    target_episodes = probe_host(target)
    problem = Problem.from_maps({'mode': 1}, context={'zone': 1}, host_kind='target')
    meta = MetaSymbioticLearner()
    source_problem = Problem.from_maps({'mode': 1}, context={'zone': 1}, host_kind='source')
    source_raw = meta.learner.propose(source_problem.goal_effect, source_episodes, context=source_problem.context)
    meta.remember(source_problem, source_raw, evidence_digest='blackbox-source-proof', verified=True)
    adapted = meta.adapt(problem, target_episodes, current_step=max(e.step for e in target_episodes))
    target.reset_task()
    target.rotate_interface({'target_new': {'mode': 1}, 'target_level': {'level': 1}})
    after_rotation = probe_host(target)
    rotated = meta.adapt(problem, after_rotation, current_step=max(e.step for e in after_rotation))
    return {'source_action': source_raw.action, 'target_action': adapted.action, 'rotated_action': rotated.action, 'target_host_private_state_hidden': True, 'target_action_space_changed': True, 'fresh_evidence_required': adapted.fresh_evidence and rotated.fresh_evidence, 'authority_granted': False}

if __name__ == '__main__': print(json.dumps(run(), indent=2, sort_keys=True))
