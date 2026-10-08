"""Evaluate decision invariance on real Banking77 stress tests efficiently."""
from __future__ import annotations
import json
from pathlib import Path
from adversarial_banking77 import mutate, metrics
from policy_selection_banking77 import fetch, split_train, cents, calibrated
from real_data_baseline_benchmark import fit_nb
from invariance_gate import DecisionInvarianceGate, views


def run():
    raw=fetch('train'); test=fetch('test'); fit,cal=split_train(raw); cal=sorted(cal,key=lambda x:x['id']); cut=len(cal)//2; tune,valid=cal[:cut],cal[cut:]
    model=fit_nb(fit); cs=cents(fit)
    cal_rows=[dict(r,text=t) for r in valid for t in views(r['text'])]
    cal_pred=calibrated(fit,tune,cal_rows,cs,model)
    cal_map={r['text']:p for r,p in zip(cal_rows,cal_pred)}
    gate=DecisionInvarianceGate(lambda text: cal_map[text]); gate.fit([r['text'] for r in valid])
    attack_rows={kind:[dict(r,text=mutate(r['text'],kind)) for r in test] for kind in ('case_punctuation','typo','deletion','irrelevant_prefix')}
    scored=[]
    for r in test: scored.extend(dict(r,text=t) for t in views(r['text']))
    for rows in attack_rows.values():
        for r in rows: scored.extend(dict(r,text=t) for t in views(r['text']))
    unique=[]; seen=set()
    for r in scored:
        if r['text'] not in seen: seen.add(r['text']); unique.append(r)
    pred=calibrated(fit,tune,unique,cs,model)
    pmap={r['text']:p for r,p in zip(unique,pred)}
    clean=[pmap[r['text']] for r in test]
    attacks={}
    for kind,attacked in attack_rows.items():
        raw_pred=[pmap[r['text']] for r in attacked]; gated=[]
        for r,p in zip(attacked,raw_pred):
            candidate=[pmap[v] for v in views(r['text'])]
            gated.append(p if all(x==candidate[0] for x in candidate[1:]) else None)
        m=metrics(test,attacked,clean,gated); m['gate_abstention_rate']=round(sum(x is None for x in gated)/len(gated),6); attacks[kind]=m
    out={'schema':'herus-invariance-banking77-v1','calibration_examples':len(valid),'holdout_examples':len(test),'scored_unique_texts':len(unique),'attacks':attacks,'protocol':{'holdout_labels_used':False,'batch_inference':True,'claim_boundary':'ablation only; no certified robustness or SOTA claim'}}
    Path('research/evidence/invariance_banking77_v1.json').write_text(json.dumps(out,indent=2,sort_keys=True)+'\n'); return out

if __name__=='__main__': print(json.dumps(run(),indent=2))
