"""Real small-transformer baseline for MIntRec temporal holdout.

Protocol: pretrained prajjwal1/bert-tiny, fine-tune S04, select checkpoint on
S05, evaluate once on S06. This is a baseline, not a HERUS claim.
"""
from __future__ import annotations
import json, os, random, time
from pathlib import Path
import numpy as np
import torch
from torch.utils.data import DataLoader, Dataset
from transformers import AutoTokenizer, AutoModelForSequenceClassification
from real_data_baseline_benchmark import fetch_split, metrics

MODEL = 'google/bert_uncased_L-2_H-128_A-2'
SEED = 17
EPOCHS = 5
BATCH = 16
MAX_LENGTH = 96
LR = 5e-5


def seed_all(seed=SEED):
    random.seed(seed); np.random.seed(seed); torch.manual_seed(seed)
    torch.use_deterministic_algorithms(True, warn_only=True)


class TextDataset(Dataset):
    def __init__(self, rows, tokenizer, label_to_id):
        self.rows = rows; self.labels = label_to_id
        self.enc = tokenizer([r['text'] for r in rows], truncation=True, padding=True, max_length=MAX_LENGTH, return_tensors='pt')
    def __len__(self): return len(self.rows)
    def __getitem__(self, i):
        item = {k: v[i] for k, v in self.enc.items()}
        item['labels'] = torch.tensor(self.labels[self.rows[i]['label']], dtype=torch.long)
        return item


def predict(model, loader, id_to_label, device):
    model.eval(); predictions=[]
    with torch.no_grad():
        for batch in loader:
            batch={k:v.to(device) for k,v in batch.items()}
            logits=model(**batch).logits
            predictions.extend(logits.argmax(dim=1).cpu().tolist())
    return [id_to_label[i] for i in predictions]


def run():
    started=time.perf_counter(); seed_all(); device=torch.device('cpu')
    rows=fetch_split('train',2224)
    fit=[r for r in rows if r['season']=='S04']; validation=[r for r in rows if r['season']=='S05']; test=[r for r in rows if r['season']=='S06']
    labels=sorted({r['label'] for r in rows}); label_to_id={x:i for i,x in enumerate(labels)}; id_to_label={i:x for x,i in label_to_id.items()}
    tokenizer=AutoTokenizer.from_pretrained(MODEL, use_fast=False)
    model=AutoModelForSequenceClassification.from_pretrained(MODEL,num_labels=len(labels),ignore_mismatched_sizes=True).to(device)
    train_loader=DataLoader(TextDataset(fit,tokenizer,label_to_id),batch_size=BATCH,shuffle=True,generator=torch.Generator().manual_seed(SEED))
    val_loader=DataLoader(TextDataset(validation,tokenizer,label_to_id),batch_size=BATCH,shuffle=False)
    test_loader=DataLoader(TextDataset(test,tokenizer,label_to_id),batch_size=BATCH,shuffle=False)
    optimizer=torch.optim.AdamW(model.parameters(),lr=LR)
    best=-1.; best_state=None; history=[]
    for epoch in range(EPOCHS):
        model.train(); losses=[]
        for batch in train_loader:
            batch={k:v.to(device) for k,v in batch.items()}; out=model(**batch); out.loss.backward(); optimizer.step(); optimizer.zero_grad(); losses.append(float(out.loss.detach()))
        val_predictions=predict(model,val_loader,id_to_label,device); val_metrics=metrics(validation,val_predictions)
        history.append({'epoch':epoch+1,'loss':round(sum(losses)/len(losses),6),'validation':val_metrics})
        if val_metrics['accuracy']>best: best=val_metrics['accuracy']; best_state={k:v.detach().cpu().clone() for k,v in model.state_dict().items()}
    model.load_state_dict(best_state); test_predictions=predict(model,test_loader,id_to_label,device)
    result={'model':MODEL,'protocol':{'seed':SEED,'epochs':EPOCHS,'batch_size':BATCH,'max_length':MAX_LENGTH,'learning_rate':LR,'device':str(device),'fit_season':'S04','validation_season':'S05','holdout_season':'S06'},'dataset':{'id':'THU-IAR/MIntRec','fit_rows':len(fit),'validation_rows':len(validation),'holdout_rows':len(test),'labels':len(labels)},'history':history,'holdout_metrics':metrics(test,test_predictions),'runtime_seconds':round(time.perf_counter()-started,3),'limits':['small pretrained transformer baseline, not state of the art','text only; audio/video excluded','one temporal holdout','no MIntRec label mapped to a HERUS event']}
    return result

if __name__=='__main__':
    print(json.dumps(run(),indent=2,sort_keys=True))
