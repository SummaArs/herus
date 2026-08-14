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
# Thirty-two suites, each independently falsifiable:
#   1  algebra      quasi-orthogonality, bundling, resonator, learning, HCP
#   2  nucleus      bounded, opt-in local semantic intelligence
#   3  voice        controlled local language and bounded haptic feedback
#   4  intent       session, confidence, ambiguity and bounded context gateway
#   5  dialogue     bounded local conversation, transient privacy and zero authority
#   6  model-lab    target evidence, resource budget, adversarial rejection and reply shield
#   7  memory-policy selective relevance, review and no autonomous retention
#   8  memory-capture physical, bounded, one-shot and transient capture session
#   9  memory-extract typed, conservative, uncertain and non-retaining candidate extraction
#  10  memory-vault authorised encrypted card, durable generation and fail-closed erase
#  11  memory-consolidation bounded human review, conflict, recall and removal
#  12  memory-retrieval typed local matching, threshold, ambiguity and zero authority
#  13  memory-retrieval-present one-shot human status, uncertainty and zero authority
#  14  memory-finale composed private-memory chain, uncertainty and zero model authority
#  15  memory-collection bounded, authorised, transactional and anti-rollback collection
#  16  memory-collection-index bounded private typed queries, ambiguity and probe budget
#  17  memory-collection-recovery portable crash-state recovery oracle
#  18  memory-collection-finale human-gated multi-card composition and abstention
#  19  memory-physical-session purpose-bound, expiring and consumed collection access
#  20  threat-model executable host-evidence, target-pending and out-of-scope classifier
#  21  assurance    fail-closed cross-module composition and revocation precedence
#  22  capstone     dialogue, model, intent, trust and one-time-handoff attack chain
#  23  trust        explicit pairing, SAS, protected persistence and revocation
#  24  control-link authenticated Core/Nucleus control-envelope, expiry and replay protection
#  25  interaction  push-to-talk, confirmation, one-shot send and telemetry
#  26  validation   deterministic adapters and telemetry log gates
#  27  readiness    frozen hardware-evidence manifest and privacy/schema gate
#  28  provenance   unsigned local inputs, declared components and fail-closed gates
#  29  study        preregistered plan, statistical gates and unsafe-send rejection
#  30  protocol     crypto vs OpenSSL, ratchet, framing, Weave, Beat, canonicality
#  31  radio        SX1262 command sequences against a recording mock bus
#  32  physical     RF, energy and the frame ledger, from tools/budget.py
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

banner "1/32 algebra (hv + sbc + lexicon + hcp)"
( cd firmware && make algebra ) > /tmp/herus_a.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_a.log
grep -q "FAIL" /tmp/herus_a.log && FAIL=1 || true

banner "2/32 nucleus (bounded local semantic intelligence)"
( cd firmware && make nucleus ) > /tmp/herus_n.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_n.log
grep -q "FAIL" /tmp/herus_n.log && FAIL=1 || true

banner "3/32 voice (controlled language, confirmation, bounded haptics)"
( cd firmware && make voice ) > /tmp/herus_v.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_v.log
grep -q "FAIL" /tmp/herus_v.log && FAIL=1 || true

banner "4/32 intent gateway (session, confidence, ambiguity and bounded context)"
( cd firmware && make intent ) > /tmp/herus_t.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_t.log
grep -q "FAIL" /tmp/herus_t.log && FAIL=1 || true

banner "5/32 dialogue (bounded local conversation and zero send authority)"
( cd firmware && make dialogue ) > /tmp/herus_d.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_d.log
grep -q "FAIL" /tmp/herus_d.log && FAIL=1 || true

banner "6/32 model acceptance lab (target evidence, budgets and reply shield)"
( cd firmware && make model-lab ) > /tmp/herus_m.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_m.log
grep -q "FAIL" /tmp/herus_m.log && FAIL=1 || true

