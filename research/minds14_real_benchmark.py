"""External real-data benchmark on MInDS-14 Portuguese.

The dataset exposes one train split, so rows are partitioned deterministically
within each intent by sorted source path: 70% fit, 15% calibration, 15%
holdout. This is not a speaker-independent split; that limitation is reported.
"""
from __future__ import annotations
import json, time, urllib.error, urllib.parse, urllib.request
from collections import Counter
from real_data_baseline_benchmark import metrics, _nb_scores, vector, cosine
from consensus_risk_coverage_mintrec import centroid_model, consensus_score

API='https://datasets-server.huggingface.co/rows'
DATASET='PolyAI/minds14'; CONFIG='pt-PT'; SPLIT='train'

def fetch_rows():
    first=urllib.parse.urlencode({'dataset':DATASET,'config':CONFIG,'split':SPLIT,'offset':0,'length':1})
    for attempt in range(4):
        try:
            with urllib.request.urlopen(API+'?'+first, timeout=60) as r: meta=json.load(r)
            break
        except urllib.error.HTTPError:
            if attempt == 3: raise
            time.sleep(1 + attempt)
    total=meta['num_rows_total']; rows=[]
    for offset in range(0,total,100):
        q=urllib.parse.urlencode({'dataset':DATASET,'config':CONFIG,'split':SPLIT,'offset':offset,'length':min(100,total-offset)})
        for attempt in range(4):
            try:
                with urllib.request.urlopen(API+'?'+q, timeout=60) as r: payload=json.load(r)
                break
            except urllib.error.HTTPError:
                if attempt == 3: raise
                time.sleep(1 + attempt)
        rows.extend({'text':item['row']['transcription'],'label':str(item['row']['intent_class']),'path':item['row']['path']} for item in payload['rows'])
    return rows

def stratified(rows):
    groups={}
    for row in rows: groups.setdefault(row['label'],[]).append(row)
    fit=[]; cal=[]; hold=[]
    for label, group in sorted(groups.items()):
        group=sorted(group,key=lambda row:row['path']); n=len(group); a=max(1,int(n*.70)); b=max(a+1,int(n*.85))
        fit.extend(group[:a]); cal.extend(group[a:b]); hold.extend(group[b:])
    return fit,cal,hold

def centroid_predictions(fit,test):
    cents=centroid_model(fit); out=[]
    for row in test:
        q=vector(row['text']); out.append(max(cents,key=lambda label:cosine(q,cents[label])))
    return out

def run():
    rows=fetch_rows(); fit,cal,hold=stratified(rows)
    nb=[_nb_scores(fit,row)[0] for row in hold]
    cent=centroid_predictions(fit,hold)
    selected=centroid_model(fit)
    cal_scores=[consensus_score(row,selected,fit) for row in cal]
    thresholds=sorted({s[2] for s in cal_scores}); best=None
    for threshold in thresholds:
        pred=[nb_label if nb_label==centroid_label and margin>=threshold else None for nb_label,centroid_label,margin in cal_scores]
        m=metrics(cal,pred)
        if m['coverage'] and m['selective_accuracy']>=.5 and (best is None or m['coverage']>best[1]['coverage']): best=(threshold,m)
    threshold=best[0] if best else float('inf')
    hold_scores=[consensus_score(row,selected,fit) for row in hold]
    consensus=[nb_label if nb_label==centroid_label and margin>=threshold else None for nb_label,centroid_label,margin in hold_scores]
    return {'dataset':{'id':DATASET,'config':CONFIG,'source':API,'rows':len(rows),'labels':len(set(r['label'] for r in rows)),'fit':len(fit),'calibration':len(cal),'holdout':len(hold)},'split':'deterministic within-label path order; no speaker-independent guarantee','methods':{'naive_bayes':metrics(hold,nb),'centroid':metrics(hold,cent),'consensus_selective':metrics(hold,consensus)},'calibration':{'minimum_precision':.5,'threshold':None if not best else round(threshold,6),'result':None if not best else best[1]},'limits':['one published train split partitioned locally','no speaker-independent split verified','audio was not decoded; transcription only','MInDS labels are not HERUS events']}

if __name__=='__main__': print(json.dumps(run(),indent=2,sort_keys=True))
