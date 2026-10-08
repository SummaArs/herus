"""Universal router with a calibrated local transformer candidate."""
from __future__ import annotations
import json, math, random
from collections import Counter, defaultdict
from pathlib import Path
import numpy as np
import torch
from torch.utils.data import DataLoader
from transformers import AutoTokenizer, AutoModelForSequenceClassification
from independent_minds14_benchmark import fetch_rows, split, TextDataset, MODEL, MAX_LENGTH, BATCH, LR
from real_data_baseline_benchmark import _nb_scores, cosine, metrics, vector
from universal_symbiotic import ParadigmCandidate, UniversalContract, UniversalSymbioticLearner

SEED=11; EPOCHS=5; HOST='minds14-transformer'; CONTRACT=UniversalContract(HOST,max_risk=0,max_cost=1,min_evidence=1,min_precision=0.80)

def seed_all():
    random.seed(SEED); np.random.seed(SEED); torch.manual_seed(SEED); torch.use_deterministic_algorithms(True,warn_only=True)

def centroids(rows):
    sums=defaultdict(Counter); counts=Counter()
    for r in rows: sums[r['label']].update(vector(r['text'])); counts[r['label']]+=1
    return {label:Counter({t:v/counts[label] for t,v in words.items()}) for label,words in sums.items()}

def classic_candidates(fit, rows, cents):
    out=[]
    for r in rows:
        label,margin=_nb_scores(fit,r); nb_score=1/(1+math.exp(-margin)); cl,cs=max(((l,cosine(vector(r['text']),c)) for l,c in cents.items()),key=lambda x:(x[1],x[0]))
        out += [ParadigmCandidate(r['path'],'supervised',label,nb_score,None,evidence=1,host_id=HOST),ParadigmCandidate(r['path'],'unsupervised',cl,max(0,cs),None,evidence=1,host_id=HOST)]
    return out

def train_and_score(fit,cal,hold):
    seed_all(); rows=fit+cal+hold; labels=sorted({r['label'] for r in rows}); l2i={x:i for i,x in enumerate(labels)}; i2l={i:x for x,i in l2i.items()}; tok=AutoTokenizer.from_pretrained(MODEL,use_fast=False); model=AutoModelForSequenceClassification.from_pretrained(MODEL,num_labels=len(labels),ignore_mismatched_sizes=True)
    train=DataLoader(TextDataset(fit,tok,l2i),batch_size=BATCH,shuffle=True,generator=torch.Generator().manual_seed(SEED)); cal_loader=DataLoader(TextDataset(cal,tok,l2i),batch_size=BATCH); hold_loader=DataLoader(TextDataset(hold,tok,l2i),batch_size=BATCH); opt=torch.optim.AdamW(model.parameters(),lr=LR); best=-1; state=None
    for _ in range(EPOCHS):
        model.train()
        for batch in train: out=model(**batch); out.loss.backward(); opt.step(); opt.zero_grad()
        model.eval(); preds=[]
        with torch.no_grad():
            for b in cal_loader: preds += model(**b).logits.argmax(1).tolist()
        acc=sum(i2l[p]==cal[i]['label'] for i,p in enumerate(preds))/len(cal)
        if acc>best: best=acc; state={k:v.detach().cpu().clone() for k,v in model.state_dict().items()}
    model.load_state_dict(state); model.eval()
    def score(loader):
        out=[]
        with torch.no_grad():
            for b in loader:
                p=model(**b).logits.softmax(-1); v,i=p.max(1); out += [(i2l[j],float(s)) for j,s in zip(i.tolist(),v.tolist())]
        return out
    return score(cal_loader),score(hold_loader)

def run():
    rows=fetch_rows(); fit,cal,hold=split(rows); cents=centroids(fit); cal_scores,hold_scores=train_and_score(fit,cal,hold); truth={r['path']:r['label'] for r in cal+hold}
    def build(rs, scores):
        out=classic_candidates(fit,rs,cents); out += [ParadigmCandidate(r['path'],'transformer',scores[i][0],scores[i][1],None,evidence=1,host_id=HOST) for i,r in enumerate(rs)]; return [ParadigmCandidate(x.example_id,x.paradigm,x.label,x.score,x.label==truth[x.example_id],x.risk,x.cost,x.evidence,x.host_id) for x in out]
    learner=UniversalSymbioticLearner(); fit_result=learner.fit(build(cal,cal_scores),CONTRACT); hold_c=build(hold,hold_scores); groups=defaultdict(list)
    for c in hold_c: groups[c.example_id].append(c)
    decisions=sorted((learner.decide(g,example_id=k) for k,g in groups.items()),key=lambda x:x.example_id); ordered=sorted(hold,key=lambda r:r['path']); pred=[d.label for d in decisions]
    nb=[_nb_scores(fit,r)[0] for r in ordered]; cent=[max(((cosine(vector(r['text']),c),l) for l,c in cents.items()))[1] for r in ordered]; transformer=[x[0] for x in hold_scores]
    ledger=[{'example_id':r['path'],'label':r['label'],'universal_prediction':d.label,'selected_paradigm':d.paradigm,'universal_correct':d.label==r['label'],'nb_prediction':nb[i],'centroid_prediction':cent[i],'transformer_prediction':transformer[i]} for i,(r,d) in enumerate(zip(ordered,decisions))]
    return {'schema':'herus-universal-transformer-minds14-v1','dataset':{'id':'PolyAI/minds14','fit':len(fit),'calibration':len(cal),'holdout':len(hold)},'protocol':{'seed':SEED,'transformer':MODEL,'epochs':EPOCHS,'candidate_paradigms':['supervised','unsupervised','transformer'],'holdout_labels_used_for_decision':False,'claim_boundary':'three-candidate proof-of-concept; no SOTA claim'},'fit':fit_result.__dict__,'metrics':{'universal':metrics(ordered,pred),'naive_bayes':metrics(ordered,nb),'centroid':metrics(ordered,cent),'transformer':metrics(ordered,transformer)},'ledger':ledger}

if __name__=='__main__':
    out=run(); p=Path('research/evidence/universal_transformer_minds14_v1.json'); p.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n'); print(json.dumps({'path':str(p),'metrics':out['metrics'],'fit':out['fit']},indent=2))
