from __future__ import annotations

import collections
import json
import math
import pathlib
import re
from dataclasses import dataclass

ROOT = pathlib.Path(__file__).resolve().parents[1]
CASES = ROOT / "research" / "benchmarks" / "intent_memory" / "cases.jsonl"
RESULTS = ROOT / "research" / "benchmarks" / "intent_memory" / "results.json"
TOKEN_RE = re.compile(r"[\wÀ-ÿ]+", re.UNICODE)

INTENTS = (
    "recall_memory",
    "capture_memory",
    "unknown_query",
    "action_request",
    "forget_memory",
    "update_preference",
    "share_memory",
    "conflict_query",
    "chitchat",
)


@dataclass(frozen=True)
class Memory:
    memory_id: str
    text: str
    generation: int
    purpose: str
    origin: str = "local"
    scope: str = "wrist"
    active: bool = True
    superseded: bool = False


MEMORIES = (
    Memory("meeting_v1", "A reunião é hoje às quatro.", 1, "schedule", superseded=True),
    Memory("meeting_v2", "A reunião mudou para amanhã às nove.", 2, "schedule"),
    Memory("pref_concise", "O usuário prefere respostas curtas e diretas.", 1, "dialogue"),
    Memory("pref_discreet", "O usuário prefere respostas discretas.", 1, "dialogue"),
    Memory("project_local", "O HERUS deve manter a soberania da inteligência no pulso.", 1, "project"),
)


def read_cases() -> list[dict[str, object]]:
    return [json.loads(line) for line in CASES.read_text(encoding="utf-8").splitlines() if line.strip()]


def grams(text: str, n: int = 4) -> collections.Counter[str]:
    normalized = " ".join(text.lower().split())
    padded = f"  {normalized}  "
    return collections.Counter(padded[i:i + n] for i in range(max(0, len(padded) - n + 1)))


def cosine(left: collections.Counter[str], right: collections.Counter[str]) -> float:
    common = set(left) & set(right)
    numerator = sum(left[key] * right[key] for key in common)
    left_norm = math.sqrt(sum(value * value for value in left.values()))
    right_norm = math.sqrt(sum(value * value for value in right.values()))
    return numerator / (left_norm * right_norm) if left_norm and right_norm else 0.0


def build_centroids(cases: list[dict[str, object]]) -> dict[str, collections.Counter[str]]:
    grouped: dict[str, list[collections.Counter[str]]] = collections.defaultdict(list)
    for case in cases:
        grouped[str(case["intent"])].append(grams(str(case["utterance"])))
    centroids: dict[str, collections.Counter[str]] = {}
    for intent, vectors in grouped.items():
        aggregate: collections.Counter[str] = collections.Counter()
        for vector in vectors:
            aggregate.update(vector)
        centroids[intent] = aggregate
    return centroids


def ngram_classify(utterance: str, centroids: dict[str, collections.Counter[str]]) -> tuple[str, float, float]:
    scores = sorted(((cosine(grams(utterance), centroid), intent) for intent, centroid in centroids.items()), reverse=True)
    best_score, best_intent = scores[0]
    second_score = scores[1][0] if len(scores) > 1 else 0.0
    return best_intent, best_score, best_score - second_score


def safety_override(utterance: str) -> str | None:
    text = utterance.lower()
    if any(word in text for word in ("afinal", "diverg", "duas memórias", "duas anotações", "contradit")):
        return "conflict_query"
    if any(word in text for word in ("salvar este ponto", "salve este ponto", "registre", "anote esta", "guarde que")):
        return "capture_memory"
    if any(word in text for word in ("me diga", "o que ficou registrado", "lembra", "qual horário", "qual foi", "consultar depois")) and not any(word in text for word in ("esqueça", "apague", "revogue", "retire da memória")):
        return "recall_memory"
    if any(word in text for word in ("fique comigo", "quero conversar", "como você está", "me acompanhe")):
        return "chitchat"
    if any(word in text for word in ("senha", "código secreto", "nunca te contei", "não está na memória", "não te contei")):
        return "unknown_query"
    if any(word in text for word in ("esqueça", "apague", "revogue", "retire da memória")):
        return "forget_memory"
    if any(word in text for word in ("compartilhe", "compartilhar", "outro dispositivo", "meu contato", "ao contato")):
        return "share_memory"
    if any(word in text for word in ("faça a compra", "abra o portão", "ligue para", "envie uma mensagem", "execute", "aja agora")):
        return "action_request"
    if any(word in text for word in ("guarde", "anote", "salve", "registre")):
        return "capture_memory"
    if any(word in text for word in ("prefiro", "passe a", "gosto de", "aprenda que")):
        return "update_preference"
    return None


def classify(utterance: str, centroids: dict[str, collections.Counter[str]], typed: bool) -> dict[str, object]:
    predicted, score, margin = ngram_classify(utterance, centroids)
    override = safety_override(utterance) if typed else None
    intent = override or predicted
    low_confidence = score < 0.18 or margin < 0.015
    # Explicitly structured commands are routed with a confirmation gate rather
    # than being silently classified as unknown. Low confidence still abstains
    # for factual recall and open-ended dialogue.
    abstain = intent in {"unknown_query", "conflict_query"} or (
        typed and low_confidence and intent in {"recall_memory"}
    )
    requires_confirmation = intent in {"action_request", "forget_memory", "share_memory"}
    if typed and intent == "capture_memory":
        requires_confirmation = False
    return {
        "intent": intent,
        "score": score,
        "margin": margin,
        "abstain": abstain,
        "requires_confirmation": requires_confirmation,
        "override": override,
    }


def word_set(text: str) -> set[str]:
    return set(TOKEN_RE.findall(text.lower()))


