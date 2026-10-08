"""Nested policy selection replication on MInDS-14."""
from __future__ import annotations
import json
from collections import defaultdict
from pathlib import Path
from independent_minds14_benchmark import fetch_rows, split
from real_data_baseline_benchmark import metrics
from universal_minds14_benchmark import HOST, CONTRACT, centroids, candidates, with_truth
from universal_symbiotic import UniversalSymbioticLearner
from score_calibrated_universal import classic, calibrator


def current(fit, train, eval_rows, cs):
    learner=UniversalSymbioticLearner(); learner.fit(with_truth(candidates(fit,train,cs,'tune'),train),CONTRACT); groups=defaultdict(list)
    for c in candidates(fit,eval_rows,cs,'eval'): groups[c.example_id].append(c)
    return [learner.decide(groups[e],example_id=e).label for e in sorted(groups)]

def calibrated(fit, train, eval_rows, cs):
    ca=classic(fit,train,cs,lambda r:r['path']); ho=classic(fit,eval_rows,cs,lambda r:r['path']); funcs={p:calibrator(ca,p) for p in ('supervised','unsupervised')}; return [max(((funcs[p](x[p][1]),x[p][0]) for p in funcs),key=lambda z:z[0])[1] for x in sorted(ho,key=lambda x:x['id'])]

def acc(rows,pred):
    ordered=sorted(rows,key=lambda x:x['path']); return sum(r['label']==p for r,p in zip(ordered,pred))/len(ordered)

def run():
    rows=fetch_rows(); fit,cal,hold=split(rows); cal=sorted(cal,key=lambda r:r['path']); cut=len(cal)//2; tune,valid=cal[:cut],cal[cut:]; cs=centroids(fit)
    a=acc(valid,current(fit,tune,valid,cs)); b=acc(valid,calibrated(fit,tune,valid,cs)); chosen='score_calibrated' if b>a else 'universal_default'; final_a=current(fit,cal,hold,cs); final_b=calibrated(fit,cal,hold,cs); pred=final_b if chosen=='score_calibrated' else final_a; ordered=sorted(hold,key=lambda r:r['path'])
    out={'schema':'herus-policy-selection-minds14-v1','dataset':{'id':'PolyAI/minds14','config':'pt-PT','fit':len(fit),'calibration':len(cal),'holdout':len(hold)},'selection':{'calibration_split':{'tune':len(tune),'validation':len(valid)},'validation_accuracy':{'universal_default':round(a,6),'score_calibrated':round(b,6)},'chosen_policy':chosen,'tie_rule':'universal_default'},'metrics':{'chosen_policy':metrics(ordered,pred),'universal_default':metrics(ordered,final_a),'score_calibrated':metrics(ordered,final_b)},'protocol':{'holdout_labels_used_for_selection':False,'nested_calibration':True,'claim_boundary':'policy selection replication; no SOTA claim'}}
    p=Path('research/evidence/policy_selection_minds14_v1.json'); p.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n'); return out

if __name__=='__main__': print(json.dumps(run(),indent=2))
