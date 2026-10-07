import json
from tensor_network_minds14 import TTClassifier
from minds14_real_benchmark import fetch_rows,stratified
from real_data_baseline_benchmark import metrics
from sklearn.feature_extraction.text import HashingVectorizer
import torch,time

def run():
 rows=fetch_rows(); fit,_,hold=stratified(rows); labels=sorted({r['label'] for r in rows}); idx={v:i for i,v in enumerate(labels)}
 vec=HashingVectorizer(n_features=256,alternate_sign=False,norm='l2',ngram_range=(1,2))
 def X(d): return torch.tensor(vec.transform([r['text'] for r in d]).toarray(),dtype=torch.float32)
 xf,yf=X(fit),torch.tensor([idx[r['label']] for r in fit]); xh=X(hold); out=[]
 for rank in (2,4,8,16,32):
  model=TTClassifier(rank=rank); opt=torch.optim.Adam(model.parameters(),lr=.03,weight_decay=1e-4); start=time.perf_counter()
  for _ in range(250):
   opt.zero_grad(); loss=torch.nn.functional.cross_entropy(model(xf),yf); loss.backward(); opt.step()
  train=(time.perf_counter()-start)*1000; start=time.perf_counter()
  with torch.no_grad(): pred=model(xh).argmax(1).tolist()
  infer=(time.perf_counter()-start)*1000; r=metrics(hold,[labels[i] for i in pred]); r.update(rank=rank,parameters=sum(p.numel() for p in model.parameters()),train_ms=round(train,2),infer_ms=round(infer,2)); out.append(r)
 return {'dataset':'PolyAI/minds14','holdout_rows':len(hold),'ranks':out}
if __name__=='__main__': print(json.dumps(run(),indent=2,sort_keys=True))
