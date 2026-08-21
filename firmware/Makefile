# Herus firmware — everything that can be proven on a host, proven on a host.
#
#   make            all host-verifiable suites
#   make algebra    the two algebra suites (hv/sbc/lexicon)
#   make nucleus    the local, bounded semantic-intelligence suite
#   make voice      the controlled-language and bounded-haptics suite
#   make intent     the session/confidence/ambiguity gateway suite
#   make intent-router the bounded text-intent, evidence and abstention suite
#   make dialogue   the bounded local conversational-intelligence suite
#   make model-lab  the measured local-model acceptance and reply-shield suite
#   make memory-policy the selective-memory relevance and retention-policy suite
#   make memory-capture the explicit, bounded and transient capture-session suite
#   make memory-extract the typed, conservative and non-retaining extraction suite
#   make watch-memory-frontend the Watch voice/relevance/physical-confirmation gate suite
#   make memory-vault the authorised, encrypted and anti-rollback memory vault suite
#   make memory-consolidation the bounded human review, recall and removal suite
#   make memory-retrieval the typed, local and ambiguity-safe presentation suite
#   make memory-retrieval-present the one-shot human status presentation suite
#   make memory-finale the selective-memory Grand Finale composition suite
#   make memory-collection the bounded transactional multi-card collection suite
#   make memory-collection-index the private bounded multi-card retrieval suite
#   make memory-collection-recovery the crash-state recovery-oracle suite
#   make memory-collection-recovery-stress the deterministic hostile collection-recovery campaign
#   make memory-collection-finale the human-gated multi-card composition suite
#   make bench-evidence-validate the privacy-preserving physical-evidence schema suite
#   make bench-interruption-sim the host-only interruption-plan simulator
#   make memory-proposal the typed local-model proposal compiler suite
#   make llm-budget-check the coarse local-model memory budget comparison
#   make memory-physical-session the purpose-bound physical-session gate suite
#   make memory-physical-session-recovery the durable reservation recovery oracle suite
#   make memory-physical-session-recovery-stress the deterministic hostile recovery/bootstrap campaign
#   make memory-physical-session-bootstrap the post-reboot session quarantine suite
#   make memory-reboot-boundary floor-only recovery and semantic-index quarantine
#   make memory-reboot-redteam deterministic stale-context recovery sabotage
#   make memory-post-reboot-reindex supersession, expiry and conflict after reindex
#   make memory-post-reboot-reindex-redteam deterministic silent-update sabotage
#   make memory-prehardware-finale the final host-only post-reboot memory-chain suite
#   make threat-model the executable evidence and residual-risk classification suite
#   make threat-model-stress the deterministic hostile evidence-classifier campaign
#   make proof-fire-mutations the deterministic selected-control mutation campaign
#   make assurance  the Grand Finale cross-module safety composition suite
#   make capstone   the Grand Finale end-to-end safety-chain suite
#   make trust      the explicit Core/Nucleus binding lifecycle suite
#   make control-link the authenticated Core/Nucleus control-envelope suite
#   make delivery-plan the bounded post-authorisation transport planner suite
#   make transport-selector the host-only multimode route-selection suite
#   make personal-telemetry the host-only consented personal-metric gate suite
#   make symbolic-reasoner the bounded generative symbolic reasoning suite
#   make resonator the bounded VSA factorization and cleanup suite
#   make semantic-compiler the controlled-language semantic compiler suite
#   make semantic-benchmark the fixed-corpus semantic equivalence benchmark
#   make knowledge-feed the authenticated, bounded and confirmation-gated Core feed
#   make knowledge-feed-cursor the durable anti-rollback journal suite
#   make memory-semantic-evidence the local temporal/conflict evidence index
#   make memory-reasoning-bridge the offline memory-to-reasoner composition suite
#   make magic-anticipation the explainable surprise and privacy-gate suite
#   make degradation-matrix capacity, conflict, ambiguity and consent composition
#   make degradation-redteam deterministic capacity/context sabotage
#   make magic-trigger the bounded attention-window and proposal-budget suite
#   make magic-dialogue-bridge the read-only dialogue presentation suite
#   make autonomy-policy the explicit A0–A4 initiative and confirmation policy suite
#   make autonomy-redteam the deterministic sabotage campaign for autonomy controls
#   make magic-redteam the deterministic sabotage campaign for magic and memory
#   make interaction the push-to-talk, confirmation and one-shot runtime suite
#   make cross-failure-matrix source loss, revocation and assurance composition
#   make cross-failure-redteam deterministic cross-module authority sabotage
#   make interaction-rig deterministic adapter and validation-lab scenarios
#   make net        the protocol suite (crypto, session, weave, beat, framing)
#   make radio      the SX1262 command sequences, against a recording mock bus
#   make syntax     type-check the ESP32-S3 app without an ESP32 present
#   make vectors    regenerate crypto vectors from an independent implementation
#   make clean
#
# HV_LUT_POPCOUNT is not optional in any timing target: neither the Xtensa LX7 nor
# the Cortex-M33 has a population-count instruction, so a benchmark using the x86
# POPCNT is not a projection, it is a fantasy.

CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra -std=c11
LDLIBS  := -lm
INC     := -Icore -Inet -Iport -Itest

CORE    := core/hv.c core/sbc.c core/lexicon.c core/hcp.c
NET     := net/crypto.c net/session.c net/region.c net/weave.c net/beat.c net/link.c

.PHONY: all algebra nucleus voice intent intent-router dialogue model-lab memory-policy memory-capture memory-extract watch-memory-frontend memory-vault memory-consolidation memory-retrieval memory-retrieval-present memory-finale memory-collection memory-collection-index memory-collection-recovery memory-collection-recovery-stress memory-collection-finale bench-evidence-validate bench-interruption-sim memory-proposal llm-budget-check memory-physical-session memory-physical-session-recovery memory-physical-session-recovery-stress memory-physical-session-bootstrap memory-reboot-boundary memory-reboot-redteam memory-post-reboot-reindex memory-post-reboot-reindex-redteam memory-prehardware-finale threat-model threat-model-stress proof-fire-mutations assurance capstone trust control-link delivery-plan transport-selector personal-telemetry symbolic-reasoner resonator semantic-compiler semantic-benchmark knowledge-feed knowledge-feed-cursor memory-semantic-evidence memory-reasoning-bridge magic-anticipation degradation-matrix degradation-redteam magic-trigger magic-dialogue-bridge autonomy-policy autonomy-redteam magic-redteam interaction interaction-rig net radio syntax vectors haptic-language haptic-language-matrix haptic-semantic-bridge haptic-adapter haptic-evidence haptic-target haptic-runner haptic-preflight cross-failure-matrix cross-failure-redteam clean

all: algebra nucleus voice intent intent-router dialogue model-lab memory-policy memory-capture memory-extract memory-vault memory-consolidation memory-retrieval memory-retrieval-present memory-finale memory-collection memory-collection-index memory-collection-recovery memory-collection-recovery-stress memory-collection-finale memory-physical-session memory-physical-session-recovery memory-physical-session-recovery-stress memory-physical-session-bootstrap memory-reboot-boundary memory-reboot-redteam memory-post-reboot-reindex memory-post-reboot-reindex-redteam memory-prehardware-finale threat-model threat-model-stress proof-fire-mutations assurance capstone trust control-link interaction interaction-rig net radio syntax haptic-language haptic-language-matrix haptic-semantic-bridge haptic-adapter haptic-evidence haptic-target haptic-runner haptic-preflight
	@echo ""
	@echo "=================================================="
	@echo "ALL FIRMWARE SUITES PASS"
	@echo "=================================================="

algebra: build
	@echo "--- algebra: hv + lexicon + hcp ---"
	$(CC) $(CFLAGS) -DHV_LUT_POPCOUNT $(INC) core/hv.c core/lexicon.c core/hcp.c \
	    core/test_herus.c -o build/test_herus $(LDLIBS)
	./build/test_herus
	@echo "--- algebra: dense vs sparse block codes ---"
	$(CC) $(CFLAGS) -DHV_LUT_POPCOUNT $(INC) core/hv.c core/sbc.c \
	    core/test_algebra.c -o build/test_algebra $(LDLIBS)
	./build/test_algebra

nucleus: build
	@echo "--- nucleus: bounded local semantic intelligence ---"
	$(CC) $(CFLAGS) $(INC) core/nucleus.c core/test_nucleus.c -o build/test_nucleus
	./build/test_nucleus

voice: build
	@echo "--- voice: controlled language and bounded haptics ---"
	$(CC) $(CFLAGS) $(INC) core/voice.c core/test_voice.c -o build/test_voice
	./build/test_voice

intent: build
	@echo "--- intent gateway: session, confidence, ambiguity and bounded context ---"
	$(CC) $(CFLAGS) $(INC) core/intent_gate.c core/test_intent_gate.c -o build/test_intent_gate
	./build/test_intent_gate

intent-router: build
	@echo "--- intent router: bounded text intent, evidence, abstention and confirmation ---"
	$(CC) $(CFLAGS) -Werror $(INC) core/intent_router.c core/test_intent_router.c -o build/test_intent_router
	./build/test_intent_router

