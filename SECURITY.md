# Security

Herus is a radio that carries private messages, so the honest starting point is
what it protects today — not what the design intends to protect eventually.

**Status: pre-hardware. No independent audit. Not yet run on silicon.**
Nothing here should be relied on to protect a person from a capable adversary
until Phase 4 closes the gaps listed below.

---

## What is implemented and proven

All of the following are checked by `./prove.sh` on every run — see
[docs/02-PROTOCOL.md](docs/02-PROTOCOL.md) §4 for the normative statements.

| Property | How it is proven |
|---|---|
| Confidentiality and integrity per frame | ChaCha20-Poly1305 AEAD, differentially tested against Python `cryptography` (OpenSSL) on RFC vectors *and* 400 random cases; a primitive that disagrees fails the build |
| Forward secrecy | Symmetric ratchet: one key per message, deleted after use. Yesterday's traffic stays unreadable if today's state leaks |
| Replay refused forever | Skip-window plus a monotonic counter; the suite replays every frame and every one is refused |
| Forgery bounded at 2⁻⁶⁴ per attempt | Exhaustive tag-mutation pass over the frame |
| Every single-bit corruption rejected | Full bit-flip sweep of the frame |
| No stable identifier on air (P6) | Addresses are ratcheted; the suite asserts nothing constant survives across frames, so there is nothing to track |
| Header integrity with a mutable TTL | TTL is outside the tag, the address is inside (errata E-P2); the suite decrements TTL and the frame still verifies end to end |
| Flooding terminates | Weave dedup bounds relay fan-out; the bench floods and the count is bounded |
| The decrypt path is rate limited | An unauthenticated stranger cannot drain the battery by making the node do work |
| Constant airtime across meaning tiers (P1) | Traffic analysis cannot read the tier off the air, because every meaning-carrying frame occupies the radio for the same 246.8 ms |
| Evidence-scoped threat classification | `make threat-model` rejects incomplete/noncanonical control evidence and distinguishes host-mitigated controls from target-pending and out-of-scope vectors; see [docs/25-MODELO-AMEACAS-EXECUTAVEL.md](docs/25-MODELO-AMEACAS-EXECUTAVEL.md) |
| Bounded multi-card collection in host | `make memory-collection` requires authorization plus physical access, encrypts/authenticates a fixed index and rejects duplicate, capacity, transaction, tag and rollback failures; see [docs/26-COLECAO-MEMORIA.md](docs/26-COLECAO-MEMORIA.md) |
| Private collection retrieval in host | `make memory-collection-index` permits only bounded typed queries under physical access, preserves abstention/ambiguity and never opens a card automatically; see [docs/27-INDICE-PRIVADO-COLECAO.md](docs/27-INDICE-PRIVADO-COLECAO.md) |
| Transactional crash-state recovery in host | `make memory-collection-recovery` promotes only an authenticated successor bound to the durable floor, discards pre-floor preparation and blocks contradictory state; see [docs/28-RECUPERACAO-TRANSACIONAL.md](docs/28-RECUPERACAO-TRANSACIONAL.md) |

## What is NOT protected yet

From [docs/05-FIRMWARE.md](docs/05-FIRMWARE.md) §4, restated as security
consequences:

- **No post-compromise security.** Sessions start from a pre-shared root key.
  `session_dh_ratchet()` is written and tested but has nothing to call: P-256
  ECDH lives in the ATECC608A, which is not wired (Phase 4). Say *forward
  secrecy*; never say *Signal-grade*.
- **No SOS signing.** Same cause: no ECDSA without the secure element.
- **Session keys sit in readable RAM.** Until flash encryption, secure boot,
  JTAG-off and UART-download-off are burned into eFuse, physical possession of
  a board is full compromise of that board. "The key never leaves the chip" is
  true and irrelevant until that ritual is done — and it is one-way, so it is
  done on a sacrificial board first.
- **No ratchet persistence.** A reboot loses the session and the pair must
  re-pair. That is a denial of service, not a leak, but it is real.
- **The cryptography is hand-written.** ~600 lines, deliberately, so it is
  auditable in an afternoon and identical on host and target. The mitigation
  for implementation error is differential testing, not confidence. It has
  **not** been reviewed by anyone but its author and its test suite, and the
  primitives are not constant-time-audited against physical side channels.
- **Metadata beyond the frame.** The protocol hides the identifier and the
  tier. It does not hide that a transmission happened, or from roughly where.
  Direction-finding a LoRa burst is not hard.
- **No supply-chain assurance yet.** The executable threat model makes this
  absence explicit; there is not yet an SBOM, signed/reproducible build policy,
  provenance verification or independent audit.
- **Threat classification is not runtime security.** The host auditor checks
  declared evidence fixtures and fails closed; it does not inspect a deployed
  device, estimate likelihood/impact, detect compromise or close platform gates.
- **Collection deletion is not physical purge.** The multi-card collection only
  removes an active logical reference and rewrites authenticated state. Old bytes,
  power-loss behavior, wear, secure storage and purge semantics belong to the
  selected backend and must be demonstrated on the chosen target.
- **Private retrieval is not cryptographic query privacy.** The index copies up to
  eight authenticated cards into transient RAM under an asserted physical session.
  It has no persisted query log or public list API, but does not implement PIR,
  ORAM, side-channel resistance, protected RAM, a real physical control or a
  privacy property against modified firmware.
- **Crash-state recovery is not durable storage assurance.** The portable oracle
  classifies decoded/authenticated records and an asserted monotonic floor. It does
  not establish that callbacks survive brownout, that flash writes are atomic, that
  a root or counter is protected, that old bytes are purged, or that a faulting
  target cannot present a topology the host contract blocks.

## Regulatory safety

`firmware/net/region.h` encodes dwell, duty cycle and power limits as
compile-time assertions — a frame that would be illegal in the configured
region does not build. Those assertions are the author's reading of FCC 15.247,
ANATEL 680/2017 + Ato 14448 and ETSI EN 300 220 as of 2026. Verify them against
the rules where you actually switch the hardware on; see the notice in
[LICENSE](LICENSE).

## Reporting a vulnerability

This is a private repository with a single maintainer. Report privately, with
enough detail to reproduce:

- GitHub → **Security** → *Report a vulnerability* (private advisory), or
- email the maintainer directly.

Please do not open a public issue for a cryptographic or radio-safety finding.
Expect a first response within seven days.

A finding is most useful when it arrives as a **failing test**: a case added to
`firmware/test/test_net.c` (or a scenario in `sim/`) that fails on the current
tree. See [CONTRIBUTING.md](CONTRIBUTING.md) — the standard here is that a
property with no test that can fail is a hope, not a property.
