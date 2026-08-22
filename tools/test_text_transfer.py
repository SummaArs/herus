from __future__ import annotations

import json
import pathlib
import re


ROOT = pathlib.Path(__file__).resolve().parents[1]
ARTIFACTS = ROOT / "research" / "model_artifacts" / "char_transfer_v1"
DATA = ROOT / "research" / "datasets" / "text_transfer"


def fail(message: str) -> None:
    raise AssertionError(message)


def main() -> int:
    metrics = json.loads((ARTIFACTS / "metrics.json").read_text(encoding="utf-8"))
    interpretation = metrics["interpretation"]
    observed = metrics["metrics"]
    storage = metrics["storage"]
    config = metrics["config"]
    if not interpretation["transfer_learning_observed"]:
        fail("adapter did not improve both HERUS held-out sets")
    if not interpretation["catastrophic_forgetting_guard"]:
        fail("public held-out forgetting guard failed")
    if observed["public_test_delta_nll"] > 0.25:
        fail("public held-out delta exceeds guard")
    if observed["herus_test_delta_nll"] >= 0 or observed["behavior_test_delta_nll"] >= 0:
        fail("one HERUS held-out set did not improve")
    if storage["adapter_parameters"] >= storage["base_parameters"]:
        fail("adapter is not smaller than frozen base")
    if config["rank"] != 4 or not metrics["not_canonical_lora"]:
        fail("artifact contract lost compact-adapter declaration")
    if not metrics["adapter_dataset"].startswith("public HERUS"):
        fail("adapter dataset is not explicitly project-public")
    if metrics["interpretation"]["authority_in_weights"]:
        fail("weights were incorrectly treated as authority")
    for path in (ARTIFACTS / "base.pt", ARTIFACTS / "herus_adapter.pt", ARTIFACTS / "vocab.json"):
        if not path.exists() or path.stat().st_size == 0:
            fail(f"missing artifact: {path}")
    behavior = (DATA / "herus_behavior.txt").read_text(encoding="utf-8").lower()
    forbidden = ("@", "latitude", "longitude", "gps", "api_key", "private key", "secret")
    if any(token in behavior for token in forbidden):
        fail("behavior corpus contains a forbidden personal or secret marker")
    if not metrics["sample"].startswith("HERUS"):
        fail("artifact sample does not preserve the HERUS seed")
    if re.search(r"\b(openai|anthropic|google gemini)\b", json.dumps(metrics).lower()):
        fail("artifact metadata names a hosted third-party model")
    if (ARTIFACTS / "base.pt").read_bytes()[:2] != b"PK":
        fail("base artifact is not a valid torch archive container")
    if (ARTIFACTS / "herus_adapter.pt").read_bytes()[:2] != b"PK":
        fail("adapter artifact is not a valid torch archive container")
    print("TEXT TRANSFER: artifact, privacy, held-out transfer and forgetting gates pass")
    print(f"TEXT TRANSFER: HERUS delta={observed['herus_test_delta_nll']:.6f}, behavior delta={observed['behavior_test_delta_nll']:.6f}, public delta={observed['public_test_delta_nll']:.6f}")
    print(f"TEXT TRANSFER: base_parameters={storage['base_parameters']}, adapter_parameters={storage['adapter_parameters']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