banner "7/32 memory policy (consent, relevance, review and no persistence)"
( cd firmware && make memory-policy ) > /tmp/herus_y.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_y.log
grep -q "FAIL" /tmp/herus_y.log && FAIL=1 || true

banner "8/32 memory capture (physical session, expiry and transient discard)"
( cd firmware && make memory-capture ) > /tmp/herus_z.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_z.log
grep -q "FAIL" /tmp/herus_z.log && FAIL=1 || true

banner "9/32 memory extract (typed candidate, uncertainty and zero retention)"
( cd firmware && make memory-extract ) > /tmp/herus_e.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_e.log
grep -q "FAIL" /tmp/herus_e.log && FAIL=1 || true

banner "10/32 memory vault (explicit authority, AEAD and durable anti-rollback)"
( cd firmware && make memory-vault ) > /tmp/herus_w.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_w.log
grep -q "FAIL" /tmp/herus_w.log && FAIL=1 || true

banner "11/32 memory consolidation (bounded human review, conflict, recall and removal)"
( cd firmware && make memory-consolidation ) > /tmp/herus_o.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_o.log
grep -q "FAIL" /tmp/herus_o.log && FAIL=1 || true

banner "12/32 memory retrieval (typed local matching, ambiguity and zero authority)"
( cd firmware && make memory-retrieval ) > /tmp/herus_u.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_u.log
grep -q "FAIL" /tmp/herus_u.log && FAIL=1 || true

banner "13/32 memory retrieval presentation (one-shot status, uncertainty and zero authority)"
( cd firmware && make memory-retrieval-present ) > /tmp/herus_p.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_p.log
grep -q "FAIL" /tmp/herus_p.log && FAIL=1 || true

banner "14/32 memory Grand Finale (composed private-memory chain and zero model authority)"
( cd firmware && make memory-finale ) > /tmp/herus_f.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_f.log
grep -q "FAIL" /tmp/herus_f.log && FAIL=1 || true

banner "15/32 memory collection (bounded transactional multi-card persistence)"
( cd firmware && make memory-collection ) > /tmp/herus_mc.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_mc.log
grep -q "FAIL" /tmp/herus_mc.log && FAIL=1 || true

banner "16/32 memory collection index (bounded private typed retrieval)"
( cd firmware && make memory-collection-index ) > /tmp/herus_mci.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_mci.log
grep -q "FAIL" /tmp/herus_mci.log && FAIL=1 || true

banner "17/32 memory collection recovery (portable crash-state oracle)"
( cd firmware && make memory-collection-recovery ) > /tmp/herus_mcr.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_mcr.log
grep -q "FAIL" /tmp/herus_mcr.log && FAIL=1 || true

banner "18/32 memory collection Grand Finale (human authority, abstention and no fallback)"
( cd firmware && make memory-collection-finale ) > /tmp/herus_mcf.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_mcf.log
grep -q "FAIL" /tmp/herus_mcf.log && FAIL=1 || true

banner "19/32 physical session (purpose-bound, expiring and consumed collection access)"
( cd firmware && make memory-physical-session ) > /tmp/herus_mps.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_mps.log
grep -q "FAIL" /tmp/herus_mps.log && FAIL=1 || true

banner "20/32 threat model (host evidence, target gaps and scope boundaries)"
( cd firmware && make threat-model ) > /tmp/herus_tm.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_tm.log
grep -q "FAIL" /tmp/herus_tm.log && FAIL=1 || true

banner "21/32 assurance (fail-closed composition and revocation precedence)"
( cd firmware && make assurance ) > /tmp/herus_q.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_q.log
grep -q "FAIL" /tmp/herus_q.log && FAIL=1 || true

banner "22/32 capstone (dialogue, model, interaction and trust chain)"
( cd firmware && make capstone ) > /tmp/herus_x.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_x.log
grep -q "FAIL" /tmp/herus_x.log && FAIL=1 || true

