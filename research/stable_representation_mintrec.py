"""Stability-aware representation selection with temporal validation."""
import json
from sklearn.feature_extraction.text import TfidfVectorizer
from sklearn.linear_model import LogisticRegression
from sklearn.svm import LinearSVC
from real_data_baseline_benchmark import fetch_split, metrics

def eval_candidates(train, validations):
 specs={
  'word_tfidf_svm': lambda:TfidfVectorizer(ngram_range=(1,2),sublinear_tf=True),
  'char_tfidf_svm': lambda:TfidfVectorizer(analyzer='char',ngram_range=(3,5),sublinear_tf=True,max_features=30000),
  'word_tfidf_logistic': lambda:TfidfVectorizer(ngram_range=(1,2),sublinear_tf=True),
 }
 out={}
 for name,make_vec in specs.items():
  vec=make_vec(); x=vec.fit_transform([r['text'] for r in train])
  model=LinearSVC(C=1.0) if name.endswith('svm') else LogisticRegression(max_iter=500,C=4.0)
  model.fit(x,[r['label'] for r in train]); scores=[]
  for val in validations:
   scores.append(metrics(val,model.predict(vec.transform([r['text'] for r in val])).tolist()))
  out[name]={'accuracy_mean':sum(s['accuracy'] for s in scores)/len(scores),'accuracy_worst':min(s['accuracy'] for s in scores),'macro_f1_mean':sum(s['macro_f1'] for s in scores)/len(scores),'folds':scores}
 return out

def run():
 rows=fetch_split('train',2224); s04=[r for r in rows if r['season']=='S04']; s05=[r for r in rows if r['season']=='S05']; s06=[r for r in rows if r['season']=='S06']
 episodes=sorted({r['episode'] for r in s04}); cut=max(1,int(len(episodes)*.7)); early={e for e in episodes[:cut]}; late={e for e in episodes[cut:]}
 train=[r for r in s04 if r['episode'] in early]; v1=[r for r in s04 if r['episode'] in late]
 stability=eval_candidates(train,[v1,s05]); chosen=max(stability,key=lambda n:(stability[n]['accuracy_worst'],stability[n]['accuracy_mean'],stability[n]['macro_f1_mean']))
 refit=s04+s05; refit_out={}
 for name in stability:
  if name=='char_tfidf_svm': vec=TfidfVectorizer(analyzer='char',ngram_range=(3,5),sublinear_tf=True,max_features=30000)
  else: vec=TfidfVectorizer(ngram_range=(1,2),sublinear_tf=True)
  x=vec.fit_transform([r['text'] for r in refit]); model=LinearSVC(C=1.0) if name.endswith('svm') else LogisticRegression(max_iter=500,C=4.0); model.fit(x,[r['label'] for r in refit]); refit_out[name]=metrics(s06,model.predict(vec.transform([r['text'] for r in s06])).tolist())
 return {'dataset':'THU-IAR/MIntRec','selection':'mean_and_worst_temporal_validation','fit_rows':len(train),'validation_rows':[len(v1),len(s05)],'refit_rows':len(refit),'holdout_rows':len(s06),'holdout_split':'S06','stability':stability,'chosen':chosen,'holdout_refit':refit_out}
if __name__=='__main__': print(json.dumps(run(),indent=2,sort_keys=True))
