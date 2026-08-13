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
# Seventeen suites, each independently falsifiable:
#   1  algebra      quasi-orthogonality, bundling, resonator, learning, HCP
#   2  nucleus      bounded, opt-in local semantic intelligence
#   3  voice        controlled local language and bounded haptic feedback
#   4  intent       session, confidence, ambiguity and bounded context gateway
#   5  dialogue     bounded local conversation, transient privacy and zero authority
#   6  model-lab    target evidence, resource budget, adversarial rejection and reply shield
#   7  assurance    fail-closed cross-module composition and revocation precedence
#   8  capstone     dialogue, model, intent, trust and one-time-handoff attack chain
#   9  trust        explicit pairing, SAS, protected persistence and revocation
#  10  control-link authenticated Core/Nucleus envelope, expiry and replay protection
#  11  interaction  push-to-talk, confirmation, one-shot send and telemetry
#  12  validation   deterministic adapters and telemetry log gates
#  13  readiness    frozen hardware-evidence manifest and privacy/schema gate
#  14  study        preregistered plan, statistical gates and unsafe-send rejection
#  15  protocol     crypto vs OpenSSL, ratchet, framing, Weave, Beat, canonicality
#  16  radio        SX1262 command sequences against a recording mock bus
#  17  physical     RF, energy and the frame ledger, from tools/budget.py
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

banner "1/16 algebra (hv + sbc + lexicon + hcp)"
( cd firmware && make algebra ) > /tmp/herus_a.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_a.log
grep -q "FAIL" /tmp/herus_a.log && FAIL=1 || true

banner "2/16 nucleus (bounded local semantic intelligence)"
( cd firmware && make nucleus ) > /tmp/herus_n.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_n.log
grep -q "FAIL" /tmp/herus_n.log && FAIL=1 || true

banner "3/16 voice (controlled language, confirmation, bounded haptics)"
( cd firmware && make voice ) > /tmp/herus_v.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_v.log
grep -q "FAIL" /tmp/herus_v.log && FAIL=1 || true

banner "4/16 intent gateway (session, confidence, ambiguity and bounded context)"
( cd firmware && make intent ) > /tmp/herus_t.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_t.log
grep -q "FAIL" /tmp/herus_t.log && FAIL=1 || true

banner "5/16 dialogue (bounded local conversation and zero send authority)"
( cd firmware && make dialogue ) > /tmp/herus_d.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_d.log
grep -q "FAIL" /tmp/herus_d.log && FAIL=1 || true

banner "6/16 model acceptance lab (target evidence, budgets and reply shield)"
( cd firmware && make model-lab ) > /tmp/herus_m.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_m.log
grep -q "FAIL" /tmp/herus_m.log && FAIL=1 || true

banner "7/16 assurance (fail-closed composition and revocation precedence)"
( cd firmware && make assurance ) > /tmp/herus_q.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_q.log
grep -q "FAIL" /tmp/herus_q.log && FAIL=1 || true

banner "8/16 capstone (dialogue, model, interaction and trust chain)"
( cd firmware && make capstone ) > /tmp/herus_x.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_x.log
grep -q "FAIL" /tmp/herus_x.log && FAIL=1 || true

banner "9/16 trust lifecycle (explicit pairing, SAS and revocation)"
( cd firmware && make trust ) > /tmp/herus_k.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_k.log
grep -q "FAIL" /tmp/herus_k.log && FAIL=1 || true

banner "10/16 Core/Nucleus control link (AEAD, expiry and replay protection)"
( cd firmware && make control-link ) > /tmp/herus_l.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_l.log
grep -q "FAIL" /tmp/herus_l.log && FAIL=1 || true

banner "11/16 interaction (push-to-talk, confirmation and one-shot send)"
( cd firmware && make interaction ) > /tmp/herus_i.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_i.log
grep -q "FAIL" /tmp/herus_i.log && FAIL=1 || true

banner "12/16 validation lab (deterministic adapters and telemetry gates)"
( cd firmware && make interaction-rig && cd .. && ./tools/test_interactionlog.sh ) > /tmp/herus_g.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_g.log
grep -q "FAIL" /tmp/herus_g.log && FAIL=1 || true

