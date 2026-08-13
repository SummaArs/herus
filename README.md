# HERUS

[![prove](https://github.com/SummaArs/herus/actions/workflows/prove.yml/badge.svg)](https://github.com/SummaArs/herus/actions/workflows/prove.yml)
&nbsp;·&nbsp; C11, no dependencies &nbsp;·&nbsp; 38 proof + 74 system invariants &nbsp;·&nbsp; pre-hardware

**A private, off-grid, semantic communicator.** Herus transmits *meaning* — a
2-byte symbol id into a shared, generatively-derived lexicon — and renders it
into speech locally at the receiver, in the receiver's own language and voice.

Not an assistant. There is no language model, no cloud, no float arithmetic in
the hot path, and no always-on microphone.

> Every other radio sends a *rendering* of a message. Herus sends the message and
> lets the receiver do the rendering.

---

### Why that one decision pays three times

- **Translation is free.** The sender transmits meaning; the receiver renders it
  in whatever language it is configured for. Nothing is translated because
  nothing linguistic is sent.
- **Forward compatibility is free.** Fields are role/filler bindings recovered by
  unbinding a known role id. An unknown role is simply never queried, so old
  firmware ignores new fields instead of rejecting the frame.
- **Range is free.** Meaning fits in the smallest legal frame; audio does not.
  Measured consequence: **meaning outranges speech by 33%** on the same radio
  (650 m vs 487 m urban), because voice is pinned to SF7 by the dwell limit and a
  glyph is not.

---

### Repository

| Path | Contents |
|---|---|
| [docs/00-HERUS-MASTER.md](docs/00-HERUS-MASTER.md) | Thesis, architecture, RF, power, mechanical, security, cost — and a full errata against the original concept |
| [docs/01-ALGEBRA.md](docs/01-ALGEBRA.md) | The VSA engine: derivations, corrections, capacity laws, all seven proposed improvements audited |
| [docs/02-PROTOCOL.md](docs/02-PROTOCOL.md) | Normative wire protocol: constant-airtime frames, five tiers, crypto, Weave, Beat, region profiles |
| [docs/03-BUILD-GUIDE.md](docs/03-BUILD-GUIDE.md) | **Start here to build.** Every phase in order, exact commands, one pre-committed kill criterion each |
| [docs/04-PRODUCT.md](docs/04-PRODUCT.md) | How it gets sold: the Anchor (+28 dB, 25× the area, value at N=1), minting, seed-as-language, SKUs, and the honest odds |
| [docs/05-FIRMWARE.md](docs/05-FIRMWARE.md) | **The firmware, and how to get it onto hardware.** What is proven, what is deliberately missing, and the exact bench sequence |
| [docs/06-NUCLEO.md](docs/06-NUCLEO.md) | **The pocket puck.** Architecture, privacy boundary, local intelligence, charging and the falsifiable hardware plan for the Nucleus |
| [docs/07-VOZ-HAPTICA.md](docs/07-VOZ-HAPTICA.md) | **Advance 1.** Local controlled-language interaction, physical confirmation and bounded vibration feedback |
| [docs/08-RUNTIME-INTERACAO.md](docs/08-RUNTIME-INTERACAO.md) | **Advance 2.** Push-to-talk runtime, one-shot confirmed send gate, adapter boundary and privacy-preserving telemetry |
| [docs/09-VALIDACAO-FISICA.md](docs/09-VALIDACAO-FISICA.md) | **Advance 3.** Physical-validation protocol, deterministic adapter rig, telemetry schema and pre-committed product gates |
| [docs/10-INVESTIGACAO-PREREGISTRADA.md](docs/10-INVESTIGACAO-PREREGISTRADA.md) | **Advance 4.** Pre-registered Core-versus-Nucleus investigation, frozen sample plan, Wilson gates and reproducible decision rule |
| [docs/11-GATEWAY-CONFIANCA.md](docs/11-GATEWAY-CONFIANCA.md) | **Advance 5.** Session-bound local-ASR confidence gateway, ambiguity handling and bounded Nucleus context |
| [docs/12-ENLACE-CORE-NUCLEO.md](docs/12-ENLACE-CORE-NUCLEO.md) | **Advance 6.** Authenticated Core↔Nucleus control envelope, session binding, expiry and replay protection |
| [docs/13-CICLO-DE-CONFIANCA.md](docs/13-CICLO-DE-CONFIANCA.md) | **Advance 7.** Explicit Core↔Nucleus provisioning, six-digit SAS, protected-storage port, revocation and zeroization |
| [docs/14-DIALOGO-LLM-LOCAL.md](docs/14-DIALOGO-LLM-LOCAL.md) | **Advance 8.** Bounded local conversation, model adapter, transient privacy and zero transmission authority |
| `firmware/core/` | Portable C: both algebras, memory, resonator, HCP rev 0.2, Nucleus intelligence, intent-confidence gateway, bounded dialogue, interaction runtime and deterministic validation rig |
| `firmware/net/` | The wire: crypto, symmetric ratchet, explicit Core↔Núcleo trust lifecycle, authenticated control envelope, region/dwell as compile-time assertions, Weave, Beat |
| `firmware/port/` | The platform: an 8-function HAL, an SX1262 driver, the ESP32-S3 app and bring-up console |
| `firmware/test/` | Twelve firmware proof targets, including Nucleus, voice/haptic, intent gateway, bounded dialogue, Core↔Núcleo trust lifecycle, control link, interaction-runtime and validation-lab invariants, crypto vectors from an independent implementation and an SX1262 mock bus |
| `firmware/ranger/` | Phase 0 measurement firmware for two LilyGO T3-S3 — no GPS, no display |
| [sim/](sim/README.md) | **The bench.** A world of metres, milliseconds and mA around the unmodified firmware: range, mesh, crowding, adversaries, battery, drift |
| `tools/budget.py` | Physical-layer, energy and frame-ledger model, stdlib only |
| `tools/fieldlog.py` | Turns a Phase 0 walk into a numeric verdict on both kill criteria |
| `tools/interactionlog.py` | Validates privacy-preserving interaction telemetry and applies the Advance 3 gates |
| `tools/studyplan.py` / `tools/interactionstudy.py` | Materializes the frozen trial plan and performs only the pre-registered Core-versus-Nucleus analysis |
| `tools/frontier.py` | Enumerates every (SF, BW) pair under both FCC regimes — where the remaining decibels are, and what each costs |
| [CONTRIBUTING.md](CONTRIBUTING.md) | How to build and test, and the six things that are settled and why |
| [SECURITY.md](SECURITY.md) | What the cryptography protects today, what it does not protect yet, and how to report a finding |

### Reproduce every number

You need a C11 compiler, `make` and Python 3. Nothing else: no ESP-IDF, no
board, no third-party library — the cryptography, the algebra and the bench are
all in this tree.

```bash
./prove.sh
```

About 30 seconds. Thirteen suites — algebra, Nucleus intelligence, controlled voice/haptics, intent gateway, **bounded local dialogue**, explicit Core↔Núcleo trust lifecycle, authenticated control link, interaction runtime, validation lab, preregistered study, protocol, radio driver and physical layer — plus the simulated bench, and **38 proof invariants and 74 system invariants checked rather than eyeballed**:

```
PASS  P1 constant AIRTIME across meaning tiers
PASS  P2 no frame exceeds the 400 ms dwell limit
PASS  Nucleus learning is opt-in, bounded and non-autonomous
PASS  Voice remains local, confirmed and haptically bounded
PASS  Intent gateway is session-bound, confidence-gated and non-autonomous
PASS  Dialogue retains no transcript and clears local UX reply
PASS  Dialogue output has zero transmission authority
PASS  Trust requires physical pairing and matched SAS before activation
PASS  Trust revocation zeroizes RAM and fails closed
PASS  Control link authenticates, expires and rejects replay
PASS  Interaction is push-to-talk, confirmed and one-shot
PASS  Interaction rig keeps adapter sequencing non-transmitting
PASS  Telemetry gates reject an unsafe send
PASS  Preregistered study keeps gates and rejects unsafe send
PASS  crypto agrees with an independent implementation
PASS  hypervectors are never transmitted
PASS  every single-bit corruption is rejected
PASS  Tier 0.5 does not avalanche
PASS  no stable identifier is on air (P6)
PASS  replay is refused forever
PASS  forgery fails at 2^-64 per attempt
PASS  flooding terminates
PASS  SetPacketType precedes SetModulationParams
PASS  the selftest diagnoses a wrong pin map
...
ALL INVARIANTS HOLD — the documents reproduce and the firmware is safe to flash.
```

If a figure appears in `docs/` and is not produced by `./prove.sh`, it is a bug in
the documentation. If a property is claimed and no test can fail, it is a hope.

---

### Where it stands

| | |
|---|---|
| Algebra, memory, resonator | implemented; cross-validated by two independent suites |
| RF, regulatory and energy model | derived and reproducible |
| Wire protocol | **implemented and proven** — crypto differentially tested against OpenSSL, ratchet, replay, forgery, flooding, Beat drift |
| Radio driver + ESP32-S3 app | **written**; command sequences verified against a recording mock bus, app type-checked. Not yet run on silicon |
| Link budget | **+15.2 dB over the published design, all of it buildable.** 8 dB of it was a mislabelled constant: `budget.py` used the European 14 dBm in a band where FCC 15.247 and ANATEL 14448 allow 30, and the SX1262's own PA reaches 22. Reach adds 2.5 dB, a receive LNA 4.7 dB. Band-antenna urban range **650 m → 1561 m**; a further 4 dB is projected from antenna work and is a bench question ([tools/frontier.py](tools/frontier.py), `sim hardware`) |
| Modulation | **at the regulatory ceiling.** Two link profiles: Rich (SF9, 34 B, 9 slots) and Reach (SF10, 24 B, 4 slots, +2.5 dB → 422 m / 750 m urban). `region.h` asserts that 25 B at SF10 is illegal and that SF11 cannot carry one byte, so there is no rung above Reach without changing the band or the law |
| Behaviour as a system | **simulated** — the unmodified firmware run against a two-ray channel, a duty cycle, a crowd and an adversary. The bench reproduces the published range and all three energy roles from first principles, and its stress pass found four real defects, all now fixed: a link that never recovered from an outage, relay silence naming the recipient, an unbounded battery drain by any stranger, and a phase loop that manufactured its own noise ([sim/](sim/README.md)) |
| Nucleus local intelligence | **implemented and host-proven** — bounded, opt-in semantic transition memory with confidence, expiry, explicit erase and no autonomous transmission; hardware integration remains future work — see [06-NUCLEO.md](docs/06-NUCLEO.md) |
| Voice and haptic interaction | **implemented and host-proven** — controlled Portuguese intent parsing, no autonomous send/SOS, physical confirmation contract and bounded driver-independent vibration plans; local ASR and hardware integration remain future work — see [07-VOZ-HAPTICA.md](docs/07-VOZ-HAPTICA.md) |
| Interaction runtime | **implemented and host-proven** — push-to-talk state machine, local-ASR adapter boundary, timeout/cancel/source-loss clearing, one-shot confirmed handoff and privacy-preserving telemetry; no microphone, ASR SDK, GPIO or radio wiring yet — see [08-RUNTIME-INTERACAO.md](docs/08-RUNTIME-INTERACAO.md) |
| Intent confidence gateway | **implemented and host-proven** — typed local-ASR output must match a physical session and pass confidence/margin rules; stale, weak and ambiguous observations cannot create a sendable draft, while Nucleus context can only break an otherwise strong tie — see [11-GATEWAY-CONFIANCA.md](docs/11-GATEWAY-CONFIANCA.md) |
| Núcleo dialogue boundary | **implemented and host-proven** — a physically initiated local turn passes one transient utterance and at most four typed topic cards to an optional local-model adapter; replies have zero send authority and are erased after UX delivery. No LLM, ASR, TTS, target memory/energy measurement or cloud fallback is implemented — see [14-DIALOGO-LLM-LOCAL.md](docs/14-DIALOGO-LLM-LOCAL.md) |
| Core↔Nucleus trust lifecycle | **implemented and host-proven** — dual physical association mode, matched six-digit SAS, HKDF-derived binding, opaque protected-storage port, fail-closed revocation, RAM zeroization and A6 sequence reset; BLE LE Secure Connections/OOB, target RNG and the secure-element/encrypted-NVS backend remain target integration work — see [13-CICLO-DE-CONFIANCA.md](docs/13-CICLO-DE-CONFIANCA.md) |
| Core↔Nucleus control link | **implemented and host-proven** — fixed-size AEAD envelope with pair binding, monotonic sequence, expiry and replay rejection before a Nucleus result can reach the intent gateway; it is now reachable through a currently active trust lifecycle — see [12-ENLACE-CORE-NUCLEO.md](docs/12-ENLACE-CORE-NUCLEO.md) |
| Physical validation path | **implemented and host-proven** — deterministic adapter rig, normative CSV, log validator and decision gates for intent, false drafts, latency, handoff and measured energy; no real device data has been collected — see [09-VALIDACAO-FISICA.md](docs/09-VALIDACAO-FISICA.md) |
| Pre-registered investigation | **implemented and host-proven** — frozen Core-versus-Nucleus trial plan, sample size, Wilson intervals, product decision rule and analyzer; no confirmatory data has been collected — see [10-INVESTIGACAO-PREREGISTRADA.md](docs/10-INVESTIGACAO-PREREGISTRADA.md) |
| Secure element (ATECC608A) | not wired: forward secrecy yes, post-compromise security and SOS signing not yet — see [05-FIRMWARE.md](docs/05-FIRMWARE.md) §4 |
| Hardware | **nothing built yet — Phase 0 is next** |

**Phase 0 is two days and it decides whether Herus is a wrist device.** Print the
30×30×10 shell and check the volume closes; strap a LoRa devkit to a wrist and
walk a measured urban route. Full procedure and both kill criteria:
[docs/03-BUILD-GUIDE.md § Phase 0](docs/03-BUILD-GUIDE.md#phase-0--the-weekend-that-decides-the-project).

### Selected results

```
regulatory ceiling   SF10 at 24 B (Reach). Enumerating all 60 (SF, BW) pairs under
                     both FCC regimes leaves 1.02 dB unclaimed, and it costs 3 of 4 slots
tx power             22 dBm (SX1262 HP PA); 14 dBm was the EU limit used by mistake
frame                34 B Rich / 24 B Reach; identical airtime within a profile
frame                34 B, 246.8 ms, identical for every meaning-carrying tier
range (urban)        878 m capsule / 1561 m strap at 22 dBm + LNA, Reach; ~4.5 km over 3 hops
                     (365/650 m was 14 dBm, bare chip, SF9 — the same model, reproduced)
RAM                  648 B per peer session (was 1896: the skip cache was 88% of it)
leaf power           3.62 mAh/day  vs  19.46 mAh/day harvested in 1.5 h sun
relay power          127.68 mAh/day — wants 26 cm² of cell, hence the solar Band
indoor solar         1% of need at 300 lux — a trickle, not a power source
search               6.4 µs/query exhaustive on host with MCU-realistic popcount;
                     two-stage is 0.14x of that at 100.00% agreement
sparse vs dense      10x smaller, 24x faster search, 32x better separation,
                     but bundle capacity 9 vs 127 — hence both algebras
resonator            91.5% solved, 2.7 mean iterations over 32768 hypotheses
RAM                  <=72 KB for the full memory cascade (int8 accumulators
                     halve it); the codebook itself costs zero
```

### The uncomfortable numbers, kept in view

An engineering document that only lists its wins is marketing. These are load
bearing and they are not going away:

- Wrist-worn urban range is **365 m**, not kilometres. At wrist height the
  two-ray breakpoint is 38 m, so almost all range lives in the d⁴ regime.
- **Range is bounded by regulation, not physics.** SF9 is the legal ceiling and
  it costs 7.5 dB against the SF12 figures most LoRa range folklore quotes.
- Continuous listen-and-classify would cost **1104 mAh/day — 57× the harvest.**
  That is why there is no always-on microphone.
- "Perpetual on light" is true for a leaf in daylight, **false indoors, and false
  for any node that relays.**
- A duty-cycled leaf link delivers **93.5%** of what is offered, and nothing in
  the protocol notices. Reaching 100% costs a second copy of every message —
  because single-use keys forbid retransmitting the first one.
- Flooding costs about **10x** the offered load in relay transmissions. Weave is
  flooding with dedup by design; that multiple is the price, and the governor now
  bounds it rather than removing it.

---

### Before you transmit anything

`firmware/net/region.h` refuses to build a frame that would be illegal in the
configured region, but those limits are one engineer's reading of FCC 15.247,
ANATEL 680/2017 + Ato 14448 and ETSI EN 300 220. Whoever transmits owns what
leaves the antenna. The security posture — forward secrecy yes, post-compromise
security not yet, no independent audit — is stated without varnish in
[SECURITY.md](SECURITY.md). Read it before trusting this with anything that
matters.

### Licence

Proprietary. Copyright © 2026 Gustavo Gonçalves, all rights reserved — see
[LICENSE](LICENSE). Access to this repository grants no right to use, copy or
redistribute any part of it.