dialogue: build
	@echo "--- dialogue: bounded local conversation and model authority boundary ---"
	$(CC) $(CFLAGS) $(INC) net/crypto.c core/dialogue.c core/test_dialogue.c -o build/test_dialogue
	./build/test_dialogue

model-lab: build
	@echo "--- model lab: target evidence, budgets and display-only reply shield ---"
	$(CC) $(CFLAGS) $(INC) net/crypto.c core/model_lab.c core/test_model_lab.c -o build/test_model_lab
	./build/test_model_lab

memory-policy: build
	@echo "--- memory policy: consent, relevance, review and no persistence ---"
	$(CC) $(CFLAGS) $(INC) core/memory_policy.c core/test_memory_policy.c -o build/test_memory_policy
	./build/test_memory_policy

memory-capture: build
	@echo "--- memory capture: physical session, timeout and transient discard ---"
	$(CC) $(CFLAGS) $(INC) core/memory_capture.c core/test_memory_capture.c -o build/test_memory_capture
	./build/test_memory_capture

memory-extract: build
	@echo "--- memory extract: typed candidate, uncertainty and zero retention ---"
	$(CC) $(CFLAGS) $(INC) core/memory_capture.c core/memory_policy.c core/memory_extract.c core/test_memory_extract.c -o build/test_memory_extract
	./build/test_memory_extract

watch-memory-frontend: build
	@echo "--- Watch memory front-end: voice filter, candidate expiry and physical confirmation ---"
	$(CC) $(CFLAGS) $(INC) core/watch_memory_frontend.c core/test_watch_memory_frontend.c -o build/test_watch_memory_frontend
	./build/test_watch_memory_frontend

memory-vault: build
	@echo "--- memory vault: explicit authority, AEAD and monotonic anti-rollback ---"
	$(CC) $(CFLAGS) $(INC) net/crypto.c core/memory_policy.c core/memory_vault.c core/test_memory_vault.c -o build/test_memory_vault
	./build/test_memory_vault

memory-consolidation: build
	@echo "--- memory consolidation: bounded human review, conflict, recall and removal ---"
	$(CC) $(CFLAGS) $(INC) net/crypto.c core/memory_policy.c core/memory_vault.c core/memory_consolidation.c core/test_memory_consolidation.c -o build/test_memory_consolidation
	./build/test_memory_consolidation

memory-retrieval: build
	@echo "--- memory retrieval: typed local matching, ambiguity and zero authority ---"
	$(CC) $(CFLAGS) $(INC) core/memory_policy.c core/memory_retrieval.c core/test_memory_retrieval.c -o build/test_memory_retrieval
	./build/test_memory_retrieval

memory-retrieval-present: build
	@echo "--- memory retrieval presentation: bounded status, uncertainty and zero authority ---"
	$(CC) $(CFLAGS) $(INC) core/voice.c core/memory_retrieval_present.c core/test_memory_retrieval_present.c -o build/test_memory_retrieval_present
	./build/test_memory_retrieval_present

memory-finale: build
	@echo "--- memory Grand Finale: composed human-gated private-memory chain ---"
	$(CC) $(CFLAGS) $(INC) net/crypto.c core/voice.c core/memory_capture.c core/memory_policy.c core/memory_extract.c core/memory_vault.c core/memory_consolidation.c core/memory_retrieval.c core/memory_retrieval_present.c core/memory_finale.c core/test_memory_finale.c -o build/test_memory_finale
	./build/test_memory_finale

memory-collection: build
	@echo "--- memory collection: bounded transactional multi-card persistence contract ---"
	$(CC) $(CFLAGS) $(INC) net/crypto.c core/memory_policy.c core/memory_physical_session.c core/memory_collection_recovery.c core/memory_collection.c core/test_memory_collection.c -o build/test_memory_collection
	./build/test_memory_collection

memory-collection-index: build
	@echo "--- memory collection index: bounded private typed retrieval ---"
	$(CC) $(CFLAGS) $(INC) net/crypto.c core/memory_policy.c core/memory_retrieval.c core/memory_physical_session.c core/memory_collection_recovery.c core/memory_collection.c core/memory_collection_index.c core/test_memory_collection_index.c -o build/test_memory_collection_index
	./build/test_memory_collection_index

memory-collection-recovery: build
	@echo "--- memory collection recovery: portable crash-state oracle ---"
	$(CC) $(CFLAGS) $(INC) core/memory_collection_recovery.c core/test_memory_collection_recovery.c -o build/test_memory_collection_recovery
	./build/test_memory_collection_recovery

