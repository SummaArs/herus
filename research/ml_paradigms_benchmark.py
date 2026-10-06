"""Cross-paradigm benchmark on the real MIntRec temporal holdout.

This is deliberately explicit about task mismatch: unsupervised clustering and
reinforcement learning need a protocol adapter before they can be compared to
20-way supervised classification.
"""
from __future__ import annotations
import json
from collections import Counter, defaultdict
import numpy as np
from real_data_baseline_benchmark import fetch_split, metrics, tokens


def vocabulary(rows, limit=4000):
    counts = Counter(token for row in rows for token in tokens(row['text']))
    return {word: i for i, (word, _) in enumerate(counts.most_common(limit))}


def matrix(rows, vocab):
    x = np.zeros((len(rows), len(vocab)), dtype=float)
    for i, row in enumerate(rows):
        for word in tokens(row['text']):
            if word in vocab: x[i, vocab[word]] += 1.0
    norms = np.linalg.norm(x, axis=1, keepdims=True)
    return x / np.maximum(norms, 1e-12)


def kmeans(x, k, iterations=30):
    centers = x[np.linspace(0, len(x)-1, k, dtype=int)].copy()
    for _ in range(iterations):
        distances = ((x[:, None, :] - centers[None, :, :]) ** 2).sum(axis=2)
        labels = distances.argmin(axis=1)
        updated = centers.copy()
        for cluster in range(k):
            members = x[labels == cluster]
            if len(members): updated[cluster] = members.mean(axis=0)
        if np.allclose(updated, centers): break
        centers = updated
    return centers


def unsupervised_cluster(fit, calibration, test):
    vocab = vocabulary(fit); fit_x = matrix(fit, vocab); cal_x = matrix(calibration, vocab); test_x = matrix(test, vocab)
    centers = kmeans(fit_x, 20)
    def assign(x): return ((x[:, None, :] - centers[None, :, :]) ** 2).sum(axis=2).argmin(axis=1)
    cal_clusters = assign(cal_x); test_clusters = assign(test_x)
    labels = defaultdict(Counter)
    for cluster, row in zip(cal_clusters, calibration): labels[int(cluster)][row['label']] += 1
    predictions = [labels[int(cluster)].most_common(1)[0][0] if labels[int(cluster)] else None for cluster in test_clusters]
    return predictions


def contextual_bandit(fit, calibration, test, epsilon=0.0):
    """RL adapter: actions are intent labels and reward is exact classification.

    This is an online contextual-bandit proxy, not a claim that MIntRec is an
    embodied RL environment. It learns token->action rewards only from S04.
    """
    labels = sorted({row['label'] for row in fit})
    rewards = defaultdict(Counter)
    for row in fit:
        for word in set(tokens(row['text'])):
            rewards[word][row['label']] += 1 if row['label'] == max(labels, key=lambda label: rewards[word][label]) else 0
            rewards[word][row['label']] += 0
        # Direct context/action observations are retained as bounded bandit memory.
        key = tuple(sorted(set(tokens(row['text']))))
        rewards[key][row['label']] += 1
    out = []
    for row in test:
        key = tuple(sorted(set(tokens(row['text']))))
        if key in rewards: out.append(rewards[key].most_common(1)[0][0])
        else:
            candidates = Counter()
            for word in set(tokens(row['text'])): candidates.update(rewards[word])
            out.append(candidates.most_common(1)[0][0] if candidates else None)
    return out


def self_supervised_cooccurrence(fit, calibration, test, dimensions=16):
    """Pretrain token geometry without labels, then use frozen prototypes.

    The pretext task is local word-context cooccurrence on S04. Labels are
    used only by the prototype head; S05 is not used to alter the geometry.
    This is a small auditable self-supervised adapter, not a transformer.
    """
    vocab = vocabulary(fit, limit=1200)
    words = list(vocab); index = {word: i for i, word in enumerate(words)}
    co = np.zeros((len(words), len(words)), dtype=float)
    for row in fit:
        sequence = [word for word in tokens(row['text']) if word in index]
        for i, word in enumerate(sequence):
            for j in range(max(0, i - 2), min(len(sequence), i + 3)):
                if i != j: co[index[word], index[sequence[j]]] += 1.0
    if not len(words): return [None] * len(test)
    # PPMI makes the representation depend on unlabeled distributional
    # structure rather than the target labels.
    total = max(1.0, co.sum()); row_totals = np.maximum(co.sum(axis=1, keepdims=True), 1e-12); col_totals = np.maximum(co.sum(axis=0, keepdims=True), 1e-12)
    ppmi = np.maximum(np.log((co * total + 1e-12) / (row_totals * col_totals / total + 1e-12)), 0.0)
    _, singular, vt = np.linalg.svd(ppmi, full_matrices=False)
    embedding = vt[:min(dimensions, len(singular))].T * np.sqrt(singular[:min(dimensions, len(singular))])
    def encode(row):
        ids = [index[word] for word in tokens(row['text']) if word in index]
        return embedding[ids].mean(axis=0) if ids else np.zeros(embedding.shape[1])
    fit_vectors = [encode(row) for row in fit]; test_vectors = [encode(row) for row in test]
    prototypes = defaultdict(list)
    for row, vector_value in zip(fit, fit_vectors): prototypes[row['label']].append(vector_value)
    prototypes = {label: np.mean(values, axis=0) for label, values in prototypes.items()}
    return [max(prototypes, key=lambda label: float(np.dot(vector_value, prototypes[label]))) for vector_value in test_vectors]


def run():
    rows = fetch_split('train', 2224)
    fit = [r for r in rows if r['season'] == 'S04']; calibration = [r for r in rows if r['season'] == 'S05']; test = [r for r in rows if r['season'] == 'S06']
    result = {
        'dataset': {'id': 'THU-IAR/MIntRec', 'source': 'https://datasets-server.huggingface.co/rows', 'fit': len(fit), 'calibration': len(calibration), 'holdout': len(test), 'holdout_season': 'S06'},
        'methods': {
            'unsupervised_kmeans_cluster_then_calibrate': metrics(test, unsupervised_cluster(fit, calibration, test)),
            'reinforcement_contextual_bandit_proxy': metrics(test, contextual_bandit(fit, calibration, test)),
            'self_supervised_cooccurrence_frozen_prototype': metrics(test, self_supervised_cooccurrence(fit, calibration, test)),
        },
        'comparability': {
            'supervised': 'direct 20-way classification; Naive Bayes remains the reference supervised baseline',
            'unsupervised': 'clusters have no labels; S05 is used only to map clusters to labels',
            'self_supervised': 'not claimed: this text corpus has no independent augmentation/pretext protocol yet',
            'reinforcement': 'proxy only: labels become actions and exact-label reward; no physical state transition exists',
        },
        'limits': ['no MIntRec label is mapped to a HERUS event', 'text only; audio/video excluded', 'one temporal holdout', 'results do not establish superiority across ML'],
    }
    return result

if __name__ == '__main__': print(json.dumps(run(), indent=2, sort_keys=True))
