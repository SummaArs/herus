# HERUS — Master Design Document

**Version 0.1 · Status: pre-Phase-0 · All numbers reproducible from `tools/budget.py` and `firmware/core/`**

> Doctrine for this document: an asserted number is a rumour, a measured number
> next to its closed form is a result. Where theory and measurement disagree,
> the measurement wins and this document gets rewritten. Every figure below is
> either produced by code in this repository or carries its derivation inline.

---

## 1. Thesis

Herus transmits **meaning**, not signal.

Every other off-grid communicator moves an encoding of sound or text and pays
for it in bandwidth, energy, and range. Herus moves a symbol — an index into a
shared, private, generatively-derived lexicon — and renders it into speech
locally, at the receiver, in the receiver's own language and voice.

Three properties fall out of that single decision, none of which are features
that had to be built:

| Property | Why it is free |
|---|---|
| **Translation** | The sender transmits meaning. The receiver renders it in whatever language it is configured for. There is no translation engine because there is nothing to translate. |
| **Forward compatibility** | Message fields are role/filler bindings recovered by unbinding a known role id. A role a receiver does not know is simply never queried. Old firmware ignores new fields instead of rejecting the frame. No version negotiation, ever. |
| **Range** | A meaning is 16 bytes. Audio is hundreds. The frame that fits in the legal dwell window is the one that reaches furthest. |

The device that results is **not an assistant**. It is a private, off-grid,
semantic communicator with a symbolic intent layer. That distinction is
load-bearing: framed as an assistant it will be compared to Alexa and lose;
framed as a communicator it competes with Meshtastic and goTenna and wins on
the axis nobody else is on.

---

## 2. What Herus is not

Refusing scope is the cheapest engineering there is. Each cut below has a
number attached, not a taste.

| Cut | Reason |
|---|---|
| Large language model, cloud or local | Nothing in the product needs open-vocabulary generation. A closed lexicon is a feature (see §4). |
| Always-on microphone / wake word | 1104 mAh/day — **57× the capsule's entire daily solar harvest**. Also the correct privacy answer. Voice is push-to-talk. |
| Piper / Flite neural TTS | Piper is a VITS ONNX model, 20–60 MB of weights. The ESP32-S3 has 512 KB SRAM + 8 MB PSRAM. It does not fit, at any optimisation level. Replaced by a compressed clip bank + eSpeak-NG fallback (§8). |
| NFC peer-to-peer key exchange | The ST25DV is a *dynamic tag*, not a reader. Tag-to-tag is physically impossible. Pairing moves to an authenticated short-string comparison (§10). |
| Real-time voice calls | Continuous transmission violates the 400 ms dwell rule (§6) and the energy budget. Push-to-talk messages only. |
| Blockchain / DePIN transaction signing | Currently a capability in search of a user story. The ATECC608A can sign an arbitrary digest, so this is a firmware feature whenever a real use case appears. It costs nothing to defer and costs credibility to claim. |
| Wi-Fi in production | 100+ mA. Development only; absent from the v2 silicon (§7). |

---

## 3. Nomenclature

Consistent names make the documentation cheap to write and the product legible.

| Name | Is |
|---|---|
| **Core** | The sealed 30×30×10 mm capsule. All electronics. |
| **Band** | The interchangeable strap. Optionally carries solar cells and the antenna, fed through the pogo pins. |
| **Lexicon** | The symbol space. Codes are *derived* from `(domain_seed, id)`, never stored, never transmitted. |
| **HCP** | Herus Composition Protocol — the wire format (§5). |
| **Weave** | The mesh layer and its energy-aware routing roles. |
| **Echo** | The local renderer: symbol → audio. |
| **Vault** | The ATECC608A identity and its on-device-generated key. |
| **Beat** | The slotted wake schedule that makes low duty cycle and low latency coexist. |

---

## 4. Architecture

