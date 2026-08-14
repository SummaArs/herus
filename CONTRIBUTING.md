# Contributing

This repository has one rule, and everything else follows from it:

> **An asserted number is a rumour. A measured number next to its closed form is
> a result.** If a figure appears in `docs/` and `./prove.sh` does not produce
> it, that is a bug in the documentation. If a property is claimed and no test
> can fail, it is a hope.

---

## 1. Get it running (two minutes, no hardware)

You need a C11 compiler, `make`, and Python 3. Nothing else — no ESP-IDF, no
board, no third-party C library.

```bash
git clone git@github.com:SummaArs/herus.git
cd herus
./prove.sh
```

Expected tail:

```
ALL INVARIANTS HOLD — the documents reproduce and the firmware is safe to flash.
```

Roughly 30 s on an Apple silicon Mac. `./prove.sh --quiet` prints verdict lines
only. Anything other than that final line means **do not flash and do not trust
the documents**.

| Target | Command | What it proves |
|---|---|---|
| Everything | `./prove.sh` | 28 suites + 65 proof invariants + 74 system invariants |
| Algebra | `cd firmware && make algebra` | Binding, bundling, resonator, HCP, dense vs sparse |
| Núcleo | `cd firmware && make nucleus` | Consentimento opt-in, memória limitada, confiança, expiração e apagamento local |
| Voz e háptica | `cd firmware && make voice` | Linguagem controlada, rascunho confirmável, SOS bloqueado e vibração limitada |
| Gateway de intenção | `cd firmware && make intent` | Sessão física, confiança, ambiguidade e contexto sem autoridade de envio |
| Diálogo local | `cd firmware && make dialogue` | Turno físico, fala transitória, cartões tipados, falha sem rede, apagamento e autoridade de transmissão zero |
| Laboratório de modelo | `cd firmware && make model-lab` | Perfil medido no alvo, orçamento de recurso, cobertura funcional/adversarial, zero rede/agência e escudo display-only |
| Política de memória | `cd firmware && make memory-policy` | Consentimento explícito, relevância conservadora, revisão obrigatória para dados sensíveis/de terceiros e nenhuma persistência no módulo |
| Captura de memória | `cd firmware && make memory-capture` | Gesto físico, janela limitada, entrega única, expiração, cancelamento e descarte/zeroização transitórios |
| Extração de candidatos | `cd firmware && make memory-extract` | Gramática local conservadora, origem/confiança tipadas, revisão de terceiros/sensível e nenhuma retenção de texto |
| Cofre de memória | `cd firmware && make memory-vault` | Cartão mínimo sem texto, autorização humana separada, AEAD com tag completa, piso monotônico e erase fail-closed em host |
| Consolidação humana | `cd firmware && make memory-consolidation` | Revisão física limitada, expiração sem retenção, conflito bloqueante, recibo explícito, recuperação por identificador e remoção controlada em host |
| Recuperação controlada | `cd firmware && make memory-retrieval` | Matching local tipado, consulta limitada, limiar, margem de ambiguidade, apresentação mínima e autoridade zero em host |
| Interface de recuperação | `cd firmware && make memory-retrieval-present` | Status simbólico one-shot, acesso físico canônico, incerteza sem vencedor, haptics abstratos limitados e autoridade zero em host |
| Grand Finale de memória | `cd firmware && make memory-finale` | Fixture composta de captura→extração→política→revisão→cofre→recuperação→apresentação, conflito/modelo bloqueantes e auditoria de autoridade zero em host |
| Coleção multi-cartão | `cd firmware && make memory-collection` | Até oito cartões mínimos, autorização/gesto físicos separados, índice AEAD, prepare→commit, recuperação autenticada, capacidade, remoção/compactação lógicas e rollback fail-closed em host |
| Índice privado de coleção | `cd firmware && make memory-collection-index` | Consulta tipada e física sobre coleção autenticada, budget transitório por sessão, match mínimo, ausência/ambiguidade sem vencedor, sem listagem ou abertura automática |
| Modelo de ameaças | `cd firmware && make threat-model` | Classificação fail-closed de evidência host, pendências de alvo e escopo residual para rádio, trust, memória, modelo, telemetria, plataforma e supply chain |
| Assurance Grand Finale | `cd firmware && make assurance` | Composição fail-closed de sessão, intenção, confirmação, trust, frescor, revogação e modelo |
| Capstone Grand Finale | `cd firmware && make capstone` | Ataque à cadeia diálogo→modelo→interação→trust; nenhum bypass do handoff físico confirmado |
| Readiness de hardware | `python3 tools/readiness_audit.py research/hardware_readiness_manifest.json --strict` | Gates pendentes, evidência obrigatória para aprovação e privacidade de logs |
| Ciclo de confiança Core↔Núcleo | `cd firmware && make trust` | Associação física dupla, SAS, persistência protegida, revogação e apagamento fail-closed |
| Enlace Core↔Núcleo | `cd firmware && make control-link` | AEAD, sequência, expiração e rejeição de replay sob um vínculo já ativo |
| Runtime | `cd firmware && make interaction` | Push-to-talk, confirmação, prazo, perda de fonte, envio único e telemetria local |
| Rig de validação | `cd firmware && make interaction-rig` | Sequenciamento determinístico de adaptadores e handoff único |
| Telemetria | `./tools/test_interactionlog.sh` | CSV normativo rejeita envio sem confirmação |
| Estudo pré-registrado | `python3 tools/test_interactionstudy.py` | Plano congelado, gates de Wilson e rejeição de envio inseguro |
| Protocol | `cd firmware && make net` | Crypto vs OpenSSL, ratchet, framing, Weave, Beat |
| Radio | `cd firmware && make radio` | SX1262 command sequences against a recording mock bus |
| ESP32-S3 app | `cd firmware && make syntax` | Type-checks the app against stub IDF headers, no board |
| The bench | `cd sim && make` | The unmodified firmware in a world of metres, ms and mA |
| Physical layer | `python3 tools/budget.py` | RF, energy and the frame ledger |

