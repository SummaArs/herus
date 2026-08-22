from __future__ import annotations

import json
import math
import pathlib
import random
import time

import numpy as np
import torch
from torch import nn

ROOT = pathlib.Path(__file__).resolve().parents[1]
DATA = ROOT / "research" / "datasets" / "text_transfer"
ARTIFACTS = ROOT / "research" / "model_artifacts" / "char_transfer_v1"
SEED = 1729
BLOCK = 128
BATCH = 16
D_MODEL = 64
HEADS = 4
LAYERS = 2
FF = 128
RANK = 4


class TinyCharLM(nn.Module):
    def __init__(self, vocab_size: int) -> None:
        super().__init__()
        self.token = nn.Embedding(vocab_size, D_MODEL)
        self.position = nn.Embedding(BLOCK, D_MODEL)
        layer = nn.TransformerEncoderLayer(
            d_model=D_MODEL,
            nhead=HEADS,
            dim_feedforward=FF,
            dropout=0.0,
            batch_first=True,
            activation="gelu",
        )
        self.encoder = nn.TransformerEncoder(layer, num_layers=LAYERS)
        self.norm = nn.LayerNorm(D_MODEL)
        self.head = nn.Linear(D_MODEL, vocab_size, bias=False)
        self.adapter_u = nn.Parameter(torch.empty(D_MODEL, RANK))
        self.adapter_v = nn.Parameter(torch.empty(RANK, vocab_size))
        nn.init.normal_(self.adapter_u, mean=0.0, std=0.01)
        nn.init.zeros_(self.adapter_v)
        self.register_buffer("causal", torch.triu(torch.full((BLOCK, BLOCK), float("-inf")), 1))

    def hidden(self, x: torch.Tensor) -> torch.Tensor:
        positions = torch.arange(x.shape[1], device=x.device)
        h = self.token(x) + self.position(positions)[None, :, :]
        return self.norm(self.encoder(h, mask=self.causal[:x.shape[1], :x.shape[1]]))

    def forward(self, x: torch.Tensor, use_adapter: bool, adapter_strength: float = 1.0) -> torch.Tensor:
        h = self.hidden(x)
        logits = self.head(h)
        if use_adapter:
            logits = logits + adapter_strength * torch.einsum("btd,dr,rv->btv", h, self.adapter_u, self.adapter_v)
        return logits


def read_text(name: str) -> str:
    return (DATA / name).read_text(encoding="utf-8")


def make_vocab(text: str) -> tuple[dict[str, int], list[str]]:
    chars = sorted(set(text))
    stoi = {char: i for i, char in enumerate(chars)}
    return stoi, chars


def encode(text: str, stoi: dict[str, int]) -> np.ndarray:
    unknown = stoi.get(" ", 0)
    return np.asarray([stoi.get(char, unknown) for char in text], dtype=np.int64)


def batch(data: np.ndarray, rng: np.random.Generator) -> tuple[torch.Tensor, torch.Tensor]:
    starts = rng.integers(0, len(data) - BLOCK - 1, size=BATCH)
    x = np.stack([data[start:start + BLOCK] for start in starts])
    y = np.stack([data[start + 1:start + BLOCK + 1] for start in starts])
    return torch.from_numpy(x), torch.from_numpy(y)


def run_steps(model: TinyCharLM, data: np.ndarray, steps: int, adapter: bool, lr: float, seed: int) -> list[float]:
    parameters = [p for p in model.parameters() if p.requires_grad]
    optimizer = torch.optim.AdamW(parameters, lr=lr, weight_decay=0.01)
    rng = np.random.default_rng(seed)
    losses = []
    model.train()
    for step in range(steps):
        x, y = batch(data, rng)
        logits = model(x, use_adapter=adapter)
        loss = nn.functional.cross_entropy(logits.reshape(-1, logits.shape[-1]), y.reshape(-1))
        optimizer.zero_grad(set_to_none=True)
        loss.backward()
        torch.nn.utils.clip_grad_norm_(parameters, 1.0)
        optimizer.step()
        losses.append(float(loss.detach()))
    return losses


def evaluate(model: TinyCharLM, data: np.ndarray, adapter: bool, adapter_strength: float = 1.0, limit: int = 32768) -> float:
    model.eval()
    data = data[: min(len(data), limit)]
    values = []
    with torch.no_grad():
        for start in range(0, len(data) - BLOCK - 1, BLOCK):
            x = torch.from_numpy(data[start:start + BLOCK][None, :])
            y = torch.from_numpy(data[start + 1:start + BLOCK + 1][None, :])
            logits = model(x, use_adapter=adapter, adapter_strength=adapter_strength)
            values.append(float(nn.functional.cross_entropy(
                logits.reshape(-1, logits.shape[-1]), y.reshape(-1)
            )))
    return float(np.mean(values))


