"""Evaluate lexical shift abstention on the real Banking77 stress tests."""
from __future__ import annotations
import json
from pathlib import Path
from adversarial_banking77 import mutate, predict, metrics
from policy_selection_banking77 import fetch, split_train, cents, calibrated
from real_data_baseline_benchmark import fit_nb
from shift_detector import LexicalShiftDetector


def run():
    raw = fetch('train'); test = fetch('test'); fit, cal = split_train(raw); cal = sorted(cal, key=lambda x: x['id'])
    cut = len(cal) // 2; tune, valid = cal[:cut], cal[cut:]
    model = fit_nb(fit); cs = cents(fit)
    detector = LexicalShiftDetector(quantile=.99); detector.fit([r['text'] for r in valid])
    clean = predict(valid, fit, valid, cs, model, 'calibrated')
    attacks = {}
    for kind in ('case_punctuation', 'typo', 'deletion', 'irrelevant_prefix'):
        attacked = [dict(r, text=mutate(r['text'], kind)) for r in test]
        raw_pred = predict(attacked, fit, valid, cs, model, 'calibrated')
        gated_pred = [p if detector.accept(r['text']) else None for r, p in zip(attacked, raw_pred)]
        m = metrics(test, attacked, predict(test, fit, valid, cs, model, 'calibrated'), gated_pred)
        m['detector_abstention_rate'] = round(sum(p is None for p in gated_pred) / len(gated_pred), 6)
        attacks[kind] = m
    out = {'schema':'herus-adversarial-shift-banking77-v1','dataset':'PolyAI-LDN/task-specific-datasets/banking_data','detector':{'name':'lexical_shift','quantile':.99,'calibration_examples':len(valid),'holdout_labels_used':False},'attacks':attacks,'protocol':{'claim_boundary':'stress-test only; no certified label-preserving robustness or OOD claim'}}
    Path('research/evidence/adversarial_shift_banking77_v1.json').write_text(json.dumps(out, indent=2, sort_keys=True)+'\n')
    return out

if __name__ == '__main__': print(json.dumps(run(), indent=2))
