"""Real MInDS-14 reference matrix.

All supervised models fit the same deterministic Portuguese fit split and are
scored on the same untouched holdout. This is comparison, not a claim that
one corpus establishes general superiority.
"""
from __future__ import annotations
import json, time
from collections import Counter
from minds14_real_benchmark import fetch_rows, stratified
from real_data_baseline_benchmark import metrics
from sklearn.feature_extraction.text import TfidfVectorizer
from sklearn.linear_model import LogisticRegression
from sklearn.svm import LinearSVC
from sklearn.ensemble import RandomForestClassifier
from sklearn.neighbors import KNeighborsClassifier
from sklearn.pipeline import make_pipeline

def run_model(name, model, fit, hold):
    start=time.perf_counter(); model.fit([r['text'] for r in fit],[r['label'] for r in fit]); train_ms=(time.perf_counter()-start)*1000
    start=time.perf_counter(); pred=model.predict([r['text'] for r in hold]); infer_ms=(time.perf_counter()-start)*1000
    result=metrics(hold,list(pred)); result.update(train_ms=round(train_ms,2),infer_ms=round(infer_ms,2),model=name)
    return result

def run():
    rows=fetch_rows(); fit,cal,hold=stratified(rows)
    models={
      'tfidf_logistic_regression':make_pipeline(TfidfVectorizer(ngram_range=(1,2),sublinear_tf=True),LogisticRegression(max_iter=500,random_state=7)),
      'tfidf_linear_svm':make_pipeline(TfidfVectorizer(ngram_range=(1,2),sublinear_tf=True),LinearSVC(random_state=7)),
      'tfidf_knn':make_pipeline(TfidfVectorizer(ngram_range=(1,2),sublinear_tf=True),KNeighborsClassifier(n_neighbors=5)),
      'tfidf_random_forest':make_pipeline(TfidfVectorizer(ngram_range=(1,2),sublinear_tf=True),RandomForestClassifier(n_estimators=100,random_state=7,n_jobs=1)),
    }
    results=[run_model(n,m,fit,hold) for n,m in models.items()]
    return {'dataset':'PolyAI/minds14','config':'pt-PT','fit_rows':len(fit),'holdout_rows':len(hold),'split':'deterministic within-label path order; no speaker-independent guarantee','models':results,'notes':['all classical models use the same text-only holdout','HERUS consensus and frozen multilingual encoder are reported in separate evidence to preserve protocol differences','no claim of universal superiority from one corpus']}
if __name__=='__main__': print(json.dumps(run(),indent=2,sort_keys=True))