memory-collection-recovery-stress: build
	@echo "--- collection recovery stress: deterministic hostile crash-state campaign ---"
	$(CC) $(CFLAGS) $(INC) core/memory_collection_recovery.c core/test_memory_collection_recovery_stress.c -o build/test_memory_collection_recovery_stress
	./build/test_memory_collection_recovery_stress

memory-collection-finale: build
	@echo "--- memory collection Grand Finale: human-gated multi-card composition ---"
	$(CC) $(CFLAGS) $(INC) net/crypto.c core/voice.c core/memory_capture.c core/memory_policy.c core/memory_extract.c core/memory_vault.c core/memory_consolidation.c core/memory_retrieval.c core/memory_retrieval_present.c core/memory_physical_session.c core/memory_collection_recovery.c core/memory_collection.c core/memory_collection_index.c core/memory_collection_finale.c core/test_memory_collection_finale.c -o build/test_memory_collection_finale
	./build/test_memory_collection_finale

bench-evidence-validate: build
	@echo "--- bench evidence: private numeric schema and digest validation ---"
	python3 ../tools/test_bench_evidence_validate.py

bench-interruption-sim: build
	@echo "--- bench interruption simulator: host-only oracle composition ---"
	$(CC) $(CFLAGS) $(INC) core/memory_collection_recovery.c core/memory_physical_session.c core/memory_physical_session_recovery.c core/memory_physical_session_bootstrap.c core/test_memory_bench_interruption_sim.c -o build/test_memory_bench_interruption_sim
	./build/test_memory_bench_interruption_sim

memory-proposal: build
	@echo "--- memory proposal: typed local-model compiler and fail-closed gate ---"
	$(CC) $(CFLAGS) $(INC) core/memory_capture.c core/memory_policy.c core/memory_extract.c core/memory_proposal.c core/test_memory_proposal.c -o build/test_memory_proposal
	./build/test_memory_proposal

llm-budget-check:
	@echo "--- local model sizing: coarse host-only memory comparison ---"
	python3 ../tools/test_llm_budget_check.py
	python3 ../tools/llm_budget_check.py --board lilygo-t3-s3-v1.3 --params 28900000 --bits 4 --artifact-bytes 14912332
	python3 ../tools/llm_budget_check.py --board esp32-s3-n16r8-article --params 28900000 --bits 4 --artifact-bytes 14912332

transport-selector: build
	@echo "--- transport selector: LoRa, urban and local route policy ---"
	$(CC) $(CFLAGS) $(INC) core/transport_selector.c core/test_transport_selector.c -o build/test_transport_selector
	./build/test_transport_selector

personal-telemetry: build
	@echo "--- personal telemetry: derived metrics, consent and transient retention ---"
	$(CC) $(CFLAGS) $(INC) core/personal_telemetry.c core/test_personal_telemetry.c -o build/test_personal_telemetry
	./build/test_personal_telemetry

symbolic-reasoner: build
	@echo "--- symbolic reasoner: bounded generative inference, proof and abstention ---"
	$(CC) $(CFLAGS) $(INC) core/symbol_registry.c core/symbolic_reasoner.c core/test_symbolic_reasoner.c -o build/test_symbolic_reasoner
	./build/test_symbolic_reasoner
	$(CC) $(CFLAGS) $(INC) core/symbol_registry.c core/symbolic_reasoner.c core/symbolic_planner.c core/test_symbolic_planner.c -o build/test_symbolic_planner
	./build/test_symbolic_planner
	$(CC) $(CFLAGS) -DHV_LUT_POPCOUNT $(INC) core/hv.c core/resonator.c core/resonator_bridge.c core/symbol_registry.c core/symbolic_reasoner.c core/symbolic_dialogue.c core/test_symbolic_dialogue.c -o build/test_symbolic_dialogue
	./build/test_symbolic_dialogue

semantic-compiler: build
	@echo "--- semantic compiler: bounded controlled-language to typed IR ---"
	$(CC) $(CFLAGS) -DHV_LUT_POPCOUNT $(INC) core/hv.c core/resonator.c core/resonator_bridge.c core/symbolic_reasoner.c core/symbolic_planner.c core/symbolic_dialogue.c core/symbol_registry.c core/semantic_compiler.c core/test_semantic_compiler.c -o build/test_semantic_compiler
	./build/test_semantic_compiler

semantic-benchmark: build
	@echo "--- semantic benchmark: exact IR, abstention and authority gates ---"
	$(CC) $(CFLAGS) -DHV_LUT_POPCOUNT $(INC) core/hv.c core/resonator.c core/resonator_bridge.c core/symbolic_reasoner.c core/symbolic_planner.c core/symbolic_dialogue.c core/symbol_registry.c core/semantic_compiler.c core/test_semantic_compiler_benchmark.c -o build/test_semantic_compiler_benchmark
	./build/test_semantic_compiler_benchmark

