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
ALL INVARIANTS HOLD — host contracts pass; controlled bench flash may begin, physical gates remain pending.
```

Roughly 30 s on an Apple silicon Mac. `./prove.sh --quiet` prints verdict lines
only. Anything other than that final line means **do not flash and do not trust
the documents**.

| Target | Command | What it proves |
|---|---|---|
| Everything | `./prove.sh` | 39 suites + 87 proof invariants + 74 system invariants |
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
| Recuperação transacional | `cd firmware && make memory-collection-recovery` | Matriz C11 para estados autenticados de `PREPARED`/`COMMITTED` e piso: promoção, descarte pré-piso, finalização de limpeza ou bloqueio sem recuperação ambígua |
| Grand Finale da coleção | `cd firmware && make memory-collection-finale` | Fixture composta de captura→política→revisão→autorização→coleção→recuperação→índice→apresentação; abstenção válida, sem abertura automática, fallback unitário ou autoridade de modelo |
| Sessão física de coleção | `cd firmware && make memory-physical-session` | Propósito fechado, ID/nonce transitórios, janela, cancelamento e consumo único ou limitado; prova host, não botão, pessoa, biometria, relógio ou anti-replay pós-reboot |
| Recuperação de reserva de sessão | `cd firmware && make memory-physical-session-recovery` | Matriz C11 de `PREPARED`/`COMMITTED` contra piso durável declarado: promove somente ID já queimado, descarta/limpa com topologia coerente, bloqueia contradições, IDs terminais e nunca reativa sessão |
| Prova de fogo de recuperação | `cd firmware && make memory-physical-session-recovery-stress` | F1 gera 120.000 snapshots sintéticos e mutações de topologias, compõe recuperação com bootstrap e exige apenas quarentena ociosa ou bloqueio limpo |
| Prova de fogo de coleção | `cd firmware && make memory-collection-recovery-stress` | F2 gera snapshots/mutações de crash-state e exige ação compatível ou bloqueio sem mutar entrada |
| Prova de fogo de ameaças | `cd firmware && make threat-model-stress` | F3 exerce formato e escopo hostis e impede escalonamento de modelo, telemetria ou alvo a mitigação host |
| Mutações da prova de fogo | `cd firmware && make proof-fire-mutations` | F4 remove temporariamente controles em cópias privadas e exige que T12/T15/F1/T9 detectem a remoção |
| Quarentena de boot de sessão | `cd firmware && make memory-physical-session-bootstrap` | Reconstrói gate em `IDLE`, importa somente piso classificado, apaga toda evidência ativa e exige nova afirmação para sessão posterior |
| Gran Finale pré-hardware | `cd firmware && make memory-prehardware-finale` | Compõe bootstrap, coleção/index reais em fixture RAM, auditor M14 e TM-04; só emite diagnóstico com gate `IDLE`, sem autoridade ativa |
| Modelo de ameaças | `cd firmware && make threat-model` | Classificação fail-closed de evidência host, pendências de alvo e escopo residual para rádio, trust, memória, modelo, telemetria, plataforma e supply chain |
| Assurance Grand Finale | `cd firmware && make assurance` | Composição fail-closed de sessão, intenção, confirmação, trust, frescor, revogação e modelo |
| Capstone Grand Finale | `cd firmware && make capstone` | Ataque à cadeia diálogo→modelo→interação→trust; nenhum bypass do handoff físico confirmado |
| Readiness de hardware | `python3 tools/readiness_audit.py research/hardware_readiness_manifest.json --strict` | Gates pendentes, evidência obrigatória para aprovação e privacidade de logs |
| Proveniência local | `python3 tools/provenance_audit.py research/software_provenance_manifest.json --strict` | Schema estrito, insumos locais hashados, inventário direto e gates pendentes; não é atestação assinada, SBOM completo ou SLSA |
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
   case: a `check` line whose grep can genuinely fail. Uma mudança em acesso da
   coleção também deve exercitar `memory-physical-session`, coleção, índice e
   Grand Finale, `memory-physical-session-recovery`, `memory-physical-session-recovery-stress`, `memory-physical-session-bootstrap` e `memory-prehardware-finale` quando tocar propósito, consumo, janela, piso, IDs/limites numéricos, boot, reboot, recuperação, coleção ou fallback.
4. **Update the number, not the adjective.** If the change moves a figure in
   `README.md` or `docs/`, move it there too, in the same commit.
5. **Se mexeu na documentação**, rode `python3 tools/docs_index.py` e
   `python3 tools/check_links.py`: o índice é gerado e os links são provados.
6. `./prove.sh` must end in `ALL INVARIANTS HOLD` before you push. CI runs the
   identical script on Linux and macOS, so a host-specific assumption fails
   there rather than on the bench.

## 2b. Mexer na camada do significado

Três regras, e cada uma existe porque o contrário já custou caro em algum
projeto.

**Vocabulário é DADO, não código.** Nunca edite `firmware/core/loom_core.{h,c}`
— eles são gerados. Mexa em `research/loom/core/*.hlx.json` e rode
`python3 tools/loom.py --emit`. O `prove.sh` verifica que os arquivos gerados
são exatamente o que o tear gera, então uma edição à mão vira falha na próxima
rodada.

**Um símbolo nunca muda de significado.** O campo `n` de um conceito é imutável.
Um pacote v2 pode acrescentar 60, nunca redefinir 7. Redefinir faria dois
aparelhos com versões diferentes concordarem nos bytes e discordarem no que eles
querem dizer — a pior falha possível de um protocolo semântico.

**Se a busca adversarial achar uma classe de defeito, a classe vira
enumeração.** Não conserte só o caso: acrescente o enumerador que cobre a classe
inteira em `tools/loom.py`, para que a busca fique livre para procurar a próxima
em vez de reencontrar a mesma. Foi assim que a enumeração de pares cresceu duas
vezes nesta revisão.

E o de sempre, que aqui aparece com mais força: **a suíte é a autoridade, não o
espelho.** `tools/babel_ref.py` é referência executável, `firmware/core/babel.c`
é o artefato que embarca. Quando os dois discordam,
`tools/test_babel_cross.py` acusa, e a pergunta certa é qual dos dois está
errado — nunca "ajusta o teste". Nesta revisão o errado foi o espelho.

Ao acrescentar um idioma, espere que o portão acuse coisas que você não previu.
Ele achou uma ambiguidade genuína do espanhol (*mañana* é manhã e amanhã) e uma
do japonês (a forma de ESPERA continha a forma de POUCO). Isso é o portão
funcionando, não o portão sendo chato.

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
- **Recusar é capacidade, não modo de falha.** Uma lacuna tipada é um pedido de
  capacidade com endereço. Um compilador que adivinha não é um compilador, e
  "quase certo" é pior que "não sei" em todo lugar deste repositório.
- **O portador chega certo ou não chega.** O Aether nunca entrega um significado
  diferente do enviado. Não entregar é aceitável — a pessoa repete. Entregar
  outra coisa faria alguém agir sobre uma frase que ninguém disse.
- **Registre o resultado negativo.** Se você tentou uma melhoria, mediu, e ela
  não pagou, deixe o número no comentário. Tolerar um símbolo de preâmbulo
  errado leva as entregas certas de 1840 para 1841 em 2000 ensaios — e essa
  linha existe para que a próxima pessoa não gaste a tarde de novo.
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

Read [docs/10-arquitetura/05-FIRMWARE.md](docs/10-arquitetura/05-FIRMWARE.md) §6 end to end, verify the pin
map with the selftest before the first flash (ninety seconds, saves an
afternoon), and never burn eFuse on a board you care about. Phase 0 and its two
pre-committed kill criteria are in
[docs/60-hardware/03-BUILD-GUIDE.md](docs/60-hardware/03-BUILD-GUIDE.md#phase-0--the-weekend-that-decides-the-project).
