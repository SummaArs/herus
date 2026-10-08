"""Second-dataset replication of the universal router on MIntRec."""
from __future__ import annotations
import json, math
from collections import Counter, defaultdict
from pathlib import Path
from real_data_baseline_benchmark import fetch_split, _nb_scores, cosine, metrics, vector
from universal_symbiotic import ParadigmCandidate, UniversalContract, UniversalSymbioticLearner

HOST='mintrec-temporal'; CONTRACT=UniversalContract(HOST,max_risk=0,max_cost=1,min_evidence=1,min_precision=0.80)

def ids(rows):
    return [f"{r['season']}/{r['episode']}/{r['clip']}" for r in rows]

def centroids(rows):
    sums=defaultdict(Counter); counts=Counter()
    for r in rows: sums[r['label']].update(vector(r['text'])); counts[r['label']]+=1
    return {label:Counter({t:v/counts[label] for t,v in words.items()}) for label,words in sums.items()}

def build(fit, rows, cents, truth):
    out=[]
    for r in rows:
        eid=f"{r['season']}/{r['episode']}/{r['clip']}"; label,margin=_nb_scores(fit,r); nb_score=1/(1+math.exp(-margin)); cl,cs=max(((l,cosine(vector(r['text']),c)) for l,c in cents.items()),key=lambda x:(x[1],x[0]))
        out += [ParadigmCandidate(eid,'supervised',label,nb_score,label==truth.get(eid),evidence=1,host_id=HOST),ParadigmCandidate(eid,'unsupervised',cl,max(0,cs),cl==truth.get(eid),evidence=1,host_id=HOST)]
    return out

def run():
    rows=fetch_split('train',2224); fit=[r for r in rows if r['season']=='S04']; cal=[r for r in rows if r['season']=='S05']; hold=[r for r in rows if r['season']=='S06']; cents=centroids(fit); truth={f"{r['season']}/{r['episode']}/{r['clip']}":r['label'] for r in rows}
    learner=UniversalSymbioticLearner(); fit_result=learner.fit(build(fit,cal,cents,truth),CONTRACT); candidates=build(fit,hold,cents,truth); groups=defaultdict(list)
    for c in candidates: groups[c.example_id].append(c)
    decisions=sorted((learner.decide(g,example_id=e) for e,g in groups.items()),key=lambda d:d.example_id); ordered=sorted(hold,key=lambda r:f"{r['season']}/{r['episode']}/{r['clip']}"); pred=[d.label for d in decisions]; nb=[_nb_scores(fit,r)[0] for r in ordered]; cent=[max(((cosine(vector(r['text']),c),l) for l,c in cents.items()))[1] for r in ordered]
    ledger=[{'example_id':f"{r['season']}/{r['episode']}/{r['clip']}",'label':r['label'],'universal_prediction':d.label,'selected_paradigm':d.paradigm,'universal_correct':d.label==r['label'],'nb_prediction':nb[i],'centroid_prediction':cent[i]} for i,(r,d) in enumerate(zip(ordered,decisions))]
    return {'schema':'herus-universal-mintrec-v1','dataset':{'id':'THU-IAR/MIntRec','fit_season':'S04','calibration_season':'S05','holdout_season':'S06','fit':len(fit),'calibration':len(cal),'holdout':len(hold)},'protocol':{'candidate_paradigms':['supervised','unsupervised'],'chronological_split':True,'holdout_labels_used_for_decision':False,'claim_boundary':'second-dataset replication; no general SOTA claim'},'fit':fit_result.__dict__,'metrics':{'universal':metrics(ordered,pred),'naive_bayes':metrics(ordered,nb),'centroid':metrics(ordered,cent)},'ledger':ledger}

if __name__=='__main__':
    out=run(); p=Path('research/evidence/universal_mintrec_v1.json'); p.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n'); print(json.dumps({'path':str(p),'metrics':out['metrics'],'fit':out['fit']},indent=2))