def generate(model: TinyCharLM, seed: str, stoi: dict[str, int], chars: list[str], adapter_strength: float, length: int = 480) -> str:
    model.eval()
    values = [stoi.get(char, stoi.get(" ", 0)) for char in seed]
    rng = random.Random(SEED)
    with torch.no_grad():
        for _ in range(length):
            context = torch.tensor([values[-BLOCK:]], dtype=torch.long)
            logits = model(context, use_adapter=True, adapter_strength=adapter_strength)[0, -1]
            logits = logits / 0.85
            top = torch.topk(logits, k=min(12, logits.numel()))
            weights = torch.softmax(top.values, dim=0).cpu().numpy()
            choice = rng.choices(range(len(top.indices)), weights=weights, k=1)[0]
            values.append(int(top.indices[choice]))
    return "".join(chars[index] for index in values)


def main() -> int:
    torch.manual_seed(SEED)
    np.random.seed(SEED)
    torch.set_num_threads(2)
    public_train = read_text("public_train.txt")
    public_holdout = read_text("public_validation.txt")
    herus_train = read_text("herus_adapter_train.txt")
    herus_tune = read_text("herus_adapter_tune.txt")
    herus_test = read_text("herus_adapter_test.txt")
    behavior_train = read_text("herus_behavior_train.txt")
    behavior_tune = read_text("herus_behavior_tune.txt")
    behavior_test = read_text("herus_behavior_test.txt")
    adapter_text = herus_train + "\n" + behavior_train
    stoi, chars = make_vocab(public_train + adapter_text)
    public_train_ids = encode(public_train, stoi)
    public_holdout_ids = encode(public_holdout, stoi)
    herus_train_ids = encode(adapter_text, stoi)
    herus_tune_ids = encode(herus_tune, stoi)
    herus_test_ids = encode(herus_test, stoi)
    behavior_train_ids = encode(behavior_train, stoi)
    behavior_tune_ids = encode(behavior_tune, stoi)
    behavior_test_ids = encode(behavior_test, stoi)
    public_tune_ids = public_holdout_ids[: len(public_holdout_ids) // 2]
    public_test_ids = public_holdout_ids[len(public_holdout_ids) // 2:]

    model = TinyCharLM(len(chars))
    started = time.time()
    base_losses = run_steps(model, public_train_ids, steps=700, adapter=False, lr=2e-3, seed=SEED)
    base_public_tune = evaluate(model, public_tune_ids, adapter=False)
    base_public_test = evaluate(model, public_test_ids, adapter=False)
    base_herus_tune = evaluate(model, herus_tune_ids, adapter=False)
    base_herus_test = evaluate(model, herus_test_ids, adapter=False)
    base_behavior_tune = evaluate(model, behavior_tune_ids, adapter=False)
    base_behavior_test = evaluate(model, behavior_test_ids, adapter=False)

    for parameter in model.parameters():
        parameter.requires_grad = False
    model.adapter_u.requires_grad = True
    model.adapter_v.requires_grad = True
    adapter_losses = run_steps(model, herus_train_ids, steps=350, adapter=True, lr=4e-3, seed=SEED + 1)
    candidate_strengths = (0.10, 0.20, 0.30, 0.40, 0.50, 0.60, 0.75, 1.00)
    candidates = []
    for strength in candidate_strengths:
        candidate_public_tune = evaluate(model, public_tune_ids, adapter=True, adapter_strength=strength)
        candidate_herus_tune = evaluate(model, herus_tune_ids, adapter=True, adapter_strength=strength)
        candidate_behavior_tune = evaluate(model, behavior_tune_ids, adapter=True, adapter_strength=strength)
        candidates.append({
            "strength": strength,
            "public_tune_nll": candidate_public_tune,
            "public_tune_delta_nll": candidate_public_tune - base_public_tune,
            "herus_tune_nll": candidate_herus_tune,
            "behavior_tune_nll": candidate_behavior_tune,
            "combined_tune_nll": (candidate_herus_tune + candidate_behavior_tune) / 2.0,
        })
    safe = [candidate for candidate in candidates if candidate["public_tune_delta_nll"] <= 0.25]
    selected = min(safe, key=lambda candidate: candidate["combined_tune_nll"]) if safe else min(
        candidates, key=lambda candidate: candidate["public_tune_delta_nll"]
    )
    adapter_strength = float(selected["strength"])
    adapted_public_tune = evaluate(model, public_tune_ids, adapter=True, adapter_strength=adapter_strength)
    adapted_public_test = evaluate(model, public_test_ids, adapter=True, adapter_strength=adapter_strength)
    adapted_herus_tune = evaluate(model, herus_tune_ids, adapter=True, adapter_strength=adapter_strength)
    adapted_herus_test = evaluate(model, herus_test_ids, adapter=True, adapter_strength=adapter_strength)
    adapted_behavior_tune = evaluate(model, behavior_tune_ids, adapter=True, adapter_strength=adapter_strength)
    adapted_behavior_test = evaluate(model, behavior_test_ids, adapter=True, adapter_strength=adapter_strength)

    ARTIFACTS.mkdir(parents=True, exist_ok=True)
    torch.save({
        "token": model.token.state_dict(),
        "position": model.position.state_dict(),
        "encoder": model.encoder.state_dict(),
        "norm": model.norm.state_dict(),
        "head": model.head.state_dict(),
        "config": {"vocab_size": len(chars), "block": BLOCK, "d_model": D_MODEL, "heads": HEADS, "layers": LAYERS, "ff": FF},
    }, ARTIFACTS / "base.pt")
    torch.save({
        "adapter_u": model.adapter_u.detach().cpu(),
        "adapter_v": model.adapter_v.detach().cpu(),
        "rank": RANK,
    }, ARTIFACTS / "herus_adapter.pt")
    (ARTIFACTS / "vocab.json").write_text(json.dumps(chars, ensure_ascii=False) + "\n", encoding="utf-8")
    sample = generate(model, "HERUS ", stoi, chars, adapter_strength)
    report = {
        "method": "local character Transformer with frozen base and rank-4 output adapter",
        "not_canonical_lora": True,
        "base_dataset": "WikiText-2 raw train, public corpus",
        "adapter_dataset": "public HERUS design documents only",
        "config": {"vocab_size": len(chars), "block": BLOCK, "d_model": D_MODEL, "heads": HEADS, "layers": LAYERS, "ff": FF, "rank": RANK, "base_steps": 700, "adapter_steps": 350, "adapter_strength": adapter_strength, "candidate_strengths": list(candidate_strengths)},
        "selection": {"constraint": "public_tune_delta_nll <= 0.25 when feasible", "candidates": candidates},
        "metrics": {
            "base_public_tune_nll": base_public_tune,
            "adapted_public_tune_nll": adapted_public_tune,
            "base_public_test_nll": base_public_test,
            "adapted_public_test_nll": adapted_public_test,
            "base_herus_tune_nll": base_herus_tune,
            "adapted_herus_tune_nll": adapted_herus_tune,
            "base_herus_test_nll": base_herus_test,
            "adapted_herus_test_nll": adapted_herus_test,
            "base_behavior_tune_nll": base_behavior_tune,
            "adapted_behavior_tune_nll": adapted_behavior_tune,
            "base_behavior_test_nll": base_behavior_test,
            "adapted_behavior_test_nll": adapted_behavior_test,
            "public_test_delta_nll": adapted_public_test - base_public_test,
            "herus_test_delta_nll": adapted_herus_test - base_herus_test,
            "behavior_test_delta_nll": adapted_behavior_test - base_behavior_test,
        },
        "loss_trace": {"base_first": base_losses[:5], "base_last": base_losses[-5:], "adapter_first": adapter_losses[:5], "adapter_last": adapter_losses[-5:]},
        "storage": {"base_parameters": sum(p.numel() for p in model.parameters() if p is not model.adapter_u and p is not model.adapter_v), "adapter_parameters": int(model.adapter_u.numel() + model.adapter_v.numel()), "elapsed_seconds": time.time() - started},
        "sample": sample,
        "interpretation": {
            "transfer_learning_observed": adapted_herus_test < base_herus_test and adapted_behavior_test < base_behavior_test,
            "catastrophic_forgetting_guard": adapted_public_test <= base_public_test + 0.25,
            "authority_in_weights": False,
            "third_party_hosted_llm": False,
            "hardware_claim": False,
        },
    }
    (ARTIFACTS / "metrics.json").write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(json.dumps({
        "vocab_size": len(chars),
        "base_herus_test_nll": base_herus_test,
        "adapted_herus_test_nll": adapted_herus_test,
        "base_public_test_nll": base_public_test,
        "adapted_public_test_nll": adapted_public_test,
        "adapted_behavior_test_nll": adapted_behavior_test,
        "elapsed_seconds": time.time() - started,
        "artifacts": str(ARTIFACTS),
    }, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
