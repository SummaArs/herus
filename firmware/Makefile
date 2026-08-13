# Herus firmware — everything that can be proven on a host, proven on a host.
#
#   make            all five suites
#   make algebra    the two algebra suites (hv/sbc/lexicon)
#   make nucleus    the local, bounded semantic-intelligence suite
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

.PHONY: all algebra nucleus net radio syntax vectors clean

all: algebra nucleus net radio syntax
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
