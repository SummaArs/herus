"""Calibrated margin ensemble; S05 selects weights, S06 is untouched."""
import itertools,json,numpy as np
from sklearn.feature_extraction.text import TfidfVectorizer
from sklearn.svm import LinearSVC
from real_data_baseline_benchmark import fetch_split,metrics

def fit_margin(name,train,test):
    if name=='char': vec=TfidfVectorizer(analyzer='char',ngram_range=(3,5),sublinear_tf=True,max_features=30000)
    else: vec=TfidfVectorizer(ngram_range=(1,2),sublinear_tf=True)
    model=LinearSVC(C=1.0); x=vec.fit_transform([r['text'] for r in train]); model.fit(x,[r['label'] for r in train]);
    return model.classes_,model.decision_function(vec.transform([r['text'] for r in test]))
def predict(classes,margins,weights):
    labels=sorted(set(classes)); out=[]
    for row in zip(*margins):
        score={label:0.0 for label in labels}
        for model_margin,weight in zip(row,weights):
            for index,cls in enumerate(classes): score[cls]+=weight*float(model_margin[index])
        out.append(max(score,key=score.get))
    return out
def run():
    rows=fetch_split('train',2224); s04=[r for r in rows if r['season']=='S04']; s05=[r for r in rows if r['season']=='S05']; s06=[r for r in rows if r['season']=='S06']
    c1,m1=fit_margin('word',s04,s05); c2,m2=fit_margin('char',s04,s05); classes=c1.tolist(); assert classes==c2.tolist()
    candidates=[(a/10,b/10) for a in range(0,11) for b in range(0,11) if a+b>0]
    scores=[]
    for w in candidates:
        pred=predict(classes,[m1,m2],w); r=metrics(s05,pred); scores.append((r['accuracy'],r['macro_f1'],w))
    best=max(scores,key=lambda x:(x[0],x[1])); weights=best[2]
    r1,t1=fit_margin('word',s04+s05,s06); r2,t2=fit_margin('char',s04+s05,s06); hold=metrics(s06,predict(r1,[t1,t2],weights))
    return {'dataset':'THU-IAR/MIntRec','selection_split':'S05','refit_splits':['S04','S05'],'holdout_split':'S06','weights':weights,'calibration_best':{'accuracy':best[0],'macro_f1':best[1]},'holdout':hold,'grid_size':len(candidates)}
if __name__=='__main__': print(json.dumps(run(),indent=2,sort_keys=True))
