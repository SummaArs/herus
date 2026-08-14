# 03 — Build Guide

**Everything you have to do, in order, from an empty bench to a working Herus.**

Revision 0.2 · read [00-HERUS-MASTER.md](00-HERUS-MASTER.md) once before starting,
then work from this file.

---

## How to use this guide

Every phase has the same five parts, and you should not start a phase without
reading all five:

| Part | Meaning |
|---|---|
| **Goal** | the one sentence that says what changes when this phase ends |
| **Buy** | what you need in hand before you start |
| **Do** | numbered steps, with exact commands |
| **Done when** | the observable that proves the phase is finished |
| **Kill criterion** | the number that means *stop and change the plan* |

**About kill criteria.** They are the point of this ordering. A kill criterion is
not pessimism, it is a pre-committed decision made while you are calm, so that a
bad measurement three months from now costs you a pivot instead of a year. Write
the number down *before* you measure. If you find yourself arguing with a kill
criterion after the fact, that is the exact moment it is doing its job.

**Phase ordering rationale.** Your draft plan ended with the enclosure and the
antenna. Those are the two things most likely to kill the project, and they were
scheduled after months of firmware. This guide inverts that: **Phase 0 spends one
weekend attacking both, before anything else is built.**

Rough calendar, assuming evenings and weekends: Phase 0 is one weekend, Phases
1–3 are about six weeks, Phase 4 adds three weeks of which two are fab lead time,
Phase 5 two weeks, Phase 6 two weeks. Call it **four months to a working
ten-unit mesh.** Certification, if you ever sell it, is a separate project.

---

## Phase −1 — Bench setup

**Goal.** Every number in this repository reproduces on your Mac in under two
minutes, so you can trust the documents.

**Buy.** Nothing.

**Do.**

1. Make sure the compiler and Python are present. macOS ships Python 3; `cc`
   comes with the Command Line Tools. The protocol suite also needs an independent
   crypto implementation to test against:

   ```bash
   xcode-select --install 2>/dev/null; cc --version && python3 --version && pip3 install cryptography
   ```

2. Prove everything. One command, thirty-nine suites, 87 proof invariants:

   ```bash
   ./prove.sh
   ```

**Done when.** `prove.sh` exits 0 and prints `ALL INVARIANTS HOLD`. That covers the
algebra (T1–T11), the opt-in bounded Nucleus predictor, controlled voice/haptic contracts, the session-bound intent-confidence gateway, bounded dialogue, local-model acceptance policy, selective-memory relevance policy, explicit transient memory-capture session, typed non-retaining candidate extraction, the authorised encrypted memory-vault contract, bounded human memory consolidation, controlled typed memory retrieval, one-shot human retrieval presentation, the composed memory Grand Finale, the bounded transactional multi-card collection, its private abstention-safe index, its crash-state recovery oracle, its human-gated collection Grand Finale, its purpose-bound collection physical-session gate, its durable session-reservation recovery oracle that cannot revive active authority, its floor-only post-reboot session-bootstrap quarantine, its final host-only memory-chain composition, its deterministic F1–F4 proof-of-fire campaigns for recovery, collection, threat scope and selected-control mutation detection, its unsigned local provenance audit and the executable threat-model classifier, Grand Finale assurance and capstone chain, the explicit Core↔Nucleus trust lifecycle and authenticated control envelope, the confirmed interaction runtime, deterministic adapter/telemetry gates, the frozen hardware-readiness manifest, the preregistered study method, the protocol (crypto vs OpenSSL, ratchet, replay, forgery,
flooding, Beat drift), the SX1262 command sequences against a mock bus, and every RF
and energy figure in these documents.

**Kill criterion.** None — if this fails it is a bug in the repository, not in
the project. Fix it before continuing, because every later decision leans on
these numbers.

> **Why start here.** Two independent implementations of this algebra were
> written and they cross-validate: σ measured 50.55 against a theoretical 50.60,
> and both suites independently reproduce the 365 m / 650 m range figures. That
> agreement is the reason you can trust the rest of the document instead of
> re-deriving it.

---

## Phase 0 — The weekend that decides the project

**Goal.** Answer the only two questions that can kill Herus outright, in two
days, for about R$ 400 — before writing product firmware.

