"""Leak-free ensemble selection on MIntRec."""
import json
from sklearn.feature_extraction.text import TfidfVectorizer
from sklearn.linear_model import LogisticRegression
from sklearn.svm import LinearSVC
from real_data_baseline_benchmark import fetch_split, metrics

def train_predict(name, train, test):
    if name == 'char_svm': vec=TfidfVectorizer(analyzer='char',ngram_range=(3,5),sublinear_tf=True,max_features=30000); model=LinearSVC(C=1.0)
    elif name == 'word_logistic': vec=TfidfVectorizer(ngram_range=(1,2),sublinear_tf=True); model=LogisticRegression(max_iter=500,C=4.0)
    else: vec=TfidfVectorizer(ngram_range=(1,2),sublinear_tf=True); model=LinearSVC(C=1.0)
    x=vec.fit_transform([r['text'] for r in train]); y=[r['label'] for r in train]; model.fit(x,y)
    return model.predict(vec.transform([r['text'] for r in test])).tolist()

def vote(predictions):
    out=[]
    for vals in zip(*predictions):
        counts={v:vals.count(v) for v in vals}; out.append(max(counts,key=lambda v:(counts[v],-vals.index(v))))
    return out

def run():
    rows=fetch_split('train',2224); s04=[r for r in rows if r['season']=='S04']; s05=[r for r in rows if r['season']=='S05']; s06=[r for r in rows if r['season']=='S06']
    names=['word_svm','char_svm','word_logistic']; cal_preds=[train_predict(n,s04,s05) for n in names]
    single={n:metrics(s05,p) for n,p in zip(names,cal_preds)}
    combos={'word_char_vote':vote(cal_preds[:2]),'all_vote':vote(cal_preds)}
    combo_scores={n:metrics(s05,p) for n,p in combos.items()}
    chosen=max({**single,**combo_scores},key=lambda n:({**single,**combo_scores}[n]['accuracy'],{**single,**combo_scores}[n]['macro_f1']))
    refit=s04+s05; test_preds=[train_predict(n,refit,s06) for n in names]
    refit_combos={'word_char_vote':vote(test_preds[:2]),'all_vote':vote(test_preds)}
    hold={n:metrics(s06,p) for n,p in zip(names,test_preds)}; hold.update({n:metrics(s06,p) for n,p in refit_combos.items()})
    return {'dataset':'THU-IAR/MIntRec','selection_split':'S05','holdout_split':'S06','fit_rows':len(s04),'calibration_rows':len(s05),'refit_rows':len(refit),'holdout_rows':len(s06),'calibration':{**single,**combo_scores},'chosen':chosen,'holdout_refit':hold}
if __name__=='__main__': print(json.dumps(run(),indent=2,sort_keys=True))
