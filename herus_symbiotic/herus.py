"""Single, didactic public facade for HERUS Symbiotic Learning."""
from __future__ import annotations
from typing import Any, Mapping, Sequence, Tuple
from herus_symbiotic.data_science import DataScienceSkill, MLPlan
from herus_symbiotic.programming import ProgrammingRequest, ProgrammingSkill, ProgrammingProposal
from research.meta_symbiotic_learning import MetaSymbioticLearner, Problem
from research.symbiotic_learning import Episode, Feedback, HostContract, MigrationResult, Proposal, State, SymbioticLearner, UpdateResult

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

    def propose(self, target_effect: Mapping[str, int], *, context: Mapping[str, int] | None = None, current_state: Mapping[str, int] | None = None, cost_budget: int = 4, current_step: int | None = 0, host: HostContract | None = None) -> Proposal:
        """Propose an action from observed evidence; never execute it."""
        state: State = tuple(sorted((str(k), int(v)) for k, v in target_effect.items()))
        ctx: State = tuple(sorted((str(k), int(v)) for k, v in (context or {}).items()))
        current: State | None = None if current_state is None else tuple(sorted((str(k), int(v)) for k, v in current_state.items()))
        return self.learning.propose(state, self.learning.snapshot()[0], cost_budget=cost_budget, context=ctx, current_step=current_step, current_state=current, host=host)

    def update(self, before: Mapping[str, int], action: str, after: Mapping[str, int], target_effect: Mapping[str, int], *, outcome: str = "positive", utility: float = 0.0, risk: float = 0.0, cost: float = 1.0, context: Mapping[str, int] | None = None, provenance: str = "public", verifier: str = "unspecified", step: int = 0, example_id: str = "", host: HostContract | None = None) -> UpdateResult:
        """Apply verified host feedback to the bounded learner; never execute."""
        to_state = lambda value: tuple(sorted((str(k), int(v)) for k, v in value.items()))
        feedback = Feedback(to_state(before), action, to_state(after), to_state(target_effect), to_state(context or {}), outcome, utility, risk, cost, 0.0, 0.0, provenance, verifier, step, example_id)
        return self.learning.update(feedback, host=host)

    def migrate(self, source: HostContract, target: HostContract) -> MigrationResult:
        """Plan a bounded host migration; incompatible evidence is quarantined."""
        return self.learning.migration_plan(source, target)

    def inspect(self) -> dict[str, object]:
        """Return an explainable snapshot suitable for logs and teaching."""
        return {"version": self.learning.version, "observations": len(self.learning.snapshot()[0]), "skills": [h.__dict__ for h in self.learning.induce()], "authority": "none"}

    @staticmethod
    def help() -> str:
        return "Herus().data(...) audita ML; .program(...) planeja código; .observe(...) aprende evidência; .update(...) aplica feedback verificado; .propose(...) propõe sem executar; .migrate(...) planeja migração sem transferir autoridade; .inspect() explica o estado."
