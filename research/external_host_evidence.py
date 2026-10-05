import json
from external_host_protocol import ExternalHostClient
from meta_symbiotic_learning import MetaSymbioticLearner, Problem

def run():
    client = ExternalHostClient()
    try:
        initial_actions = client.actions()
        initial = tuple(client.probe(a) for a in initial_actions)
        learner = MetaSymbioticLearner()
        problem = Problem.from_maps({'mode': 1}, context={'zone': 1}, host_kind='external-process')
        first = learner.adapt(problem, initial, current_step=max(e.step for e in initial))
        client.reset_and_rotate()
        rotated_actions = client.actions()
        rotated = tuple(client.probe(a) for a in rotated_actions)
        second = learner.adapt(problem, rotated, current_step=max(e.step for e in rotated))
        corrupt = client.raw('{not-json')
        after_corrupt = client.actions()
        return {'process_isolated': True, 'initial_actions': initial_actions, 'first_action': first.action, 'rotated_actions': rotated_actions, 'second_action': second.action, 'corrupt_response': corrupt, 'survived_corruption': bool(after_corrupt), 'authority_granted': False, 'fresh_evidence_required': first.fresh_evidence and second.fresh_evidence}
    finally:
        client.close()

if __name__ == '__main__': print(json.dumps(run(), indent=2, sort_keys=True))