banner "23/32 trust lifecycle (explicit pairing, SAS and revocation)"
( cd firmware && make trust ) > /tmp/herus_k.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_k.log
grep -q "FAIL" /tmp/herus_k.log && FAIL=1 || true

banner "24/32 Core/Nucleus control link (AEAD, expiry and replay protection)"
( cd firmware && make control-link ) > /tmp/herus_l.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_l.log
grep -q "FAIL" /tmp/herus_l.log && FAIL=1 || true

banner "25/32 interaction (push-to-talk, confirmation and one-shot send)"
( cd firmware && make interaction ) > /tmp/herus_i.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_i.log
grep -q "FAIL" /tmp/herus_i.log && FAIL=1 || true

banner "26/32 validation lab (deterministic adapters and telemetry gates)"
( cd firmware && make interaction-rig && cd .. && ./tools/test_interactionlog.sh ) > /tmp/herus_g.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_g.log
grep -q "FAIL" /tmp/herus_g.log && FAIL=1 || true

banner "27/32 readiness manifest (frozen evidence and privacy gates)"
( python3 tools/readiness_audit.py research/hardware_readiness_manifest.json --strict && python3 tools/test_readiness_audit.py ) > /tmp/herus_h.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_h.log
grep -q "FAIL" /tmp/herus_h.log && FAIL=1 || true

banner "28/32 local provenance manifest (unsigned inputs and pending supply-chain gates)"
( python3 tools/provenance_audit.py research/software_provenance_manifest.json --strict && python3 tools/test_provenance_audit.py ) > /tmp/herus_pv.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_pv.log
grep -q "FAIL" /tmp/herus_pv.log && FAIL=1 || true

banner "29/32 preregistered study (frozen plan, gates and unsafe-send rejection)"
python3 tools/test_interactionstudy.py > /tmp/herus_s.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_s.log
grep -q "FAIL" /tmp/herus_s.log && FAIL=1 || true

banner "30/32 protocol (crypto, ratchet, framing, Weave, Beat)"
( cd firmware && make net ) > /tmp/herus_b.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_b.log
grep -q "FAIL" /tmp/herus_b.log && FAIL=1 || true

banner "31/32 radio driver (SX1262 command sequences, no hardware)"
( cd firmware && make radio && make syntax ) > /tmp/herus_r.log 2>&1 || FAIL=1
[ "$QUIET" = 0 ] && cat /tmp/herus_r.log
grep -q "FAIL" /tmp/herus_r.log && FAIL=1 || true

banner "32/32 physical layer, energy and frame ledger"
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

# --- selective memory policy --------------------------------------------
check "Memory policy requires consent and rejects ambiguity" "unconsented speech is discarded" /tmp/herus_y.log
check "Memory policy sends sensitive or third-party candidates to review" "never auto-retained" /tmp/herus_y.log

# --- explicit memory capture ---------------------------------------------
check "Memory capture is physical, one-shot, bounded and transient" "MEMORY CAPTURE INVARIANTS HOLD" /tmp/herus_z.log
check "Memory capture scrubs stale, late and failed buffers" "never consumed and is scrubbed immediately" /tmp/herus_z.log

# --- candidate extraction -----------------------------------------------
check "Memory extraction is typed, conservative and non-retaining" "MEMORY EXTRACT INVARIANTS HOLD" /tmp/herus_e.log
check "Memory extraction routes low confidence and third-party sensitive input safely" "forced to review" /tmp/herus_e.log

# --- encrypted memory vault ---------------------------------------------
check "Memory vault requires explicit human authority and excludes sensitive cards" "cannot persist without canonical explicit human authorization" /tmp/herus_w.log
check "Memory vault authenticates records and rejects durable rollback fail-closed" "MEMORY VAULT INVARIANTS HOLD" /tmp/herus_w.log

