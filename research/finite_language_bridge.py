"""Finite language bridge: language is input translation, never authority."""
from __future__ import annotations

from dataclasses import dataclass
import unicodedata


@dataclass(frozen=True)
class LanguageResult:
    status: str
    intent: str | None
    normalized: str
    missing: tuple[str, ...]
    reason: str


LEXICON = {
    "cheguei": "STATUS_ARRIVED",
    "cheguei em seguranca": "STATUS_ARRIVED",
    "preciso de ajuda": "ALERT_HELP",
    "pare": "STOP_PROPOSAL",
    "parar": "STOP_PROPOSAL",
    "confirme a ação": "CONFIRM_PROPOSAL",
}


def normalize(text: str) -> str:
    folded = unicodedata.normalize("NFKD", text.casefold())
    return "".join(char for char in folded if not unicodedata.combining(char)).strip()


def translate(text: str) -> LanguageResult:
    normalized = normalize(text)
    if not normalized:
        return LanguageResult("BLOCKED", None, normalized, ("intent",), "empty_input")
    intent = LEXICON.get(normalized)
    if intent is None:
        return LanguageResult("BLOCKED", None, normalized, ("finite_vocab_entry",), "unknown_language_form")
    return LanguageResult("PROPOSAL_ONLY", intent, normalized, (), "finite_translation")