- **Q1.** Does a sub-GHz radio work *on a wrist* in your city?
- **Q2.** How much range does a header-less, CRC-less frame actually buy? (This
  is the entire Tier 0.5 thesis.)

**Buy.**

| Item | Qty | Note |
|---|---|---|
| LilyGO T3-S3 (ESP32-S3 + SX1262, 915 MHz) | 2 | the exact board `firmware/ranger` targets |
| 915 MHz antennas that came with them | 2 | do not substitute yet |
| USB-C cables | 2 | one must reach a phone or laptop you can carry |
| 150 mAh LiPo, ~30×20×3.8 mm, with protection circuit | 1 | for the volume test |
| Piezo bender, ~20 mm diameter, ≤0.6 mm thick | 1 | for the volume test |
| 3D print or have printed: the 30×30×10 mm shell | 1 | plus a 28×28 mm piece of 1 mm scrap as a dummy PCB |
| Tape measure or a mapped route | 1 | Google Maps' measure tool is fine |
| Stopwatch | 1 | your phone |

### Day 1, morning — the volume test (3 hours)

This is the cheapest possible test of the hardest mechanical constraint, and it
either confirms or kills the form factor before you design anything.

**Do.**

1. Print the shell at 30 × 30 × 10 mm, walls 0.8 mm, with an O-ring groove.
2. Stack, in this order, and close the lid: dummy PCB (1.0 mm) → LiPo (3.8 mm) →
   piezo (0.6 mm).
3. Measure the closed height with calipers.
4. Check the two faces are respected: the solar window faces *out*, the piezo
   sits against the *skin* side. They must not compete for the same face — that
   is the whole reason the stack works (see 00-HERUS-MASTER §9).

**Done when.** The lid closes and you can read a real closed height.

**Kill criterion.** **> 12 mm needed → the 30×30 form factor changes.** Do not
negotiate with this. Your options, in order of preference: 34 × 34 × 10 (still a
watch), or a puck/clip that is not a watch at all. Note that a clip is *also* the
Q2 fallback, so if this fails you should suspect the answer is the clip.

### Day 1, afternoon — flash the ranger (2 hours)

**Do.**

1. Install the Arduino IDE, then in Boards Manager add `esp32` by Espressif, and
   in Library Manager install **RadioLib**.
2. Open `firmware/ranger/ranger.ino`.
3. **Verify the pin map against your board revision.** LilyGO has shipped
   several T3-S3 pinouts. A wrong `BUSY` or `DIO1` gives a silent `begin()`
   failure that looks exactly like a dead radio, and it will cost you an
   afternoon. Cross-check the `#define PIN_*` block against the schematic for
   your revision.
4. Flash board A with `ROLE_TX` + `MODE_PDR`, board B with `ROLE_RX` +
   `MODE_PDR` (edit the two `#define`s at the top).
5. On the bench, one metre apart, confirm the RX board prints rows ending in
   `ok` at a steady 2 s cadence with RSSI around −40 dBm.

**Done when.** You see clean CSV at 2 s intervals. If not, fix it now — you
cannot debug a radio while walking.

### Day 2 — the walk (4 hours, both passes)

The measurement needs no GPS and no display. Frames go out on a strict 2.000 s
cadence, so **elapsed time is your distance bookkeeping.**

**Do.**

1. Pick a real route in dense urban clutter — the environment you actually want
   this to work in, not a park. Mark distances on it beforehand: 0, 50, 100, 150,
   200, 300, 400 m.
2. Leave the **RX** board stationary at a table or windowsill, logging to a file:

   ```bash
   python3 -m serial.tools.miniterm /dev/cu.usbmodem* 115200 | tee walk_pdr.csv
   ```

3. **Strap the TX board to your wrist**, antenna included, and hold your arm as
   you normally would. This is the measurement — a board in your hand at head
   height is a different experiment and it will flatter you.
4. Start the stopwatch as the TX powers up. Walk the route. **Note the stopwatch
   time at each distance mark.**
5. Reflash both boards to `MODE_PROBE` and walk the **same route with the same
   marks**, logging to `walk_probe.csv`.
6. Analyse:

   ```bash
   python3 tools/fieldlog.py --rx walk_pdr.csv --probe walk_probe.csv \
       --marks 0:0,45:50,95:100,150:150,215:200,290:300,380:400
   ```

   (Replace the marks with your actual stopwatch times.)