knowledge-feed: build
	@echo "--- knowledge feed: Core proposal, digest, rollback and local authority gates ---"
	$(CC) $(CFLAGS) -DHV_LUT_POPCOUNT $(INC) net/crypto.c core/hv.c core/resonator.c core/resonator_bridge.c core/symbol_registry.c core/symbolic_reasoner.c core/symbolic_dialogue.c core/knowledge_feed.c core/test_knowledge_feed.c -o build/test_knowledge_feed
	./build/test_knowledge_feed

knowledge-feed-cursor: build
	@echo "--- knowledge feed cursor: durable two-slot anti-rollback journal ---"
	$(CC) $(CFLAGS) $(INC) net/crypto.c core/knowledge_feed_cursor.c core/test_knowledge_feed_cursor.c -o build/test_knowledge_feed_cursor
	./build/test_knowledge_feed_cursor

knowledge-feed-cursor-powerfail: build
	@echo "--- knowledge feed cursor power-fail: interrupted writes and exhaustive bit corruption ---"
	$(CC) $(CFLAGS) $(INC) net/crypto.c core/knowledge_feed_cursor.c core/test_knowledge_feed_cursor_powerfail.c -o build/test_knowledge_feed_cursor_powerfail
	./build/test_knowledge_feed_cursor_powerfail

memory-semantic-evidence: build
	@echo "--- memory semantic evidence: bounded temporal facts and conflict abstention ---"
	$(CC) $(CFLAGS) $(INC) core/memory_semantic_evidence.c core/test_memory_semantic_evidence.c -o build/test_memory_semantic_evidence
	./build/test_memory_semantic_evidence

memory-reasoning-bridge: build
	@echo "--- memory reasoning bridge: offline composition, conflict and abstention ---"
	$(CC) $(CFLAGS) $(INC) core/memory_semantic_evidence.c core/memory_reasoning_bridge.c core/symbolic_reasoner.c core/test_memory_reasoning_bridge.c -o build/test_memory_reasoning_bridge
	./build/test_memory_reasoning_bridge

magic-anticipation: build
	@echo "--- magic anticipation: explainable surprise, privacy and abstention ---"
	$(CC) $(CFLAGS) $(INC) core/memory_semantic_evidence.c core/memory_reasoning_bridge.c core/symbolic_reasoner.c core/magic_anticipation.c core/test_magic_anticipation.c -o build/test_magic_anticipation
	./build/test_magic_anticipation

degradation-matrix: build
	@echo "--- degradation matrix: capacity, conflict, ambiguity, consent and Core absence ---"
	$(CC) $(CFLAGS) $(INC) core/memory_semantic_evidence.c core/memory_reasoning_bridge.c core/symbolic_reasoner.c core/magic_anticipation.c core/test_degradation_matrix.c -o build/test_degradation_matrix
	./build/test_degradation_matrix

degradation-redteam:
	@echo "--- degradation redteam: capacity and contextual-magic sabotage ---"
	python3 ../tools/test_degradation_redteam.py

magic-trigger: build
	@echo "--- magic trigger: bounded attention window and proposal budget ---"
	$(CC) $(CFLAGS) $(INC) core/memory_semantic_evidence.c core/memory_reasoning_bridge.c core/symbolic_reasoner.c core/magic_anticipation.c core/magic_trigger.c core/test_magic_trigger.c -o build/test_magic_trigger
	./build/test_magic_trigger

magic-dialogue-bridge: build
	@echo "--- magic dialogue bridge: read-only local presentation ---"
	$(CC) $(CFLAGS) $(INC) core/hv.c core/resonator.c core/resonator_bridge.c core/memory_semantic_evidence.c core/memory_reasoning_bridge.c core/symbolic_reasoner.c core/magic_anticipation.c core/magic_dialogue_bridge.c core/symbolic_dialogue.c core/test_magic_dialogue_bridge.c -o build/test_magic_dialogue_bridge
	./build/test_magic_dialogue_bridge

ambient-presence: build
	@echo "--- ambient presence: quiet, bounded and contact-aware local offer ---"
	$(CC) $(CFLAGS) $(INC) core/ambient_presence.c core/test_ambient_presence.c -o build/test_ambient_presence
	./build/test_ambient_presence

ambient-presence-redteam:
	@echo "--- ambient presence redteam: deterministic invisibility sabotage ---"
	python3 ../tools/test_ambient_presence_redteam.py