Regenerating the crypto vectors (`make vectors`) is the only step that needs a
third-party package — Python `cryptography` — and it is deliberately *not* part
of `prove.sh`: the vectors are committed precisely so the proof does not depend
on the machine that runs it.

## 2. What a change looks like here

1. **Write the failing test first.** `firmware/test/test_net.c` for the wire,
   `firmware/core/test_*.c` for algebra, Núcleo, voz/háptica, gateway, runtime e rig; `firmware/net/test_core_link.c` for controle autenticado; `tools/test_interactionstudy.py` for pesquisa; `firmware/test/test_radio.c` for
   the driver, `sim/scenarios.c` for behaviour of the system.
2. **Make it pass**, without weakening any other invariant.
3. **Add it to the ledger** in `prove.sh` if it is a property and not just a
   case: a `check` line whose grep can genuinely fail.
4. **Update the number, not the adjective.** If the change moves a figure in
   `README.md` or `docs/`, move it there too, in the same commit.
5. `./prove.sh` must end in `ALL INVARIANTS HOLD` before you push. CI runs the
   identical script on Linux and macOS, so a host-specific assumption fails
   there rather than on the bench.

## 3. Things that are settled, and why

Not rules for their own sake — each of these was paid for once already.

- **No floats in the hot path.** Fixed point or nothing.
- **No hypervector ever goes on air.** Ids only; the receiver rebuilds the
  vector. A test asserts this and it is not negotiable — it is the whole thesis.
- **No software P-256.** A scalar in MCU RAM defeats the only reason the secure
  element exists. Public key stays in the ATECC608A.
- **`HV_LUT_POPCOUNT` in every timing target.** Neither the Xtensa LX7 nor the
  Cortex-M33 has a popcount instruction, so a benchmark using the host's
  `POPCNT` is a fantasy, not a projection.
- **Region limits stay compile-time assertions.** An illegal frame must fail to
  build, not fail in the field.
- **`sim/` compiles `firmware/` unmodified.** There is no simulator-only
  variant. If there were, a passing run would mean nothing.
- **Keep the uncomfortable numbers.** The 365 m wrist range, the 10× flooding
  cost, the 93.5 % delivery, the indoor solar trickle. A document that only
  lists its wins is marketing. If a change makes one of them worse, say so in
  the same commit.

## 4. Style

C11, 4 spaces, no tabs, `-Wall -Wextra` clean. Comments explain **why**, not
what — the surrounding code is the reference for tone: state the constraint or
the mistake the line prevents. Commit messages: imperative subject, then the
number or the invariant that changed.

## 5. Before touching hardware

Read [docs/05-FIRMWARE.md](docs/05-FIRMWARE.md) §6 end to end, verify the pin
map with the selftest before the first flash (ninety seconds, saves an
afternoon), and never burn eFuse on a board you care about. Phase 0 and its two
pre-committed kill criteria are in
[docs/03-BUILD-GUIDE.md](docs/03-BUILD-GUIDE.md#phase-0--the-weekend-that-decides-the-project).
