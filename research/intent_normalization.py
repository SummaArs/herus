"""Bounded surface normalization for the shortcut probe.

Only exact leading politeness phrases are removed. The function does not
attempt open-ended semantic rewriting and never changes labels or authority.
"""
from __future__ import annotations
import re

_PREFIXES = (
    re.compile(r"^\s*if you can\s+", re.I),
    re.compile(r"^\s*please\s+", re.I),
    re.compile(r"^\s*thanks\s+", re.I),
)


def normalize_surface(text: str) -> str:
    result = text
    for pattern in _PREFIXES:
        result = pattern.sub('', result, count=1)
    return result.strip()
