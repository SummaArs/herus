from pathlib import Path
from provenance_audit import sha256_file, sha256_tree

root = Path(__file__).resolve().parents[1]
for relative, kind in (("prove.sh", "file"), ("firmware", "tree"), ("tools", "tree")):
    path = root / relative
    digest = sha256_file(path) if kind == "file" else sha256_tree(path)
    print(f"{relative} {digest}")
