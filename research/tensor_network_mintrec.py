"""Wide Tensor-Train on MIntRec with temporal S06 holdout."""
import json,time,urllib.error,urllib.parse,urllib.request,torch
from sklearn.feature_extraction.text import HashingVectorizer
from real_data_baseline_benchmark import metrics
API='https://datasets-server.huggingface.co/rows'; DATASET='THU-IAR/MIntRec'
def fetch():
 rows=[]
 for offset in range(0,2224,100):
  q=urllib.parse.urlencode({'dataset':DATASET,'config':'default','split':'train','offset':offset,'length':min(100,2224-offset)})
  for attempt in range(4):
   try:
    with urllib.request.urlopen(API+'?'+q,timeout=60) as r: payload=json.load(r)
    break
   except urllib.error.HTTPError:
    if attempt==3: raise
    time.sleep(1+attempt)
  rows.extend({'text':x['row']['text'],'label':x['row']['label'],'season':x['row']['season'],'episode':x['row']['episode']} for x in payload['rows'])
 return rows
class TT20(torch.nn.Module):
 def __init__(self,rank):
  super().__init__(); torch.manual_seed(7)
  self.left=torch.nn.Parameter(torch.randn(4,32,rank)*.02); self.right=torch.nn.Parameter(torch.randn(rank,5,32)*.02)
 def forward(self,x): return torch.einsum('bij,air,roj->bao',x.reshape(-1,32,32),self.left,self.right).reshape(-1,20)
def run():
 rows=fetch(); fit=[r for r in rows if r['season'] in ('S04','S05')]; hold=[r for r in rows if r['season']=='S06']; labels=sorted({r['label'] for r in rows}); idx={v:i for i,v in enumerate(labels)}
 vec=HashingVectorizer(n_features=1024,alternate_sign=False,norm='l2',ngram_range=(1,2))
 def X(d): return torch.tensor(vec.transform([r['text'] for r in d]).toarray(),dtype=torch.float32)
 xf,yf=X(fit),torch.tensor([idx[r['label']] for r in fit]); xh=X(hold); out=[]
 for rank in (32,64):
  model=TT20(rank); opt=torch.optim.Adam(model.parameters(),lr=.03,weight_decay=1e-4); start=time.perf_counter()
  for _ in range(300):
   opt.zero_grad(); loss=torch.nn.functional.cross_entropy(model(xf),yf); loss.backward(); opt.step()
  train=(time.perf_counter()-start)*1000; start=time.perf_counter()
  with torch.no_grad(): pred=model(xh).argmax(1).tolist()
  infer=(time.perf_counter()-start)*1000; r=metrics(hold,[labels[i] for i in pred]); r.update(rank=rank,parameters=sum(p.numel() for p in model.parameters()),train_ms=round(train,2),infer_ms=round(infer,2)); out.append(r)
 return {'dataset':'THU-IAR/MIntRec','fit_seasons':['S04','S05'],'holdout_season':'S06','fit_rows':len(fit),'holdout_rows':len(hold),'input_features':1024,'ranks':out}
if __name__=='__main__': print(json.dumps(run(),indent=2,sort_keys=True))
