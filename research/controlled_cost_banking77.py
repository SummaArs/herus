"""Controlled real-data cost comparison for Banking77 feature caching."""
from __future__ import annotations
import gc, json, os, resource, subprocess, sys, time
from pathlib import Path
from policy_selection_banking77 import fetch, split_train, cents, default, calibrated, acc
from real_data_baseline_benchmark import fit_nb


def one(mode: str) -> dict:
    raw_train=fetch('train'); test=fetch('test')
    fit,cal=split_train(raw_train); cal=sorted(cal,key=lambda x:x['id'])
    cut=len(cal)//2; tune,valid=cal[:cut],cal[cut:]
    model=fit_nb(fit)
    features={} if mode=='cached' else None
    cs=cents(fit,features)
    gc.collect()
    start_wall=time.perf_counter(); start_cpu=time.process_time()
    dv=default(fit,tune,valid,cs,model,features)
    av=calibrated(fit,tune,valid,cs,model,features)
    chosen='score_calibrated' if acc(valid,av)>acc(valid,dv) else 'universal_default'
    d=default(fit,cal,test,cs,model,features)
    a=calibrated(fit,cal,test,cs,model,features)
    elapsed_wall=time.perf_counter()-start_wall; elapsed_cpu=time.process_time()-start_cpu
    return {'mode':mode,'wall_seconds':round(elapsed_wall,6),'cpu_seconds':round(elapsed_cpu,6),'peak_rss_kb':resource.getrusage(resource.RUSAGE_SELF).ru_maxrss,'chosen_policy':chosen,'holdout':len(test),'default_accuracy':round(acc(test,d),6),'calibrated_accuracy':round(acc(test,a),6),'feature_cache_entries':len(features) if features is not None else 0}


def run() -> dict:
    if len(sys.argv)>1 and sys.argv[1] in {'cached','uncached'}:
        return one(sys.argv[1])
    root=Path(__file__).resolve()
    env=os.environ.copy(); env['PYTHONPATH']=str(root.parent)+':'+str(root.parent/'research')
    rows=[]
    for mode in ('uncached','cached'):
        rows.append(json.loads(subprocess.check_output([sys.executable,str(root),mode],env=env,text=True)))
    u,c=rows
    return {'schema':'herus-controlled-cost-banking77-v1','dataset':'PolyAI-LDN/task-specific-datasets/banking_data','protocol':{'same_real_dataset':True,'same_split':True,'timed_region':'model evaluation only; network fetch excluded','separate_processes_for_rss':True,'claim_boundary':'controlled CPU/latency comparison; no energy or SOTA claim'},'uncached':u,'cached':c,'relative':{'wall_speedup':round(u['wall_seconds']/c['wall_seconds'],4),'cpu_speedup':round(u['cpu_seconds']/c['cpu_seconds'],4),'predictions_equal':u['default_accuracy']==c['default_accuracy'] and u['calibrated_accuracy']==c['calibrated_accuracy']}}

if __name__=='__main__': print(json.dumps(run(),indent=2,sort_keys=True))
