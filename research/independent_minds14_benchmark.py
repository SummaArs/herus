"""Independent real-data benchmark on MInDS-14 Portuguese.

This is an intent-classification proxy, not a HERUS-event benchmark.
"""
from __future__ import annotations
import json, random, time, urllib.error, urllib.parse, urllib.request
from collections import Counter, defaultdict
from pathlib import Path
import numpy as np
import torch
from torch.utils.data import DataLoader, Dataset
from transformers import AutoTokenizer, AutoModelForSequenceClassification
from real_data_baseline_benchmark import _nb_scores, cosine, metrics, vector

API='https://datasets-server.huggingface.co/rows'; DATASET='PolyAI/minds14'; CONFIG='pt-PT'; SEEDS=(11,23,47)
MODEL='google/bert_uncased_L-2_H-128_A-2'; EPOCHS=5; BATCH=16; MAX_LENGTH=96; LR=5e-5

def fetch_rows():
    def get(q):
        for attempt in range(5):
            try:
                with urllib.request.urlopen(API+'?'+urllib.parse.urlencode(q), timeout=60) as r: return json.load(r)
            except (urllib.error.HTTPError, urllib.error.URLError, TimeoutError):
                if attempt == 4: raise
                time.sleep(2**attempt)
    meta=get({'dataset':DATASET,'config':CONFIG,'split':'train','offset':0,'length':1}); total=meta['num_rows_total']; rows=[]
    for offset in range(0,total,100):
        payload=get({'dataset':DATASET,'config':CONFIG,'split':'train','offset':offset,'length':min(100,total-offset)})
        rows.extend({'text':x['row']['transcription'],'label':str(x['row']['intent_class']),'path':x['row']['path']} for x in payload['rows'])
    return rows

def split(rows):
    groups=defaultdict(list)
    for row in rows: groups[row['label']].append(row)
    fit=[]; cal=[]; hold=[]
    for label, group in sorted(groups.items()):
        group=sorted(group,key=lambda x:x['path']); n=len(group); a=max(1,int(n*.70)); b=max(a+1,int(n*.85)); fit += group[:a]; cal += group[a:b]; hold += group[b:]
    return fit,cal,hold

def centroid_model(fit):
    sums=defaultdict(Counter); counts=Counter()
    for row in fit: sums[row['label']].update(vector(row['text'])); counts[row['label']]+=1
    return {label:Counter({t:v/counts[label] for t,v in words.items()}) for label,words in sums.items()}

def selective(fit, calibration, holdout):
    cents=centroid_model(fit); cal_scores=[]
    for row in calibration:
        nb,margin=_nb_scores(fit,row); ranked=sorted(((cosine(vector(row['text']),c),l) for l,c in cents.items()), reverse=True); cal_scores.append((nb,ranked[0][1],margin))
    candidates=sorted({x[2] for x in cal_scores}); best=None
    for threshold in candidates:
        pred=[nb if nb==cent and margin>=threshold else None for nb,cent,margin in cal_scores]; m=metrics(calibration,pred)
        if m['coverage'] and m['selective_accuracy']>=.5 and (best is None or m['coverage']>best[1]['coverage']): best=(threshold,m)
    threshold=best[0] if best else float('inf'); scores=[]
    for row in holdout:
        nb,margin=_nb_scores(fit,row); ranked=sorted(((cosine(vector(row['text']),c),l) for l,c in cents.items()), reverse=True); cent=ranked[0][1]; scores.append((nb if nb==cent and margin>=threshold else None, nb, cent))
    return [x[0] for x in scores], threshold

class TextDataset(Dataset):
    def __init__(self,rows,tok,l2i): self.rows=rows; self.enc=tok([r['text'] for r in rows],truncation=True,padding=True,max_length=MAX_LENGTH,return_tensors='pt'); self.l2i=l2i
    def __len__(self): return len(self.rows)
    def __getitem__(self,i):
        x={k:v[i] for k,v in self.enc.items()}; x['labels']=torch.tensor(self.l2i[self.rows[i]['label']]); return x