**Done when.** `fieldlog.py` prints a verdict for Q1 and Q2.

**Kill criteria.** Both are evaluated for you by the tool.

- **Q1: < 150 m urban at ≥50% PDR → the wrist is the wrong place for a sub-GHz
  radio.** This is not a firmware problem and no protocol work fixes it. Pivot
  the Core to a lapel or backpack clip: better height, no body block. The modular
  capsule was always the hedge for exactly this, and your draft's "tactical clip"
  may turn out to be the primary product rather than an accessory.
- **Q2: < 2 dB equivalent gain → drop Tier 0.5** and keep CRC'd ids. No loss of
  function, only of novelty.

**Optional, high value if you have time.** Repeat pass 1 with the antenna taped
along a strap instead of standing off the board, to measure the Band-antenna
gain. The model predicts about +10 dB across the link, which nearly doubles
range — the single biggest lever in the system, and the biggest unknown, because
contact impedance through pogo pins varies.

> **Record every number from this weekend in a file you keep.** Every later
> antenna, enclosure and PCB change is measured as a delta against this baseline.
> Without the baseline you will not know whether a change helped.

---

## Phase 1 — Protocol on devkits (2–3 weeks)

**Goal.** Two devkits exchange real Herus frames: derived lexicon, constant
34-byte framing, ratchet, Tier 0/1 end to end.

**Buy.** Nothing new.

**Do.**

1. Move from Arduino to **ESP-IDF** in VS Code. You need `deep sleep`,
   `SetRxDutyCycle` and eFuse control later, and the Arduino layer fights you on
   all three.
2. **This is already written.** `firmware/net/` and `firmware/port/` contain the
   whole stack, and `./prove.sh` verifies 87 proof invariants over the full repository before you flash
   anything — read [05-FIRMWARE.md](05-FIRMWARE.md) and go straight to §6, which
   takes you from an unopened board to a meaning crossing a street. The order below
   is retained because it is the order to DEBUG in if something does not work, and
   because each step is testable alone:
   1. **Lexicon** — port `hv.c` and `sbc.c`; confirm two boards generate
      byte-identical codes from one 64-bit domain seed. This is the cheapest
      possible confidence check on the whole "derived, never transmitted" idea.
   2. **Framing** — the constant 34-byte frame from
      [02-PROTOCOL.md](02-PROTOCOL.md) §3. Pad the plaintext to 22 bytes
      **before** encryption.
   3. **Crypto** — ChaCha20-Poly1305 with an 8-byte tag, nonce derived from the
      chain key and counter, never transmitted. Add the receive-side rate limit
      immediately; the truncated tag assumes it.
   4. **Tier 1** — the 9-slot composed record, `role:5 | filler:11`. Test that a
      slot with an unknown role id is *skipped, not rejected* (rule P4).
      Forward compatibility is a two-line property and a nightmare to retrofit.
   5. **Ratchet + ephemeral addresses** — precompute a 32-address window and
      filter before attempting decryption.
3. Do **not** write your own mesh routing yet. Flash Meshtastic onto a third
   board and use it as a baseline for PDR and latency. Reinventing mesh routing
   is a six-month detour that teaches you nothing about Herus.
4. Record a wrist-worn voice sample in a street and run it through Codec2 700C
   offline on your Mac. Listen to it.

**Done when.** A Tier 1 message composed on board A is rendered as text on board
B's serial console, at 300 m, with the frame constant at 34 bytes.

**Kill criteria.**
- Tier 0 round trip > 3 s, or PDR < 80% at 300 m → the protocol timing is wrong;
  revisit Beat before adding features.
- **Codec2 700C is unintelligible** on your street recording → cut Tier 2 now,
  before it shapes the PCB. A voice tier nobody can understand is worse than no
  voice tier, and 700 bps is genuinely marginal.

---

## Phase 2 — Algebra on target (1–2 weeks)

**Goal.** The measured host numbers hold on real silicon.

**Do.**

1. Cross-compile `hv.c` / `sbc.c` and run T1–T11 on the ESP32-S3. Compare
   against the host figures.
