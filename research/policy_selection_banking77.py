"""Third independent host: Banking77 policy selection."""
from __future__ import annotations
import csv, json, math, urllib.request
from collections import Counter, defaultdict
from pathlib import Path
from real_data_baseline_benchmark import fit_nb, cosine, metrics, vector
from score_calibrated_universal import calibrator

HOST='banking77'; BASE='https://raw.githubusercontent.com/PolyAI-LDN/task-specific-datasets/master/banking_data/'

def _features(row, feature_cache):
    if feature_cache is None:
        return vector(row['text'])
    key=row['id']
    features=feature_cache.get(key)
    if features is None:
        features=vector(row['text']); feature_cache[key]=features
    return features

def fetch(name):
    with urllib.request.urlopen(BASE+name+'.csv',timeout=60) as r: rows=list(csv.DictReader(r.read().decode().splitlines()))
    return [{'text':x['text'],'label':x['category'],'id':f'{name}/{i:05d}'} for i,x in enumerate(rows)]

def split_train(rows):
    groups=defaultdict(list)
    for r in rows: groups[r['label']].append(r)
    fit=[]; cal=[]
    for label,group in sorted(groups.items()):
        group=sorted(group,key=lambda x:x['text']); cut=max(1,int(len(group)*.70)); fit+=group[:cut]; cal+=group[cut:]
    return fit,cal

def cents(rows, feature_cache=None):
    sums=defaultdict(Counter); counts=Counter()
    for r in rows:
        features=_features(r,feature_cache)
        sums[r['label']].update(features); counts[r['label']]+=1
    return {l:Counter({t:v/counts[l] for t,v in ws.items()}) for l,ws in sums.items()}

def classic(fit,rows,cs,model=None,feature_cache=None):
    out=[]; model=model or fit_nb(fit)
    for r in rows:
        nl,nm=model.score(r); features=_features(r,feature_cache)
        cl,cm=max(((l,cosine(features,c)) for l,c in cs.items()),key=lambda x:(x[1],x[0]))
        out.append({'id':r['id'],'label':r['label'],'supervised':(nl,1/(1+math.exp(-nm))),'unsupervised':(cl,max(0,cm))})
    return out

def default(fit,train,rows,cs,model=None,feature_cache=None):
    model=model or fit_nb(fit); ca=classic(fit,train,cs,model,feature_cache); thresholds={}
    for p in ('supervised','unsupervised'):
        good=[x[p][1] for x in ca if x[p][0]==x['label']]
        thresholds[p]=min(good) if good else float('inf')
    ho=classic(fit,rows,cs,model,feature_cache); pred=[]
    for x in ho:
        valid=[(x[p][1],x[p][0]) for p in thresholds if x[p][1]>=thresholds[p]]
        pred.append(max(valid,key=lambda z:z[0])[1] if valid else None)
    return pred

def calibrated(fit,train,rows,cs,model=None,feature_cache=None):
    model=model or fit_nb(fit); ca=classic(fit,train,cs,model,feature_cache); ho=classic(fit,rows,cs,model,feature_cache); funcs={p:calibrator(ca,p) for p in ('supervised','unsupervised')}; pred=[]
    for x in ho:
        pred.append(max(((funcs[p](x[p][1]),x[p][0]) for p in funcs),key=lambda z:z[0])[1])
    return pred

def acc(rows,pred): return sum(r['label']==p for r,p in zip(rows,pred))/len(rows)

def run():
    raw_train=fetch('train'); test=fetch('test'); fit,cal=split_train(raw_train); cal=sorted(cal,key=lambda x:x['id']); cut=len(cal)//2; tune,valid=cal[:cut],cal[cut:]
    model=fit_nb(fit); cs=cents(fit); a=acc(valid,default(fit,tune,valid,cs,model)); b=acc(valid,calibrated(fit,tune,valid,cs,model)); chosen='score_calibrated' if b>a else 'universal_default'; pa=default(fit,cal,test,cs,model); pb=calibrated(fit,cal,test,cs,model); pred=pb if chosen=='score_calibrated' else pa
    out={'schema':'herus-policy-selection-banking77-v1','dataset':{'id':'PolyAI-LDN/task-specific-datasets/banking_data','source':BASE,'fit':len(fit),'calibration':len(cal),'holdout':len(test),'labels':len(set(r['label'] for r in raw_train+test))},'selection':{'calibration_split':{'tune':len(tune),'validation':len(valid)},'validation_accuracy':{'universal_default':round(a,6),'score_calibrated':round(b,6)},'chosen_policy':chosen,'tie_rule':'universal_default'},'metrics':{'chosen_policy':metrics(test,pred),'universal_default':metrics(test,pa),'score_calibrated':metrics(test,pb)},'protocol':{'holdout_labels_used_for_selection':False,'nested_calibration':True,'train_test_separate':True,'claim_boundary':'third-host replication; no SOTA claim'}}
    p=Path('research/evidence/policy_selection_banking77_v1.json'); p.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n'); return out

if __name__=='__main__': print(json.dumps(run(),indent=2))