```
   MIC (2x MEMS, button-gated)          NFC tag (ST25DV)
            |                                  |
     [ feature front-end ]              [ phone pairing ]
            |                                  |
     [ SBC perception ]  <-- learns from 3 examples, no backprop
            |
     symbol id  ------> [ HCP encode, 16 B ] -> [ ratchet ] -> [ SX1262 ]
            |                                                       |
     [ dense semantic memory ]                                   Weave
       context / episode / semantic                                 |
            |                                                       v
     [ Echo renderer ] <---- [ HCP decode ] <- [ ratchet ] <- [ SX1262 ]
            |
        piezo, skin-side
```

Two algebras run side by side, each where its geometry pays. This is not
redundancy; they have different capacity curves and different costs, both
measured in `firmware/core/test_algebra.c`.

| | Sparse Block Codes (SBC) | Dense binary (MAP-B) |
|---|---|---|
| Used for | lexicon, prototypes, relations, on-air payloads | context and episodic accumulators |
| Vector size | **128 B** | 1280 B |
| Random similarity | 1/B = **1.6%** | 50% |
| Compare cost | byte equality — **25× faster measured** | XOR + byte-table popcount |
| Bundle capacity | ~9–15 operands | **~308 operands** |

The trade is explicit: SBC has far better separation and far cheaper compares
but poor bundle capacity; dense degrades gracefully under heavy superposition.
Few operands → SBC. Many operands → dense.

> **Why SBC and not "sparse vectors" as originally drafted.** If A and B each
> carry *s* ones out of *D*, then `A XOR B` carries ≈ `2s(1 − s/D)` ones.
> Sparsity is not preserved; bind twice and the representation has drifted to
> dense and the algebra collapses. SBC fixes this properly: partition D into S
> blocks of B slots, exactly one slot active per block, and bind becomes
> `(a[i] + b[i]) mod B` — sparsity preserved *exactly*, inverse exact.
> Full derivation and the corrected form of all seven proposed improvements
> are in **[01-ALGEBRA.md](01-ALGEBRA.md)**.

---

## 5. The message ladder

Every tier that carries **meaning** occupies **exactly the same airtime** — 246.8 ms
at SF9 — regardless of tier or content. Full byte ledger in
**[02-PROTOCOL.md](02-PROTOCOL.md)**; generated and asserted by
`tools/budget.py` §8.

> This is not padding hygiene, it is a security requirement. A 2-byte panic
> glyph and a 12-byte status report are separable by airtime alone. Without
> constant-length framing the confidentiality claim is void against anyone with
> a spectrum analyser.
>
> **Corrected in rev 0.2.** An earlier draft specified 26 bytes and gave Tier 0.5
> its own ~280 ms row, silently breaking the invariant above. The fix was to unify
> *upward*: Tier 0/1 grow their plaintext from 16 B to 24 B. That growth is not
> waste — with slots packed as `role:5 | filler:11` it raises Tier 1 from 4
> role/filler slots to **9**, across 32 roles and a 2048-symbol filler space.
>
> **Corrected again in rev 0.3 (erratum E-P1).** Rev 0.2 landed both tiers on 34
> *bytes* and asserted `len(Tier0) == len(Tier0.5)` — an assertion that passed while
> the invariant it named stayed broken. Tier 0.5 runs implicit header with CRC off,
> so 34 B costs 226.3 ms against 246.8 ms: 20.5 ms of difference, readable with a
> spectrum analyser. **P1 is about airtime, and bytes were only a proxy.** Tier 0.5
> is now 38 B (2 addr + 32 sketch + 4 keystream pad), which lands on the same 48
> payload symbols and therefore the same airtime. `budget.py` asserts equal airtime,
> and `firmware/net/region.c` asserts it again at compile time. Full derivation in
> [05-FIRMWARE.md](05-FIRMWARE.md) §3.

