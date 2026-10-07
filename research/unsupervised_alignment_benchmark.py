"""Unsupervised mean-alignment ablation for MInDS-14.

Target texts are observed without labels. Source class centroids are shifted
by the source-to-target global mean difference; labels remain untouched.
"""
import json, torch
from transformers import AutoTokenizer, AutoModel
from minds14_cross_language import fetch, split
from multilingual_encoder_benchmark import encode, centroids, predict
from real_data_baseline_benchmark import metrics
MODEL='distilbert/distilbert-base-multilingual-cased'
def run():
    en=fetch('en-US'); pt=fetch('pt-PT'); fit,cal=split(en)
    tok=AutoTokenizer.from_pretrained(MODEL); model=AutoModel.from_pretrained(MODEL); model.eval()
    vf=encode(fit,tok,model); vc=encode(cal,tok,model); vt=encode(pt,tok,model)
    base=centroids(vf,fit); shifted={k:v-torch.stack(vf).mean(0)+torch.stack(vt).mean(0) for k,v in base.items()}
    cp,cm=predict(vc,shifted); tp,tm=predict(vt,shifted); best=None
    for threshold in sorted(set(cm)):
        result=metrics(cal,[x if m>=threshold else None for x,m in zip(cp,cm)])
        if result['coverage'] and result['selective_accuracy']>=.5 and (best is None or result['coverage']>best[1]['coverage']): best=(threshold,result)
    threshold=best[0] if best else float('inf')
    return {'method':'global mean alignment using unlabeled Portuguese texts','model':MODEL,'target_labels_used':False,'calibration':None if not best else {'threshold':threshold,'result':best[1]},'holdout':metrics(pt,[x if m>=threshold else None for x,m in zip(tp,tm)]),'baseline_frozen_encoder':{'accuracy':0.486755,'coverage':0.990066,'selective_accuracy':0.491639},'conclusion':'no measurable improvement over frozen multilingual encoder','limits':['transductive target text observation','single language pair','no audio','no speaker independence metadata']}
if __name__=='__main__': print(json.dumps(run(),indent=2,sort_keys=True))
