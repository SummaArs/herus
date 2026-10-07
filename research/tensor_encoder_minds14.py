"""Hybrid: frozen multilingual encoder plus compact two-core TT head."""
import json,time,torch
from transformers import AutoTokenizer,AutoModel
from minds14_real_benchmark import fetch_rows,stratified
from multilingual_encoder_benchmark import encode,MODEL
from real_data_baseline_benchmark import metrics

class TTHead(torch.nn.Module):
 def __init__(self,rank=32):
  super().__init__(); torch.manual_seed(7)
  self.left=torch.nn.Parameter(torch.randn(2,32,rank)*.02)
  self.right=torch.nn.Parameter(torch.randn(rank,7,24)*.02)
 def forward(self,x): return torch.einsum('bij,air,roj->bao',x.reshape(-1,32,24),self.left,self.right).reshape(-1,14)

def run(rank=32):
 rows=fetch_rows(); fit,_,hold=stratified(rows); labels=sorted({r['label'] for r in rows}); idx={v:i for i,v in enumerate(labels)}
 tok=AutoTokenizer.from_pretrained(MODEL); enc=AutoModel.from_pretrained(MODEL); enc.eval()
 vf=encode(fit,tok,enc); vh=encode(hold,tok,enc); xf=torch.stack(vf); xh=torch.stack(vh); yf=torch.tensor([idx[r['label']] for r in fit])
 head=TTHead(rank); opt=torch.optim.Adam(head.parameters(),lr=.03,weight_decay=1e-4); start=time.perf_counter()
 for _ in range(400):
  opt.zero_grad(); loss=torch.nn.functional.cross_entropy(head(xf),yf); loss.backward(); opt.step()
 train_ms=(time.perf_counter()-start)*1000; start=time.perf_counter()
 with torch.no_grad(): pred=head(xh).argmax(1).tolist()
 infer_ms=(time.perf_counter()-start)*1000; out=metrics(hold,[labels[i] for i in pred]); out.update(model='frozen_multilingual_encoder_plus_tt_head',rank=rank,parameters=sum(p.numel() for p in head.parameters()),encoder_parameters='frozen_external',train_ms=round(train_ms,2),infer_ms=round(infer_ms,2),holdout_rows=len(hold),dataset='PolyAI/minds14'); return out
if __name__=='__main__': print(json.dumps(run(),indent=2,sort_keys=True))