| Tier | Payload | On air | SF | Airtime | Urban range (Band) | Use |
|---|---|---|---|---|---|---|
| **0 Glyph** | 2 B id, 24 B plaintext | 34 B | 9 | 246.8 ms | 650 m | fixed-lexicon intent, beacon |
| **1 Composed** | intent + up to 9 role/filler | 34 B | 9 | 246.8 ms | 650 m | "arriving, 10 min, north gate" |
| **0.5 Sketch** | 256-bit sketch + pad, no CRC, no MAC | 38 B | 9 | 246.8 ms | 650 m + graceful | broadcast where no ACK is possible |
| **2 Voice** | Codec2 700C, 4 s, 2 frags | 178 B | **7** | 2 × 287 ms | **487 m** | push-to-talk |
| **SOS** | coords + key id + ECDSA sig | 82 B | **8** | 256.5 ms | 563 m | deliberately public (§10) |

Two consequences worth reading twice:

- **Meaning outranges speech by 33%** (650 m vs 487 m urban, Band antennas).
  Voice is pinned to SF7 because a 178 B frame takes 502 ms at SF8 and blows the
  dwell limit; a glyph is not pinned. The thesis of §1 is not a slogan, it is
  163 metres.
- **SOS runs SF8, not SF9.** It must carry a 64 B ECDSA signature so that a
  *stranger* can verify it, which pushes the frame to 82 B; at SF9 that is
  472 ms and illegal. Reachability was bought with a spreading factor, and the
  trade is deliberate.

Tier 1 is the interesting one. A message is an intent plus bindings:

```
MSG = ROLE_intent ⊗ ARRIVE  ⊕  ROLE_time ⊗ MIN_10  ⊕  ROLE_place ⊗ NORTH_GATE
```

The hypervector is **never transmitted** — only the 2-byte symbol ids are. The
receiver regenerates the codes from the shared domain seed and rebuilds the
structure locally. Verified round-trip, including correct rejection of an
absent role, in `test_herus` T11.

### The resonator

When a receiver must decode a composed record whose role structure it was never
told, it faces a factorisation problem: given `S = a ⊗ b ⊗ c` with all three
unknown, recover all three. Brute force is 32 768 hypotheses for three factors
of 32 candidates.

A **resonator network** holds all hypotheses superposed and collapses them by
alternating soft cleanup. Measured: **91.5% solved in a mean of 2.7 iterations**
(`test_herus` T8). Hard argmax cleanup does not work — it falls into limit
cycles; signed-weight soft cleanup is required. This is the 2020s state of the
art in VSA and it is ~40 lines of C.

### Tier 0.5 — graceful decode, and the honest caveat

A subsampled 256-bit sketch of a code decodes by nearest neighbour with **no CRC
and no retransmission**. Measured against a 512-symbol lexicon
(`test_herus` T7):

| raw BER | sketch decodes | CRC'd 8 B frame survives |
|---|---|---|
| 15% | 100.00% | 2.3 × 10⁻⁶ |
| 25% | 100.00% | 1.0 × 10⁻¹⁰ |
| 35% | 95.95% | 1.1 × 10⁻¹⁵ |
| 40% | 57.10% | — |

The cliff is at 40%. Below it the sketch is essentially perfect where a framed
message is *certainly* lost.

**The caveat, stated plainly.** The SX1262 does not hand you graded bit errors:
it delivers a payload or drops it. To get errors through you must run *implicit
header* mode with *CRC disabled*, so the only remaining gate is preamble
detection. Preamble detection works roughly 2–4 dB below packet-decode
threshold. **That gap — 2 to 4 dB, not the 35% BER figure — is the real size of
the Tier 0.5 prize.** In the d⁴ regime 4 dB buys ~25% more distance.

Against simple repetition (send the 16 B frame twice, ~2× airtime, ~3 dB of
selection diversity) Tier 0.5 is close to break-even. Its genuine advantage is
**broadcast with no ACK possible** — an SOS beacon to unknown listeners, where
retransmission logic has nobody to negotiate with. Ship it there; treat it as a
research track everywhere else. Falsification criterion in §12, Phase 2.

---

## 6. RF and the regulatory ceiling

**The binding constraint on Herus range is regulation, not physics.**

