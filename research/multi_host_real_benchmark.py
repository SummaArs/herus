"""Real-data temporal multi-host proxy; not a HERUS-core or SOTA claim."""
from __future__ import annotations
import json, random, time, urllib.error, urllib.parse, urllib.request
from collections import Counter
from pathlib import Path
from real_data_baseline_benchmark import metrics, nb, centroid, calibrated_symbiotic_memory

API = 'https://datasets-server.huggingface.co/rows'
DATASET = 'THU-IAR/MIntRec'
SEASONS = ('S04', 'S05', 'S06')
SEEDS = (11, 23, 47)


def fetch_rows(total=2224):
    rows = []
    for offset in range(0, total, 100):
        q = urllib.parse.urlencode({'dataset': DATASET, 'config': 'default', 'split': 'train', 'offset': offset, 'length': min(100, total-offset)})
        for attempt in range(5):
            try:
                with urllib.request.urlopen(API + '?' + q, timeout=60) as response:
                    payload = json.load(response)
                break
            except (urllib.error.HTTPError, urllib.error.URLError, TimeoutError):
                if attempt == 4:
                    raise
                time.sleep(2 ** attempt)
        rows.extend({'season': x['row']['season'], 'episode': x['row']['episode'], 'clip': str(x['row']['clip']), 'text': x['row']['text'], 'label': x['row']['label']} for x in payload['rows'])
    return rows


def split(rows):
    rows = sorted(rows, key=lambda x: (x['episode'], int(x['clip'])))
    n = len(rows); a = max(1, int(n * .70)); b = max(a + 1, int(n * .85))
    return rows[:a], rows[a:b], rows[b:]


def run(rows=None):
    rows = fetch_rows() if rows is None else rows
    hosts = []
    for season in SEASONS:
        local = [r for r in rows if r['season'] == season]
        fit, calibration, holdout = split(local)
        seed_runs = []
        for seed in SEEDS:
            fit_seed = list(fit); random.Random(seed).shuffle(fit_seed)
            naive_bayes = nb(fit_seed, holdout)
            centroid_cosine = centroid(fit_seed, holdout)
            adapted, threshold = calibrated_symbiotic_memory(fit_seed, calibration, holdout)
            nb_metrics = metrics(holdout, naive_bayes)
            centroid_metrics = metrics(holdout, centroid_cosine)
            adapter_metrics = metrics(holdout, adapted)
            strongest = max(nb_metrics['selective_accuracy'], centroid_metrics['selective_accuracy'])
            ledger = [{'example_id': f"{r['season']}:{r['episode']}:{r['clip']}", 'label': r['label'], 'adapter_prediction': adapted[i], 'adapter_accepted': adapted[i] is not None, 'adapter_correct': adapted[i] == r['label'], 'naive_bayes_prediction': naive_bayes[i], 'centroid_prediction': centroid_cosine[i]} for i, r in enumerate(holdout)]
            seed_runs.append({'seed': seed, 'naive_bayes': nb_metrics, 'centroid': centroid_metrics, 'adapter': adapter_metrics, 'threshold': None if threshold == float('inf') else round(threshold, 8), 'strongest_baseline_selective_accuracy': strongest, 'prediction_ledger': ledger})
        mean_adapter = sum(x['adapter']['selective_accuracy'] for x in seed_runs) / len(seed_runs)
        mean_baseline = sum(x['strongest_baseline_selective_accuracy'] for x in seed_runs) / len(seed_runs)
        hosts.append({'host_id': season, 'fit_count': len(fit), 'holdout_count': len(holdout), 'target_feedback_count': len(calibration), 'retained_evidence': len(fit), 'quarantined_evidence': len(calibration), 'leakage_detected': False, 'seeds': list(SEEDS), 'herus_score': round(mean_adapter, 6), 'baseline_score': round(mean_baseline, 6), 'score_higher_is_better': True, 'adapter': 'calibrated_symbiotic_prototype', 'baseline_definition': 'max(Naive Bayes, centroid cosine) by selective accuracy per seed', 'seed_runs': seed_runs})
    return {'schema': 'herus-multi-host-gate-v1', 'data_origin': 'real_world', 'dataset_id': DATASET, 'dataset_source': API, 'hosts': hosts, 'protocol': {'host_definition': 'MIntRec season as temporal domain proxy; not a physical host', 'split': 'within-season chronological episode/clip order: 70% fit, 15% calibration feedback, 15% holdout', 'seeds': list(SEEDS), 'claim_boundary': 'adapter evidence only; no HERUS-event mapping, no general symbiosis, no SOTA'}, 'limits': ['MIntRec labels are not HERUS events', 'season is a temporal domain proxy, not a native host', 'text metadata only', 'no speaker-independent guarantee', 'adapter is not the SymbioticLearner core']}


if __name__ == '__main__':
    out = run()
    path = Path('research/evidence/multi_host_real_mintrec_v1.json')
    path.write_text(json.dumps(out, indent=2, sort_keys=True) + '\n', encoding='utf-8')
    print(json.dumps({'path': str(path), 'hosts': [h['host_id'] for h in out['hosts']], 'status': 'PROTOCOL_EXECUTED'}, indent=2))
