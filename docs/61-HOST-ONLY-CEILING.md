# HERUS — Host-Only Ceiling

## Status

The host-only adaptive discovery track has reached its current measurable ceiling. HERUS can actively select finite probes, record their hypotheses and costs, construct partial host profiles, detect conflicting or changing observations, abstain under insufficient budget, and preserve authority as `NONE`.

## Evidence

The active discovery benchmark covers three structurally distinct hidden laboratory hosts. At 4096 bytes, all three reach full laboratory coverage without false positives. At 768 bytes, the resulting profiles remain partial and include abstention. At 192 bytes, every host abstains before promoting a complete capability set. The official suite passes 221 tests with one pre-existing skip.

## What is resolved host-only

- Finite active host discovery without receiving the complete profile.
- Cost-aware probe selection and deterministic experiment records.
- Explicit distinction between proven, unknown, absent and conflicting capabilities.
- Replay, session, digest, contradiction and drift refusal.
- Transfer and proposal-only Skill selection under a finite host contract.
- Zero discovered authority and zero allowed physical effects.

## What remains physically unproven

- Real MCU memory, timing, power, thermal and reset behavior.
- Radio interference, packet loss, antenna performance and SX1262 behavior.
- I2C behavior with a real DRV2605L and LRA.
- Physical discovery from board-visible capabilities rather than a laboratory oracle.
- Persistence and renegotiation across brownout, reboot and power loss.
- Human recognition of haptic patterns.

## Resolution boundary

The host-only ceiling is not a claim of universal adaptation or AGI. Further software work may improve language, memory, planning and domain transfer, but it cannot replace the measurements above. The next non-simulated claim requires the first physical host and the B1–B10 evidence protocol.
