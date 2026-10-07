"""Compact two-core tensor-network classifier on real MInDS-14 text."""
from __future__ import annotations
import json,time
import numpy as np, torch
from sklearn.feature_extraction.text import HashingVectorizer
from minds14_real_benchmark import fetch_rows,stratified
from real_data_baseline_benchmark import metrics

class TTClassifier(torch.nn.Module):
    def __init__(self, rank=8, seed=7):
        super().__init__(); torch.manual_seed(seed)
        self.core_left=torch.nn.Parameter(torch.randn(2,16,rank)*0.03)
        self.core_right=torch.nn.Parameter(torch.randn(rank,7,16)*0.03)
    def forward(self,x):
        x=x.reshape(-1,16,16)
        return torch.einsum('bij,air,roj->bao',x,self.core_left,self.core_right).reshape(-1,14)

def run():
    rows=fetch_rows(); fit,cal,hold=stratified(rows)
    labels=sorted({r['label'] for r in rows}); idx={v:i for i,v in enumerate(labels)}
    vectorizer=HashingVectorizer(n_features=256,alternate_sign=False,norm='l2',ngram_range=(1,2))
    def X(data): return torch.tensor(vectorizer.transform([r['text'] for r in data]).toarray(),dtype=torch.float32)
    xf,yf=X(fit),torch.tensor([idx[r['label']] for r in fit]); xh=X(hold)
    model=TTClassifier(); opt=torch.optim.Adam(model.parameters(),lr=0.03,weight_decay=1e-4); start=time.perf_counter()
    model.train()
    for _ in range(250):
        opt.zero_grad(); loss=torch.nn.functional.cross_entropy(model(xf),yf); loss.backward(); opt.step()
    train_ms=(time.perf_counter()-start)*1000
    model.eval(); start=time.perf_counter()
    with torch.no_grad(): pred=model(xh).argmax(1).tolist()
    infer_ms=(time.perf_counter()-start)*1000
    out=metrics(hold,[labels[i] for i in pred]); out.update(model='two_core_tensor_train_classifier',rank=8,parameters=sum(p.numel() for p in model.parameters()),input_features=256,train_ms=round(train_ms,2),infer_ms=round(infer_ms,2),epochs=250,dataset='PolyAI/minds14',holdout_rows=len(hold),seed=7)
    return out
if __name__=='__main__': print(json.dumps(run(),indent=2,sort_keys=True))
