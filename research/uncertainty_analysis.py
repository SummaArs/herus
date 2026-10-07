"""Uncertainty analysis for published aggregate metrics.

Uses Wilson intervals for binomial accuracy/coverage. It intentionally does
not invent paired bootstrap predictions: the source artifacts contain metrics,
not per-example prediction vectors.
"""
from __future__ import annotations
import json, math
from pathlib import Path
ROOT=Path(__file__).parent; E=ROOT/'evidence'

def wilson(k,n,z=1.96):
    if n<=0: return None
    p=k/n; den=1+z*z/n; ctr=(p+z*z/(2*n))/den; half=z*math.sqrt(p*(1-p)/n+z*z/(4*n*n))/den
    return {'estimate':round(p,6),'low':round(max(0,ctr-half),6),'high':round(min(1,ctr+half),6)}

def run():
    specs=[('MIntRec S06','multinomial_naive_bayes',386,.492228),('MIntRec S06','distilbert_multilingual_cased',386,.466321),('MIntRec S06','tensor_train_rank_64',386,.422280),('MIntRec S06','bert_tiny',386,.176166),('MInDS-14 pt-PT','tfidf_linear_svm',98,.959184),('MInDS-14 pt-PT','tfidf_random_forest',98,.959184)]
    out=[]
    for ds,model,n,metric in specs:
        k=round(metric*n); out.append({'dataset':ds,'model':model,'n':n,'correct_rounded':k,'accuracy_wilson_95':wilson(k,n)})
    # HERUS selective evidence has accepted/total counts directly.
    out.append({'dataset':'MIntRec S06','model':'herus_selective_target_95','n':386,'accepted':88,'correct_accepted':82,'coverage_wilson_95':wilson(88,386),'selective_accuracy_wilson_95':wilson(82,88)})
    result={'protocol':'Wilson 95% intervals from published aggregate counts; rounded counts; no paired bootstrap because prediction vectors are not archived','results':out,'limits':['intervals quantify sampling uncertainty only','rounded published metrics introduce at most one-count ambiguity','macro-F1 confidence intervals not computed','no causal or universal superiority claim']}
    (E/'uncertainty_analysis_v1.json').write_text(json.dumps(result,indent=2,ensure_ascii=False)+'\n')
    return result
if __name__=='__main__': print(json.dumps(run(),indent=2,ensure_ascii=False))
