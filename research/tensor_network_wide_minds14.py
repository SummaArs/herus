"""Tensor-Train classifier with a wider 1024-feature input ablation."""
import json,time,torch
from sklearn.feature_extraction.text import HashingVectorizer
from minds14_real_benchmark import fetch_rows,stratified
from real_data_baseline_benchmark import metrics
class WideTT(torch.nn.Module):
 def __init__(self,rank):
  super().__init__(); torch.manual_seed(7)
  self.left=torch.nn.Parameter(torch.randn(2,32,rank)*.02); self.right=torch.nn.Parameter(torch.randn(rank,7,32)*.02)
 def forward(self,x): return torch.einsum('bij,air,roj->bao',x.reshape(-1,32,32),self.left,self.right).reshape(-1,14)
def run():
 rows=fetch_rows(); fit,_,hold=stratified(rows); labels=sorted({r['label'] for r in rows}); idx={v:i for i,v in enumerate(labels)}
 vec=HashingVectorizer(n_features=1024,alternate_sign=False,norm='l2',ngram_range=(1,2))
 def X(d): return torch.tensor(vec.transform([r['text'] for r in d]).toarray(),dtype=torch.float32)
 xf,yf=X(fit),torch.tensor([idx[r['label']] for r in fit]); xh=X(hold); out=[]
 for rank in (8,16,32,64):
  model=WideTT(rank); opt=torch.optim.Adam(model.parameters(),lr=.03,weight_decay=1e-4); start=time.perf_counter()
  for _ in range(300):
   opt.zero_grad(); loss=torch.nn.functional.cross_entropy(model(xf),yf); loss.backward(); opt.step()
  train=(time.perf_counter()-start)*1000; start=time.perf_counter()
  with torch.no_grad(): pred=model(xh).argmax(1).tolist()
  infer=(time.perf_counter()-start)*1000; r=metrics(hold,[labels[i] for i in pred]); r.update(rank=rank,parameters=sum(p.numel() for p in model.parameters()),train_ms=round(train,2),infer_ms=round(infer,2)); out.append(r)
 return {'dataset':'PolyAI/minds14','input_features':1024,'holdout_rows':len(hold),'ranks':out}
if __name__=='__main__': print(json.dumps(run(),indent=2,sort_keys=True))
