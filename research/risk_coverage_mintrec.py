"""Risk-coverage analysis on the real MIntRec temporal holdout.

Thresholds are selected on S05 only; S06 is evaluated once as an untouched
holdout. This makes the current low-coverage result auditable instead of
hiding it behind one operating point.
"""
from __future__ import annotations
import json
from real_data_baseline_benchmark import fetch_split, _nb_scores, metrics

TARGETS = (0.50, 0.60, 0.70, 0.80, 0.90)

def select_threshold(fit, calibration, minimum_precision):
    scores = [_nb_scores(fit, row) for row in calibration]
    candidates = sorted({margin for _, margin in scores})
    best = None
    for threshold in candidates:
        predictions = [label if margin >= threshold else None for label, margin in scores]
        result = metrics(calibration, predictions)
        if result['coverage'] and result['selective_accuracy'] >= minimum_precision:
            key = (result['coverage'], result['selective_accuracy'], -threshold)
            if best is None or key > best[0]:
                best = (key, threshold, result)
    return None if best is None else {'threshold': round(best[1], 6), 'calibration': best[2]}

def run():
    rows = fetch_split('train', 2224)
    fit = [r for r in rows if r['season'] == 'S04']
    calibration = [r for r in rows if r['season'] == 'S05']
    holdout = [r for r in rows if r['season'] == 'S06']
    holdout_scores = [_nb_scores(fit + calibration, row) for row in holdout]
    points = []
    for target in TARGETS:
        selected = select_threshold(fit, calibration, target)
        if selected is None:
            points.append({'minimum_precision': target, 'status': 'NO_CALIBRATION_OPERATING_POINT'})
            continue
        predictions = [label if margin >= selected['threshold'] else None for label, margin in holdout_scores]
        points.append({'minimum_precision': target, 'status': 'EVALUATED', 'threshold': selected['threshold'], 'calibration': selected['calibration'], 'holdout': metrics(holdout, predictions)})
    return {'dataset': {'id': 'THU-IAR/MIntRec', 'source': 'https://datasets-server.huggingface.co/rows', 'fit_season': 'S04', 'calibration_season': 'S05', 'holdout_season': 'S06', 'fit_rows': len(fit), 'calibration_rows': len(calibration), 'holdout_rows': len(holdout)}, 'selection_rule': 'maximize calibration coverage subject to minimum selective precision', 'points': points, 'limits': ['text-only intent benchmark is not a HERUS event benchmark', 'no threshold was selected using S06', 'single temporal holdout; repeat across datasets is required']}

if __name__ == '__main__':
    print(json.dumps(run(), indent=2, sort_keys=True))
