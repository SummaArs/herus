"""Pair published transformer predictions with deterministic baseline predictions."""
from __future__ import annotations
import json,random
from pathlib import Path
from real_data_baseline_benchmark import fetch_split,nb,contextual_memory,metrics
ROOT=Path(__file__).parent; E=ROOT/'evidence'

def bootstrap(rows,a,b,iterations=4000,seed=17):
 rng=random.Random(seed); n=len(rows); ds=[]
 for _ in range(iterations):
  ids=[rng.randrange(n) for _ in rows]
  ds.append(sum(rows[i]['label']==a[i] for i in ids)/n-sum(rows[i]['label']==b[i] for i in ids)/n)
 ds.sort(); return {'observed_delta_accuracy':round(sum(r['label']==x for r,x in zip(rows,a))/n-sum(r['label']==x for r,x in zip(rows,b))/n,6),'ci95_low':round(ds[int(.025*iterations)],6),'ci95_high':round(ds[int(.975*iterations)-1],6),'iterations':iterations,'seed':seed}

def run():
 all_rows=fetch_split('train',2224); train=[r for r in all_rows if r['season'] in {'S04','S05'}]; test=[r for r in all_rows if r['season']=='S06']
 t=json.loads((E/'transformer_multilingual_mintrec_v1.json').read_text())
 assert len(t.get('prediction_ledger',[]))==len(test), 'transformer ledger missing or wrong length'
 tp=[x['prediction'] for x in t['prediction_ledger']]; bp=nb(train,test); hp=contextual_memory(train,test)
 result={'protocol':'same chronological S06 holdout; paired bootstrap over identical example indices','dataset':'THU-IAR/MIntRec','holdout_rows':len(test),'models':{'naive_bayes':metrics(test,bp),'herus_context_memory':metrics(test,hp),'distilbert_multilingual':metrics(test,tp)},'comparisons':{'herus_minus_transformer':bootstrap(test,hp,tp),'transformer_minus_naive_bayes':bootstrap(test,tp,bp),'herus_minus_naive_bayes':bootstrap(test,hp,bp)},'limits':['HERUS memory is not the full proposal pipeline','one holdout and one transformer seed','MIntRec labels are not HERUS events']}
 (E/'paired_transformer_analysis_v1.json').write_text(json.dumps(result,indent=2,ensure_ascii=False)+'\n'); return result
if __name__=='__main__': print(json.dumps(run(),indent=2,ensure_ascii=False))
