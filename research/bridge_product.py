from __future__ import annotations
from dataclasses import dataclass

CONDITIONS = ('CB_CONVENTIONAL_BASELINE','NS_NO_SYMBIONT','H_HERUS_CHECKED')

@dataclass
class BridgeSession:
    condition: str
    state: str = 'SETUP'
    events: list[dict] | None = None
    def __post_init__(self):
        if self.condition not in CONDITIONS: raise ValueError('unknown condition')
        self.events = [] if self.events is None else self.events
    def emit(self, event: str, **data):
        allowed = {'SETUP': {'define'}, 'DEFINED': {'switch'}, 'SWITCHED': {'propose'}, 'PROPOSED': {'preview'}, 'PREVIEWED': {'confirm','cancel','abstain','stop'}, 'CONFIRMED': set(), 'CANCELLED': set(), 'ABSTAINED': set(), 'STOPPED': set()}
        if event not in allowed.get(self.state, set()): raise ValueError(f'invalid_transition:{self.state}:{event}')
        self.events.append({'event':event, **data})
        self.state = {'define':'DEFINED','switch':'SWITCHED','propose':'PROPOSED','preview':'PREVIEWED','confirm':'CONFIRMED','cancel':'CANCELLED','abstain':'ABSTAINED','stop':'STOPPED'}[event]
        return self.state

def run_synthetic(condition: str, outcome: str = 'cancel'):
    session = BridgeSession(condition)
    session.emit('define', options=3, no_action=True)
    session.emit('switch', target='local-target')
    session.emit('propose', proposal='prepared-response-1' if condition == 'H_HERUS_CHECKED' else 'manual')
    session.emit('preview', text='prepared response preview')
    session.emit(outcome)
    return {'schema':'herus-bridge-product-v1','condition':condition,'state':session.state,'events':session.events,'external_effects':0,'audio_transmitted':False}

if __name__ == '__main__':
    for condition in CONDITIONS: print(run_synthetic(condition))
