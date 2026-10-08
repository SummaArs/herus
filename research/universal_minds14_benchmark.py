"""Paired real-data benchmark for the Universal Symbiotic Learner."""
from __future__ import annotations
import json, math
from collections import Counter, defaultdict
from pathlib import Path
from independent_minds14_benchmark import fetch_rows, split
from real_data_baseline_benchmark import _nb_scores, cosine, metrics, vector
from universal_symbiotic import ParadigmCandidate, UniversalContract, UniversalSymbioticLearner

HOST='minds14-text'; CONTRACT=UniversalContract(HOST,max_risk=0,max_cost=1,min_evidence=1,min_precision=0.90)

def centroids(rows):
    sums=defaultdict(Counter); counts=Counter()
    for r in rows: sums[r['label']].update(vector(r['text'])); counts[r['label']]+=1
    return {label:Counter({t:v/counts[label] for t,v in words.items()}) for label,words in sums.items()}

def candidates(fit, rows, cents, prefix):
    out=[]
    for r in rows:
        nb_label, margin=_nb_scores(fit,r); nb_score=1/(1+math.exp(-margin))
        cent_label, cent_score=max(((label,cosine(vector(r['text']),c)) for label,c in cents.items()),key=lambda x:(x[1],x[0]))
        out.extend([
            ParadigmCandidate(r['path'], 'supervised', nb_label, nb_score, None, evidence=1, host_id=HOST),
            ParadigmCandidate(r['path'], 'unsupervised', cent_label, max(0.0,cent_score), None, evidence=1, host_id=HOST),
        ])
    return out

def with_truth(cands, rows):
    truth={r['path']:r['label'] for r in rows}
    return [ParadigmCandidate(x.example_id,x.paradigm,x.label,x.score,x.label==truth[x.example_id],x.risk,x.cost,x.evidence,x.host_id) for x in cands]

def run():
    rows=fetch_rows(); fit,cal,hold=split(rows); cents=centroids(fit)
    cal_truth=with_truth(candidates(fit,cal,cents,'cal'),cal)
    learner=UniversalSymbioticLearner(); fit_result=learner.fit(cal_truth,CONTRACT)
    hold_cands=candidates(fit,hold,cents,'hold'); decisions=[]
    grouped=defaultdict(list)
    for c in hold_cands: grouped[c.example_id].append(c)
    for example_id, group in grouped.items(): decisions.append(learner.decide(group,example_id=example_id))
    decisions.sort(key=lambda x:x.example_id)
    ordered=sorted(hold,key=lambda r:r['path']); pred=[d.label for d in decisions]
    nb=[_nb_scores(fit,r)[0] for r in ordered]; cent=[max(((cosine(vector(r['text']),c),label) for label,c in cents.items()))[1] for r in ordered]
    ledger=[{'example_id':r['path'],'label':r['label'],'universal_prediction':d.label,'universal_status':d.status,'universal_reason':d.reason,'selected_paradigm':d.paradigm,'nb_prediction':nb[i],'centroid_prediction':cent[i],'universal_correct':d.label==r['label'],'nb_correct':nb[i]==r['label'],'centroid_correct':cent[i]==r['label']} for i,(r,d) in enumerate(zip(ordered,decisions))]
    return {'schema':'herus-universal-minds14-v1','dataset':{'id':'PolyAI/minds14','config':'pt-PT','rows':len(rows),'fit':len(fit),'calibration':len(cal),'holdout':len(hold)},'protocol':{'candidate_paradigms':['supervised','unsupervised'],'contract':CONTRACT.__dict__,'calibration_labels_used':True,'holdout_labels_used_for_decision':False,'claim_boundary':'universal router proof-of-concept; no general SOTA claim'},'fit':fit_result.__dict__,'metrics':{'universal':metrics(ordered,pred),'naive_bayes':metrics(ordered,nb),'centroid':metrics(ordered,cent)},'ledger':ledger}

if __name__=='__main__':
    out=run(); p=Path('research/evidence/universal_minds14_v1.json'); p.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n'); print(json.dumps({'path':str(p),'metrics':out['metrics'],'fit':out['fit']},indent=2))
