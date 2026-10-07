"""Split-conformal wrapper around the finite HERUS/NB adapter."""
from __future__ import annotations
import json,math
from pathlib import Path
import real_data_baseline_benchmark as b
E=Path(__file__).parent/'evidence'

def probs(train,row):
 labels=sorted({x['label'] for x in train}); docs={x:0 for x in labels}; words={x:{} for x in labels}; totals={x:0 for x in labels}; vocab=set()
 for item in train:
  y=item['label']; docs[y]+=1
  for t,c in b.vector(item['text']).items(): words[y][t]=words[y].get(t,0)+c; totals[y]+=c; vocab.add(t)
 counts=b.vector(row['text']); raw=[]
 for y in labels:
  s=math.log((docs[y]+1)/(len(train)+len(labels))); den=totals[y]+max(1,len(vocab))
  s+=sum(c*math.log((words[y].get(t,0)+1)/den) for t,c in counts.items()); raw.append((y,s))
 m=max(s for _,s in raw); ex=[(y,math.exp(s-m)) for y,s in raw]; z=sum(v for _,v in ex); return {y:v/z for y,v in ex}

def quantile(vals,q):
 vals=sorted(vals); return vals[min(len(vals)-1,max(0,math.ceil(q*len(vals))-1))]

def evaluate(train,cal,test,coverage_target):
 cal_scores=[1-probs(train,r)[r['label']] for r in cal]
 q=quantile(cal_scores,coverage_target)
 sets=[]
 for r in test:
  p=probs(train,r); sets.append(sorted([y for y,v in p.items() if 1-v<=q]))
 covered=sum(r['label'] in s for r,s in zip(test,sets)); singleton=[(r,s) for r,s in zip(test,sets) if len(s)==1]
 return {'target_marginal_coverage':coverage_target,'quantile':round(q,6),'test_set_coverage':round(covered/len(test),6),'mean_set_size':round(sum(map(len,sets))/len(sets),6),'singleton_coverage':round(len(singleton)/len(test),6),'singleton_accuracy':round(sum(r['label']==s[0] for r,s in singleton)/len(singleton),6) if singleton else 0.0}

def run():
 rows=b.fetch_split('train',2224); train=[r for r in rows if r['season']=='S04']; cal=[r for r in rows if r['season']=='S05']; test=[r for r in rows if r['season']=='S06']
 result={'protocol':'split conformal; S04 fit, S05 calibration, S06 chronological holdout','dataset':'THU-IAR/MIntRec','holdout_rows':len(test),'results':[evaluate(train,cal,test,q) for q in (.80,.90,.95,.99)],'limits':['marginal coverage is not per-class coverage','prediction sets are not natural-language reasoning','MIntRec labels are not HERUS events']}
 (E/'conformal_mintrec_v1.json').write_text(json.dumps(result,indent=2,ensure_ascii=False)+'\n'); return result
if __name__=='__main__': print(json.dumps(run(),indent=2,ensure_ascii=False))
