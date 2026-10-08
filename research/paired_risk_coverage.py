"""Paired bootstrap over archived real multi-host prediction ledgers."""
from __future__ import annotations
import json, random
from pathlib import Path

E = Path(__file__).parent / 'evidence'
ITERATIONS = 4000
SEED = 17


def bootstrap(ledger, iterations=ITERATIONS, seed=SEED):
    rng = random.Random(seed); n = len(ledger); raw = []; selective = []
    for _ in range(iterations):
        sample = [ledger[rng.randrange(n)] for _ in range(n)]
        h_raw = sum(x['herus_correct'] for x in sample) / n
        t_raw = sum(x['transformer_correct'] for x in sample) / n
        h_acc = [x for x in sample if x['herus_accepted']]
        t_acc = [x for x in sample if x['transformer_accepted']]
        h_sel = sum(x['herus_correct'] for x in h_acc) / len(h_acc) if h_acc else 0.0
        t_sel = sum(x['transformer_selective_correct'] for x in t_acc) / len(t_acc) if t_acc else 0.0
        raw.append(h_raw - t_raw); selective.append(h_sel - t_sel)
    raw.sort(); selective.sort()
    return {
        'raw_accuracy': {'delta_herus_minus_transformer': round(sum(raw) / len(raw), 6), 'ci95_low': round(raw[int(.025 * iterations)], 6), 'ci95_high': round(raw[int(.975 * iterations) - 1], 6)},
        'selective_precision': {'delta_herus_minus_transformer': round(sum(selective) / len(selective), 6), 'ci95_low': round(selective[int(.025 * iterations)], 6), 'ci95_high': round(selective[int(.975 * iterations) - 1], 6)},
        'iterations': iterations, 'seed': seed, 'examples': n,
    }


def run():
    herus = json.loads((E / 'multi_host_real_mintrec_v1.json').read_text())
    transformer = json.loads((E / 'multi_host_transformer_mintrec_v1.json').read_text())
    hosts = []
    for hh, tt in zip(herus['hosts'], transformer['hosts']):
        hrun = next(x for x in hh['seed_runs'] if x['seed'] == 11)
        trun = next(x for x in tt['runs'] if x['seed'] == 11)
        hm = {x['example_id']: x for x in hrun['prediction_ledger']}; tm = {x['example_id']: x for x in trun['prediction_ledger']}
        ids = sorted(set(hm) & set(tm))
        ledger = [{'example_id': i, 'label': hm[i]['label'], 'herus_correct': hm[i]['adapter_correct'], 'herus_accepted': hm[i]['adapter_accepted'], 'transformer_correct': tm[i]['transformer_correct'], 'transformer_accepted': tm[i]['transformer_accepted'], 'transformer_selective_correct': tm[i]['transformer_selective_correct']} for i in ids]
        hosts.append({'host_id': hh['host_id'], 'ledger_count': len(ledger), 'comparison': bootstrap(ledger), 'ledger': ledger})
    return {'schema': 'herus-paired-risk-coverage-v1', 'dataset_id': 'THU-IAR/MIntRec', 'protocol': {'paired_seed': 11, 'bootstrap_iterations': ITERATIONS, 'bootstrap_seed': SEED, 'claim_boundary': 'paired MIntRec result only; independent dataset and SOTA gate remain pending'}, 'hosts': hosts, 'independent_dataset_evidence': False, 'limits': ['one corpus', 'season is a temporal proxy, not a native host', 'adapter rather than HERUS core', 'selective metric is coverage-dependent']}


if __name__ == '__main__':
    out = run(); path = E / 'paired_risk_coverage_mintrec_v1.json'; path.write_text(json.dumps(out, indent=2, sort_keys=True) + '\n'); print(json.dumps({'path': str(path), 'hosts': [h['host_id'] for h in out['hosts']]}, indent=2))
