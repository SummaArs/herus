"""Cross-domain shift-detection benchmark; no classifier claim."""
from __future__ import annotations
import json, re, time, urllib.parse, urllib.request
from pathlib import Path
from shift_detector import LexicalShiftDetector
from adversarial_banking77 import mutate

API='https://datasets-server.huggingface.co/rows'

def fetch(dataset, config, split='train', total=None):
    meta={"dataset":dataset,"config":config,"split":split,"offset":0,"length":1}
    with urllib.request.urlopen(API+'?'+urllib.parse.urlencode(meta),timeout=60) as r: payload=json.load(r)
    n=total or payload['num_rows_total']; rows=[]
    for off in range(0,n,100):
        q={"dataset":dataset,"config":config,"split":split,"offset":off,"length":min(100,n-off)}
        with urllib.request.urlopen(API+'?'+urllib.parse.urlencode(q),timeout=60) as r: page=json.load(r)
        rows.extend(page['rows'])
    return rows

def run():
    m=fetch('THU-IAR/MIntRec','default',total=2224)
    m_cal=[x['row']['text'] for x in m if x['row'].get('season')=='S05']
    m_hold=[x['row']['text'] for x in m if x['row'].get('season')=='S06']
    d=fetch('PolyAI/minds14','pt-PT')
    d_sorted=sorted(d,key=lambda x:x['row']['path']); cut1=int(len(d_sorted)*.70); cut2=int(len(d_sorted)*.85)
    d_cal=[x['row']['transcription'] for x in d_sorted[cut1:cut2]]; d_hold=[x['row']['transcription'] for x in d_sorted[cut2:]]
    domains={'MIntRec':(m_cal,m_hold),'MInDS-14':(d_cal,d_hold)}; out={'schema':'herus-cross-domain-shift-v1','protocol':{'calibration_is_clean_and_disjoint':True,'holdout_labels_used':False,'claim_boundary':'shift detection only; no classifier robustness or SOTA claim'},'domains':{}}
    for name,(cal,hold) in domains.items():
        detector=LexicalShiftDetector(quantile=.99); detector.fit(cal)
        clean_rate=sum(detector.accept(x) for x in hold)/len(hold)
        attacks={}
        for kind in ('case_punctuation','typo','deletion','irrelevant_prefix'):
            attacked=[mutate(x,kind) for x in hold]
            attacks[kind]={'n':len(attacked),'clean_acceptance':round(clean_rate,6),'attacked_acceptance':round(sum(detector.accept(x) for x in attacked)/len(attacked),6),'abstention_rate':round(1-sum(detector.accept(x) for x in attacked)/len(attacked),6)}
        out['domains'][name]={'calibration_examples':len(cal),'holdout_examples':len(hold),'attacks':attacks}
    Path('research/evidence/cross_domain_shift_v1.json').write_text(json.dumps(out,indent=2,sort_keys=True)+'\n'); return out

if __name__=='__main__': print(json.dumps(run(),indent=2))
