"""Selective prediction: calibrate SVM margin thresholds on S05 only."""
import json
from sklearn.feature_extraction.text import TfidfVectorizer
from sklearn.svm import LinearSVC
from real_data_baseline_benchmark import fetch_split

def scores(model, vec, rows):
    raw = model.decision_function(vec.transform([r['text'] for r in rows]))
    return [(model.classes_[int(row.argmax())], float(sorted(row)[-1] - sorted(row)[-2])) for row in raw]

def select_threshold(rows, scored, target):
    candidates = sorted({margin for _, margin in scored})
    chosen = float('inf')
    best = (-1.0, -1.0)
    for threshold in candidates:
        accepted = [(row['label'], label, margin) for row, (label, margin) in zip(rows, scored) if margin >= threshold]
        if not accepted:
            continue
        precision = sum(true_label == label for true_label, label, _ in accepted) / len(accepted)
        coverage = len(accepted) / len(rows)
        if precision >= target and (coverage, precision) > best:
            chosen = threshold
            best = (coverage, precision)
    return chosen

def evaluate(rows, predicted, threshold):
    accepted = [(row, label) for row, (label, margin) in zip(rows, predicted) if margin >= threshold]
    if not accepted:
        return {'coverage': 0.0, 'selective_accuracy': 0.0, 'risk': 1.0, 'accepted': 0}
    precision = sum(row['label'] == label for row, label in accepted) / len(accepted)
    coverage = len(accepted) / len(rows)
    return {'coverage': round(coverage, 6), 'selective_accuracy': round(precision, 6), 'risk': round(1 - precision, 6), 'accepted': len(accepted)}

def run():
    rows = fetch_split('train', 2224)
    s04 = [r for r in rows if r['season'] == 'S04']
    s05 = [r for r in rows if r['season'] == 'S05']
    s06 = [r for r in rows if r['season'] == 'S06']
    vec = TfidfVectorizer(ngram_range=(1, 2), sublinear_tf=True)
    x = vec.fit_transform([r['text'] for r in s04])
    model = LinearSVC(C=1.0)
    model.fit(x, [r['label'] for r in s04])
    cal = scores(model, vec, s05)
    thresholds = {str(t): select_threshold(s05, cal, t) for t in (0.70, 0.80, 0.90, 0.95)}
    refit_vec = TfidfVectorizer(ngram_range=(1, 2), sublinear_tf=True)
    rx = refit_vec.fit_transform([r['text'] for r in s04 + s05])
    refit = LinearSVC(C=1.0)
    refit.fit(rx, [r['label'] for r in s04 + s05])
    hold = scores(refit, refit_vec, s06)
    return {'dataset': 'THU-IAR/MIntRec', 'fit_split': 'S04', 'calibration_split': 'S05', 'refit_splits': ['S04', 'S05'], 'holdout_split': 'S06', 'calibration_rows': len(s05), 'holdout_rows': len(s06), 'thresholds': thresholds, 'calibration': {k: evaluate(s05, cal, v) for k, v in thresholds.items()}, 'holdout': {k: evaluate(s06, hold, v) for k, v in thresholds.items()}, 'math': {'margin': 'top_decision_score - second_decision_score', 'coverage': 'accepted / total', 'risk': '1 - selective_accuracy', 'threshold_selection': 'maximize calibration coverage subject to selective_accuracy >= target'}}

if __name__ == '__main__':
    print(json.dumps(run(), indent=2, sort_keys=True))
