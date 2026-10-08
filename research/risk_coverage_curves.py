"""Risk-coverage curves for the finite HERUS selector and Naive Bayes."""
from __future__ import annotations
import json, urllib.parse, urllib.request, urllib.error, time
from collections import Counter, defaultdict
from pathlib import Path
from real_data_baseline_benchmark import _nb_scores, metrics, vector, cosine
from multi_host_real_benchmark import fetch_rows as fetch_mintrec, split as temporal_split
from independent_minds14_benchmark import fetch_rows as fetch_minds14, split as stratified_split, centroid_model


def scores(fit, hold):
    cents=centroid_model(fit); result=[]
    for row in hold:
        nb, margin = _nb_scores(fit,row); ranked=sorted(((cosine(vector(row['text']),c),label) for label,c in cents.items()), reverse=True); cent=ranked[0][1]
        result.append({'label':row['label'],'nb_correct':nb==row['label'],'herus_correct':nb==row['label'],'nb_score':margin,'herus_score':margin if nb==cent else 0.0})
    return result

def curve(rows, key, points=21):
    values=sorted({r[key] for r in rows}, reverse=True); thresholds=[values[int(i*(len(values)-1)/(points-1))] for i in range(points)] if values else []
    out=[]
    for t in thresholds:
        selected=[r for r in rows if r[key]>=t]; out.append({'threshold':round(t,8),'coverage':round(len(selected)/len(rows),6),'selective_accuracy':round(sum(r['herus_correct'] if key=='herus_score' else r['nb_correct'] for r in selected)/len(selected),6) if selected else 0.0})
    return out

def run():
    results=[]
    mint=fetch_mintrec()
    for season in ('S04','S05','S06'):
        local=[r for r in mint if r['season']==season]; fit,cal,hold=temporal_split(local); rows=scores(fit,hold); results.append({'dataset':'MIntRec','domain':season,'examples':len(rows),'herus_curve':curve(rows,'herus_score'),'naive_bayes_curve':curve(rows,'nb_score')})
    minds=fetch_minds14(); fit,cal,hold=stratified_split(minds); rows=scores(fit,hold); results.append({'dataset':'MInDS-14','domain':'pt-PT','examples':len(rows),'herus_curve':curve(rows,'herus_score'),'naive_bayes_curve':curve(rows,'nb_score')})
    return {'schema':'herus-risk-coverage-curves-v1','protocol':{'points':21,'score':'NB margin for baseline; NB-centroid agreement gated by margin for HERUS','claim_boundary':'curve evidence only; no SOTA claim'},'results':results,'limits':['selector is built from NB and centroid','not the isolated SymbioticLearner core','MInDS speaker independence not verified']}

if __name__=='__main__':
    out=run(); p=Path('research/evidence/risk_coverage_curves_v1.json'); p.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n'); print(json.dumps({'path':str(p),'datasets':[(x['dataset'],x['domain']) for x in out['results']]},indent=2))
