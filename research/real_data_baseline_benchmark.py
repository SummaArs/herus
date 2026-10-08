"""Real-data benchmark on MIntRec text metadata.

The benchmark compares algorithms on the source task (20-way intent
classification) without pretending that MIntRec labels are HERUS events.
The Symbiotic reference memory is reported as a finite contextual memory
baseline, not as proof of open-language reasoning.
"""
from __future__ import annotations
import json, math, re, time, urllib.error, urllib.parse, urllib.request
from collections import Counter, defaultdict
from pathlib import Path

API = 'https://datasets-server.huggingface.co/rows'
DATASET = 'THU-IAR/MIntRec'

def fetch_split(split: str, total: int, page: int = 100) -> list[dict[str, str]]:
    rows = []
    for offset in range(0, total, page):
        query = urllib.parse.urlencode({'dataset': DATASET, 'config': 'default', 'split': split, 'offset': offset, 'length': min(page, total-offset)})
        for attempt in range(5):
            try:
                with urllib.request.urlopen(API + '?' + query, timeout=60) as response:
                    payload = json.load(response)
                break
            except (urllib.error.HTTPError, urllib.error.URLError, TimeoutError):
                if attempt == 4:
                    raise
                time.sleep(2 ** attempt)
        rows.extend({'text': item['row']['text'], 'label': item['row']['label'], 'season': item['row']['season'], 'episode': item['row']['episode'], 'clip': str(item['row']['clip'])} for item in payload['rows'])
    return rows

def tokens(text: str) -> list[str]:
    return re.findall(r"[a-z]+", text.lower())

def vector(text: str) -> Counter[str]:
    return Counter(tokens(text))

def majority(train, test):
    label = Counter(row['label'] for row in train).most_common(1)[0][0]
    return [label] * len(test)

def nb(train, test):
    model = fit_nb(train)
    return [model.score(row)[0] for row in test]

def cosine(left: Counter[str], right: Counter[str]) -> float:
    dot = sum(value * right.get(token, 0) for token, value in left.items())
    norm_left = math.sqrt(sum(v*v for v in left.values())); norm_right = math.sqrt(sum(v*v for v in right.values()))
    return dot / (norm_left * norm_right) if norm_left and norm_right else 0.0

def knn(train, test):
    indexed = [(vector(row['text']), row['label']) for row in train]
    out = []
    for row in test:
        query = vector(row['text'])
        best = max(indexed, key=lambda item: (cosine(query, item[0]), item[1]))
        out.append(best[1])
    return out

def centroid(train, test):
    sums = defaultdict(Counter); counts = Counter()
    for row in train:
        sums[row['label']].update(vector(row['text'])); counts[row['label']] += 1
    centroids = {label: Counter({token: value / counts[label] for token, value in words.items()}) for label, words in sums.items()}
    return [max(centroids, key=lambda label: cosine(vector(row['text']), centroids[label])) for row in test]

def contextual_memory(train, test):
    """Finite reference memory: exact two-token contexts, abstains otherwise."""
    memory = defaultdict(Counter)
    for row in train:
        key = tuple(sorted(set(tokens(row['text']))))
        memory[key][row['label']] += 1
    predictions = []
    for row in test:
        key = tuple(sorted(set(tokens(row['text']))))
        predictions.append(memory[key].most_common(1)[0][0] if key in memory else None)
    return predictions

class MultinomialNBModel:
    """Cached text model; fitting is separated from per-row scoring."""
    def __init__(self, train):
        self.labels = sorted({item['label'] for item in train}); self.docs = Counter(item['label'] for item in train)
        self.words = {label: Counter() for label in self.labels}; self.totals = Counter(); vocabulary = set()
        for item in train:
            counts = vector(item['text']); self.words[item['label']].update(counts)
            self.totals[item['label']] += sum(counts.values()); vocabulary.update(counts)
        self.v = max(1, len(vocabulary)); self.n = len(train)
    def score(self, row):
        counts = vector(row['text']); scores = {}
        for label in self.labels:
            score = math.log((self.docs[label] + 1) / (self.n + len(self.labels))); denom = self.totals[label] + self.v
            scores[label] = score + sum(c * math.log((self.words[label][token] + 1) / denom) for token, c in counts.items())
        ordered = sorted(scores.items(), key=lambda pair: pair[1], reverse=True)
        return ordered[0][0], ordered[0][1] - ordered[1][1]