# --- human memory consolidation -----------------------------------------
check "Memory consolidation expires and cancels without autonomous retention" "an expired review scrubs the proposal and never turns timeout into retention" /tmp/herus_o.log
check "Memory consolidation blocks conflict and deletion failure without false success" "MEMORY CONSOLIDATION INVARIANTS HOLD" /tmp/herus_o.log

# --- controlled semantic retrieval ---------------------------------------
check "Memory retrieval requires a bounded typed query and physical access" "an unbounded query cannot enumerate every card in memory" /tmp/herus_u.log
check "Memory retrieval exposes ambiguity rather than choosing a close score" "MEMORY RETRIEVAL INVARIANTS HOLD" /tmp/herus_u.log

# --- retrieval human presentation ---------------------------------------
check "Retrieval presentation is physical, one-shot and scrubs transient state" "presentation is one-shot and cannot repeat a retrieval result silently" /tmp/herus_p.log
check "Retrieval presentation exposes uncertainty without selecting a contender" "MEMORY RETRIEVAL PRESENTATION INVARIANTS HOLD" /tmp/herus_p.log

# --- memory Grand Finale -----------------------------------------------
check "Memory Grand Finale composes capture, policy, review, vault, retrieval and presentation" "the entire authorised fixture chain is compositionally consistent" /tmp/herus_f.log
check "Memory Grand Finale blocks conflict, unsafe review and model agency" "MEMORY GRAND FINALE INVARIANTS HOLD" /tmp/herus_f.log

# --- multi-card memory collection ----------------------------------------
check "Memory collection is capacity-bounded, physically authorised and does not overwrite" "capacity exhaustion does not evict or overwrite" /tmp/herus_mc.log
check "Memory collection authenticates transactions and rejects rollback without false success" "MEMORY COLLECTION INVARIANTS HOLD" /tmp/herus_mc.log

# --- private collection index ---------------------------------------------
check "Collection index requires typed physical queries and limits repeated probes" "each physical session has a bounded probe budget" /tmp/herus_mci.log
check "Collection index preserves ambiguity and never auto-opens a card" "COLLECTION INDEX INVARIANTS HOLD" /tmp/herus_mci.log

# --- portable crash recovery ---------------------------------------------
check "Collection recovery discards a pre-floor prepared record without promoting it" "first prepared write before floor commit is discarded" /tmp/herus_mcr.log
check "Collection recovery promotes only an authenticated immediate successor bound to floor" "only floor-bound authenticated states recover" /tmp/herus_mcr.log

# --- collection Grand Finale ----------------------------------------------
check "Collection Grand Finale composes human authority, authenticated recovery and typed retrieval" "typed physical query returns only a minimal match" /tmp/herus_mcf.log
check "Collection Grand Finale preserves abstention and blocks auto-open, fallback and model agency" "COLLECTION FINALE INVARIANTS HOLD" /tmp/herus_mcf.log

# --- physical session gate -----------------------------------------------
check "Physical session binds purpose, expiry and one-time/bounded consumption" "PHYSICAL SESSION INVARIANTS HOLD" /tmp/herus_mps.log
check "Collection Finale requires purpose-bound session evidence and rejects legacy access" "collection uses a separate purpose-bound session gate" /tmp/herus_mcf.log

# --- executable threat model --------------------------------------------
check "Threat model classifies host controls only with complete canonical evidence" "THREAT MODEL INVARIANTS HOLD" /tmp/herus_tm.log
check "Threat model retains model agency, physical platform and unsupported scope as non-success" "future model loses mitigation classification" /tmp/herus_tm.log

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

# --- local software provenance -------------------------------------------
check "Local provenance manifest validates only declared unsigned inputs" "PROVENANCE MANIFEST VALID" /tmp/herus_pv.log
check "Local provenance rejects tampered inputs, secrets and trust escalation" "PROVENANCE AUDIT INVARIANTS HOLD" /tmp/herus_pv.log

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
