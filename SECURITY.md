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
| Local build-input integrity in host | `provenance_audit.py` rejects drift in declared source/proof inputs, unsafe paths, secret-like metadata and unsupported trust claims; see [docs/29-PROVENIENCIA-LOCAL-BUILD.md](docs/29-PROVENIENCIA-LOCAL-BUILD.md) |
| Composed multi-card memory authority in host | `make memory-collection-finale` connects human-authorized admission, authenticated recovery, bounded typed query and one-shot abstention; it fails if an index auto-opens, legacy retrieval fallback or model authority appears; see [docs/30-GRAND-FINALE-COLECAO.md](docs/30-GRAND-FINALE-COLECAO.md) |
| Purpose-bound collection session in host | `make memory-physical-session` requires closed operation purpose, non-reused RAM session ID, nonzero transient nonce, canonical adapter assertion, bounded time, cancellation and consumption; collection/index reject an unbound session; see [docs/31-SESSAO-FISICA-PROPOSITO.md](docs/31-SESSAO-FISICA-PROPOSITO.md) |
| Durable session-reservation recovery in host | `make memory-physical-session-recovery` classifies authenticated `PREPARED`/`COMMITTED` markers against an adapter-declared durable floor, advances only an already-burned ID and blocks contradictions; it has no output path that restores a live session; see [docs/32-RECUPERACAO-RESERVA-SESSAO.md](docs/32-RECUPERACAO-RESERVA-SESSAO.md) |
| Post-reboot session quarantine in host | `make memory-physical-session-bootstrap` reinitializes the gate to `IDLE`, scrubs any active fixture and imports only the classified floor; old/piso IDs cannot validate or consume and every new session requires a new adapter assertion; see [docs/33-QUARENTENA-BOOT-SESSAO.md](docs/33-QUARENTENA-BOOT-SESSAO.md) |
| Final host memory-chain composition | `make memory-prehardware-finale` composes bootstrap, M14 and TM-04; any mismatch blocks/scrubs the gate and a success remains `IDLE`, never an active collection capability; see [docs/34-GRAN-FINALE-PRE-HARDWARE.md](docs/34-GRAN-FINALE-PRE-HARDWARE.md) |

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
- **No authenticated supply-chain assurance yet.** The local provenance audit
  detects drift against an unsigned, declared input set and rejects unsafe manifest
  shape, but TM-09 remains `PENDING_TARGET`. There is still no authenticated
  checkout, signed provenance, pinned/isolated builder, complete SBOM, independent
  reproducible build, artifact-to-source verification, vulnerability assessment or
  independent audit.
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
- **Collection composition is not physical memory authority.** M14 now requires
  host evidence that collection access was purpose-bound, time-limited and consumed,
  but it still does not prove a human performed a gesture, that vault authorization
  and collection share a protected root, that the adapter event is authentic, that
  freshness/non-replay survive reboot, that the target presenter preserves
  abstention, that RAM is protected, that access patterns are hidden, or that an
  ASR/LLM could not bypass a faulty target adapter or modified firmware.
- **The session gate is not an authenticator.** Its nonce, session floor and state
  live only in portable RAM; an adapter must provide event provenance, entropy,
  trusted monotonic time, reset/power-loss behavior, a durable floor or equivalent
  replay strategy, protected execution and target tests before any statement about
  button, gesture, PIN, biometrics, identity, liveness or anti-replay across reboot.
- **Reservation recovery is not durable replay resistance.** The C11 oracle
  classifies a snapshot whose authentication and durable floor are asserted by an
  adapter. It can consolidate a burned reservation ID or fail closed, but cannot
  authenticate the storage, order media writes, survive a real reboot, prove a new
  gesture or revive a live session. Anti-replay across reboot therefore remains
  pending until the selected backend, boot chain and fault campaign establish it.
- **Boot quarantine is not a physical reset defense.** The bootstrap overwrites a
  host `struct`, imports one floor and refuses to reconstruct nonce, purpose,
  deadline or use budget. It does not observe a reset, clear real RAM, authenticate
  a backend, protect boot code or prove that a post-boot event is new. Those remain
  target-adapter and hardware-test requirements.
- **The Gran Finale is not target readiness.** Its fixture drives real portable
  collection/index code over RAM and checks M14 plus TM-04, but it neither reads a
  physical reset nor operates a durable backend. `ready_for_target_validation` is
  a diagnostic that the host contracts compose; it is not permission to retain,
  open, send, fabricate, deploy or make a physical-security claim.

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
