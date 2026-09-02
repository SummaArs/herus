from __future__ import annotations

from collections import Counter
from pathlib import Path
import json
import os

from sygus_compatibility import inspect_file
from sygus_lia_subset import synthesize_bounded


def main() -> None:
    root = Path(os.environ.get("SYGUS_ROOT", "/tmp/sygus-official-benchmarks"))
    if not root.is_dir():
        raise SystemExit(f"missing SyGuS root: {root}")
    rows = []
    for path in sorted(root.rglob("*.sl")):
        inventory_row = inspect_file(path)
        if not inventory_row.supported:
            continue
        result = synthesize_bounded(path.read_text(encoding="utf-8"), lower=-2, upper=2, max_depth=2, max_candidates=500)
        rows.append({"path": str(path.relative_to(root)), "status": result.status, "reason": result.reason, "checked_points": result.checked_points, "expression": result.expression})
    counts = Counter(row["status"] for row in rows)
    reason_counts = Counter(row["reason"] for row in rows)
    report = {"corpus": "SyGuS-Org/benchmarks", "files": sum(1 for _ in root.rglob("*.sl")), "lexically_supported": len(rows), "semantic_status": dict(sorted(counts.items())), "top_reasons": dict(reason_counts.most_common(12)), "domain": [-2, 2], "max_depth": 2, "max_candidates": 500, "rows": rows}
    output = Path(__file__).resolve().parent / "evidence" / "sygus_subset_benchmark.json"
    output.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(json.dumps({k: report[k] for k in ("files", "lexically_supported", "semantic_status", "top_reasons", "domain", "max_depth", "max_candidates")}, sort_keys=True))


if __name__ == "__main__":
    main()
