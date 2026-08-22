from __future__ import annotations

import hashlib
import json
import pathlib
import re
from typing import Iterable

import pyarrow.parquet as pq

ROOT = pathlib.Path(__file__).resolve().parents[1]
DATA = ROOT / "research" / "datasets" / "text_transfer"
HERUS_DOCS = [
    ROOT / "docs" / "47-HERUS-INDISPENSAVEL-E-INTELIGENCIA-PROPRIA.md",
    ROOT / "docs" / "50-HERUS-NUCLEO-GENERATIVO-SIMBOLICO.md",
    ROOT / "docs" / "60-HERUS-AUDITORIA-INTELIGENCIA-STATE-OF-ART.md",
    ROOT / "docs" / "80-HERUS-INTELIGENCIA-INVISIVEL-E-CRITERIO-DE-EXCELENCIA.md",
    ROOT / "docs" / "87-HERUS-INVESTIGACAO-ACADEMICA-PERGUNTA-E-MATRIZ.md",
    ROOT / "docs" / "88-HERUS-AGSC-CONTINUIDADE-SEMANTICA-GOVERNADA.md",
    ROOT / "docs" / "89-HERUS-AGSC-D-MUDANCA-REVOGACAO-E-ESQUECIMENTO.md",
    ROOT / "docs" / "90-HERUS-POISONING-L1-L2-L3-E-GUARD-DE-COMPOSICAO.md",
    ROOT / "docs" / "91-HERUS-ATTRIBUTION-GUARD-E-AUTORIDADE-IMPLICITA.md",
    ROOT / "docs" / "92-HERUS-COMPOSICAO-CONTRAFACTUAL-E-ISOLAMENTO-DE-PRINCIPAIS.md",
    ROOT / "docs" / "93-HERUS-DELEGACAO-NAO-TRANSITIVA-E-REVOGACAO-ENTRE-PRINCIPAIS.md",
]


def sha256(path: pathlib.Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def clean(text: str) -> str:
    text = text.replace("\x00", " ")
    text = re.sub(r"https?://\S+", " URL ", text)
    text = re.sub(r"\[[^\]]+\]\([^)]*\)", " ", text)
    text = re.sub(r"`[^`]*`", " token ", text)
    text = re.sub(r"[^\w\s.,!?;:'\"()/%+\-]", " ", text, flags=re.UNICODE)
    text = re.sub(r"\s+", " ", text).strip()
    return text


def join_lines(lines: Iterable[str], minimum: int = 40) -> str:
    cleaned = [clean(line) for line in lines]
    cleaned = [line for line in cleaned if len(line) >= minimum]
    return "\n".join(cleaned) + "\n"


def write_text(path: pathlib.Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def main() -> int:
    DATA.mkdir(parents=True, exist_ok=True)
    train_path = DATA / "wikitext-2-raw-v1-train.parquet"
    table = pq.read_table(train_path, columns=["text"])
    public_lines = [str(value) for value in table.column("text").to_pylist() if value]
    public_text = join_lines(public_lines, minimum=20)
    split = max(1, int(len(public_text) * 0.9))
    write_text(DATA / "public_train.txt", public_text[:split])
    write_text(DATA / "public_validation.txt", public_text[split:])

    herus_lines = []
    for path in HERUS_DOCS:
        if not path.exists():
            raise FileNotFoundError(path)
        herus_lines.extend(path.read_text(encoding="utf-8").splitlines())
    herus_text = join_lines(herus_lines, minimum=24)
    behavior_path = DATA / "herus_behavior.txt"
    behavior_text = join_lines(behavior_path.read_text(encoding="utf-8").splitlines(), minimum=24)
    behavior_train_end = max(1, int(len(behavior_text) * 0.70))
    behavior_tune_end = max(behavior_train_end + 1, int(len(behavior_text) * 0.85))
    write_text(DATA / "herus_behavior_train.txt", behavior_text[:behavior_train_end])
    write_text(DATA / "herus_behavior_tune.txt", behavior_text[behavior_train_end:behavior_tune_end])
    write_text(DATA / "herus_behavior_test.txt", behavior_text[behavior_tune_end:])
    herus_train_end = max(1, int(len(herus_text) * 0.70))
    herus_tune_end = max(herus_train_end + 1, int(len(herus_text) * 0.85))
    write_text(DATA / "herus_adapter_train.txt", herus_text[:herus_train_end])
    write_text(DATA / "herus_adapter_tune.txt", herus_text[herus_train_end:herus_tune_end])
    write_text(DATA / "herus_adapter_test.txt", herus_text[herus_tune_end:])

    manifest = {
        "purpose": "local text adaptation experiment; no personal memory or product logs",
        "public_dataset": {
            "name": "Salesforce/wikitext wikitext-2-raw-v1 train parquet",
            "license": "CC BY-SA 4.0 as declared by the dataset card; verify upstream terms before redistribution",
            "source_url": "https://huggingface.co/datasets/Salesforce/wikitext",
            "file": str(train_path.relative_to(ROOT)),
            "sha256": sha256(train_path),
            "bytes": train_path.stat().st_size,
        },
        "private_or_project_adapter_dataset": {
            "name": "HERUS public design documents",
            "license": "repository terms; not a personal-user corpus",
            "files": [str(path.relative_to(ROOT)) for path in HERUS_DOCS],
            "sha256": {str(path.relative_to(ROOT)): sha256(path) for path in HERUS_DOCS},
        },
        "behavior_dataset": {
            "name": "curated HERUS behavior contract examples",
            "license": "repository terms; synthetic project examples; no personal data",
            "file": str(behavior_path.relative_to(ROOT)),
            "sha256": sha256(behavior_path),
        },
        "outputs": {
            "public_train": str((DATA / "public_train.txt").relative_to(ROOT)),
            "public_validation": str((DATA / "public_validation.txt").relative_to(ROOT)),
            "herus_adapter_train": str((DATA / "herus_adapter_train.txt").relative_to(ROOT)),
            "herus_adapter_tune": str((DATA / "herus_adapter_tune.txt").relative_to(ROOT)),
            "herus_adapter_test": str((DATA / "herus_adapter_test.txt").relative_to(ROOT)),
            "herus_behavior_train": str((DATA / "herus_behavior_train.txt").relative_to(ROOT)),
            "herus_behavior_tune": str((DATA / "herus_behavior_tune.txt").relative_to(ROOT)),
            "herus_behavior_test": str((DATA / "herus_behavior_test.txt").relative_to(ROOT)),
        },
    }
    write_text(DATA / "corpus_manifest.json", json.dumps(manifest, indent=2, ensure_ascii=False) + "\n")
    print(json.dumps({
        "public_train_chars": len(public_text[:split]),
        "public_validation_chars": len(public_text[split:]),
        "herus_adapter_train_chars": len(herus_text[:herus_train_end]),
        "herus_adapter_tune_chars": len(herus_text[herus_train_end:herus_tune_end]),
        "herus_adapter_test_chars": len(herus_text[herus_tune_end:]),
        "herus_behavior_train_chars": len(behavior_text[:behavior_train_end]),
        "herus_behavior_tune_chars": len(behavior_text[behavior_train_end:behavior_tune_end]),
        "herus_behavior_test_chars": len(behavior_text[behavior_tune_end:]),
        "manifest": str(DATA / "corpus_manifest.json"),
    }, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
