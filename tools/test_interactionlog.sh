#!/bin/sh
# test_interactionlog.sh — host proof for the Phase 3 telemetry gate.
set -eu
cd "$(dirname "$0")/.."
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
header='run_id,trial_id,scenario,source,expected,observed,button_ms,draft_ms,confirm_ms,send_ms,energy_uj,outcome'

{
  echo "$header"
  echo 'lab-a,001,arrival-10,core,arrive,arrive,1000,1520,1710,1720,18400,sent'
  echo 'lab-a,002,negative-noise,nucleus,none,none,0,0,0,0,11200,rejected'
} > "$tmp/valid.csv"

python3 tools/interactionlog.py --csv "$tmp/valid.csv" --strict > "$tmp/valid.out"
grep -q 'PASS  exactly one send hand-off per positive confirmation' "$tmp/valid.out"
grep -q 'PASS  zero false drafts in negative trials' "$tmp/valid.out"

{
  echo "$header"
  echo 'lab-b,001,illegal-send,core,arrive,arrive,1000,1500,0,1600,9000,sent'
} > "$tmp/invalid.csv"
if python3 tools/interactionlog.py --csv "$tmp/invalid.csv" --strict > "$tmp/invalid.out" 2>&1; then
  echo 'FAIL telemetry analyzer accepted send without confirmation'
  exit 1
fi
grep -q 'FAIL  all rows obey state/log invariants' "$tmp/invalid.out"
echo 'INTERACTIONLOG INVARIANTS HOLD — invalid telemetry cannot hide an unsafe send.'
