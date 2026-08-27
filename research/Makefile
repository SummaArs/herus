.PHONY: test real-corpus real-corpus-sanitizers real-corpus-audit demo

PYTHON ?= python3
CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -Werror -std=c11

# Host-only research track. This target is intentionally not part of the
# firmware proof ledger until its semantics and resource costs are reviewed.
test:
	$(PYTHON) -m unittest -v test_finite_reasoner.py test_semantic_ir.py test_semantic_ir_fuzz.py test_semantic_ir_corpus.py test_convergence.py test_datasets_manifest.py

real-corpus:
	@test -f source_data/MIntRec/metadata/train.tsv
	@test -f source_data/MIntRec/metadata/test.tsv
	$(CC) $(CFLAGS) -I../firmware/core ../firmware/core/voice.c real_corpus_voice_runner.c -o /tmp/herus-real-corpus-runner
	/tmp/herus-real-corpus-runner source_data/MIntRec/metadata/train.tsv source_data/MIntRec/metadata/test.tsv | tee evidence/wide_cycle_04/real_corpus_voice_raw.txt
	rm -f /tmp/herus-real-corpus-runner

real-corpus-sanitizers:
	@test -f source_data/MIntRec/metadata/train.tsv
	@test -f source_data/MIntRec/metadata/test.tsv
	$(CC) -fsanitize=address,undefined -fno-omit-frame-pointer -O1 -g -Wall -Wextra -Werror -std=c11 -I../firmware/core ../firmware/core/voice.c real_corpus_voice_runner.c -o /tmp/herus-real-corpus-runner-san
	ASAN_OPTIONS=detect_leaks=1 /tmp/herus-real-corpus-runner-san source_data/MIntRec/metadata/train.tsv source_data/MIntRec/metadata/test.tsv | tee evidence/wide_cycle_04/real_corpus_runner_sanitizers_raw.txt
	rm -f /tmp/herus-real-corpus-runner-san

real-corpus-audit:
	$(PYTHON) analyze_mintrec_metadata.py

demo:
	$(PYTHON) finite_reasoner.py
