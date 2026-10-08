"""MInDS-14 benchmark for the native SymbioticLearner selector.

The selector receives only a feature state at prediction time. The holdout
label is used solely for scoring, never as target_effect or decision input.
"""
from __future__ import annotations
import hashlib, json, re
from pathlib import Path
from research.symbiotic_learning import Episode, SymbioticLearner, _state
from independent_minds14_benchmark import fetch_rows, split

def encode(text):
    tokens=re.findall(r"[a-zà-ÿ]+", text.lower())
    # Deliberately finite, auditable coarse state; no label-derived feature.
    stable=lambda token: int.from_bytes(hashlib.sha256(token.encode()).digest()[:4], 'big') % 997
    return _state({'length_bin': min(len(tokens)//3, 20), 'first_1': stable(tokens[0]) if tokens else 0, 'first_2': stable(tokens[1]) if len(tokens)>1 else 0})

def run():
    rows=fetch_rows(); fit,cal,hold=split(rows); learner=SymbioticLearner(max_observations=len(fit)+1, max_cost=len(fit)+1, max_risk=0, max_age=10**9)
    episodes=[]
    for i,row in enumerate(fit):
        before=encode(row['text']); after=_state({'intent':i})
        episode=Episode(before,row['label'],after,cost=1,risk=0,step=i)
        episodes.append(episode); learner.observe(episode)
    ledger=[]
    for i,row in enumerate(hold):
        proposal=learner.propose_action(encode(row['text']), episodes, current_step=len(fit)+i)
        accepted=proposal.action is not None
        ledger.append({'example_id':row['path'],'label':row['label'],'prediction':proposal.action,'accepted':accepted,'correct':accepted and proposal.action==row['label'],'reason':proposal.reason,'confidence_milli':proposal.confidence_milli})
    accepted=[x for x in ledger if x['accepted']]
    return {'schema':'herus-native-core-minds14-v1','dataset':{'id':'PolyAI/minds14','config':'pt-PT','rows':len(rows),'fit':len(fit),'calibration':len(cal),'holdout':len(hold)},'protocol':{'input':'coarse text-derived state only','target_effect_passed_to_selector':False,'adapter':'none','calibration_labels_used':False,'claim_boundary':'native core identity test; not SOTA claim'},'metrics':{'accuracy':sum(x['correct'] for x in ledger)/len(ledger),'coverage':len(accepted)/len(ledger),'selective_accuracy':sum(x['correct'] for x in accepted)/len(accepted) if accepted else 0.0,'abstentions':len(ledger)-len(accepted)},'ledger':ledger,'limits':['exact state matching is intentionally conservative','coarse state is not a language model','MInDS speaker independence not verified']}

if __name__=='__main__':
    out=run(); p=Path('research/evidence/native_core_minds14_v1.json'); p.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n'); print(json.dumps({'path':str(p),'metrics':out['metrics']},indent=2))