FCC 15.247(a)(1)(iii), mirrored by ANATEL Ato 14448 for 902–907.5 / 915–928 MHz,
limits a frequency-hopping system to **400 ms of dwell per channel**.

| SF | airtime, 26 B | legal? | sensitivity |
|---|---|---|---|
| 7 | 61.7 ms | yes | −124.5 dBm |
| 8 | 113.2 ms | yes | −127.0 dBm |
| **9** | **205.8 ms** | **yes** | **−129.5 dBm** |
| 10 | 411.6 ms | **no** | −132.0 dBm |
| 11 | 823.3 ms | **no** | −134.5 dBm |
| 12 | 1646.6 ms | **no** | −137.0 dBm |

**SF9 is the ceiling.** Every LoRa range figure quoted at SF12 — including most
of the Meshtastic folklore — is quoting a transmission that exceeds the dwell
limit. SF10–12 stay disabled in the shipped region profile; enabling them
requires an experimental or licensed basis.

### Range, derived rather than quoted

Realised antenna gain **includes radiation efficiency and body loading**. A
30 mm radiator at 915 MHz is λ/11 — electrically tiny — and a wrist is high-
permittivity tissue. Capsule-internal: −8 dBi. Strap-fed: −3 dBi.

Propagation uses the **two-ray plane-earth model**. Beyond the breakpoint
`d_bp = 4πh₁h₂/λ` the ground reflection arrives antiphase and path loss grows
as d⁴, not d². At wrist height d_bp is **38 m**, so essentially the entire
useful range lives in the d⁴ regime. Using free-space loss out to kilometres at
1 m above ground is the single most common way LoRa range gets overestimated.

| Configuration | SF9 budget | open field | suburban | dense urban |
|---|---|---|---|---|
| capsule ↔ capsule | 124 dB | 1296 m | 650 m | **365 m** |
| Band ↔ Band | 134 dB | 2305 m | 1155 m | **650 m** |

Three-hop Weave with Band antennas: **~1.9 km effective urban reach**.

**Height is the cheapest lever in the system.** Both ends at 1.6 m instead of
1.0 m gives 3688 m open field instead of 2305 m. Range scales as h² in the d⁴
regime — *raising your arm is worth more than any firmware change*. Build that
into the UX: an explicit "reach" gesture.

---

## 7. Silicon

| | ESP32-S3 | nRF54L15 |
|---|---|---|
| Core | 240 MHz dual Xtensa LX7, 128-bit SIMD | 128 MHz Cortex-M33 + RISC-V coprocessor |
| Sleep | 15 µA | 1.2 µA |
| Active | 45 mA | ~3 mA |
| Two-stage query (projected, §11) | 0.44 ms | 0.83 ms |
| **Energy per query** | 0.006 µAh | **0.001 µAh** |

Neither core has a population-count instruction, so dense similarity costs a
byte-table lookup per byte on both. The S3's SIMD advantage makes it **1.9×
faster** per query — and **6× worse** per query in energy. For a device that is
sleep-bound rather than compute-bound, the faster chip loses.

**Decision:** ESP32-S3 for Phases 1–3 (development velocity, Wi-Fi for
instrumentation, SIMD headroom for algebra research). Lay the PCB so v2 drops
in an nRF54L15 once the workload is characterised. State the crossover in the
schematic review so nobody re-litigates it.

---

## 8. Echo — local rendering

Two paths, no neural TTS:

1. **Clip bank (primary).** The lexicon is closed, so every glyph maps to a
   pre-recorded clip. ~1000 utterances as ADPCM or Codec2 in a few MB of flash.
   Latency < 20 ms, CPU ≈ 0, and the voice is whatever voice you recorded.
   *This is philosophically consistent: a symbolic device speaks from a
   symbol table.*
2. **eSpeak-NG (fallback).** Formant synthesis, a few hundred KB, robotic but
   open-vocabulary, for names and numbers outside the bank.

Because rendering is receiver-side, the clip bank is per-device and per-language.
Translation is a configuration setting.

---

## 9. Mechanical

