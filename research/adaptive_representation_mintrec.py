"""Domain-adaptive representation selection on MIntRec.
Selection uses S05 only; S06 remains a temporal holdout."""
import json
from pathlib import Path
from sklearn.feature_extraction.text import TfidfVectorizer
from sklearn.linear_model import LogisticRegression
from sklearn.svm import LinearSVC
from real_data_baseline_benchmark import fetch_split, metrics

def run():
 rows=fetch_split('train',2224)
 fit=[r for r in rows if r['season']=='S04']; cal=[r for r in rows if r['season']=='S05']; hold=[r for r in rows if r['season']=='S06']
 methods={
  'word_tfidf_svm': TfidfVectorizer(ngram_range=(1,2),min_df=1,sublinear_tf=True),
  'char_tfidf_svm': TfidfVectorizer(analyzer='char',ngram_range=(3,5),min_df=1,sublinear_tf=True,max_features=30000),
  'word_tfidf_logistic': TfidfVectorizer(ngram_range=(1,2),min_df=1,sublinear_tf=True),
 }
 cal_results={}; hold_results={}
 for name,vec in methods.items():
  xf=vec.fit_transform([r['text'] for r in fit]); xc=vec.transform([r['text'] for r in cal]); xh=vec.transform([r['text'] for r in hold])
  if name.endswith('svm'): model=LinearSVC(C=1.0)
  else: model=LogisticRegression(max_iter=500,C=4.0)
  model.fit(xf,[r['label'] for r in fit])
  pc=model.predict(xc).tolist(); ph=model.predict(xh).tolist()
  cal_results[name]=metrics(cal,pc); hold_results[name]=metrics(hold,ph)
 chosen=max(cal_results,key=lambda n:(cal_results[n]['accuracy'],cal_results[n]['macro_f1']))
 # Only after selection is fixed may S05 join the training data.
 refit_train=fit+cal
 refit_results={}
 for name in methods:
  vec=methods[name]
  xr=vec.fit_transform([r['text'] for r in refit_train]); xh=vec.transform([r['text'] for r in hold])
  model=LinearSVC(C=1.0) if name.endswith('svm') else LogisticRegression(max_iter=500,C=4.0)
  model.fit(xr,[r['label'] for r in refit_train])
  refit_results[name]=metrics(hold,model.predict(xh).tolist())
 return {'dataset':'THU-IAR/MIntRec','selection_split':'S05','fit_split':'S04','refit_splits':['S04','S05'],'holdout_split':'S06','fit_rows':len(fit),'calibration_rows':len(cal),'refit_rows':len(refit_train),'holdout_rows':len(hold),'calibration':cal_results,'holdout_pre_refit':hold_results,'holdout_refit':refit_results,'chosen_by_calibration':chosen,'chosen_holdout':refit_results[chosen]}
if __name__=='__main__': print(json.dumps(run(),indent=2,sort_keys=True))
