"""Confidence-gated policy selection for the universal router."""
from __future__ import annotations
import random
from typing import Sequence


def paired_bootstrap_delta(default: Sequence[str|None], alternative: Sequence[str|None], labels: Sequence[str], *, seed: int=20261008, rounds: int=2000) -> dict[str,float]:
    if not (len(default)==len(alternative)==len(labels)) or not labels:
        raise ValueError('paired_inputs_must_have_equal_nonzero_length')
    n=len(labels); rng=random.Random(seed); deltas=[]
    for _ in range(rounds):
        idx=[rng.randrange(n) for _ in range(n)]
        deltas.append(sum(int(alternative[i]==labels[i])-int(default[i]==labels[i]) for i in idx)/n)
    deltas.sort(); lo=deltas[max(0,int(rounds*.025)-1)]; hi=deltas[min(rounds-1,int(rounds*.975))]
    observed=sum(int(a==y)-int(d==y) for d,a,y in zip(default,alternative,labels))/n
    return {'observed_delta':round(observed,6),'lower_95':round(lo,6),'upper_95':round(hi,6),'rounds':rounds,'seed':seed}


def choose(default: Sequence[str|None], alternative: Sequence[str|None], labels: Sequence[str], *, seed: int=20261008, rounds: int=2000) -> tuple[str,dict[str,float]]:
    ci=paired_bootstrap_delta(default,alternative,labels,seed=seed,rounds=rounds)
    return ('alternative' if ci['lower_95']>0 else 'default'),ci
