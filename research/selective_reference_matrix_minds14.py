"""Risk-coverage comparison on the same real MInDS-14 protocol."""
from __future__ import annotations
import json
from minds14_real_benchmark import fetch_rows, stratified
from real_data_baseline_benchmark import metrics
from sklearn.feature_extraction.text import TfidfVectorizer
from sklearn.linear_model import LogisticRegression
from sklearn.svm import LinearSVC
from sklearn.ensemble import RandomForestClassifier
from sklearn.pipeline import make_pipeline

def score_model(model, fit, calibration, holdout, score):
    model.fit([r['text'] for r in fit],[r['label'] for r in fit])
    cal_pred=model.predict([r['text'] for r in calibration]); hold_pred=model.predict([r['text'] for r in holdout])
    cal_score=score(model, calibration); hold_score=score(model, holdout)
    candidates=sorted(set(cal_score))
    best=None
    for threshold in candidates:
        cp=[p if s>=threshold else None for p,s in zip(cal_pred,cal_score)]
        cm=metrics(calibration,cp)
        if cm['coverage'] and cm['selective_accuracy']>=.95 and (best is None or cm['coverage']>best[1]['coverage']): best=(threshold,cm)
    threshold=best[0] if best else float('inf')
    hp=[p if s>=threshold else None for p,s in zip(hold_pred,hold_score)]
    return {'calibration':None if not best else best[1],'holdout':metrics(holdout,hp),'threshold':threshold}

def margin_score(model, rows):
    values=model.decision_function([r['text'] for r in rows]);
    if values.ndim==1: return [abs(float(x)) for x in values]
    ordered=sorted((float(x) for x in row),reverse=True) if False else None
    return [float(sorted(row,reverse=True)[0]-sorted(row,reverse=True)[1]) for row in values]

def proba_score(model, rows): return [float(max(x)) for x in model.predict_proba([r['text'] for r in rows])]

def run():
    rows=fetch_rows(); fit,cal,hold=stratified(rows)
    models={
      'logistic_regression':(make_pipeline(TfidfVectorizer(ngram_range=(1,2),sublinear_tf=True),LogisticRegression(max_iter=500,random_state=7)),proba_score),
      'linear_svm':(make_pipeline(TfidfVectorizer(ngram_range=(1,2),sublinear_tf=True),LinearSVC(random_state=7)),margin_score),
      'random_forest':(make_pipeline(TfidfVectorizer(ngram_range=(1,2),sublinear_tf=True),RandomForestClassifier(n_estimators=100,random_state=7,n_jobs=1)),proba_score),
    }
    return {'dataset':'PolyAI/minds14','target_precision':.95,'fit_rows':len(fit),'calibration_rows':len(cal),'holdout_rows':len(hold),'models':{name:score_model(model,fit,cal,hold,score) for name,(model,score) in models.items()},'limits':['single text corpus','threshold calibrated on 89 examples','no speaker-independent split verified','not a universal benchmark']}
if __name__=='__main__': print(json.dumps(run(),indent=2,sort_keys=True))