The 8 mm target does not close. A 250 mAh LiPo alone is 3.7 cm³ of a 7.2 cm³
capsule, and a 502030 pouch is 25×20×5 mm — nearly the whole PCB footprint.

**Volume ledger, 30×30 footprint, 150 mAh cell:**

| Layer | mm |
|---|---|
| Front window (PC) | 0.80 |
| Solar cell + adhesive | 0.40 |
| Air gap | 0.30 |
| PCB, 4-layer | 1.00 |
| Tallest component (shielded module) | 1.40 |
| Battery | 3.80 |
| Piezo bender, skin side | 0.60 |
| Back wall | 0.80 |
| O-ring compression + tolerance | 0.90 |
| **Total** | **10.00** |

**30 × 30 × 10 mm is buildable. 8 mm is not, at any battery size that also
fits a transducer.**

The stack has a pleasant property: **the solar cell faces the sky and the piezo
faces the skin.** A piezo bender pressed against the wrist is a serviceable
bone/skin-conduction transducer, it is 0.6 mm instead of the 3 mm a coin
transducer needs, and it does not compete with the solar aperture.

Battery sizing: at 3.62 mAh/day, 150 mAh gives **41 days of dark autonomy**.
Thirty days is already far past need for a solar device — spend the surplus on
thickness, not capacity.

Two notes that bite late:
- **Mic ports** through a sealed case need an acoustic mesh membrane (Gore/Saati
  class) over a 0.8 mm port. Standard practice, IPX7 achievable, but it must be
  in the CAD from the start.
- **A sealed capsule with an O-ring and no vent is a pressure vessel** if the
  cell inflates. Verify the pouch has an integrated protection circuit, and
  design the back wall to fail before the seal does.

Charging: **1C, ~60 minutes to full.** The draft's "20 minutes" implies 3C,
which inflates or kills a cell this size.

---

## 10. Security

### Threat model

| Adversary | Mitigation |
|---|---|
| Passive RF eavesdropper | Double Ratchet AEAD; constant-length frames; rotating 2-byte short addresses derived from the ratchet |
| Active injector / replayer | Ratchet ordering + ATECC monotonic counter |
| Lost-device attacker with physical access | Key non-readable in ATECC; flash encryption + secure boot v2 + eFuse lockdown; ratchet state at rest wrapped by an ATECC-held key |
| Malicious Weave relay | Relays forward ciphertext only. Drop/delay is **not** defended — accepted limitation, end-to-end ACK with deadline and tier fallback |
| Traffic analysis | Constant length, rotating addresses, dummy traffic in the Beat slot |
| Lexicon compromise | Compartmentalisation: without the domain seed a payload is an index into nothing |

### Five decisions

1. **Generate the identity key on-device at first boot**, inside the ATECC, in a
   slot configured non-readable and then locked. Factory generation means the
   factory knows the key. This change is free.
2. **The ATECC protects identity, not session state.** It does P-256 ECDH and
   ECDSA; the Double Ratchet's symmetric half runs in ESP32 RAM. Therefore:
3. **eFuse lockdown is a mandatory provisioning step** — JTAG disabled, UART
   download mode disabled, flash encryption and secure boot v2 enabled. Without
   it the ratchet keys are readable over debug and "cryptographic identity" is
   decoration. This step does not exist in the original plan; it is the
   difference between a claim and a property.
4. **Pairing by short authentication string.** Since NFC P2P is impossible with
   an ST25DV, derive a 6-digit code from `H(pk_A ‖ pk_B ‖ nonce)`; both units
   speak it through Echo and both users confirm verbally. Cryptographically
   equivalent to NFC out-of-band, costs nothing, works on a screenless device.
   Phone-mediated BLE pairing is the primary path; audio-SAS is the off-grid one.
5. **Voice biometrics is a UX gate, never an authorisation factor.** A 50 KB
   quantised speaker model gets 5–10% EER in quiet and degrades badly in noise,
   and it is trivially replay-attackable. Authorisation is a physical button
   press, plus an NFC tap for anything high-value.