banner "13/16 readiness manifest (frozen evidence and privacy gates)"
( python3 tools/readiness_audit.py research/hardware_readiness_manifest.json --strict && python3 tools/test_readiness_audit.py ) > /tmp/herus_h.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_h.log
grep -q "FAIL" /tmp/herus_h.log && FAIL=1 || true

banner "14/16 preregistered study (frozen plan, gates and unsafe-send rejection)"
python3 tools/test_interactionstudy.py > /tmp/herus_s.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_s.log
grep -q "FAIL" /tmp/herus_s.log && FAIL=1 || true

banner "15/16 protocol (crypto, ratchet, framing, Weave, Beat)"
( cd firmware && make net ) > /tmp/herus_b.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_b.log
grep -q "FAIL" /tmp/herus_b.log && FAIL=1 || true

banner "16/17 radio driver (SX1262 command sequences, no hardware)"
( cd firmware && make radio && make syntax ) > /tmp/herus_r.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_r.log
grep -q "FAIL" /tmp/herus_r.log && FAIL=1 || true

banner "17/17 physical layer, energy and frame ledger"
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

# --- voice ---------------------------------------------------------------
check "Voice remains local, confirmed and haptically bounded" "VOICE/HAPTIC INVARIANTS HOLD" /tmp/herus_v.log

# --- intent gateway ------------------------------------------------------
check "Intent gateway is session-bound, confidence-gated and non-autonomous" "INTENT GATE INVARIANTS HOLD" /tmp/herus_t.log

# --- dialogue -------------------------------------------------------------
check "Dialogue retains no transcript and clears local UX reply" "retains no transcript and zeroizes" /tmp/herus_d.log
check "Dialogue output has zero transmission authority" "action-looking model text is only a local reply" /tmp/herus_d.log

# --- model acceptance lab ------------------------------------------------
check "Model lab requires target-measured local and identified weights" "host-only, connected or unidentified weights cannot enter production" /tmp/herus_m.log
check "Model lab rejects resource, network and authority regressions" "network attempt or authority escalation fails closed" /tmp/herus_m.log

# --- Grand Finale assurance and capstone -------------------------------
check "Assurance fails closed across physical, trust and model state" "ASSURANCE INVARIANTS HOLD" /tmp/herus_q.log
check "Capstone keeps dialogue and revoked trust out of handoff" "CAPSTONE INVARIANTS HOLD" /tmp/herus_x.log

# --- Core/Nucleus trust lifecycle ----------------------------------------
check "Trust requires physical pairing and matched SAS before activation" "pending offer has a six-digit SAS but cannot seal" /tmp/herus_k.log
check "Trust revocation zeroizes RAM and fails closed" "erase failure fails closed and zeroizes RAM" /tmp/herus_k.log

# --- Core/Nucleus control link -------------------------------------------
check "Control link authenticates, expires and rejects replay" "CORE LINK INVARIANTS HOLD" /tmp/herus_l.log

# --- interaction ---------------------------------------------------------
check "Interaction is push-to-talk, confirmed and one-shot" "INTERACTION INVARIANTS HOLD" /tmp/herus_i.log

# --- validation lab ------------------------------------------------------
check "Interaction rig keeps adapter sequencing non-transmitting" "INTERACTION RIG INVARIANTS HOLD" /tmp/herus_g.log
check "Telemetry gates reject an unsafe send" "INTERACTIONLOG INVARIANTS HOLD" /tmp/herus_g.log

# --- readiness manifest --------------------------------------------------
check "Hardware readiness manifest keeps evidence pending and private" "READINESS AUDIT INVARIANTS HOLD" /tmp/herus_h.log

# --- preregistered study -------------------------------------------------
check "Preregistered study keeps gates and rejects unsafe send" "PREREGISTRATION INVARIANTS HOLD" /tmp/herus_s.log

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
    echo "ALL INVARIANTS HOLD — host contracts pass; controlled bench flash may begin, physical gates remain pending."
    exit 0
else
    echo "SOMETHING REGRESSED — do not flash, and do not trust the documents."
    exit 1
fi
