"""Score calibration ablation for the universal router.

Each paradigm's raw score is mapped to empirical precision using calibration
examples only. The holdout is never used to choose thresholds.
"""
from __future__ import annotations
import json, math
from collections import Counter, defaultdict
from pathlib import Path
from independent_minds14_benchmark import fetch_rows, split
from real_data_baseline_benchmark import fetch_split, _nb_scores, cosine, metrics, vector

MIN_PRECISION=.80

def cents(rows):
    sums=defaultdict(Counter); counts=Counter()
    for r in rows: sums[r['label']].update(vector(r['text'])); counts[r['label']]+=1
    return {l:Counter({t:v/counts[l] for t,v in ws.items()}) for l,ws in sums.items()}

def classic(fit, rows, cs, ids):
    out=[]
    for r in rows:
        eid=ids(r); nl,nm=_nb_scores(fit,r); cl,cm=max(((l,cosine(vector(r['text']),c)) for l,c in cs.items()),key=lambda x:(x[1],x[0]))
        out.append({'id':eid,'label':r['label'],'supervised':(nl,1/(1+math.exp(-nm))),'unsupervised':(cl,max(0,cm))})
    return out

def calibrator(cal, paradigm):
    ordered=sorted(cal,key=lambda x:x[paradigm][1],reverse=True); total=0; table=[]
    for x in ordered:
        total += int(x[paradigm][0]==x['label']); table.append((x[paradigm][1], total/(len(table)+1)))
    def estimate(score):
        eligible=[p for s,p in table if s>=score]
        return eligible[-1] if eligible else 0.0
    return estimate

def evaluate(name, rows, fit, cal, hold, idfn):
    cs=cents(fit); ca=classic(fit,cal,cs,idfn); ho=classic(fit,hold,cs,idfn); funcs={p:calibrator(ca,p) for p in ('supervised','unsupervised')}; pred=[]; nb=[]; cent=[]; ledger=[]
    for x in ho:
        scored=sorted(((funcs[p](x[p][1]),x[p][1],p,x[p][0]) for p in funcs),reverse=True); best=scored[0]; pred.append(best[3]); nb.append(x['supervised'][0]); cent.append(x['unsupervised'][0]); ledger.append({'example_id':x['id'],'label':x['label'],'prediction':best[3],'selected_paradigm':best[2],'estimated_precision':best[0],'nb_prediction':x['supervised'][0],'centroid_prediction':x['unsupervised'][0]})
    return {'dataset':name,'metrics':{'calibrated_universal':metrics(hold,pred),'naive_bayes':metrics(hold,nb),'centroid':metrics(hold,cent)},'ledger':ledger,'protocol':{'min_precision':MIN_PRECISION,'calibration_only':True,'claim_boundary':'score calibration ablation; no SOTA claim'}}

def run():
    m=fetch_rows(); mf,mc,mh=split(m); r1=evaluate('PolyAI/minds14',m,mf,mc,mh,lambda r:r['path'])
    rows=fetch_split('train',2224); f=[r for r in rows if r['season']=='S04']; c=[r for r in rows if r['season']=='S05']; h=[r for r in rows if r['season']=='S06']; r2=evaluate('THU-IAR/MIntRec',rows,f,c,h,lambda r:f"{r['season']}/{r['episode']}/{r['clip']}")
    return {'schema':'herus-score-calibrated-universal-v1','results':[r1,r2]}

if __name__=='__main__':
    out=run(); p=Path('research/evidence/score_calibrated_universal_v1.json'); p.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n'); print(json.dumps({'path':str(p),'metrics':[x['metrics'] for x in out['results']]},indent=2))
