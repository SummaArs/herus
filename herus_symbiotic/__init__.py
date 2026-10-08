"""Public import surface for HERUS Symbiotic Learning.

The package exposes proposal-only learners. It never executes host actions,
opens network connections, or grants authority as a side effect of import.
"""
from research.symbiotic_learning import Episode, Proposal, SkillHypothesis, State, SymbioticLearner
from research.meta_symbiotic_learning import MetaProposal, MetaSymbioticLearner, Problem, VerifiedSolution
from herus_symbiotic.programming import ProgrammingProposal, ProgrammingQuestion, ProgrammingRequest, ProgrammingSkill, RepairProposal, TestFailure
from herus_symbiotic.programming_evidence import EvidenceRecord, LedgerResult, ProgrammingObligation, build_programming_ledger
from herus_symbiotic.data_science import DataIssue, DatasetProfile, DataScienceSkill, MLPlan
from research.symbiotic_policy import Candidate, Decision, HostBudget, PolicyFit, SymbioticDecisionPolicy, VerifiedFeedback
from herus_symbiotic.herus import Herus

__all__ = [
    'Episode', 'Proposal', 'SkillHypothesis', 'State', 'SymbioticLearner',
    'MetaProposal', 'MetaSymbioticLearner', 'Problem', 'VerifiedSolution',
    'ProgrammingProposal', 'ProgrammingQuestion', 'ProgrammingRequest', 'ProgrammingSkill', 'RepairProposal', 'TestFailure',
    'EvidenceRecord', 'LedgerResult', 'ProgrammingObligation', 'build_programming_ledger',
    'DataIssue', 'DatasetProfile', 'DataScienceSkill', 'MLPlan',
    'Candidate', 'Decision', 'HostBudget', 'PolicyFit', 'SymbioticDecisionPolicy', 'VerifiedFeedback', 'Herus',
]
__version__ = '0.1.0'