def fit_nb(train):
    return MultinomialNBModel(train)

def _nb_scores(train, row):
    return fit_nb(train).score(row)

def calibrated_symbiotic_memory(fit, calibration, test, minimum_precision=0.80):
    """Use a confidence margin calibrated without seeing the final holdout."""
    calibration_scores = [_nb_scores(fit, row) for row in calibration]
    candidates = sorted({margin for _, margin in calibration_scores})
    accepted_threshold = float('inf'); best_coverage = -1.0
    for threshold in candidates:
        predictions = [(label if margin >= threshold else None) for (label, margin) in calibration_scores]
        result = metrics(calibration, predictions)
        if result['coverage'] and result['selective_accuracy'] >= minimum_precision and result['coverage'] > best_coverage:
            accepted_threshold = threshold
            best_coverage = result['coverage']
    test_scores = [_nb_scores(fit, row) for row in test]
    return [(label if margin >= accepted_threshold else None) for label, margin in test_scores], accepted_threshold

def metrics(test, predictions):
    labels = sorted({row['label'] for row in test})
    correct = sum(row['label'] == pred for row, pred in zip(test, predictions) if pred is not None)
    covered = sum(pred is not None for pred in predictions)
    accuracy = correct / len(test)
    selective_accuracy = correct / covered if covered else 0.0
    f1s = []
    for label in labels:
        tp = sum(row['label'] == label and pred == label for row, pred in zip(test, predictions))
        fp = sum(row['label'] != label and pred == label for row, pred in zip(test, predictions))
        fn = sum(row['label'] == label and pred != label for row, pred in zip(test, predictions))
        precision = tp / (tp + fp) if tp + fp else 0.0; recall = tp / (tp + fn) if tp + fn else 0.0
        f1s.append(2 * precision * recall / (precision + recall) if precision + recall else 0.0)
    return {'accuracy': round(accuracy, 6), 'macro_f1': round(sum(f1s)/len(f1s), 6), 'coverage': round(covered/len(test), 6), 'selective_accuracy': round(selective_accuracy, 6)}

def run():
    all_rows = fetch_split('train', 2224)
    # The public HF mirror exposes one split. Use a chronological, episode-level
    # holdout rather than inventing random train/test labels: S06 is held out,
    # while S04/S05 are used for fitting. This prevents utterance leakage.
    train = [row for row in all_rows if row['season'] in {'S04', 'S05'}]
    test = [row for row in all_rows if row['season'] == 'S06']
    fit = [row for row in all_rows if row['season'] == 'S04']; calibration = [row for row in all_rows if row['season'] == 'S05']
    calibrated, threshold = calibrated_symbiotic_memory(fit, calibration, test)
    methods = {'majority': majority(train, test), 'multinomial_naive_bayes': nb(train, test), '1nn_cosine': knn(train, test), 'centroid_cosine': centroid(train, test), 'symbiotic_finite_context_memory': contextual_memory(train, test), 'symbiotic_calibrated_prototype': calibrated}
    result = {'dataset': {'id': DATASET, 'source': API, 'published_split': 'train', 'train_rows': len(train), 'test_rows': len(test), 'train_seasons': ['S04', 'S05'], 'calibration_season': 'S05', 'fit_season_for_calibration': 'S04', 'holdout_seasons': ['S06'], 'labels': len(set(row['label'] for row in train + test))}, 'calibration': {'minimum_precision': 0.80, 'margin_threshold': round(threshold, 6)}, 'task': 'MIntRec 20-way text intent classification; no HERUS label mapping', 'methods': {name: metrics(test, predictions) for name, predictions in methods.items()}, 'limits': ['MIntRec labels are not HERUS events', 'text metadata only; audio and video are excluded', 'the current SymbioticLearner is proposal/effect oriented, so calibrated prototype is an adapter baseline', 'chronological season holdout is not the official paper split', 'no claim of superiority is made from this single corpus']}
    return result

if __name__ == '__main__':
    print(json.dumps(run(), indent=2, sort_keys=True))
