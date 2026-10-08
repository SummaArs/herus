#!/usr/bin/env python3
"""Analytical capacity bounds for D-bit majority bundles."""
from __future__ import annotations
import argparse, math
from statistics import NormalDist

D=10240; L=512; EPS=1e-3; DOC_CLAIMED_KMAX=308; MISS_BUDGETS=(.5,.1,.01,.001,.0001); N=NormalDist()

def p_member(k:int)->float:
    if k<1 or k%2==0: raise ValueError('k_must_be_positive_odd')
    m=(k-1)//2
    return .5+math.exp(math.lgamma(2*m+1)-2*math.lgamma(m+1)-k*math.log(2))

def z_fa(eps=EPS,l=L):
    if not (0<eps<1 and l>=1): raise ValueError('invalid_false_accept_budget')
    return N.inv_cdf(1-eps/l)

def theta(d=D,eps=EPS,l=L): return .5+z_fa(eps,l)/(2*math.sqrt(d))
def miss_rate(k:int,d=D,eps=EPS,l=L)->float:
    p=p_member(k); sd=math.sqrt(p*(1-p)/d); return N.cdf(-(p-theta(d,eps,l))/sd) if sd else 0.0
def closed_form_kmax(d=D,eps=EPS,l=L,z_miss=0.0)->float: return (2/math.pi)*d/(z_fa(eps,l)+z_miss)**2

def assess(k=DOC_CLAIMED_KMAX):
    valid=k >= 1 and k % 2 == 1
    miss=miss_rate(k) if valid else None
    return {'D':D,'L':L,'eps':EPS,'theta':theta(),'claimed_kmax':k,'claim_valid_odd':valid,'closed_form_kmax_z0':closed_form_kmax(),'member_miss_at_claim':miss,'safe_for_all_budgets':valid and all(miss<=b for b in MISS_BUDGETS)}

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--strict',action='store_true'); a=ap.parse_args(); out=assess(); print('D={D} L={L} eps={eps} theta={theta:.8f}'.format(**out)); print('claimed Kmax={claimed_kmax} odd_valid={claim_valid_odd} closed-form Kmax(z_miss=0)={closed_form_kmax_z0:.3f} miss={member_miss_at_claim}'.format(**out));
    for b in MISS_BUDGETS: print(f'miss_budget={b:g} safe={out["member_miss_at_claim"] is not None and out["member_miss_at_claim"]<=b}')
    if a.strict and not out['safe_for_all_budgets']: raise SystemExit(1)
if __name__=='__main__': main()