def transformer_once(fit,cal,hold,seed):
    random.seed(seed); np.random.seed(seed); torch.manual_seed(seed); torch.use_deterministic_algorithms(True,warn_only=True)
    rows=fit+cal+hold; labels=sorted({r['label'] for r in rows}); l2i={x:i for i,x in enumerate(labels)}; i2l={i:x for x,i in l2i.items()}; tok=AutoTokenizer.from_pretrained(MODEL,use_fast=False); model=AutoModelForSequenceClassification.from_pretrained(MODEL,num_labels=len(labels),ignore_mismatched_sizes=True)
    train=DataLoader(TextDataset(fit,tok,l2i),batch_size=BATCH,shuffle=True,generator=torch.Generator().manual_seed(seed)); valid=DataLoader(TextDataset(cal,tok,l2i),batch_size=BATCH); test=DataLoader(TextDataset(hold,tok,l2i),batch_size=BATCH); opt=torch.optim.AdamW(model.parameters(),lr=LR); best=-1; state=None
    for _ in range(EPOCHS):
        model.train()
        for batch in train: out=model(**batch); out.loss.backward(); opt.step(); opt.zero_grad()
        model.eval(); preds=[]
        with torch.no_grad():
            for batch in valid: preds += model(**batch).logits.argmax(1).tolist()
        score=sum(labels.index(cal[i]['label'])==preds[i] for i in range(len(cal)))/len(cal)
        if score>best: best=score; state={k:v.detach().clone() for k,v in model.state_dict().items()}
    model.load_state_dict(state); model.eval(); predictions=[]; confidences=[]
    with torch.no_grad():
        for batch in test:
            p=model(**batch).logits.softmax(-1); val,idx=p.max(1); predictions += [i2l[x] for x in idx.tolist()]; confidences += val.tolist()
    return predictions, confidences

def run():
    rows=fetch_rows(); fit,cal,hold=split(rows); accepted,threshold=selective(fit,cal,hold)
    nb=[_nb_scores(fit,r)[0] for r in hold]; cents=centroid_model(fit); centroid=[max(((cosine(vector(r['text']),c),label) for label,c in cents.items()))[1] for r in hold]
    result={'schema':'herus-independent-minds14-v1','dataset':{'id':DATASET,'config':CONFIG,'source':API,'rows':len(rows),'labels':len(set(r['label'] for r in rows)),'fit':len(fit),'calibration':len(cal),'holdout':len(hold)},'protocol':{'split':'within-label deterministic path order 70/15/15','seeds':list(SEEDS),'claim_boundary':'independent intent proxy; no HERUS-event or SOTA claim'},'baselines':{'naive_bayes':metrics(hold,nb),'centroid':metrics(hold,centroid)},'adapter':{'metrics':metrics(hold,accepted),'threshold':None if threshold==float('inf') else round(threshold,8)}}
    result['transformer']=[]
    for seed in SEEDS:
        pred,conf=transformer_once(fit,cal,hold,seed); result['transformer'].append({'seed':seed,'metrics':metrics(hold,pred),'ledger':[{'example_id':r['path'],'label':r['label'],'transformer_prediction':pred[i],'transformer_correct':pred[i]==r['label']} for i,r in enumerate(hold)]})
    result['ledger']=[{'example_id':r['path'],'label':r['label'],'herus_prediction':accepted[i],'herus_accepted':accepted[i] is not None,'herus_correct':accepted[i]==r['label'],'naive_bayes_prediction':nb[i],'centroid_prediction':centroid[i]} for i,r in enumerate(hold)]
    return result

if __name__=='__main__':
    started=time.perf_counter(); out=run(); out['runtime_seconds']=round(time.perf_counter()-started,3); p=Path('research/evidence/independent_minds14_v1.json'); p.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n'); print(json.dumps({'path':str(p),'runtime_seconds':out['runtime_seconds']},indent=2))
