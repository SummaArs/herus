import importlib.util
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
path = ROOT / "tools" / "loom.py"
spec = importlib.util.spec_from_file_location("herus_loom_under_test", path)
loom = importlib.util.module_from_spec(spec)
spec.loader.exec_module(loom)

pack_dir = ROOT / "research" / "loom" / "core"
pack = loom.B.Pack.load(pack_dir)
measured = loom._emitted_table_string_bytes(pack)
assert measured > 0

gate = loom.Gate(pack, pack_dir)
old = loom.ENVELOPE["table_bytes"]
loom.ENVELOPE["table_bytes"] = 1
try:
    gate.l10_envelope()
finally:
    loom.ENVELOPE["table_bytes"] = old

assert any(failure.startswith("L10:") and "payload textual" in failure
           for failure in gate.failures), gate.failures
print(f"LOOM ENVELOPE: PASS measured={measured}B; undersized envelope rejected")
