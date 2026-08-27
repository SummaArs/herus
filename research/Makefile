.PHONY: test demo

PYTHON ?= python3

# Host-only research track. This target is intentionally not part of the
# firmware proof ledger until its semantics and resource costs are reviewed.
test:
	$(PYTHON) -m unittest -v test_finite_reasoner.py test_semantic_ir.py

demo:
	$(PYTHON) finite_reasoner.py
