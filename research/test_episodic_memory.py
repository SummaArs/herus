from __future__ import annotations

import unittest

from episodic_memory import Episode, abstract_repeated


class EpisodicMemoryTests(unittest.TestCase):
    def test_episode_id_is_deterministic(self) -> None:
        left = Episode.create("robotics", "host-a", "stop", "hazard", "stop", "e1")
        right = Episode.create("robotics", "host-a", "stop", "hazard", "stop", "e1")
        self.assertEqual(left.episode_id, right.episode_id)

    def test_repeated_observation_across_domains_becomes_abstraction(self) -> None:
        episodes = [
            Episode.create("robotics", "host-r", "avoid", "hazard", "stop", "e-r"),
            Episode.create("critical", "host-c", "avoid", "hazard", "stop", "e-c"),
        ]
        abstractions = abstract_repeated(episodes)
        self.assertEqual(len(abstractions), 1)
        self.assertEqual(abstractions[0].domains, ("critical", "robotics"))
        self.assertEqual(abstractions[0].consequence, "stop")

    def test_untrusted_episode_is_not_abstracted(self) -> None:
        episode = Episode.create("robotics", "host-r", "avoid", "hazard", "stop", "e-r")
        rejected = Episode(episode.episode_id, episode.domain, episode.host_digest, episode.goal, episode.observation, episode.outcome, episode.evidence_digest, False)
        self.assertEqual(abstract_repeated([episode, rejected]), ())

    def test_single_domain_does_not_claim_transfer(self) -> None:
        episodes = [
            Episode.create("robotics", "host-r1", "avoid", "hazard", "stop", "e-r1"),
            Episode.create("robotics", "host-r2", "avoid", "hazard", "stop", "e-r2"),
        ]
        self.assertEqual(abstract_repeated(episodes), ())


if __name__ == "__main__":
    unittest.main()