autonomy-policy: build
	@echo "--- autonomy policy: A0-A4 initiative and confirmation envelope ---"
	$(CC) $(CFLAGS) $(INC) core/autonomy_policy.c core/test_autonomy_policy.c -o build/test_autonomy_policy
	./build/test_autonomy_policy

autonomy-redteam:
	@echo "--- autonomy redteam: deterministic critical-control sabotage ---"
	python3 ../tools/test_autonomy_redteam.py

magic-redteam:
	@echo "--- magic/memory redteam: deterministic critical-control sabotage ---"
	python3 ../tools/test_magic_redteam.py

core-resilience-matrix:
	@echo "--- combined failures: Core absence, reboot, consent and bounded-memory composition ---"
	python3 ../tools/test_combined_failures.py

core-redteam:
	@echo "--- Core redteam: deterministic feed and cursor control sabotage ---"
	python3 ../tools/test_core_redteam.py

transport-redteam:
	@echo "--- transport redteam: deterministic haptic and SX1262 adapter sabotage ---"
	python3 ../tools/test_transport_redteam.py

symbol-registry-model:
	@echo "--- symbol registry model: versioned collision-aware host contract ---"
	python3 ../tools/test_symbol_registry_model.py

symbol-registry-c: build
	@echo "--- symbol registry C: portable versioned collision-aware handles ---"
	$(CC) $(CFLAGS) $(INC) core/symbol_registry.c core/test_symbol_registry.c -o build/test_symbol_registry
	./build/test_symbol_registry

haptic-language: build
	@echo "--- haptic language: DRV2605L-independent semantic frame encoder ---"
	$(CC) $(CFLAGS) $(INC) core/haptic_language.c core/test_haptic_language.c -o build/test_haptic_language
	./build/test_haptic_language

haptic-language-matrix: build
	@echo "--- haptic language matrix: 720 tuples per actuator, directed corruption and mismatch ---"
	$(CC) $(CFLAGS) $(INC) core/haptic_language.c core/test_haptic_language_matrix.c -o build/test_haptic_language_matrix
	./build/test_haptic_language_matrix

haptic-semantic-bridge: build
	@echo "--- haptic semantic bridge: presentation, authority and abstention ---"
	$(CC) $(CFLAGS) $(INC) core/haptic_language.c core/haptic_semantic_bridge.c core/test_haptic_semantic_bridge.c -o build/test_haptic_semantic_bridge
	./build/test_haptic_semantic_bridge

haptic-adapter: build
	@echo "--- haptic adapter: DRV2605L bus ordering, safety and fail-closed state ---"
	$(CC) $(CFLAGS) $(INC) core/haptic_language.c core/haptic_adapter.c core/test_haptic_adapter.c -o build/test_haptic_adapter
	./build/test_haptic_adapter

haptic-evidence: build
	@echo "--- haptic evidence: private numeric schema and fail-closed validator ---"
	python3 ../tools/test_haptic_bench_evidence_validate.py

haptic-runner: build
	@echo "--- haptic runner: no-hardware block and origin-bound evidence ---"
	python3 ../tools/test_haptic_bench_evidence_validate.py
	python3 ../tools/haptic_bench_runner.py --prepare-blocked build/haptic_bench_blocked.json
	python3 ../tools/haptic_bench_runner.py --validate build/haptic_bench_blocked.json

haptic-preflight: build
	@echo "--- haptic preflight: pre-energization checklist and no-hardware block ---"
	python3 ../tools/test_haptic_preflight.py
	python3 ../tools/haptic_preflight.py --prepare-blocked build/haptic_preflight_blocked.json
	python3 ../tools/haptic_preflight.py --validate build/haptic_preflight_blocked.json

haptic-target: build
	@echo "--- haptic target: disabled board gate and explicit I2C test override ---"
	$(CC) $(CFLAGS) -Itest/idf_stub -Icore -Iport/esp32s3 \
	    core/haptic_language.c core/haptic_adapter.c \
	    port/esp32s3/haptic_drv2605l_esp32s3.c \
	    port/esp32s3/test_haptic_target.c -o build/test_haptic_target_default
	./build/test_haptic_target_default
	$(CC) $(CFLAGS) -DBOARD_HAS_HAPTIC_I2C=1 -DPIN_HAPTIC_ENABLE=16 \
	    -DPIN_HAPTIC_ENABLE_VALID=1 -Itest/idf_stub -Icore -Iport/esp32s3 \
	    core/haptic_language.c core/haptic_adapter.c \
	    port/esp32s3/haptic_drv2605l_esp32s3.c \
	    port/esp32s3/test_haptic_target.c -o build/test_haptic_target_override
	./build/test_haptic_target_override

