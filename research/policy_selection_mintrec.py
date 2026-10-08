"""Nested calibration/validation selection for MIntRec policy choice."""
from __future__ import annotations
import json
from collections import defaultdict
from pathlib import Path
from real_data_baseline_benchmark import fetch_split, metrics
from universal_mintrec_benchmark import CONTRACT, build, centroids
from universal_symbiotic import UniversalSymbioticLearner
from score_calibrated_universal import classic, calibrator


def ids(r): return f"{r['season']}/{r['episode']}/{r['clip']}"

def current_policy(fit, train, eval_rows, cs):
    truth={ids(r):r['label'] for r in train+eval_rows}; learner=UniversalSymbioticLearner(); learner.fit(build(fit,train,cs,truth),CONTRACT); groups=defaultdict(list)
    for c in build(fit,eval_rows,cs,truth): groups[c.example_id].append(c)
    return [learner.decide(groups[e],example_id=e).label for e in sorted(groups)]

def calibrated_policy(fit, train, eval_rows, cs):
    ca=classic(fit,train,cs,ids); ho=classic(fit,eval_rows,cs,ids); funcs={p:calibrator(ca,p) for p in ('supervised','unsupervised')}; return [max(((funcs[p](x[p][1]),x[p][0]) for p in funcs),key=lambda z:z[0])[1] for x in sorted(ho,key=lambda x:x['id'])]

def accuracy(rows,pred): return sum(r['label']==p for r,p in zip(sorted(rows,key=ids),pred))/len(rows)

def run():
    rows=fetch_split('train',2224); fit=[r for r in rows if r['season']=='S04']; cal=[r for r in rows if r['season']=='S05']; hold=[r for r in rows if r['season']=='S06']; cal=sorted(cal,key=ids); cut=len(cal)//2; tune,valid=cal[:cut],cal[cut:]; cs=centroids(fit)
    current_valid=accuracy(valid,current_policy(fit,tune,valid,cs)); calibrated_valid=accuracy(valid,calibrated_policy(fit,tune,valid,cs)); chosen='score_calibrated' if calibrated_valid>current_valid else 'universal_default'
    final_current=current_policy(fit,cal,hold,cs); final_calibrated=calibrated_policy(fit,cal,hold,cs); pred=final_calibrated if chosen=='score_calibrated' else final_current; ordered=sorted(hold,key=ids)
    out={'schema':'herus-policy-selection-mintrec-v1','dataset':{'id':'THU-IAR/MIntRec','fit':'S04','calibration':'S05','holdout':'S06','holdout_examples':len(hold)},'selection':{'calibration_split':{'tune':len(tune),'validation':len(valid)},'validation_accuracy':{'universal_default':round(current_valid,6),'score_calibrated':round(calibrated_valid,6)},'chosen_policy':chosen,'tie_rule':'universal_default'},'metrics':{'chosen_policy':metrics(ordered,pred),'universal_default':metrics(ordered,final_current),'score_calibrated':metrics(ordered,final_calibrated)},'protocol':{'holdout_labels_used_for_selection':False,'nested_calibration':True,'claim_boundary':'policy selection replication; no SOTA claim'}}
    p=Path('research/evidence/policy_selection_mintrec_v1.json'); p.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n'); return out

if __name__=='__main__': print(json.dumps(run(),indent=2))
