# Herus simulated bench.
#
#   make          build and run every scenario
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
NET     := $(FW)/net/crypto.c $(FW)/net/session.c $(FW)/net/region.c \
           $(FW)/net/weave.c $(FW)/net/beat.c $(FW)/net/link.c
SIM     := world.c channel.c render.c node.c scenarios.c stress.c compose.c study.c learn.c main.c

.PHONY: all run sanitizers clean

all: run

run: build/herus-sim
	./build/herus-sim

# Diagnostic build: undefined behavior and memory corruption are release-blocking.
# LeakSanitizer is not available in the default Apple clang runtime, so disable
# leak reporting on macOS while keeping ASan/UBSan active on every platform.
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
SAN_ENV := ASAN_OPTIONS=detect_leaks=0
else
SAN_ENV := ASAN_OPTIONS=detect_leaks=1
endif

sanitizers:
	$(MAKE) clean
	$(MAKE) CC='$(CC) -fsanitize=address,undefined -fno-omit-frame-pointer -g' \
		CFLAGS='-O1 -Wall -Wextra -Werror=return-type -Werror=implicit-function-declaration -std=c11' \
		build/herus-sim
	$(SAN_ENV) ./build/herus-sim

build/herus-sim: $(SIM) sim.h $(CORE) $(NET) | build
	$(CC) $(CFLAGS) -DHV_LUT_POPCOUNT $(INC) $(SIM) $(CORE) $(NET) \
	    -o $@ $(LDLIBS)

build:
	@mkdir -p build

clean:
	rm -rf build
