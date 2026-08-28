"""Measure bounded generation growth for the fixed formal demo signature."""
from __future__ import annotations

import json
import time

from .benchmark import demo_signature
from .generator import generate
from .core import Budget


def run() -> dict[str, object]:
    signature = demo_signature()
    rows: list[dict[str, object]] = []
    for depth in range(0, 7):
        start = time.perf_counter()
        result = generate(signature, Budget(max_depth=depth, max_nodes=128, max_steps=256, max_terms=512))
        elapsed_ms = (time.perf_counter() - start) * 1000.0
        rows.append(
            {
                "max_depth": depth,
                "status": result.status.value,
                "terms": len(result.terms),
                "terms_by_depth": list(result.by_depth),
                "elapsed_ms": round(elapsed_ms, 3),
            }
        )
    return {"schema": "herus.generative_lab.measurement", "version": 1, "authority": "none", "rows": rows}


if __name__ == "__main__":
    print(json.dumps(run(), indent=2, sort_keys=True))
