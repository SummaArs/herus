"""Configurable transformer baseline for the real MIntRec temporal holdout."""
from __future__ import annotations
import json, os, random, time
import numpy as np
import torch
from torch.utils.data import DataLoader, Dataset
from transformers import AutoTokenizer, AutoModelForSequenceClassification
from real_data_baseline_benchmark import fetch_split, metrics

MODEL=os.environ.get('HERUS_TRANSFORMER_MODEL','google/bert_uncased_L-2_H-128_A-2')
SEED=int(os.environ.get('HERUS_TRANSFORMER_SEED','17'))
EPOCHS=int(os.environ.get('HERUS_TRANSFORMER_EPOCHS','5'))
BATCH=int(os.environ.get('HERUS_TRANSFORMER_BATCH','16'))
MAX_LENGTH=int(os.environ.get('HERUS_TRANSFORMER_MAX_LENGTH','96'))
LR=float(os.environ.get('HERUS_TRANSFORMER_LR','5e-5'))

def seed_all():
    random.seed(SEED); np.random.seed(SEED); torch.manual_seed(SEED)
    torch.use_deterministic_algorithms(True,warn_only=True)

class TextDataset(Dataset):
    def __init__(self,rows,tokenizer,label_to_id):
        self.rows=rows; self.labels=label_to_id
        self.enc=tokenizer([r['text'] for r in rows],truncation=True,padding=True,max_length=MAX_LENGTH,return_tensors='pt')
    def __len__(self): return len(self.rows)
    def __getitem__(self,i):
        item={k:v[i] for k,v in self.enc.items()}; item['labels']=torch.tensor(self.labels[self.rows[i]['label']],dtype=torch.long); return item

def predict(model,loader,id_to_label,device):
    model.eval(); out=[]
    with torch.no_grad():
        for batch in loader:
            batch={k:v.to(device) for k,v in batch.items()}; out.extend(model(**batch).logits.argmax(1).cpu().tolist())
    return [id_to_label[i] for i in out]

def run():
    started=time.perf_counter(); seed_all(); device=torch.device('cpu')
    rows=fetch_split('train',2224); fit=[r for r in rows if r['season']=='S04']; val=[r for r in rows if r['season']=='S05']; test=[r for r in rows if r['season']=='S06']
    labels=sorted({r['label'] for r in rows}); l2i={x:i for i,x in enumerate(labels)}; i2l={i:x for x,i in l2i.items()}
    tok=AutoTokenizer.from_pretrained(MODEL,use_fast=False)
    model=AutoModelForSequenceClassification.from_pretrained(MODEL,num_labels=len(labels),ignore_mismatched_sizes=True).to(device)
    train=DataLoader(TextDataset(fit,tok,l2i),batch_size=BATCH,shuffle=True,generator=torch.Generator().manual_seed(SEED))
    valid=DataLoader(TextDataset(val,tok,l2i),batch_size=BATCH); hold=DataLoader(TextDataset(test,tok,l2i),batch_size=BATCH)
    opt=torch.optim.AdamW(model.parameters(),lr=LR); best=-1.; best_state=None; history=[]
    for epoch in range(EPOCHS):
        model.train(); losses=[]
        for batch in train:
            batch={k:v.to(device) for k,v in batch.items()}; out=model(**batch); out.loss.backward(); opt.step(); opt.zero_grad(); losses.append(float(out.loss.detach()))
        vm=metrics(val,predict(model,valid,i2l,device)); history.append({'epoch':epoch+1,'loss':round(sum(losses)/len(losses),6),'validation':vm})
        if vm['accuracy']>best: best=vm['accuracy']; best_state={k:v.detach().cpu().clone() for k,v in model.state_dict().items()}
    model.load_state_dict(best_state); test_predictions=predict(model,hold,i2l,device)
    result={'model':MODEL,'protocol':{'seed':SEED,'epochs':EPOCHS,'batch_size':BATCH,'max_length':MAX_LENGTH,'learning_rate':LR,'device':'cpu','fit_season':'S04','validation_season':'S05','holdout_season':'S06'},'dataset':{'id':'THU-IAR/MIntRec','fit_rows':len(fit),'validation_rows':len(val),'holdout_rows':len(test),'labels':len(labels)},'history':history,'holdout_metrics':metrics(test,test_predictions),'prediction_ledger':[{'index':i,'label':row['label'],'prediction':test_predictions[i],'correct':row['label']==test_predictions[i]} for i,row in enumerate(test)],'runtime_seconds':round(time.perf_counter()-started,3),'limits':['single model is not universal transformer evidence','text only; audio/video excluded','one temporal holdout','no MIntRec label mapped to a HERUS event']}
    return result

if __name__=='__main__': print(json.dumps(run(),indent=2,sort_keys=True))
