from __future__ import annotations
import json
from pathlib import Path
from internet_fetcher import FetchPolicy, fetch_many

URLS = (
    "https://wiki.lilygo.cc/products/t3-series/t3-s3-v1.3/",
    "https://documentation.espressif.com/esp32-s3_datasheet_en.pdf",
)

if __name__ == "__main__":
    records = fetch_many(URLS, FetchPolicy(frozenset({"wiki.lilygo.cc", "documentation.espressif.com"}), max_bytes_per_url=2_000_000))
    result = {
        "schema": "herus.internet-live-probe.v1",
        "retrieved": [
            {"url": r.source_url, "title": r.title, "content_chars": len(r.content), "source_digest": r.source_digest, "claims": list(r.claims)}
            for r in records
        ],
        "execution": False,
        "authority": "NONE",
    }
    Path(__file__).parent.joinpath("evidence", "internet_live_probe.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(result, indent=2))
