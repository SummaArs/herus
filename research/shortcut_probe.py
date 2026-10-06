"""Shortcut/invariance probe on real MIntRec utterances.

The transformation is deliberately narrow: add politeness/filler language
that should not change the intent label. This tests prediction stability, not
semantic equivalence in every possible language case.
"""
from __future__ import annotations
import json
from collections import Counter
from real_data_baseline_benchmark import fetch_split, nb, knn, centroid, contextual_memory, metrics
from intent_normalization import normalize_surface

FILLERS = ('please', 'if you can', 'thanks')

def transformed(rows, filler):
    return [{**row, 'text': f"{filler} {row['text']}"} for row in rows]

def flip_rate(original, changed):
    comparable = sum(a is not None and b is not None for a, b in zip(original, changed))
    flips = sum(a != b for a, b in zip(original, changed) if a is not None and b is not None)
    return flips / comparable if comparable else 0.0

def normalized(rows):
    return [{**row, 'text': normalize_surface(row['text'])} for row in rows]

def run():
    rows = fetch_split('train', 2224)
    train = [r for r in rows if r['season'] in {'S04', 'S05'}]
    test = [r for r in rows if r['season'] == 'S06']
    methods = {
        'multinomial_naive_bayes': lambda data: nb(train, data),
        '1nn_cosine': lambda data: knn(train, data),
        'centroid_cosine': lambda data: centroid(train, data),
        'symbiotic_finite_context_memory': lambda data: contextual_memory(train, data),
    }
    base = {name: fn(test) for name, fn in methods.items()}
    probes = {}
    for filler in FILLERS:
        changed = transformed(test, filler)
        canonical = normalized(changed)
        probes[filler] = {
            name: {
                'original_metrics': metrics(test, base[name]),
                'transformed_metrics': metrics(changed, fn(changed)),
                'canonical_metrics': metrics(canonical, fn(canonical)),
                'prediction_flip_rate': round(flip_rate(base[name], fn(changed)), 6),
                'canonical_recovery_rate': round(1.0 - flip_rate(base[name], fn(canonical)), 6),
            }
            for name, fn in methods.items()
        }
    return {
        'dataset': {'id': 'THU-IAR/MIntRec', 'source': 'https://datasets-server.huggingface.co/rows', 'train_seasons': ['S04', 'S05'], 'holdout_season': 'S06', 'holdout_rows': len(test)},
        'probe': {'type': 'filler_invariance', 'fillers': list(FILLERS), 'assumption': 'the added politeness/filler prefix should not change the original intent label', 'no_labels_changed': True},
        'results': probes,
        'limits': ['synthetic perturbation is not proof of full semantic invariance', 'only text metadata', 'does not measure transformer because its checkpoint is not persisted in this probe', 'a low flip rate can coexist with systematic wrong predictions'],
    }

if __name__ == '__main__':
    print(json.dumps(run(), indent=2, sort_keys=True))
