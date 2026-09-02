.PHONY: test hcae-sink-audit call-path-audit policy-refinement assurance-certificate assurance-case memory-vault-assurance memory-vault-extraction guardian-model sygus-subset-benchmark real-corpus real-corpus-sanitizers real-corpus-audit paired-audio-audit slurp-text-audit slurp-text-sanitizers minds14-sample-audit minds14-batch-audit minds14-batch-sanitizers free-reasoning-embedded-profile free-reasoning-structured-profile critical-state-profile demo

PYTHON ?= python3
CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -Werror -std=c11

# Host-only research track. This target is intentionally not part of the
# firmware proof ledger until its semantics and resource costs are reviewed.
test:
	$(PYTHON) -m unittest -v test_finite_reasoner.py test_semantic_ir.py test_semantic_ir_fuzz.py test_semantic_ir_corpus.py test_convergence.py test_annotation_protocol.py test_evolution_policy.py test_evolution_compatibility.py test_generative_lab.py free_reasoner.test_structured_terms free_reasoner.test_structured_search test_critical_systems_health.py test_causal_surprise.py test_critical_state_verifier.py test_critical_state_synthesis.py test_critical_state_benchmark.py test_critical_state_refinement.py test_critical_policy_refinement.py test_critical_assurance_certificate.py test_critical_assurance_case.py test_memory_vault_assurance_case.py test_memory_vault_structural_extractor.py test_sygus_compatibility.py test_sygus_lia_subset.py test_critical_sink_audit.py test_critical_runtime_guardian.py test_critical_call_path_audit.py test_datasets_manifest.py test_paired_audio_audit.py test_source_audit.py test_probe_remote_zip.py test_benchmark_ir.py test_slurp_text.py test_minds14_sample.py test_minds14_batch.py test_identity_audit.py

hcae-sink-audit:
		$(PYTHON) critical_sink_audit.py hcae_profile.json --root ..

call-path-audit:
		$(PYTHON) critical_call_path_audit.py critical_call_path_profile.json --root ..

policy-refinement:
		$(PYTHON) -m unittest -v test_critical_policy_refinement.py

assurance-certificate:
		$(PYTHON) -m unittest -v test_critical_assurance_certificate.py

assurance-case:
		$(PYTHON) run_assurance_case.py evidence/reference_assurance_case.json --output evidence/reference_assurance_case.result.json

memory-vault-assurance:
		$(PYTHON) run_assurance_case.py evidence/memory_vault_assurance_case.json --output evidence/memory_vault_assurance_case.result.json

memory-vault-extraction:
		$(PYTHON) run_memory_vault_extraction.py ../firmware/core/memory_vault.c evidence/memory_vault_assurance_case.json --output evidence/memory_vault_structural_extraction.result.json

guardian-model:
		$(PYTHON) -m unittest -v test_critical_runtime_guardian.py

sygus-subset-benchmark:
		@test -d "$(SYGUS_ROOT)" || (echo "missing SYGUS_ROOT; clone SyGuS-Org/benchmarks separately" >&2; exit 2)
		$(PYTHON) run_sygus_subset_benchmark.py

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

paired-audio-audit:
	@test -n "$(FSC_ROOT)" || (echo "missing FSC_ROOT; no Fluent package was downloaded" >&2; exit 2)
	@test -n "$(FSC_CSV)" || (echo "missing FSC_CSV; no Fluent metadata file was selected" >&2; exit 2)
	$(PYTHON) paired_audio_audit.py --manifest datasets_manifest.json --dataset fluent_speech_commands --csv "$(FSC_CSV)" --data-root "$(FSC_ROOT)" $(if $(FSC_LIMIT),--limit $(FSC_LIMIT),)

real-corpus-audit:
	$(PYTHON) analyze_mintrec_metadata.py

slurp-text-audit:
	@test -n "$(SLURP_ROOT)" || (echo "missing SLURP_ROOT; no SLURP repository was provided" >&2; exit 2)
	@test -f "$(SLURP_ROOT)/dataset/slurp/train.jsonl"
	@test -f "$(SLURP_ROOT)/dataset/slurp/devel.jsonl"
	@test -f "$(SLURP_ROOT)/dataset/slurp/test.jsonl"
	@test -f "$(SLURP_METADATA)" || (echo "missing SLURP_METADATA; metadata member was not extracted by bounded probe" >&2; exit 2)
	$(CC) $(CFLAGS) -I../firmware/core ../firmware/core/voice.c real_jsonl_voice_runner.c -o /tmp/herus-slurp-voice-runner
	$(PYTHON) run_slurp_voice.py --runner /tmp/herus-slurp-voice-runner --output evidence/wide_cycle_05/slurp_voice_parser_audit.json "$(SLURP_ROOT)/dataset/slurp/train.jsonl" "$(SLURP_ROOT)/dataset/slurp/devel.jsonl" "$(SLURP_ROOT)/dataset/slurp/test.jsonl"
	$(PYTHON) analyze_slurp_text.py --train "$(SLURP_ROOT)/dataset/slurp/train.jsonl" --devel "$(SLURP_ROOT)/dataset/slurp/devel.jsonl" --test "$(SLURP_ROOT)/dataset/slurp/test.jsonl" --metadata "$(SLURP_METADATA)" --output evidence/wide_cycle_05/slurp_text_audit.json
	rm -f /tmp/herus-slurp-voice-runner

