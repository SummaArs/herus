"""Banking77 policy selection with a paired-bootstrap evidence gate."""
from __future__ import annotations
import json
from pathlib import Path
from policy_selection_banking77 import fetch, split_train, cents, default, calibrated, acc
from policy_selection_confidence import choose, paired_bootstrap_delta

def run():
    train=fetch('train'); test=fetch('test'); fit,cal=split_train(train); cal=sorted(cal,key=lambda x:x['id']); cut=len(cal)//2; tune,valid=cal[:cut],cal[cut:]; cs=cents(fit)
    dv=default(fit,tune,valid,cs); av=calibrated(fit,tune,valid,cs); labels=[r['label'] for r in valid]; chosen,ci=choose(dv,av,labels)
    d=default(fit,cal,test,cs); a=calibrated(fit,cal,test,cs); pred=a if chosen=='alternative' else d
    out={'schema':'herus-policy-selection-banking77-v2','dataset':{'id':'PolyAI-LDN/task-specific-datasets/banking_data','fit':len(fit),'calibration':len(cal),'holdout':len(test),'labels':len(set(r['label'] for r in train+test))},'selection':{'calibration_split':{'tune':len(tune),'validation':len(valid)},'validation_bootstrap':ci,'chosen_policy':chosen,'alternative_name':'score_calibrated','default_name':'universal_default','rule':'choose alternative only when paired 95% lower confidence bound is strictly positive'},'metrics':{'chosen_policy':{'accuracy':acc(test,pred)},'universal_default':{'accuracy':acc(test,d)},'score_calibrated':{'accuracy':acc(test,a)}},'protocol':{'holdout_labels_used_for_selection':False,'paired_bootstrap':True,'claim_boundary':'confidence-gated third-host replication; no SOTA claim'}}
    p=Path('research/evidence/policy_selection_banking77_v2.json'); p.write_text(json.dumps(out,indent=2,sort_keys=True)+'\n'); return out
if __name__=='__main__': print(json.dumps(run(),indent=2))
