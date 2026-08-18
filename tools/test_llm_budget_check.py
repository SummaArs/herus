#!/usr/bin/env python3
"""Host-only tests for the HERUS local-model budget checker."""
from __future__ import annotations

import sys

import llm_budget_check as budget


FAILED = 0


def ok(condition: bool, label: str) -> None:
    global FAILED
    print(f"  {'PASS' if condition else 'FAIL'}  {label}")
    if not condition:
        FAILED = 1


def test_article_vs_boards() -> None:
    print("== LLM budget: article artifact versus HERUS targets ==")
    result = budget.check_budget(
        budget.BOARDS["lilygo-t3-s3-v1.3"],
        28_900_000,
        4,
        14_912_332,
        None,
        None,
    )
    ok(result["model"]["raw_weight_bytes"] == 14_450_000,
       "4-bit raw weight sizing is deterministic")
    ok(result["fit"]["stored_artifact_in_flash"] is False,
       "14.9 MB article artifact does not fit LilyGO 4 MB flash")
    ok(result["fit"]["coarse_capacity_fit"] is False,
       "LilyGO article configuration is not accepted as a coarse fit")

    article = budget.check_budget(
        budget.BOARDS["esp32-s3-n16r8-article"],
        28_900_000,
        4,
        14_912_332,
        None,
        None,
    )
    ok(article["fit"]["stored_artifact_in_flash"] is True,
       "article artifact fits the article's declared 16 MB flash")


def test_runtime_headroom_is_not_ignored() -> None:
    result = budget.check_budget(
        budget.BOARDS["esp32-s3-n16r8-article"],
        1_000_000,
        4,
        None,
        8_000_001,
        None,
    )
    ok(result["fit"]["stored_artifact_in_flash"] is True,
       "small raw model can fit the declared flash budget")
    ok(result["fit"]["required_psram_in_capacity"] is False,
       "working-set requirement above PSRAM capacity is rejected")
    ok(result["fit"]["coarse_capacity_fit"] is False,
       "storage fit does not override working-set failure")


def test_bad_inputs() -> None:
    try:
        budget.model_bytes(-1, 4)
    except ValueError:
        rejected = True
    else:
        rejected = False
    ok(rejected, "negative parameter count is rejected")


if __name__ == "__main__":
    test_article_vs_boards()
    test_runtime_headroom_is_not_ignored()
    test_bad_inputs()
    if FAILED:
        print("LLM BUDGET TESTS FAILED")
        raise SystemExit(1)
    print("LLM BUDGET INVARIANTS HOLD — sizing is coarse and not a hardware claim")
