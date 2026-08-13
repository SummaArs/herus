# Herus firmware — everything that can be proven on a host, proven on a host.
#
#   make            all eighteen suites
#   make algebra    the two algebra suites (hv/sbc/lexicon)
#   make nucleus    the local, bounded semantic-intelligence suite
#   make voice      the controlled-language and bounded-haptics suite
#   make intent     the session/confidence/ambiguity gateway suite
#   make dialogue   the bounded local conversational-intelligence suite
#   make model-lab  the measured local-model acceptance and reply-shield suite
#   make memory-policy the selective-memory relevance and retention-policy suite
#   make memory-capture the explicit, bounded and transient capture-session suite
#   make memory-extract the typed, conservative and non-retaining extraction suite
#   make memory-vault the authorised, encrypted and anti-rollback memory vault suite
#   make memory-consolidation the bounded human review, recall and removal suite
#   make memory-retrieval the typed, local and ambiguity-safe presentation suite
#   make memory-retrieval-present the one-shot human status presentation suite
#   make memory-finale the selective-memory Grand Finale composition suite
#   make assurance  the Grand Finale cross-module safety composition suite
#   make capstone   the Grand Finale end-to-end safety-chain suite
#   make trust      the explicit Core/Nucleus binding lifecycle suite
#   make control-link the authenticated Core/Nucleus control-envelope suite
#   make interaction the push-to-talk, confirmation and one-shot runtime suite
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

.PHONY: all algebra nucleus voice intent dialogue model-lab memory-policy memory-capture memory-extract memory-vault memory-consolidation memory-retrieval memory-retrieval-present memory-finale assurance capstone trust control-link interaction interaction-rig net radio syntax vectors clean

all: algebra nucleus voice intent dialogue model-lab memory-policy memory-capture memory-extract memory-vault memory-consolidation memory-retrieval memory-retrieval-present memory-finale assurance capstone trust control-link interaction interaction-rig net radio syntax
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

interaction: build
	@echo "--- interaction: push-to-talk, confirmation and one-shot send gate ---"
	$(CC) $(CFLAGS) $(INC) net/crypto.c net/core_link.c core/voice.c core/intent_gate.c core/assurance.c core/interaction.c core/test_interaction.c -o build/test_interaction
	./build/test_interaction

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
	    port/esp32s3/main/app_main.c port/esp32s3/hal_esp32s3.c
	@echo "    ok — app_main.c and hal_esp32s3.c type-check"

vectors:
	python3 ../tools/gen_vectors.py > test/vectors.h
	@echo "regenerated test/vectors.h from Python cryptography (OpenSSL)"

build:
	@mkdir -p build

clean:
	rm -rf build test_herus test_algebra
