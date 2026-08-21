# HERUS text-transfer artifact v1

This directory contains a host-side, locally trained experiment. The frozen base is a compact character Transformer pretrained on the public WikiText-2 raw training split. The adapter is a rank-4 output delta trained on public HERUS design documents and a synthetic behavior-contract corpus.

The artifact is **not a hosted LLM**, is **not canonical LoRA**, and is **not yet a firmware-ready model**. It does not contain user memory, audio, identity, location, secrets, or action authority.

To reproduce the experiment from a clean checkout, install the local CPU training dependency, run `python3 tools/fetch_text_transfer_data.py`, run `python3 tools/prepare_text_corpus.py`, and then run `python3 tools/train_char_transfer.py`. The artifact gate is `python3 tools/test_text_transfer.py`; the global ledger invokes that gate without importing PyTorch.

The selected checkpoint was chosen on held-out tuning data with a public-corpus forgetting constraint. The committed metrics report is the result of the last local run and must be regenerated if the corpus, code, PyTorch version, or seed changes.

The generated sample remains visibly imperfect. The artifact therefore demonstrates a bounded transfer-learning signal, not conversational parity with a general LLM, reasoning competence, speech quality, hardware feasibility, or physical sovereignty.
