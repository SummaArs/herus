"""Frozen multilingual encoder benchmark on MInDS-14.

The encoder is pretrained, frozen, and never sees target labels. English fit /
calibration is evaluated on Portuguese holdout with shared dataset labels.
"""
from __future__ import annotations
import json
import torch
from transformers import AutoTokenizer, AutoModel
from minds14_cross_language import fetch, split
from real_data_baseline_benchmark import metrics

MODEL='distilbert/distilbert-base-multilingual-cased'

def encode(rows, tokenizer, model):
    vectors=[]
    for start in range(0,len(rows),32):
        batch=rows[start:start+32]
        tok=tokenizer([r['text'] for r in batch],padding=True,truncation=True,max_length=96,return_tensors='pt')
        with torch.no_grad(): out=model(**tok).last_hidden_state
        mask=tok['attention_mask'].unsqueeze(-1)
        pooled=(out*mask).sum(1)/mask.sum(1).clamp_min(1)
        vectors.extend(pooled.cpu())
    return vectors

def normalize(v): return v/(v.norm(dim=1,keepdim=True).clamp_min(1e-9))

def centroids(vectors,rows):
    sums={}; counts={}
    for v,r in zip(vectors,rows): sums.setdefault(r['label'],torch.zeros_like(v)); sums[r['label']]+=v; counts[r['label']]=counts.get(r['label'],0)+1
    return {label: sums[label]/counts[label] for label in sums}

def predict(vectors,cents):
    labels=list(cents); mat=normalize(torch.stack([cents[l] for l in labels])); q=normalize(torch.stack(vectors)); scores=q@mat.T; best=scores.argmax(1); margins=scores.topk(min(2,len(labels)),dim=1).values; return [labels[i] for i in best.tolist()],[float(x[0]-x[1]) for x in margins]

def run():
    en=fetch('en-US'); pt=fetch('pt-PT'); fit,cal=split(en)
    tok=AutoTokenizer.from_pretrained(MODEL); model=AutoModel.from_pretrained(MODEL); model.eval()
    vf=encode(fit,tok,model); vc=encode(cal,tok,model); vt=encode(pt,tok,model)
    cents=centroids(vf,fit); _,cal_margin=predict(vc,cents); cal_pred,_=predict(vc,cents)
    best=None
    for threshold in sorted(set(cal_margin)):
        pred=[label if margin>=threshold else None for label,margin in zip(cal_pred,cal_margin)]
        result=metrics(cal,pred)
        if result['coverage'] and result['selective_accuracy']>=.5 and (best is None or result['coverage']>best[1]['coverage']): best=(threshold,result)
    pt_pred,pt_margin=predict(vt,cents); threshold=best[0] if best else float('inf'); selective=[label if margin>=threshold else None for label,margin in zip(pt_pred,pt_margin)]
    return {'model':MODEL,'frozen':True,'fit_language':'en-US','calibration_language':'en-US','holdout_language':'pt-PT','fit_rows':len(fit),'calibration_rows':len(cal),'holdout_rows':len(pt),'calibration':None if not best else {'threshold':round(threshold,6),'result':best[1]},'holdout':metrics(pt,selective),'limits':['single frozen encoder','no target labels used','no audio decoded','no speaker independence metadata verified','single language pair']}

if __name__=='__main__': print(json.dumps(run(),indent=2,sort_keys=True))
