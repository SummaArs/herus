"""Public import surface for HERUS Symbiotic Learning.

The package exposes proposal-only learners. It never executes host actions,
opens network connections, or grants authority as a side effect of import.
"""
from research.symbiotic_learning import Episode, Proposal, SkillHypothesis, State, SymbioticLearner
from research.meta_symbiotic_learning import MetaProposal, MetaSymbioticLearner, Problem, VerifiedSolution

__all__ = [
    'Episode', 'Proposal', 'SkillHypothesis', 'State', 'SymbioticLearner',
    'MetaProposal', 'MetaSymbioticLearner', 'Problem', 'VerifiedSolution',
]
__version__ = '0.1.0'
