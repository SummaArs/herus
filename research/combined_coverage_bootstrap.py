"""Combined paired analysis at the exact HERUS acceptance subset.

This does not claim model superiority: it measures whether HERUS-selected cases
are solved more reliably than the paired transformer on the same examples.
"""
from __future__ import annotations
import json, random
from pathlib import Path
E=Path(__file__).parent/'evidence'; ITER=4000; SEED=17

def ci(values):
    values=sorted(values); return {'mean':round(sum(values)/len(values),6),'ci95_low':round(values[int(.025*len(values))],6),'ci95_high':round(values[int(.975*len(values))-1],6)}

def bootstrap(rows):
    rng=random.Random(SEED); n=len(rows); raw=[]; matched=[]
    for _ in range(ITER):
        s=[rows[rng.randrange(n)] for _ in range(n)]
        raw.append(sum(x['herus_correct']-x['transformer_correct'] for x in s)/n)
        matched.append(sum(x['herus_correct']-x['transformer_correct'] for x in s)/n)
    return {'raw_accuracy_delta':ci(raw),'same_herus_acceptance_subset_delta':ci(matched),'examples':n,'iterations':ITER,'seed':SEED}

def run():
    h=json.loads((E/'multi_host_real_mintrec_v1.json').read_text()); t=json.loads((E/'multi_host_transformer_mintrec_v1.json').read_text()); m=json.loads((E/'independent_minds14_v1.json').read_text()); out=[]
    # MIntRec: use seed 11 and only HERUS-accepted examples; transformer full prediction is paired.
    for hh,tt in zip(h['hosts'],t['hosts']):
        hl=next(x for x in hh['seed_runs'] if x['seed']==11)['prediction_ledger']; tl=next(x for x in tt['runs'] if x['seed']==11)['prediction_ledger']; tm={x['example_id']:x for x in tl}; rows=[{'example_id':x['example_id'],'herus_correct':x['adapter_correct'],'transformer_correct':tm[x['example_id']]['transformer_correct']} for x in hl if x['adapter_accepted']]
        out.append({'dataset':'MIntRec','host_id':hh['host_id'],'herus_coverage':len(rows)/len(hl),'comparison':bootstrap(rows)})
    # MInDS: transformer ledger is aligned; compare on HERUS accepted subset.
    tm={x['example_id']:x for x in m['transformer'][0]['ledger']}; rows=[{'example_id':x['example_id'],'herus_correct':x['herus_correct'],'transformer_correct':tm[x['example_id']]['transformer_correct']} for x in m['ledger'] if x['herus_accepted']]
    out.append({'dataset':'MInDS-14','host_id':'pt-PT','herus_coverage':len(rows)/len(m['ledger']),'comparison':bootstrap(rows)})
    return {'schema':'herus-combined-coverage-bootstrap-v1','protocol':{'same_examples':'transformer evaluated on the exact HERUS-accepted subset','bootstrap_iterations':ITER,'seed':SEED,'claim_boundary':'selection effect, not general model superiority'},'results':out,'limits':['selection subset is defined by HERUS','no coverage-matched retraining','transformer is small','MInDS speaker independence not verified']}

if __name__=='__main__':
    out=run(); p=E/'combined_coverage_bootstrap_v1.json'; p.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n'); print(json.dumps({'path':str(p),'results':out['results']},indent=2))
