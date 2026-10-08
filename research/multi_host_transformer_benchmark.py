"""Paired small-transformer benchmark on the real multi-host temporal protocol."""
from __future__ import annotations
import json, random, time
from pathlib import Path
import numpy as np
import torch
from torch.utils.data import DataLoader, Dataset
from transformers import AutoTokenizer, AutoModelForSequenceClassification
from real_data_baseline_benchmark import fetch_split, metrics
from multi_host_real_benchmark import SEASONS, SEEDS, split

MODEL = 'google/bert_uncased_L-2_H-128_A-2'
EPOCHS = 5
BATCH = 16
MAX_LENGTH = 96
LR = 5e-5


def seed_all(seed):
    random.seed(seed); np.random.seed(seed); torch.manual_seed(seed)
    torch.use_deterministic_algorithms(True, warn_only=True)


class TextDataset(Dataset):
    def __init__(self, rows, tokenizer, label_to_id):
        self.rows = rows; self.labels = label_to_id
        self.enc = tokenizer([r['text'] for r in rows], truncation=True, padding=True, max_length=MAX_LENGTH, return_tensors='pt')
    def __len__(self): return len(self.rows)
    def __getitem__(self, i):
        item = {k: v[i] for k, v in self.enc.items()}
        item['labels'] = torch.tensor(self.labels[self.rows[i]['label']], dtype=torch.long)
        return item


def predict(model, loader, id_to_label, with_confidence=False):
    model.eval(); out = []
    with torch.no_grad():
        for batch in loader:
            batch = {k: v for k, v in batch.items()}
            probs = model(**batch).logits.softmax(-1)
            values, indices = probs.max(1)
            out.extend(zip(indices.cpu().tolist(), values.cpu().tolist()))
    if with_confidence:
        return [(id_to_label[i], float(confidence)) for i, confidence in out]
    return [id_to_label[i] for i, _ in out]


def one_host(rows, seed):
    seed_all(seed); fit, calibration, holdout = split(rows)
    labels = sorted({r['label'] for r in rows}); l2i = {x: i for i, x in enumerate(labels)}; i2l = {i: x for x, i in l2i.items()}
    tokenizer = AutoTokenizer.from_pretrained(MODEL, use_fast=False)
    model = AutoModelForSequenceClassification.from_pretrained(MODEL, num_labels=len(labels), ignore_mismatched_sizes=True)
    train = DataLoader(TextDataset(fit, tokenizer, l2i), batch_size=BATCH, shuffle=True, generator=torch.Generator().manual_seed(seed))
    cal = DataLoader(TextDataset(calibration, tokenizer, l2i), batch_size=BATCH)
    hold = DataLoader(TextDataset(holdout, tokenizer, l2i), batch_size=BATCH)
    opt = torch.optim.AdamW(model.parameters(), lr=LR); best = -1.0; best_state = None; history = []
    for epoch in range(EPOCHS):
        model.train(); losses = []
        for batch in train:
            out = model(**batch); out.loss.backward(); opt.step(); opt.zero_grad(); losses.append(float(out.loss.detach()))
        cm = metrics(calibration, predict(model, cal, i2l)); history.append({'epoch': epoch + 1, 'loss': round(sum(losses) / len(losses), 6), 'calibration': cm})
        if cm['accuracy'] > best:
            best = cm['accuracy']; best_state = {k: v.detach().cpu().clone() for k, v in model.state_dict().items()}
    model.load_state_dict(best_state)
    cal_scored = predict(model, cal, i2l, with_confidence=True)
    thresholds = sorted({score for _, score in cal_scored})
    best_threshold = None; best_coverage = -1.0
    for threshold in thresholds:
        selected = [label if confidence >= threshold else None for label, confidence in cal_scored]
        cm = metrics(calibration, selected)
        if cm['coverage'] and cm['selective_accuracy'] >= 0.80 and cm['coverage'] > best_coverage:
            best_threshold, best_coverage = threshold, cm['coverage']
    hold_scored = predict(model, hold, i2l, with_confidence=True)
    predictions = [label for label, _ in hold_scored]
    selective = [label if best_threshold is not None and confidence >= best_threshold else None for label, confidence in hold_scored]
    return {'seed': seed, 'metrics': metrics(holdout, predictions), 'selective_metrics': metrics(holdout, selective), 'confidence_threshold': None if best_threshold is None else round(best_threshold, 8), 'fit_count': len(fit), 'calibration_count': len(calibration), 'holdout_count': len(holdout), 'history': history}


def run():
    rows = fetch_split('train', 2224)
    result = {'schema': 'herus-multi-host-transformer-v1', 'dataset_id': 'THU-IAR/MIntRec', 'model': MODEL, 'protocol': {'split': 'same within-season 70/15/15 protocol as adapter', 'seeds': list(SEEDS), 'epochs': EPOCHS, 'batch_size': BATCH, 'max_length': MAX_LENGTH, 'learning_rate': LR, 'claim_boundary': 'paired transformer evidence only; no SOTA claim'}, 'hosts': []}
    for season in SEASONS:
        host_rows = [r for r in rows if r['season'] == season]
        runs = [one_host(host_rows, seed) for seed in SEEDS]
        result['hosts'].append({'host_id': season, 'runs': runs, 'mean_selective_accuracy': round(sum(r['metrics']['selective_accuracy'] for r in runs) / len(runs), 6), 'mean_coverage': round(sum(r['metrics']['coverage'] for r in runs) / len(runs), 6), 'mean_calibrated_selective_accuracy': round(sum(r['selective_metrics']['selective_accuracy'] for r in runs) / len(runs), 6), 'mean_calibrated_coverage': round(sum(r['selective_metrics']['coverage'] for r in runs) / len(runs), 6)})
    return result


if __name__ == '__main__':
    started = time.perf_counter(); out = run(); out['runtime_seconds'] = round(time.perf_counter() - started, 3)
    path = Path('research/evidence/multi_host_transformer_mintrec_v1.json'); path.write_text(json.dumps(out, indent=2, sort_keys=True) + '\n', encoding='utf-8')
    print(json.dumps({'path': str(path), 'hosts': [h['host_id'] for h in out['hosts']], 'runtime_seconds': out['runtime_seconds']}, indent=2))