resonator: build
	@echo "--- resonator: bounded VSA factorization and cleanup ---"
	$(CC) $(CFLAGS) -DHV_LUT_POPCOUNT $(INC) core/hv.c core/resonator.c core/test_resonator.c -o build/test_resonator
	./build/test_resonator
	$(CC) $(CFLAGS) -DHV_LUT_POPCOUNT $(INC) core/hv.c core/resonator.c core/symbolic_reasoner.c core/resonator_bridge.c core/test_resonator_bridge.c -o build/test_resonator_bridge
	./build/test_resonator_bridge
	$(CC) $(CFLAGS) -DHV_LUT_POPCOUNT $(INC) core/hv.c core/resonator.c core/resonator_bridge.c core/test_resonator_stress.c -o build/test_resonator_stress
	./build/test_resonator_stress

memory-physical-session: build
	@echo "--- physical session: purpose-bound bounded replay gate ---"
	$(CC) $(CFLAGS) $(INC) core/memory_physical_session.c core/test_memory_physical_session.c -o build/test_memory_physical_session
	./build/test_memory_physical_session

memory-physical-session-recovery: build
	@echo "--- physical session recovery: durable reservation oracle ---"
	$(CC) $(CFLAGS) $(INC) core/memory_physical_session_recovery.c core/test_memory_physical_session_recovery.c -o build/test_memory_physical_session_recovery
	./build/test_memory_physical_session_recovery

memory-physical-session-recovery-stress: build
	@echo "--- recovery stress: deterministic hostile reservation and bootstrap campaign ---"
	$(CC) $(CFLAGS) $(INC) core/memory_physical_session.c core/memory_physical_session_recovery.c core/memory_physical_session_bootstrap.c core/test_memory_physical_session_recovery_stress.c -o build/test_memory_physical_session_recovery_stress
	./build/test_memory_physical_session_recovery_stress

memory-physical-session-bootstrap: build
	@echo "--- physical session bootstrap: post-reboot floor-only quarantine ---"
	$(CC) $(CFLAGS) $(INC) core/memory_physical_session.c core/memory_physical_session_recovery.c core/memory_physical_session_bootstrap.c core/test_memory_physical_session_bootstrap.c -o build/test_memory_physical_session_bootstrap
	./build/test_memory_physical_session_bootstrap

memory-reboot-boundary: build
	@echo "--- memory reboot boundary: floor-only recovery and semantic-index quarantine ---"
	$(CC) $(CFLAGS) $(INC) core/memory_semantic_evidence.c core/memory_physical_session.c core/memory_physical_session_recovery.c core/memory_physical_session_bootstrap.c core/symbolic_reasoner.c core/memory_reasoning_bridge.c core/magic_anticipation.c core/magic_trigger.c core/memory_reboot_boundary.c core/test_memory_reboot_boundary.c -o build/test_memory_reboot_boundary
	./build/test_memory_reboot_boundary

memory-reboot-redteam:
	@echo "--- memory reboot redteam: stale-context and partial-recovery sabotage ---"
	python3 ../tools/test_memory_reboot_redteam.py

memory-post-reboot-reindex: build
	@echo "--- post-reboot reindex: supersession, expiry, conflict and abstention ---"
	$(CC) $(CFLAGS) $(INC) core/memory_semantic_evidence.c core/test_memory_post_reboot_reindex.c -o build/test_memory_post_reboot_reindex
	./build/test_memory_post_reboot_reindex

memory-post-reboot-reindex-redteam:
	@echo "--- post-reboot reindex redteam: stale and silent-update sabotage ---"
	python3 ../tools/test_post_reboot_reindex_redteam.py



memory-prehardware-finale: build
	@echo "--- pre-hardware Grand Finale: post-reboot memory-chain composition ---"
	$(CC) $(CFLAGS) $(INC) net/crypto.c core/memory_policy.c core/memory_physical_session.c core/memory_physical_session_recovery.c core/memory_physical_session_bootstrap.c core/memory_collection_recovery.c core/memory_collection.c core/memory_retrieval.c core/memory_collection_index.c core/memory_collection_finale.c core/threat_model.c core/memory_prehardware_finale.c core/test_memory_prehardware_finale.c -o build/test_memory_prehardware_finale
	./build/test_memory_prehardware_finale

threat-model: build
	@echo "--- threat model: evidence-scoped mitigation and residual-risk boundaries ---"
	$(CC) $(CFLAGS) $(INC) core/threat_model.c core/test_threat_model.c -o build/test_threat_model
	./build/test_threat_model