2. Benchmark the two-stage SBC search on target. Host measures **6.4 µs**
   exhaustive over L=512 with an MCU-realistic byte-table popcount, and the
   two-stage path is 0.14× of that at **100.00% agreement** with exhaustive.
3. Wire the memory cascade: L1 context (20 KB) → L2 episodic (20 KB) → L3
   semantic SBC prototypes (32 KB). **Total 72 KB.** Watch it with
   `heap_caps_get_free_size` — your draft budgeted 500 KB and the chip only has
   512 KB total, with IDF and the radio stack already taking 150–250 KB.
4. Implement learning with the **separation invariant**: after each update,
   recompute the minimum pairwise prototype distance and **refuse the update** if
   it would fall below the floor. Measured on host: 33 of 40 adversarially
   mislabelled updates refused, separation held at 3013 bits against a 3000-bit
   floor. Without this guard the device can be talked into anything by whoever
   is holding it.
5. Build the demo: **teach a new symbol in three repetitions.** No cloud, no
   dataset, no retraining. This is the thing that will make people understand the
   project in ten seconds.

**Done when.** T1–T11 pass on target, cognition fits in 72 KB, and the
three-repetition demo works.

**Kill criterion.** Measured MCU search > 10× the host-scaled projection → the
two-algebra split needs revisiting (00-HERUS-MASTER §4.3) before you commit the
PCB.

---

## Phase 3 — Power (1 week)

**Goal.** Prove the leaf budget, which is the entire basis of the autonomy claim.

**Buy.** BQ25570 breakout, a small flexible solar cell (~4 cm² usable), the
150 mAh cell, and — this matters — a **power analyser**, not a multimeter. You
are measuring a 74 µA average punctuated by 45 mA bursts; a multimeter cannot
see that. A Nordic PPK2 or a Joulescope is the right tool.

**Do.**

1. Get the ESP32-S3 into deep sleep at ~15 µA. Measure it. Chase down every
   peripheral that keeps a rail alive; this step is usually where a week goes.
2. Implement `SX1262 SetRxDutyCycle` — 20 ms RX every 2 s, radio-autonomous, MCU
   asleep through every empty window. Do **not** wake the MCU to run CAD: 43 200
   wakes a day costs ~1.8 mAh/day, more than the radio you were trying to help.
3. Measure the average. Target from `budget.py` §5: **74 µA → 1.78 mAh/day.**
4. Add the BQ25570 and cell. Measure harvest indoors and in real sun. Expect the
   documented result: 5.4× surplus in 1.5 h of sun, and **1% of need** at 300 lux
   indoors. Indoor light is a trickle, not a power source.
5. Implement the three Weave roles and the transitions between them
   (02-PROTOCOL §5.1).

**Done when.** Leaf idle measures ≤ 100 µA and a full day of ordinary use stays
under 5 mAh.

**Kill criterion.** **Leaf idle > 200 µA → the perpetual-solar story is dead.**
Do not abandon the project; reframe honestly as "weeks of autonomy" and keep
building. The pogo pins were always the primary charge path.

---

## Phase 4 — PCB (3–4 weeks, two of them waiting)

**Goal.** A 28 × 28 mm four-layer board that fits the shell.

**Do.**

1. KiCad, 28 × 28 mm, four layers. Use **pre-certified modules**, not bare
   chips: `ESP32-S3-MINI-1` and an SX1262 module. This saves you RF matching
   agony and makes certification enormously cheaper later.
2. RF discipline: solid ground plane, a keep-out under the antenna, the matching
   network *at* the feed, a 50 Ω controlled-impedance trace from module to
   antenna. Bring the pogo-pin antenna path out so you can test both
   configurations on one board.
3. Route the pogo pins for **double duty**: charging input *and* Band solar
   input. This is what makes the modular strap an engineering feature.
4. Mic ports: 0.8 mm holes with an acoustic mesh membrane (Gore/Saati class).
   Standard practice; IPX7 is achievable.
5. Order from JLCPCB or PCBWay: 5 boards with a stencil. Order **three** sets of
   parts — you will kill one.
