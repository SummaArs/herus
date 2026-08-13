#!/bin/sh
# prove.sh — regenerate and re-verify every number and every invariant.
#
# Doctrine: an asserted number is a rumour, a measured number next to its closed
# form is a result. Run this before trusting any figure in any document, before
# flashing anything, and after any change to the algebra, the protocol or the
# budget assumptions.
#
#   ./prove.sh            full run
#   ./prove.sh --quiet    verdict lines only
#
# Five suites, each independently falsifiable:
#   1  algebra      quasi-orthogonality, bundling, resonator, learning, HCP
#   2  nucleus      bounded, opt-in local semantic intelligence
#   3  protocol     crypto vs OpenSSL, ratchet, framing, Weave, Beat, canonicality
#   4  radio        SX1262 command sequences against a recording mock bus
#   5  physical     RF, energy and the frame ledger, from tools/budget.py
#
# The Nucleus suite is intentionally separate: privacy and non-autonomy are
# properties that must fail a build when regressed, not promises in a document.
set -e
cd "$(dirname "$0")"

QUIET=0
[ "$1" = "--quiet" ] && QUIET=1
say() { [ "$QUIET" = 0 ] && echo "$@" || true; }
banner() { say ""; say "=================================================="; say "$1"; say "=================================================="; }

FAIL=0
mkdir -p firmware/build

banner "1/4  algebra (hv + sbc + lexicon + hcp)"
( cd firmware && make algebra ) > /tmp/herus_a.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_a.log
grep -q "FAIL" /tmp/herus_a.log && FAIL=1 || true

banner "2/5  nucleus (bounded local semantic intelligence)"
( cd firmware && make nucleus ) > /tmp/herus_n.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_n.log
grep -q "FAIL" /tmp/herus_n.log && FAIL=1 || true

banner "3/5  protocol (crypto, ratchet, framing, Weave, Beat)"
( cd firmware && make net ) > /tmp/herus_b.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_b.log
grep -q "FAIL" /tmp/herus_b.log && FAIL=1 || true

banner "4/5  radio driver (SX1262 command sequences, no hardware)"
( cd firmware && make radio && make syntax ) > /tmp/herus_r.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_r.log
grep -q "FAIL" /tmp/herus_r.log && FAIL=1 || true

banner "5/5  physical layer, energy and frame ledger"
python3 tools/budget.py > /tmp/herus_c.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_c.log

echo ""
echo "--------------------------------------------------"
echo "INVARIANT CHECKS"
echo "--------------------------------------------------"

check() {
    if grep -q "$2" "$3" 2>/dev/null; then echo "  PASS  $1"
    else echo "  FAIL  $1"; FAIL=1; fi
}

# --- physical layer -------------------------------------------------------
check "P1 constant AIRTIME across meaning tiers" "INVARIANT HOLDS" /tmp/herus_c.log
check "P2 no frame exceeds the 400 ms dwell limit" "highest legal spreading factor for this frame: SF9" /tmp/herus_c.log

# --- algebra --------------------------------------------------------------
check "binding is exactly invertible"        "round-trip failures: 0" /tmp/herus_a.log
check "role order survives (errata E6)"      "roles now carry direction" /tmp/herus_a.log
check "9-slot composed record round-trips"   "9/9 recovered exactly" /tmp/herus_a.log

# --- nucleus --------------------------------------------------------------
check "Nucleus learning is opt-in, bounded and non-autonomous" "NUCLEUS INVARIANTS HOLD" /tmp/herus_n.log

# --- protocol -------------------------------------------------------------
check "crypto agrees with an independent implementation" "V6 AEAD encrypt+decrypt matches the reference" /tmp/herus_b.log
check "hypervectors are never transmitted"   "rebuild the identical 10240-bit vector from ids alone" /tmp/herus_b.log
check "P4 unknown roles are skipped, not rejected" "the message is usable, not discarded" /tmp/herus_b.log
check "every single-bit corruption is rejected" "rejects every single-bit corruption" /tmp/herus_b.log
check "Tier 0.5 does not avalanche"          "one bit in, one bit out" /tmp/herus_b.log
check "no stable identifier is on air (P6)"  "there is nothing to track" /tmp/herus_b.log
check "ttl is mutable, the address is not (E-P2)" "still verifies end to end" /tmp/herus_b.log
check "replay is refused forever"            "every frame is refused" /tmp/herus_b.log
check "forgery fails at 2^-64 per attempt"   "holds against every forgery attempt" /tmp/herus_b.log
check "the decrypt path is rate limited"     "means something" /tmp/herus_b.log
check "flooding terminates"                  "bounded by dedup" /tmp/herus_b.log
check "Beat guard covers crystal drift"      "fits inside the guard" /tmp/herus_b.log
check "encoding is canonical"                "a gap in the slot list is rejected" /tmp/herus_b.log

# --- radio ----------------------------------------------------------------
check "SetPacketType precedes SetModulationParams" "BEFORE SetModulationParams" /tmp/herus_r.log
check "the frequency word is correct"        "freq \* 2\^25 / 32 MHz" /tmp/herus_r.log
check "PA config and TX power agree"         "not a different one" /tmp/herus_r.log
check "the private sync word is written"     "not the SX1276-era" /tmp/herus_r.log
check "BUSY is honoured before every command" "before every single command" /tmp/herus_r.log
check "the selftest diagnoses a wrong pin map" "names the file to edit" /tmp/herus_r.log
check "the ESP32-S3 app type-checks"         "type-check" /tmp/herus_r.log

# P2 is stronger than one grep: no airtime in the ledger may be OVER.
if grep -q " OVER " /tmp/herus_c.log; then
    echo "  FAIL  P2 a frame in the ledger exceeds 400 ms"; FAIL=1
else
    echo "  PASS  P2 every frame in the ledger is within dwell"
fi

# ---------------------------------------------------------------- the bench
# The suites above prove properties of the CODE. The bench proves properties of
# the SYSTEM: the same code, plus distance, plus a duty cycle, plus other people
# transmitting, plus somebody hostile. It links firmware/core and firmware/net
# unmodified, so a delivery there is a real decode and not a model of one.
echo ""
echo "--- simulated bench: the firmware in a world with distance and adversaries ---"
if ( cd sim && make -s build/herus-sim ) 2>/tmp/herus_sim_build.log; then
    if ./sim/build/herus-sim > /tmp/herus_sim.log 2>&1; then
        SIMN=$(grep -c "  PASS" /tmp/herus_sim.log)
        echo "  PASS  $SIMN system invariants hold in simulation (sim/build/herus-sim)"
        grep -E "^  (measured cliff|delivered|hostile frames)" /tmp/herus_sim.log | head -3
    else
        echo "  FAIL  the bench disagrees with the design — see /tmp/herus_sim.log"; FAIL=1
    fi
else
    echo "  FAIL  the bench does not build — see /tmp/herus_sim_build.log"; FAIL=1
fi

echo ""
if [ "$FAIL" = 0 ]; then
    echo "ALL INVARIANTS HOLD — the documents reproduce and the firmware is safe to flash."
    exit 0
else
    echo "SOMETHING REGRESSED — do not flash, and do not trust the documents."
    exit 1
fi