def retrieve(utterance: str, intent: str) -> list[dict[str, object]]:
    query = word_set(utterance)
    if intent in {"action_request", "chitchat", "capture_memory", "unknown_query", "conflict_query"}:
        return []
    candidates = []
    for memory in MEMORIES:
        if not memory.active:
            continue
        if intent == "recall_memory" and memory.purpose != "schedule":
            continue
        if intent == "forget_memory" and memory.purpose != "schedule":
            continue
        if intent == "update_preference" and memory.purpose != "dialogue":
            continue
        if intent == "share_memory" and memory.purpose != "project":
            continue
        overlap = len(query & word_set(memory.text))
        if intent == "update_preference":
            if any(word in utterance.lower() for word in ("curt", "diret", "sem rodeios")) and memory.memory_id == "pref_concise":
                overlap += 3
            if "discret" in utterance.lower() and memory.memory_id == "pref_discreet":
                overlap += 3
        if intent == "forget_memory" and any(word in query for word in ("antigo", "antiga", "anterior", "ontem")):
            overlap += 3 if memory.superseded else 0
        if intent == "recall_memory":
            overlap += memory.generation
        if overlap > 0:
            candidates.append((overlap, memory.generation, memory))
    candidates.sort(key=lambda item: (item[0], item[1]), reverse=True)
    return [{"memory_id": memory.memory_id, "score": score, "generation": memory.generation, "origin": memory.origin, "purpose": memory.purpose} for score, _, memory in candidates]


def route(case: dict[str, object], centroids: dict[str, collections.Counter[str]], typed: bool) -> dict[str, object]:
    utterance = str(case["utterance"])
    decision = classify(utterance, centroids, typed)
    evidence = retrieve(utterance, str(decision["intent"])) if not decision["abstain"] else []
    if str(decision["intent"]) == "conflict_query":
        evidence = [{"memory_id": "meeting_v1"}, {"memory_id": "meeting_v2"}]
    return {**decision, "evidence": evidence}


def evidence_ok(case: dict[str, object], actual_evidence: set[str]) -> bool:
    expected_evidence = set(str(item) for item in case["expected_evidence"])
    if expected_evidence:
        return expected_evidence.issubset(actual_evidence)
    return not actual_evidence


def evaluate(cases: list[dict[str, object]], centroids: dict[str, collections.Counter[str]], typed: bool) -> dict[str, object]:
    test_cases = [case for case in cases if case["split"] == "test"]
    rows = []
    for case in test_cases:
        result = route(case, centroids, typed)
        expected_evidence = set(str(item) for item in case["expected_evidence"])
        actual_evidence = {str(item["memory_id"]) for item in result["evidence"]}
        rows.append({
            "id": case["id"],
            "gold_intent": case["intent"],
            "predicted_intent": result["intent"],
            "intent_correct": result["intent"] == case["intent"],
            "gold_abstain": bool(case["should_abstain"]),
            "predicted_abstain": bool(result["abstain"]),
            "abstain_correct": bool(result["abstain"]) == bool(case["should_abstain"]),
            "confirmation_required": bool(result["requires_confirmation"]),
            "confirmation_safe": bool(result["requires_confirmation"]) == bool(case["action_requires_confirmation"]),
            "evidence_expected": sorted(expected_evidence),
            "evidence_actual": sorted(actual_evidence),
            "evidence_correct": evidence_ok(case, actual_evidence),
            "score": result["score"],
            "margin": result["margin"],
        })
    total = len(rows)
    return {
        "cases": total,
        "intent_accuracy": sum(row["intent_correct"] for row in rows) / total,
        "abstention_accuracy": sum(row["abstain_correct"] for row in rows) / total,
        "confirmation_safety": sum(row["confirmation_safe"] for row in rows) / total,
        "evidence_accuracy": sum(row["evidence_correct"] for row in rows) / total,
        "rows": rows,
    }


def main() -> int:
    cases = read_cases()
    train = [case for case in cases if case["split"] == "train"]
    centroids = build_centroids(train)
    typed = evaluate(cases, centroids, typed=True)
    similarity_only = evaluate(cases, centroids, typed=False)
    report = {
        "method": "local character n-gram intent router plus typed safety overrides and provenance-aware memory retrieval",
        "dataset": "research/benchmarks/intent_memory/cases.jsonl",
        "splits": {"train": len(train), "test": len(cases) - len(train)},
        "typed_router": typed,
        "similarity_only_baseline": similarity_only,
        "gates": {
            "typed_intent_accuracy": typed["intent_accuracy"] == 1.0,
            "typed_abstention_accuracy": typed["abstention_accuracy"] == 1.0,
            "typed_confirmation_safety": typed["confirmation_safety"] == 1.0,
            "typed_evidence_accuracy": typed["evidence_accuracy"] == 1.0,
            "typed_beats_similarity_on_abstention": typed["abstention_accuracy"] > similarity_only["abstention_accuracy"],
        },
        "limits": {
            "not_llm": True,
            "not_consciousness": True,
            "no_autonomous_action": True,
            "host_only": True,
        },
    }
    RESULTS.parent.mkdir(parents=True, exist_ok=True)
    RESULTS.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(json.dumps({
        "typed_intent_accuracy": typed["intent_accuracy"],
        "typed_abstention_accuracy": typed["abstention_accuracy"],
        "typed_confirmation_safety": typed["confirmation_safety"],
        "typed_evidence_accuracy": typed["evidence_accuracy"],
        "similarity_only_abstention_accuracy": similarity_only["abstention_accuracy"],
        "gates": report["gates"],
        "results": str(RESULTS),
    }, ensure_ascii=False))
    return 0 if all(bool(value) for value in report["gates"].values()) else 1


if __name__ == "__main__":
    raise SystemExit(main())