slurp-text-sanitizers:
	@test -n "$(SLURP_ROOT)" || (echo "missing SLURP_ROOT; no SLURP repository was provided" >&2; exit 2)
	@test -f "$(SLURP_ROOT)/dataset/slurp/train.jsonl"
	@test -f "$(SLURP_ROOT)/dataset/slurp/devel.jsonl"
	@test -f "$(SLURP_ROOT)/dataset/slurp/test.jsonl"
	$(CC) -fsanitize=address,undefined -fno-omit-frame-pointer -O1 -g -Wall -Wextra -Werror -std=c11 -I../firmware/core ../firmware/core/voice.c real_jsonl_voice_runner.c -o /tmp/herus-slurp-voice-runner-san
		ASAN_OPTIONS=detect_leaks=1 $(PYTHON) run_slurp_voice.py --runner /tmp/herus-slurp-voice-runner-san --output evidence/wide_cycle_05/slurp_voice_parser_sanitizers.json "$(SLURP_ROOT)/dataset/slurp/train.jsonl" "$(SLURP_ROOT)/dataset/slurp/devel.jsonl" "$(SLURP_ROOT)/dataset/slurp/test.jsonl"

	ASAN_OPTIONS=detect_leaks=1 /tmp/herus-slurp-voice-runner-san < /dev/null > /dev/null
	rm -f /tmp/herus-slurp-voice-runner-san

minds14-sample-audit:
	@test "$(MINDS14_ALLOW_AUDIO)" = "1" || (echo "missing MINDS14_ALLOW_AUDIO=1; explicit local research audio gate" >&2; exit 2)
	$(CC) $(CFLAGS) -I../firmware/core ../firmware/core/voice.c real_jsonl_voice_runner.c -o /tmp/herus-minds14-parser
	$(PYTHON) fetch_minds14_sample.py --c-runner /tmp/herus-minds14-parser --output-audio /tmp/herus-minds14-sample.wav --output-json evidence/wide_cycle_06/minds14_single_sample_audit.json
	rm -f /tmp/herus-minds14-parser /tmp/herus-minds14-sample.wav

minds14-batch-audit:
	@test "$(MINDS14_ALLOW_AUDIO)" = "1" || (echo "missing MINDS14_ALLOW_AUDIO=1; explicit local research audio gate" >&2; exit 2)
	$(CC) $(CFLAGS) -I../firmware/core ../firmware/core/voice.c real_jsonl_voice_runner.c -o /tmp/herus-minds14-parser
	$(PYTHON) fetch_minds14_batch.py --c-runner /tmp/herus-minds14-parser --offsets "$${MIND14_OFFSETS:-0,100,500,600}" --output evidence/wide_cycle_06/minds14_batch_audit.json
	rm -f /tmp/herus-minds14-parser

minds14-batch-sanitizers:
	@test "$(MINDS14_ALLOW_AUDIO)" = "1" || (echo "missing MINDS14_ALLOW_AUDIO=1; explicit local research audio gate" >&2; exit 2)
	$(CC) -fsanitize=address,undefined -fno-omit-frame-pointer -O1 -g -Wall -Wextra -Werror -std=c11 -I../firmware/core ../firmware/core/voice.c real_jsonl_voice_runner.c -o /tmp/herus-minds14-parser-san
	ASAN_OPTIONS=detect_leaks=1 $(PYTHON) fetch_minds14_batch.py --c-runner /tmp/herus-minds14-parser-san --offsets "$${MIND14_OFFSETS:-0,100,500,600}" --output evidence/wide_cycle_06/minds14_batch_sanitizers.json
	rm -f /tmp/herus-minds14-parser-san

generative-lab:
		$(PYTHON) -m generative_lab.benchmark

generative-lab-scale:
		$(PYTHON) -m generative_lab.measure

free-reasoning-embedded-profile:
			$(CC) $(CFLAGS) free_reasoner/embedded_profile.c -o /tmp/herus-free-reasoning-profile
			/tmp/herus-free-reasoning-profile
			rm -f /tmp/herus-free-reasoning-profile

free-reasoning-structured-profile:
				$(CC) $(CFLAGS) free_reasoner/structured_embedded_profile.c -o /tmp/herus-free-reasoning-structured-profile
				/tmp/herus-free-reasoning-structured-profile
				rm -f /tmp/herus-free-reasoning-structured-profile

critical-state-profile:
				$(CC) $(CFLAGS) critical_state_embedded_profile.c -o /tmp/herus-critical-state-profile
				/tmp/herus-critical-state-profile
				rm -f /tmp/herus-critical-state-profile

demo:
		$(PYTHON) finite_reasoner.py