6. **The eFuse ritual — a mandatory checklist, not an optional hardening pass.**
   Until all four are burned, "the key never leaves the chip" is true and
   irrelevant, because the session keys sit in readable RAM:
   - [ ] flash encryption enabled
   - [ ] secure boot v2 enabled
   - [ ] JTAG disabled by eFuse
   - [ ] UART download mode disabled
   - [ ] ATECC608A keypair generated **on-device at first boot**, slot
         configured non-readable, then locked
   - [ ] ratchet state in NVS wrapped by a key derived inside the ATECC
   - [ ] memory-vault blob mapped to reviewed encrypted storage; root is never exported to product code
   - [ ] independent durable generation floor tested against power-loss, reflash and rollback attempts

eFuses are **one-way**. Burn them on a sacrificial board first and confirm you
can still flash your production firmware afterwards. The portable `memory-vault`
suite proves the contract and failure policy only; it does not satisfy either
memory-vault checklist item until the target backend and adversarial hardware
procedure are evidenced. The portable consolidation suite likewise proves a bounded
review state machine only; it does not evidence a physical control, trusted clock,
calendar retention policy or physical-media sanitization. The portable retrieval suite proves only bounded typed matching in RAM; it does not evidence natural-language search, personal relevance, a model, NVS access or a physical recovery interaction. The portable presentation suite proves only symbolic status and bounded haptic plans; it does not evidence voice intelligibility, haptic perception, display legibility, WCAG conformance, physical controls, accessibility, user understanding, energy or latency. The memory Grand Finale additionally proves a host fixture composition and cross-layer refusal policy; it does not evidence a multi-card persistent store, protected root/NVS, power-loss recovery, ASR, physical interaction, user comprehension or any hardware metric. The executable threat model then classifies this host evidence and keeps platform/supply-chain gaps visible; it is not a runtime monitor, penetration test, hardware assurance or risk-probability estimate. The portable collection additionally proves a fixed-capacity authenticated transaction and logical recovery in RAM; it does not prove NVS semantics, physical purge, power-loss behavior, root protection, wear, capacity on target or the suitability of any particular MCU/secure element. Its private index adds typed, physically gated and budget-limited matching over that authenticated record; it does not prove search at scale, private-information-retrieval cryptography, ORAM, language understanding, relevance to a person, persistent index privacy, latency, energy or a backend on the target. The crash-state recovery oracle additionally proves only C11 classification of authenticated `PREPARED`/`COMMITTED` records against an asserted monotonic floor; it does not prove callbacks are durable, brownout/power-loss behavior, flash atomicity, partial-write detection, wear, eFuse/secure-element counters, root protection, physical purge or a selected target platform. The portable provenance audit additionally compares declared local source inputs against unsigned SHA-256 references and rejects unsafe manifest shape; it does not prove checkout authenticity, component completeness, signed provenance, SLSA level, builder isolation, target artifact reproducibility, CI trust or supply-chain physical integrity. The collection physical-session gate additionally binds collection operations to a transient purpose, time window and consumed local use; it does not prove a button, gesture, user identity, biometrics, event source, trusted clock, nonce quality, reset-resistant counter, anti-replay across reboot or protected execution until the selected adapter is tested on target. The durable session-reservation recovery oracle additionally classifies authenticated `PREPARED`/`COMMITTED` markers against a floor declared by that future adapter and can only advance a burned ID or block; it never recreates a live session and does not prove durable storage, atomic ordering, reboot behavior, event origin, physical confirmation, secure boot, protected RAM or post-reboot replay resistance. The post-reboot bootstrap then reinitializes the host gate as `IDLE` and imports only that floor; it does not observe a reset, clear physical RAM, authenticate storage, prove a new event or establish replay resistance until the selected adapter is interrupted and measured on target. The Gran Finale composes this gate state with the collection chain and TM-04 in a host fixture, but it remains only a diagnostic of contract coherence; it cannot turn target validation, physical persistence or any human event into a fact. The F1 host campaign additionally rejects a terminal `UINT32_MAX` reservation floor because no successor session would remain representable; it does not demonstrate a target counter, durable media or counter-rotation strategy.

**Done when.** An assembled board runs the Phase 1–3 firmware and matches the
Phase 0 range baseline within 3 dB.

**Kill criterion.** Range more than 6 dB below the devkit baseline → the antenna
integration is wrong. Fix it before the enclosure, because a sealed shell only
makes it worse.

---

## Phase 5 — Enclosure and seal (2 weeks)

**Goal.** A sealed capsule you would wear.

