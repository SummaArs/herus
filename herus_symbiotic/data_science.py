"""Proposal-only data science and ML engineering skill for HERUS."""
from __future__ import annotations
from dataclasses import dataclass
from typing import Any, Mapping, Tuple
from collections import Counter

@dataclass(frozen=True)
class DataIssue:
    code: str
    severity: str
    detail: str

@dataclass(frozen=True)
class DatasetProfile:
    rows: int
    columns: Tuple[str, ...]
    missing: Tuple[Tuple[str, int], ...]
    duplicates: int
    label_column: str
    label_counts: Tuple[Tuple[str, int], ...]
    issues: Tuple[DataIssue, ...]

@dataclass(frozen=True)
class MLPlan:
    status: str
    objective: str
    profile: DatasetProfile | None
    protocol: Tuple[str, ...]
    baselines: Tuple[str, ...]
    metrics: Tuple[str, ...]
    risks: Tuple[str, ...]
    next_tests: Tuple[str, ...]
    authority: str = "none"

class DataScienceSkill:
    """Deterministic senior-analyst planner; never trains or executes by itself."""
    def analyze(self, records: Tuple[Mapping[str, Any], ...], label_column: str = "label", objective: str = "") -> MLPlan:
        if not records:
            return MLPlan("ABSTAIN", objective, None, (), (), (), ("empty dataset",), ("provide non-empty records",), authority="none")
        columns = tuple(sorted({str(k) for r in records for k in r}))
        missing = tuple((c, sum(c not in r or r.get(c) in (None, "") for r in records)) for c in columns if any(c not in r or r.get(c) in (None, "") for r in records))
        signatures = [tuple(sorted((str(k), repr(v)) for k, v in r.items())) for r in records]
        duplicates = len(signatures) - len(set(signatures))
        labels = Counter(str(r.get(label_column)) for r in records if label_column in r)
        issues = []
        if label_column not in columns:
            issues.append(DataIssue("MISSING_LABEL", "blocking", f"label column '{label_column}' is absent"))
        elif len(labels) < 2:
            issues.append(DataIssue("SINGLE_CLASS", "blocking", "at least two labels are required"))
        if duplicates:
            issues.append(DataIssue("DUPLICATES", "high", f"{duplicates} duplicated rows detected"))
        if missing:
            issues.append(DataIssue("MISSING_VALUES", "medium", "missing values require an explicit policy"))
        if labels and max(labels.values()) / sum(labels.values()) >= 0.8:
            issues.append(DataIssue("CLASS_IMBALANCE", "high", "majority class exceeds 80%"))
        profile = DatasetProfile(len(records), columns, missing, duplicates, label_column, tuple(sorted(labels.items())), tuple(issues))
        blocking = any(i.severity == "blocking" for i in issues)
        protocol = ("freeze schema and provenance", "deduplicate without touching holdout", "split by time or entity when available", "fit preprocessing on train only", "calibrate confidence on validation only", "evaluate once on untouched holdout")
        baselines = ("majority classifier", "TF-IDF + logistic regression", "TF-IDF + linear SVM", "small tree/nearest-neighbor baseline", "abstention baseline")
        metrics = ("accuracy", "macro-F1", "per-class precision/recall", "coverage-risk curve", "latency and memory", "seed and confidence interval")
        risks = tuple(i.detail for i in issues) + ("random split may leak entities or time", "accuracy alone can hide minority-class failure", "benchmark selection can overfit the holdout")
        tests = ("schema and type validation", "duplicate and near-duplicate audit", "temporal/entity leakage probe", "baseline comparison on real data", "adversarial and out-of-distribution holdout", "reproducibility and resource measurement")
        return MLPlan("ABSTAIN" if blocking else "PROPOSE", objective, profile, protocol, baselines, metrics, risks, tests)
