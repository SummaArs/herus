"""Selective consensus on real MIntRec.

The model is fitted only on S04. S05 chooses the margin threshold. S06 is
scored once, with no calibration labels entering the scorer.
"""
from __future__ import annotations
import json
from collections import Counter, defaultdict
from real_data_baseline_benchmark import fetch_split, metrics, _nb_scores, vector, cosine


def centroid_model(fit):
    sums = defaultdict(Counter); counts = Counter()
    for row in fit:
        sums[row['label']].update(vector(row['text'])); counts[row['label']] += 1
    return {label: Counter({token: value / counts[label] for token, value in words.items()}) for label, words in sums.items()}


def consensus_score(row, centroids, fit):
    nb_label, nb_margin = _nb_scores(fit, row)
    query = vector(row['text'])
    ranked = sorted(((cosine(query, centroid), label) for label, centroid in centroids.items()), reverse=True)
    centroid_label = ranked[0][1]
    return nb_label, centroid_label, nb_margin


def choose_threshold(fit, calibration, minimum_precision=0.50):
    centroids = centroid_model(fit)
    scores = [consensus_score(row, centroids, fit) for row in calibration]
    thresholds = sorted({score[2] for score in scores})
    best = None
    for threshold in thresholds:
        predictions = [nb if nb == centroid and margin >= threshold else None for nb, centroid, margin in scores]
        result = metrics(calibration, predictions)
        key = (result['coverage'], result['selective_accuracy'], -threshold)
        if result['coverage'] and result['selective_accuracy'] >= minimum_precision and (best is None or key > best[0]):
            best = (key, threshold, result)
    return None if best is None else (centroids, best[1], best[2])


def run():
    rows = fetch_split('train', 2224)
    fit = [row for row in rows if row['season'] == 'S04']
    calibration = [row for row in rows if row['season'] == 'S05']
    holdout = [row for row in rows if row['season'] == 'S06']
    selected = choose_threshold(fit, calibration, 0.50)
    if selected is None:
        return {'status': 'NO_CALIBRATION_OPERATING_POINT'}
    centroids, threshold, calibration_result = selected
    scores = [consensus_score(row, centroids, fit) for row in holdout]
    predictions = [nb if nb == centroid and margin >= threshold else None for nb, centroid, margin in scores]
    return {'dataset': {'id': 'THU-IAR/MIntRec', 'source': 'https://datasets-server.huggingface.co/rows', 'fit_season': 'S04', 'calibration_season': 'S05', 'holdout_season': 'S06', 'fit_rows': len(fit), 'calibration_rows': len(calibration), 'holdout_rows': len(holdout)}, 'policy': 'accept only when Naive Bayes and centroid agree and NB margin clears threshold', 'minimum_precision': 0.50, 'threshold': round(threshold, 6), 'calibration': calibration_result, 'holdout': metrics(holdout, predictions), 'limits': ['text-only MIntRec is not a HERUS event benchmark', 'single temporal holdout', 'consensus improves coverage only marginally and is not general superiority']}

if __name__ == '__main__':
    print(json.dumps(run(), indent=2, sort_keys=True))