threat-model-stress: build
	@echo "--- threat-model stress: deterministic hostile evidence campaign ---"
	$(CC) $(CFLAGS) $(INC) core/threat_model.c core/test_threat_model_stress.c -o build/test_threat_model_stress
	./build/test_threat_model_stress

proof-fire-mutations: build
	@echo "--- proof-fire mutations: selected critical controls must be detected when removed ---"
	python3 ../tools/test_proof_fire_mutations.py

assurance: build
	@echo "--- assurance: Grand Finale composition, revocation and zero agency ---"
	$(CC) $(CFLAGS) $(INC) core/assurance.c core/test_assurance.c -o build/test_assurance
	./build/test_assurance

capstone: build
	@echo "--- capstone: Grand Finale dialogue, model, interaction and trust chain ---"
	$(CC) $(CFLAGS) $(INC) net/crypto.c net/core_link.c net/trust.c core/voice.c core/intent_gate.c core/interaction.c core/dialogue.c core/model_lab.c core/assurance.c core/test_capstone.c -o build/test_capstone
	./build/test_capstone

trust: build
	@echo "--- trust lifecycle: explicit pairing, protected storage and revocation ---"
	$(CC) $(CFLAGS) $(INC) net/crypto.c net/core_link.c net/trust.c net/test_trust.c -o build/test_trust
	./build/test_trust

control-link: build
	@echo "--- Core/Nucleus control link: AEAD, expiry and replay protection ---"
	$(CC) $(CFLAGS) $(INC) net/crypto.c net/core_link.c net/test_core_link.c -o build/test_core_link
	./build/test_core_link

delivery-plan: build
	@echo "--- delivery plan: bounded transport retry after explicit authorisation ---"
	$(CC) $(CFLAGS) $(INC) core/delivery_plan.c core/test_delivery_plan.c -o build/test_delivery_plan
	./build/test_delivery_plan

interaction: build
	@echo "--- interaction: push-to-talk, confirmation and one-shot send gate ---"
	$(CC) $(CFLAGS) $(INC) net/crypto.c net/core_link.c core/voice.c core/intent_gate.c core/assurance.c core/interaction.c core/test_interaction.c -o build/test_interaction
	./build/test_interaction

cross-failure-matrix: build
	@echo "--- cross failure matrix: source loss, revocation, assurance and one-shot handoff ---"
	$(CC) $(CFLAGS) $(INC) net/crypto.c net/core_link.c core/voice.c core/intent_gate.c core/assurance.c core/interaction.c core/test_cross_failure_matrix.c -o build/test_cross_failure_matrix
	./build/test_cross_failure_matrix

cross-failure-redteam:
	@echo "--- cross failure redteam: authority and source-loss sabotage ---"
	python3 ../tools/test_cross_failure_redteam.py

interaction-rig: build
	@echo "--- interaction rig: deterministic adapter and validation-lab scenarios ---"
	$(CC) $(CFLAGS) $(INC) net/crypto.c net/core_link.c core/voice.c core/intent_gate.c core/assurance.c core/interaction.c core/interaction_rig.c core/test_interaction_rig.c -o build/test_interaction_rig
	./build/test_interaction_rig

net: build
	@echo "--- protocol: crypto, ratchet, framing, weave, beat ---"
	$(CC) $(CFLAGS) -DHV_LUT_POPCOUNT $(INC) test/test_net.c $(NET) $(CORE) \
	    -o build/test_net $(LDLIBS)
	./build/test_net

radio: build
	@echo "--- SX1262 command sequences (mock bus, no hardware) ---"
	$(CC) $(CFLAGS) $(INC) test/test_radio.c port/sx1262.c net/region.c \
	    -o build/test_radio
	./build/test_radio

# Catches what would otherwise be twenty minutes of bench time: name collisions
# with newlib (mode_t, link), wrong types, missing includes. It does NOT catch an
# ESP-IDF API change — only the real build does that.
syntax:
	@echo "--- ESP32-S3 app: host type-check against stub headers ---"
	$(CC) -fsyntax-only -std=c11 -Wall -Wextra -Wno-unused-parameter \
	    -DHV_LUT_POPCOUNT -include unistd.h -include sys/types.h \
	    -Itest/idf_stub $(INC) -Iport/esp32s3 \
	            port/esp32s3/main/app_main.c port/esp32s3/hal_esp32s3.c \
        port/esp32s3/haptic_drv2605l_esp32s3.c

	@echo "    ok — app_main.c and hal_esp32s3.c type-check"

vectors:
	python3 ../tools/gen_vectors.py > test/vectors.h
	@echo "regenerated test/vectors.h from Python cryptography (OpenSSL)"

build:
	@mkdir -p build

clean:
	rm -rf build test_herus test_algebra
