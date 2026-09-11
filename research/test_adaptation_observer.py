from __future__ import annotations

import unittest
from pathlib import Path

from adaptation_observer import observe_artifact


class AdaptationObserverTests(unittest.TestCase):
    def test_real_ofr_artifact_is_observed_with_bounded_output(self) -> None:
        path = Path(__file__).parent / "evidence" / "adaptation_data_v1" / "ofr_tyld_2020_2025.json"
        observations = observe_artifact(path, limit=12)
        self.assertEqual(len(observations), 12)
        self.assertEqual(observations[0].points, 72)
        self.assertEqual(observations[0].first_date, "2020-01-31")
        self.assertEqual(observations[0].last_date, "2025-12-31")
        self.assertEqual(observations[0].regime, "RISING")
        self.assertEqual(observations[0].source_digest, "2b4f797d0c9d2d811f214aef125eff8825b03ce23d5efdc0ccda3c50da3d8b48")

    def test_limit_is_bounded(self) -> None:
        path = Path(__file__).parent / "evidence" / "adaptation_data_v1" / "ofr_tyld_2020_2025.json"
        self.assertEqual(len(observe_artifact(path, limit=2)), 2)


if __name__ == "__main__":
    unittest.main()
