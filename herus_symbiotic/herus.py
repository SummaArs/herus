"""Single, didactic public facade for HERUS Symbiotic Learning."""
from __future__ import annotations
from typing import Any, Mapping, Sequence, Tuple
from herus_symbiotic.data_science import DataScienceSkill, MLPlan
from herus_symbiotic.programming import ProgrammingRequest, ProgrammingSkill, ProgrammingProposal
from research.meta_symbiotic_learning import MetaSymbioticLearner, Problem
from research.symbiotic_learning import Episode, Proposal, State, SymbioticLearner

class Herus:
    """One importable object for beginners and senior users.

    Every operation is local, deterministic where possible, proposal-only, and
    has no authority to execute code, contact the network, or mutate a host.
    """
    def __init__(self, *, max_observations: int = 32, max_cost: int = 32, max_risk: int = 0) -> None:
        self.learning = SymbioticLearner(max_observations=max_observations, max_cost=max_cost, max_risk=max_risk)
        self.meta = MetaSymbioticLearner(self.learning)
        self.data_science = DataScienceSkill()
        self.programming = ProgrammingSkill()

    def data(self, records: Tuple[Mapping[str, Any], ...], *, label: str = "label", objective: str = "") -> MLPlan:
        """Audit data and return a senior-level ML plan, never execute it."""
        return self.data_science.analyze(records, label_column=label, objective=objective)

    def program(self, goal: str, *, language: str = "python", constraints: Tuple[str, ...] = ()) -> ProgrammingProposal:
        """Propose a bounded programming solution with tests and questions."""
        return self.programming.propose(ProgrammingRequest(goal, language, constraints))

    def observe(self, before: Mapping[str, int], action: str, after: Mapping[str, int], **kwargs: Any) -> bool:
        """Record an observed episode; rejected observations fail closed."""
        return self.learning.observe(Episode.from_maps(before, action, after, **kwargs))

    def propose(self, target_effect: Mapping[str, int], *, context: Mapping[str, int] | None = None, cost_budget: int = 4, current_step: int = 0) -> Proposal:
        """Propose an action from observed evidence; never execute it."""
        state: State = tuple(sorted((str(k), int(v)) for k, v in target_effect.items()))
        ctx: State = tuple(sorted((str(k), int(v)) for k, v in (context or {}).items()))
        return self.learning.propose(state, self.learning.snapshot()[0], cost_budget=cost_budget, context=ctx, current_step=current_step)

    def inspect(self) -> dict[str, object]:
        """Return an explainable snapshot suitable for logs and teaching."""
        return {"version": self.learning.version, "observations": len(self.learning.snapshot()[0]), "skills": [h.__dict__ for h in self.learning.induce()], "authority": "none"}

    @staticmethod
    def help() -> str:
        return "Herus().data(...) audita ML; .program(...) planeja código; .observe(...) aprende evidência; .propose(...) propõe sem executar; .inspect() explica o estado."
