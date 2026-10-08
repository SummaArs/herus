"""Deterministic text stress tests on the real Banking77 test split.

These are not certified label-preserving attacks without human adjudication.
"""
from __future__ import annotations
import json, re
from pathlib import Path
from policy_selection_banking77 import fetch, split_train, cents, default, calibrated, acc
from real_data_baseline_benchmark import fit_nb


def mutate(text, kind):
    if kind=='case_punctuation':
        return re.sub(r'[^\w\s]', '', text).lower()
    if kind=='irrelevant_prefix':
        return 'hello, I have a question: '+text
    if kind=='deletion':
        for i,ch in enumerate(text):
            if ch.isalpha(): return text[:i]+text[i+1:]
        return text
    if kind=='typo':
        words=text.split()
        for j,w in enumerate(words):
            if len(w)>=5 and w.isalpha():
                k=len(w)//2; words[j]=w[:k+1]+w[k]+w[k+2:]
                return ' '.join(words)
        return text
    raise ValueError(kind)


def predict(rows, fit, calibration_rows, cs, model, policy):
    if policy=='default': return default(fit, calibration_rows, rows, cs, model)
    return calibrated(fit, calibration_rows, rows, cs, model)


def metrics(clean_rows, attacked_rows, clean_pred, attacked_pred):
    n=len(clean_rows); clean_acc=sum(r['label']==p for r,p in zip(clean_rows,clean_pred))/n
    attacked_acc=sum(r['label']==p for r,p in zip(attacked_rows,attacked_pred))/n
    accepted=[i for i,p in enumerate(attacked_pred) if p is not None]
    coverage=len(accepted)/n
    risk=(sum(attacked_rows[i]['label']!=attacked_pred[i] for i in accepted)/len(accepted)) if accepted else None
    clean_good=[i for i,p in enumerate(clean_pred) if p==clean_rows[i]['label']]
    flip=sum(attacked_pred[i] is not None and attacked_pred[i]!=clean_pred[i] for i in clean_good)/len(clean_good) if clean_good else None
    abstain=sum(attacked_pred[i] is None and i in clean_good for i in range(n))/len(clean_good) if clean_good else None
    return {'n':n,'clean_accuracy':round(clean_acc,6),'attacked_accuracy':round(attacked_acc,6),'coverage':round(coverage,6),'selective_risk':None if risk is None else round(risk,6),'abstention_rate':round(1-coverage,6),'wrong_label_flip_rate':None if flip is None else round(flip,6),'abstention_induced_rate':None if abstain is None else round(abstain,6),'prediction_change_rate':round(sum(a!=c for a,c in zip(attacked_pred,clean_pred))/n,6)}


def run():
    raw=fetch('train'); test=fetch('test'); fit,cal=split_train(raw); cal=sorted(cal,key=lambda x:x['id'])
    cut=len(cal)//2; tune,valid=cal[:cut],cal[cut:]; model=fit_nb(fit); cs=cents(fit)
    dv=default(fit,tune,valid,cs,model); av=calibrated(fit,tune,valid,cs,model); policy='calibrated' if acc(valid,av)>acc(valid,dv) else 'default'
    clean=predict(test,fit,valid,cs,model,policy)
    out={'schema':'herus-adversarial-banking77-v1','dataset':'PolyAI-LDN/task-specific-datasets/banking_data','split':{'fit':len(fit),'calibration':len(cal),'test':len(test)},'policy':policy,'protocol':{'test_labels_used_for_selection':False,'deterministic_transformations':True,'human_semantic_adjudication':False,'claim_boundary':'text stress-test only; not certified label-preserving robustness or OOD'},'attacks':{}}
    for kind in ('case_punctuation','typo','deletion','irrelevant_prefix'):
        attacked=[dict(r,text=mutate(r['text'],kind)) for r in test]
        pred=predict(attacked,fit,valid,cs,model,policy)
        out['attacks'][kind]=metrics(test,attacked,clean,pred)
    Path('research/evidence/adversarial_banking77_v1.json').write_text(json.dumps(out,indent=2,sort_keys=True)+'\n')
    return out

if __name__=='__main__': print(json.dumps(run(),indent=2,sort_keys=True))