### SOS is deliberately not private

If you are in danger and the ratchet state is corrupt, reachability beats
confidentiality. SOS is an ECDSA-signed **plaintext** beacon carrying identity
and coordinates that any Herus will relay. This is an opt-in trade, stated in
the UI, not an oversight.

---

## 11. Measured performance

From `firmware/core/test_herus.c` and `test_algebra.c`, host-measured with the
byte-table popcount path so the projection reflects what the MCU executes.

```
quasi-orthogonality  D=10240: mean 5120.0 (theory 5120.0), sigma 50.55 (theory 50.60)
                     P(pair >55% similar) = 2.3e-24   [draft claimed <1e-6]
bundle capacity      measured matches closed form to 4 decimals, K_max ≈ 308
two-stage search     100% agreement with exhaustive at 20% query noise
                     30.4x faster, 24.6x fewer bytes touched
                     ESP32-S3 0.44 ms/query, nRF54L15 0.83 ms/query
resonator            91.5% solved, mean 2.7 iterations over 32768 hypotheses
learning             100% in-domain, 0.00% false alarms out-of-domain
                     33/40 adversarial mislabels refused by the drift guard
RAM                  codebook 0 B (derived), sketch table 16 KB, working set 5 KB

protocol (firmware/test/test_net.c, new in rev 0.3):
crypto               300 random cases vs OpenSSL: 0 mismatches
                     (SHA-256, HMAC, HKDF, ChaCha20, Poly1305, AEAD)
framing              9-slot composed record round-trips exactly; 24 B of ids
                     rebuild a bit-identical 10240-bit vector at both ends
integrity            256/256 single-bit corruptions rejected on the AEAD tiers
                     Tier 0.5: 1 bit in -> 1 bit out (no avalanche, by design)
forgery              2000 attempts against a valid address: 0 accepted
rate limit           100 attempts in 1 s -> 20 reach the AEAD, 80 dropped
replay               24 replays of 8 frames: 0 accepted (single-use keys)
reorder              arrival order 3,0,7,1 -> 4/4 opened with correct counters
privacy              64 consecutive frames, 0 address repeats
flooding             10 relays, ttl 3 -> 10 transmissions, terminates in 2 rounds
cost                 15.8 us per seal+open round trip, vs 246 800 us of airtime
```

Two of those deserve calling out.

**RAM is 40 KB, not the 500 KB estimated.** Because codes are generated from
`(domain, id)` on demand — 160 PRNG steps, cheaper than a flash read — the
codebook costs *zero* bytes. Only the sketch table is materialised.

**The drift guard is the invariant made executable.** Quasi-orthogonality is
the assumption the entire architecture rests on. `proto_learn()` rejects any
update that would pull two prototypes closer than a floor, and rolls it back.
Fed 40 deliberately mislabelled samples, it refused 33 and held separation at
3013 bits. The assumption is checked at runtime, not hoped for.

---

## 12. Roadmap — every phase has a kill criterion

The original plan deferred both project-killing unknowns to the end: the
antenna inside a 30 mm plastic capsule against a wrist, and whether the volume
closes. Both would surface in month three, after the software was written.
This ordering surfaces them **in a weekend**.

### Phase 0 — de-risk (one weekend, ~R$400)

Two experiments, run before any firmware exists.

1. **Volume mockup.** 3D-print the 30×30×10 shell. Physically insert a real
   150 mAh cell, a dummy 28×28 PCB, and a piezo bender.
   *Kill: if it needs > 12 mm, the form factor changes — accept 34×34, or move
   to a puck/clip.*
2. **Range truth.** Two LilyGO T3S3 boards, SF9, +14 dBm. Strap one to a wrist.
   Walk a measured urban route logging RSSI/SNR and packet delivery.
   *Kill: if wrist-to-wrist urban PDR at 300 m is below 50%, **the wrist is the
   wrong place for a sub-GHz radio** — pivot the Core to a lapel or pack clip.*

