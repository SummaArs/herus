"""Meta-Symbiotic Learning: reusable verified solution memory.

The meta-layer learns how to adapt from prior verified episodes. It never
turns a historical solution into authority: every new host still requires
fresh public evidence and the wrapped SymbioticLearner may abstain.
"""
from __future__ import annotations
from dataclasses import dataclass, asdict
from hashlib import sha256
from typing import Mapping, Sequence
from symbiotic_learning import Episode, Proposal, State, SymbioticLearner, _digest, _state, _context_matches

@dataclass(frozen=True)
class Problem:
    goal_effect: State
    context: State = ()
    host_kind: str = "unknown"
    risk: int = 0

    @classmethod
    def from_maps(cls, goal_effect: Mapping[str, int], *, context: Mapping[str, int] | None = None, host_kind: str = "unknown", risk: int = 0) -> "Problem":
        return cls(_state(goal_effect), _state(context or {}), host_kind, risk)

    @property
    def digest(self) -> str:
        return _digest((self.goal_effect, self.context, self.host_kind, self.risk))

@dataclass(frozen=True)
class VerifiedSolution:
    solution_id: str
    problem_digest: str
    goal_effect: State
    context: State
    source_host_kind: str
    action: str
    evidence_digest: str
    verification: str = "verified"
    version: int = 0

@dataclass(frozen=True)
class MetaProposal:
    action: str | None
    status: str
    reason: str
    reference_id: str | None
    strategy: str
    fresh_evidence: bool
    confidence_milli: int = 0

class MetaSymbioticLearner:
    """Self-improving adaptation policy with immutable verified references."""
    def __init__(self, learner: SymbioticLearner | None = None) -> None:
        self.learner = learner or SymbioticLearner()
        self._history: list[VerifiedSolution] = []
        self._strategy_success: dict[str, int] = {}

    @property
    def history(self) -> tuple[VerifiedSolution, ...]:
        return tuple(self._history)

    def strategy(self, problem: Problem) -> str:
        if not self._history:
            return "bounded_effect_exploration"
        compatible = [r for r in self._history if r.goal_effect == problem.goal_effect and _context_matches(r.context, problem.context)]
        return "verified_reference_then_fresh_probe" if compatible else "analogy_then_bounded_effect_exploration"

    def references(self, problem: Problem) -> tuple[VerifiedSolution, ...]:
        matches = [r for r in self._history if r.goal_effect == problem.goal_effect and _context_matches(r.context, problem.context) and r.verification == "verified" and problem.risk <= self.learner.max_risk]
        return tuple(sorted(matches, key=lambda r: (r.source_host_kind != problem.host_kind, -r.version)))

    def rank_candidates(self, problem: Problem, candidates: Sequence[Episode]) -> tuple[Episode, ...]:
        """Order observed candidates using history; never removes evidence.

        Ranking is a search heuristic only. The verifier still sees every
        candidate and can abstain on aliases, drift, budget or risk.
        """
        refs = self.references(problem)
        known_actions = {ref.action for ref in refs}
        def score(episode: Episode) -> tuple[int, int, int]:
            context = 1 if _context_matches(problem.context, episode.context) else 0
            prior_action = 1 if episode.action in known_actions else 0
            return (context, prior_action, -episode.cost)
        return tuple(sorted(candidates, key=score, reverse=True))

    def remember(self, problem: Problem, proposal: Proposal, *, evidence_digest: str, verified: bool) -> VerifiedSolution | None:
        if not verified or proposal.status != "PROPOSE" or proposal.action is None or not evidence_digest:
            return None
        record = VerifiedSolution(_digest((problem.digest, proposal.action, evidence_digest, len(self._history))), problem.digest, problem.goal_effect, problem.context, problem.host_kind, proposal.action, evidence_digest, "verified", len(self._history) + 1)
        self._history.append(record)
        self._strategy_success["verified_reference_then_fresh_probe"] = self._strategy_success.get("verified_reference_then_fresh_probe", 0) + 1
        return record

    def adapt(self, problem: Problem, candidates: Sequence[Episode], *, cost_budget: int = 4, current_step: int = 0) -> MetaProposal:
        refs = self.references(problem)
        strategy = self.strategy(problem)
        ranked = self.rank_candidates(problem, candidates)
        proposal = self.learner.propose(problem.goal_effect, ranked, cost_budget=cost_budget, context=problem.context, current_step=current_step)
        if proposal.status != "PROPOSE":
            reason = "reference_requires_fresh_evidence" if refs else proposal.reason
            return MetaProposal(None, "ABSTAIN", reason, refs[0].solution_id if refs else None, strategy, False, 0)
        return MetaProposal(proposal.action, "PROPOSE", "fresh_evidence_matches_reference" if refs else "fresh_evidence_no_reference", refs[0].solution_id if refs else None, strategy, True, proposal.confidence_milli)

    def export(self) -> dict[str, object]:
        return {"algorithm": "meta-symbiotic-learning-v1", "history": [asdict(r) for r in self._history], "strategies": dict(self._strategy_success)}
