"""Ablation analysis for the universal router on archived real ledgers."""
from __future__ import annotations
import json, random
from pathlib import Path

ROOT=Path(__file__).resolve().parent

def summarize(path, universal_key, nb_key, cent_key):
    data=json.loads((ROOT/'evidence'/path).read_text()); rows=data['ledger']
    def acc(key): return sum(x[key] == x['label'] for x in rows)/len(rows)
    universal=acc(universal_key); nb=acc(nb_key); cent=acc(cent_key)
    oracle=sum(any(x[k]==x['label'] for k in (nb_key,cent_key)) for x in rows)/len(rows)
    random.seed(20261008); trials=[]
    for _ in range(20000): trials.append(sum((x[nb_key] if random.random()<.5 else x[cent_key])==x['label'] for x in rows)/len(rows))
    trials.sort();
    return {'dataset':data['dataset'],'n':len(rows),'universal_accuracy':round(universal,6),'always_naive_bayes':round(nb,6),'always_centroid':round(cent,6),'random_mean':round(sum(trials)/len(trials),6),'random_ci95':[round(trials[500],6),round(trials[19499],6)],'oracle_candidate_ceiling':round(oracle,6),'universal_gain_vs_nb':round(universal-nb,6),'claim_boundary':'ablation only; not a SOTA claim'}

def run(): return {'schema':'herus-universal-ablation-v1','results':[summarize('universal_minds14_v1.json','universal_prediction','nb_prediction','centroid_prediction'),summarize('universal_mintrec_v1.json','universal_prediction','nb_prediction','centroid_prediction')]}

if __name__=='__main__':
    out=run(); p=ROOT/'evidence/universal_ablation_v1.json'; p.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n'); print(json.dumps(out,indent=2))