That second kill criterion is the honest fork in this project, and the modular
capsule is already the hedge for it. A clip has better height and no body
block. Two days of walking answers it.

### Phase 1 — protocol on devkits
**Written and proven as of rev 0.3.** HCP rev 0.2, the symmetric ratchet, ephemeral
addresses, constant-airtime framing, Weave and Beat all exist in `firmware/net/`,
with an SX1262 driver and a bring-up console in `firmware/port/`. What remains is
measurement on real radios — [05-FIRMWARE.md](05-FIRMWARE.md) §6 takes you from an
unopened board to a meaning crossing a street. Do **not** reinvent mesh routing —
benchmark against Meshtastic.
*Kill: Tier 0 round trip > 3 s, or PDR < 80% at the Phase-0 measured range.*

### Phase 2 — algebra on target + the Tier 0.5 experiment
Port `firmware/core` to the S3. Confirm the projected 0.44 ms/query. Then the
falsification test: implicit header, CRC off, measure PDR versus distance for
sketch decode against framed decode.
*Kill: if Tier 0.5 yields < 2 dB, drop it and keep CRC'd ids. No loss — just
less novelty. Say so publicly; that is what makes the rest credible.*

### Phase 3 — power
BQ25570 + cell + `SetRxDutyCycle`. Measure with a proper analyser, not a
multimeter.
*Kill: leaf idle > 200 µA (4.8 mAh/day) means the solar story fails and the
framing becomes "weeks of battery", not "perpetual".*

### Phase 4 — PCB
4-layer, 28×28 mm, **pre-certified modules** (ESP32-S3-MINI-1 + an SX1262
module) rather than bare silicon. Saves RF tuning agony and eases certification.
Ground-plane keep-out under the antenna. Test the strap-fed Band antenna here —
it is the single biggest range lever (+10 dB, §6).

### Phase 5 — enclosure, seal, IP test
### Phase 6 — ten-unit field mesh trial

---

## 13. Cost, honestly

| | Draft | Real |
|---|---|---|
| Prototype BOM, 1 unit | R$ 179 | R$ 250–350 landed (import duty, FX spread) |
| Phases 0–4 all-in | — | **R$ 800–1200** (buy 3 of everything; you will kill one) |
| BOM at 1k units | R$ 85 | R$ 85 is BOM-only — correct |
| **Landed** at 1k units | — | **R$ 200–250** with assembly and test |
| Injection mould, O-ring groove | — | US$ 3 000–8 000 |
| ANATEL homologation (+ FCC/CE to export) | — | US$ 5 000–15 000 |

Prototypes need no homologation. A product does. Budget it before you promise
a price.

---

## 14. Open risks

| Risk | Severity | Resolved by |
|---|---|---|
| Wrist is RF-hostile; range unusable | **project-defining** | Phase 0 experiment 2 |
| Volume does not close even at 10 mm | high | Phase 0 experiment 1 |
| Strap-fed antenna varies with pogo contact impedance | medium | Phase 4; mitigate with the matching network at the feed |
| Tier 0.5 gain is under 2 dB | low | Phase 2; it is a bonus, not a dependency |
| Codec2 700C intelligibility on a piezo transducer | medium | Phase 1; fall back to 1300 bps |
| Mesh relay availability with mostly-leaf nodes | medium | Weave energy-aware role advertisement; needs Phase 6 to validate at scale |

---

## 15. Do this first

```bash
cd ~/Python/herus
./prove.sh                        # 6 suites, 28 code invariants, all green
```

That covers the algebra, the opt-in bounded Nucleus predictor, controlled local voice/haptic contracts, the protocol (crypto differentially tested against
OpenSSL, ratchet, replay, forgery, flooding, Beat drift), the SX1262 command
sequences against a recording mock bus, and every RF and energy number.

Then order two LilyGO T3-S3 boards and print the shell. The firmware is written and
proven — [05-FIRMWARE.md](05-FIRMWARE.md) takes you from an unopened board to a
meaning crossing a street. Phase 0 is two days and it decides whether Herus is a
wrist device.
