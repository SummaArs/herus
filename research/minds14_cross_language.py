"""Cross-language transfer on real MInDS-14 intent labels.

English is split into fit/calibration; Portuguese is an untouched external
holdout. The labels are shared by the dataset, so no inferred mapping is used.
"""
from __future__ import annotations
import json, time, urllib.error, urllib.parse, urllib.request
from collections import Counter
from real_data_baseline_benchmark import metrics, _nb_scores, vector
from consensus_risk_coverage_mintrec import centroid_model, consensus_score

API='https://datasets-server.huggingface.co/rows'; DATASET='PolyAI/minds14'

def fetch(config):
    q=urllib.parse.urlencode({'dataset':DATASET,'config':config,'split':'train','offset':0,'length':1})
    for attempt in range(4):
        try:
            with urllib.request.urlopen(API+'?'+q,timeout=60) as r: meta=json.load(r)
            break
        except urllib.error.HTTPError:
            if attempt == 3: raise
            time.sleep(1 + attempt)
    rows=[]
    for offset in range(0,meta['num_rows_total'],100):
        q=urllib.parse.urlencode({'dataset':DATASET,'config':config,'split':'train','offset':offset,'length':min(100,meta['num_rows_total']-offset)})
        for attempt in range(4):
            try:
                with urllib.request.urlopen(API+'?'+q,timeout=60) as r: payload=json.load(r)
                break
            except urllib.error.HTTPError:
                if attempt == 3: raise
                time.sleep(1 + attempt)
        rows.extend({'text':x['row']['transcription'],'label':str(x['row']['intent_class']),'path':x['row']['path']} for x in payload['rows'])
    return rows

def split(rows):
    groups={}
    for row in rows: groups.setdefault(row['label'],[]).append(row)
    fit=[]; cal=[]
    for label, group in sorted(groups.items()):
        group=sorted(group,key=lambda r:r['path']); cut=max(1,int(len(group)*.75)); fit.extend(group[:cut]); cal.extend(group[cut:])
    return fit,cal

def run():
    english=fetch('en-US'); portuguese=fetch('pt-PT'); fit,cal=split(english)
    cents=centroid_model(fit); cal_scores=[consensus_score(r,cents,fit) for r in cal]
    best=None
    for threshold in sorted({s[2] for s in cal_scores}):
        pred=[nb if nb==cent and margin>=threshold else None for nb,cent,margin in cal_scores]
        result=metrics(cal,pred)
        if result['coverage'] and result['selective_accuracy']>=.5 and (best is None or result['coverage']>best[1]['coverage']): best=(threshold,result)
    threshold=best[0] if best else float('inf')
    nb=[_nb_scores(fit,r)[0] for r in portuguese]
    cent_pred=[]
    for r in portuguese:
        q=vector(r['text']); cent_pred.append(max(cents,key=lambda label: __import__('real_data_baseline_benchmark').cosine(q,cents[label])))
    consensus=[a if a==b and _nb_scores(fit,r)[1]>=threshold else None for r,a,b in zip(portuguese,nb,cent_pred)]
    return {'dataset':{'id':DATASET,'fit_language':'en-US','calibration_language':'en-US','holdout_language':'pt-PT','fit_rows':len(fit),'calibration_rows':len(cal),'holdout_rows':len(portuguese),'shared_labels':len(set(r['label'] for r in english)&set(r['label'] for r in portuguese))},'policy':'NB and centroid agreement; threshold selected on English calibration','calibration':None if not best else {'threshold':round(threshold,6),'result':best[1]},'holdout':{'naive_bayes':metrics(portuguese,nb),'consensus_selective':metrics(portuguese,consensus)},'limits':['cross-language transfer is not product language understanding','no audio decoded','no speaker independence metadata verified','single language pair']}

if __name__=='__main__': print(json.dumps(run(),indent=2,sort_keys=True))
