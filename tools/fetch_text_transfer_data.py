from __future__ import annotations

import hashlib
import pathlib
import sys
import urllib.request

ROOT = pathlib.Path(__file__).resolve().parents[1]
OUT = ROOT / "research" / "datasets" / "text_transfer" / "wikitext-2-raw-v1-train.parquet"
URL = "https://huggingface.co/datasets/Salesforce/wikitext/resolve/main/wikitext-2-raw-v1/train-00000-of-00001.parquet?download=true"
# Recompute if the upstream artifact changes; never silently accept a replacement.
EXPECTED_SHA256 = "e83889baabc497075506f91975be5fac0d45c5290b6b20582c8cd1e853d0c9f7"


def digest(path: pathlib.Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def main() -> int:
    OUT.parent.mkdir(parents=True, exist_ok=True)
    print(f"downloading {URL}")
    urllib.request.urlretrieve(URL, OUT)
    actual = digest(OUT)
    if actual != EXPECTED_SHA256:
        OUT.unlink(missing_ok=True)
        print(f"checksum mismatch: expected {EXPECTED_SHA256}, got {actual}", file=sys.stderr)
        return 1
    if OUT.read_bytes()[:4] != b"PAR1":
        OUT.unlink(missing_ok=True)
        print("format mismatch: expected Parquet", file=sys.stderr)
        return 1
    print(f"verified {OUT} sha256={actual}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