**Do.**

1. Fusion 360, from the volume ledger in 00-HERUS-MASTER §9. Print in **SLA
   resin** — FDM will not hold the O-ring groove or the snap tolerances.
2. Silicone O-ring, quick-release press fit for the Band, pogo pins on the back
   around the piezo.
3. Confirm the cell has an **integrated protection circuit**. A sealed capsule
   with no vent around a LiPo is a pressure vessel if the cell swells. Add a fuel
   gauge.
4. IPX7 test: 1 m of water, 30 minutes. Test with a board you can afford to lose.
5. Re-measure range **with the shell closed and worn.** Plastic, your wrist, and
   the solar cell's conductive backing all detune the antenna.

**Done when.** IPX7 passes and worn-and-sealed range is within 3 dB of the open
board.

**Kill criterion.** Sealed range more than 6 dB below the open board → the
antenna must move to the Band. That is a known-good fallback, not a disaster.

---

## Phase 6 — Ten-unit field trial (2 weeks)

**Goal.** The first phase where Weave and Beat can be measured instead of argued
about.

**Do.**

1. Build ten units. Give them to people who will actually carry them.
2. Instrument: per-frame PDR, hop counts, role transitions, battery curves.
3. Measure what only a real mesh can show you: does energy-aware role switching
   actually keep enough relays alive? Do three hops materialise in practice, or
   does everyone sit in leaf mode and nothing routes?
4. Validate the Beat guard window against real crystal drift across ten devices
   and a temperature range, not two on a bench.

**Done when.** A message crosses three hops and ~1.9 km of city, and no unit
needs charging for a week.

**Kill criterion.** Fewer than 20% of nodes ever entering relay mode → the role
thresholds are wrong, or the Band cells are undersized. Fix the thresholds before
blaming the routing.

---

## Phase 7 — Only if you are going to sell it

Not engineering; budgeting and paperwork. Know the numbers before you promise
anyone a price.

- **ANATEL homologação** is mandatory for a product in Brazil. FCC/CE if you
  export. Budget **US$ 5–15k**. Pre-certified modules cut this substantially.
- **Injection mould** with an O-ring groove: **US$ 3–8k**.
- **Landed cost at 1000 units is R$ 200–250**, not the R$ 85 BOM. Assembly, test
  and yield are real. Quote BOM as BOM, never as unit cost.
- Prototype spend for Phases 0–5, honestly: **R$ 800–1200**, not the R$ 185 BOM
  — import duty, FX spread, and three of everything.

---

## The running checklist

```
[ ] Phase −1  three commands reproduce every number
[ ] Phase −1  local provenance digest audit passes; signed release provenance remains pending
[ ] Phase −1  collection Grand Finale, purpose-bound session gate, reservation-recovery oracle, boot-quarantine bridge and final host-chain composition pass; target backend/event/time/reset/UI evidence remains pending
[ ] Phase 0   volume closes at ≤ 12 mm
[ ] Phase 0   wrist range ≥ 150 m urban at ≥ 50% PDR     <-- decides the product
[ ] Phase 0   Tier 0.5 gain measured (keep if ≥ 2 dB)
[ ] Phase 0   baseline numbers written down and kept
[ ] Phase 1   Tier 1 message rendered on a second board at 300 m
[ ] Phase 1   Codec2 700C judged intelligible on a street recording
[ ] Phase 2   T1–T11 pass on target, cognition ≤ 72 KB
[ ] Phase 2   three-repetition learning demo works
[ ] Phase 3   leaf idle ≤ 100 µA measured on a power analyser
[ ] Phase 4   four eFuses burned, key generated on-device
[ ] Phase 4   each collection transition and reservation `PREPARED`/floor/`COMMITTED`/boot-quarantine/final-chain path and terminal-ID/exhaustion policy interrupted and recovered against its target backend
[ ] Phase 4   assembled board within 3 dB of the Phase 0 baseline
[ ] Phase 5   IPX7 passes, sealed range within 3 dB
[ ] Phase 6   three hops, ~1.9 km, one week without charging
```

---

## If you only do one thing this week

Phase 0, Day 2. Strap a T3-S3 to your wrist and walk your street.

Everything else in this repository is downstream of that one number, and you can
have it on Sunday.
