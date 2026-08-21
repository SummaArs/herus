# Herus simulated bench.
#
#   make          build and run every scenario
#   make virtual  run only the deterministic pre-hardware virtual lab
#   make virtual-mutation  kill deliberately unsafe contract mutants
#   make build/herus-sim
#   ./build/herus-sim relay --verbose 1
#
# The firmware sources are compiled from ../firmware UNMODIFIED. There is no
# second copy and no simulator-only variant: if this binary links, the code the
# scenarios exercise is the code that would be flashed. Anything else would make
# a passing run meaningless.
#
# HV_LUT_POPCOUNT matches firmware/Makefile for the same reason it does there —
# neither the Xtensa LX7 nor the Cortex-M33 has a popcount instruction, so a
# timing figure taken with the host's POPCNT is a fantasy, not a projection.

CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra -std=c11
LDLIBS  := -lm

FW      := ../firmware
INC     := -I$(FW)/core -I$(FW)/net -I$(FW)/port

CORE    := $(FW)/core/hv.c $(FW)/core/sbc.c $(FW)/core/lexicon.c $(FW)/core/hcp.c $(FW)/core/text.c
VCORE   := $(FW)/core/transport_selector.c $(FW)/core/personal_telemetry.c \
           $(FW)/core/ambient_presence.c $(FW)/core/memory_semantic_evidence.c
NET     := $(FW)/net/crypto.c $(FW)/net/session.c $(FW)/net/region.c \
           $(FW)/net/weave.c $(FW)/net/beat.c $(FW)/net/link.c
SIM     := world.c channel.c render.c node.c scenarios.c stress.c compose.c study.c learn.c virtual_lab.c personal_scenario.c personal_sim.c semantic_scenario.c semantic_life.c physical_fault_scenario.c authority_transition.c authority_scenario.c authority_benchmark.c main.c

.PHONY: all run personal semantic-life physical-faults authority authority-benchmark authority-redteam virtual virtual-mutation clean

all: run

run: build/herus-sim
	./build/herus-sim

personal: build/herus-sim
	./build/herus-sim personal

semantic-life: build/herus-sim
	./build/herus-sim semantic-life

physical-faults: build/herus-sim
	./build/herus-sim physical-faults

authority: build/herus-sim
	./build/herus-sim authority

authority-benchmark: build/herus-sim
	./build/herus-sim authority-benchmark

authority-redteam:
	cd .. && python3 tools/test_authority_transition_redteam.py

virtual: build/herus-sim
	./build/herus-sim virtual

virtual-mutation:
	cd .. && python3 tools/virtual_mutation.py

build/herus-sim: $(SIM) sim.h $(CORE) $(VCORE) $(NET) | build
	$(CC) $(CFLAGS) -DHV_LUT_POPCOUNT $(INC) $(SIM) $(CORE) $(VCORE) $(NET) \
	    -o $@ $(LDLIBS)

build:
	@mkdir -p build

clean:
	rm -rf build
