from __future__ import annotations
import json
from collections import Counter
from pathlib import Path
from adversarial_banking77 import fetch, mutate
from policy_selection_banking77 import split_train, cents, calibrated
from real_data_baseline_benchmark import fit_nb
from invariance_gate import views
from risk_coverage_policy import CalibratedRiskCoverage


def run():
    raw=fetch('train'); test=fetch('test'); fit,cal=split_train(raw); cal=sorted(cal,key=lambda x:x['id']); cut=len(cal)//2; tune,valid=cal[:cut],cal[cut:]
    model=fit_nb(fit); cs=cents(fit)
    cal_rows=[dict(r,text=t) for r in valid for t in views(r['text'])]
    cal_pred=calibrated(fit,tune,cal_rows,cs,model); cmap={r['text']:p for r,p in zip(cal_rows,cal_pred)}
    agreements=[]; correct=[]
    for r in valid:
        ps=[cmap[v] for v in views(r['text'])]; agreements.append(sum(x==ps[0] for x in ps)); correct.append(ps[0]==r['label'])
    # .20 has no safe calibration region; .32 is the smallest admissible
    # Wilson-upper-bound target for this host and is intentionally recorded.
    policy=CalibratedRiskCoverage(.32); decision=policy.fit(agreements,correct)
    attack_rows={k:[dict(r,text=mutate(r['text'],k)) for r in test] for k in ('case_punctuation','typo','deletion','irrelevant_prefix')}
    all_rows=[]; seen=set()
    for r in test: all_rows.extend(dict(r,text=t) for t in views(r['text']))
    for rows in attack_rows.values():
        for r in rows: all_rows.extend(dict(r,text=t) for t in views(r['text']))
    unique=[]
    for r in all_rows:
        if r['text'] not in seen: seen.add(r['text']); unique.append(r)
    pred=calibrated(fit,tune,unique,cs,model); pmap={r['text']:p for r,p in zip(unique,pred)}
    clean_pred=[pmap[r['text']] for r in test]
    out={'schema':'herus-risk-coverage-banking77-v1','decision':decision.__dict__,'holdout_examples':len(test),'attacks':{}}
    for kind,rows in attack_rows.items():
        accepted=[]; flips=0; errors=0
        for i,r in enumerate(rows):
            ps=[pmap[v] for v in views(r['text'])]; ok=policy.accept(ps); accepted.append(ok)
            if ok:
                errors += ps[0] != r['label']; flips += ps[0] != clean_pred[i]
        n=sum(accepted); out['attacks'][kind]={'coverage':n/len(rows),'selective_risk':errors/n if n else 1.0,'wrong_label_flip_rate':flips/len(rows)}
    out['protocol']={'calibration_labels_used':True,'holdout_labels_used_for_policy':False,'claim_boundary':'calibrated selective ablation; not SOTA'}
    Path('research/evidence/risk_coverage_banking77_v1.json').write_text(json.dumps(out,indent=2,sort_keys=True)+'\n'); print(json.dumps(out,indent=2)); return out

if __name__=='__main__': run()
