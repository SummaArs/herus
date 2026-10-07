"""Paired evaluation ledger for deterministic real-data baselines."""
from __future__ import annotations
import json,random
from pathlib import Path
from real_data_baseline_benchmark import fetch_split, nb, contextual_memory, metrics
ROOT=Path(__file__).parent; E=ROOT/'evidence'

def paired_bootstrap(rows,pred_a,pred_b,iterations=4000,seed=17):
    rng=random.Random(seed); n=len(rows); deltas=[]
    for _ in range(iterations):
        idx=[rng.randrange(n) for _ in range(n)]
        a=sum(rows[i]['label']==pred_a[i] for i in idx)/n
        b=sum(rows[i]['label']==pred_b[i] for i in idx)/n
        deltas.append(a-b)
    deltas.sort(); lo=deltas[int(.025*iterations)]; hi=deltas[int(.975*iterations)-1]
    observed=sum(r['label']==a for r,a in zip(rows,pred_a))/n-sum(r['label']==b for r,b in zip(rows,pred_b))/n
    return {'observed_delta_accuracy':round(observed,6),'bootstrap_mean':round(sum(deltas)/len(deltas),6),'ci95_low':round(lo,6),'ci95_high':round(hi,6),'iterations':iterations,'seed':seed}

def run():
    all_rows=fetch_split('train',2224); train=[r for r in all_rows if r['season'] in {'S04','S05'}]; test=[r for r in all_rows if r['season']=='S06']
    nb_pred=nb(train,test); mem_pred=contextual_memory(train,test)
    ledger=[{'index':i,'label':r['label'],'nb_prediction':nb_pred[i],'memory_prediction':mem_pred[i],'nb_correct':r['label']==nb_pred[i],'memory_accepted':mem_pred[i] is not None,'memory_correct':r['label']==mem_pred[i]} for i,r in enumerate(test)]
    result={'protocol':'chronological S04+S05 fit; S06 holdout; paired bootstrap over saved per-example predictions','dataset':'THU-IAR/MIntRec','holdout_rows':len(test),'models':{'multinomial_naive_bayes':metrics(test,nb_pred),'symbiotic_finite_context_memory':metrics(test,mem_pred)},'paired_comparison':paired_bootstrap(test,nb_pred,mem_pred),'prediction_ledger':ledger,'limits':['does not compare to transformer because transformer prediction vectors were not archived','bootstrap measures this holdout only','MIntRec labels are not HERUS events']}
    (E/'paired_holdout_analysis_v1.json').write_text(json.dumps(result,indent=2,ensure_ascii=False)+'\n')
    return result
if __name__=='__main__': print(json.dumps(run(),indent=2,ensure_ascii=False))
