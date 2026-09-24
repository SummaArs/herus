# HERUS

**HERUS é uma pesquisa de arquitetura simbiótica verificável.** O projeto investiga se um núcleo persistente pode observar um hospedeiro, aprender uma rotina finita, remapeá-la para outro hospedeiro e produzir uma proposta revisável sem transformar capacidade em autoridade.

> **Estado atual:** `host-only / mechanism candidate / not_proven`.

O HERUS não é uma AGI, não é um produto lançado, não é um sistema de execução segura e ainda não provou simbiose útil ou benefício humano. O repositório contém uma cadeia de engenharia, pesquisa e documentação para descobrir se essas afirmações poderiam algum dia ser justificadas.

## Comece aqui

| Ordem | Documento | Pergunta respondida |
|---:|---|---|
| 1 | [Leia isto primeiro](docs/00-LEIA-ME-PRIMEIRO.md) | Como navegar pelo projeto? |
| 2 | [Linha do tempo](docs/01-LINHA-DO-TEMPO.md) | O que o HERUS foi, como mudou e por quê? |
| 3 | [Mapa da arquitetura](docs/02-MAPA-DA-ARQUITETURA.md) | Como as partes se relacionam? |
| 4 | [Mapa do código](docs/03-MAPA-DO-CODIGO.md) | Onde cada conceito vive? |
| 5 | [Mapa da evidência](docs/04-MAPA-DA-EVIDENCIA.md) | O que foi demonstrado e o que não foi? |
| 6 | [Glossário](docs/05-GLOSSARIO.md) | O que cada termo significa? |
| 7 | [Roadmap e gates](docs/06-ROADMAP-E-GATES.md) | Qual é a próxima sequência legítima? |
| 8 | [Direção soberana](docs/57-DIRECAO-SOBERANA-E-SIMBIONTE.md) | Qual é o futuro objetivo do HERUS? |

## O núcleo da tese

O **ASA Core** mantém uma identidade persistente, observa um hospedeiro, aprende transições finitas e compõe uma Skill verificável. Ao mudar de hospedeiro, ele precisa reconstruir o contexto local. O objetivo futuro é que o Symbiont habite sistemas autorizados sem retirar os dados privados do domínio do usuário, usando uma autoridade física, revogável e auditável.

O mecanismo mais defensável está no pacote [Symbiont v2](research/symbiont_v2/README.md). O primeiro produto de pesquisa é o [HERUS Bridge](docs/56-RODADA-DECISIVA-HERUS-BRIDGE.md), uma demonstração local de proposta, prévia, confirmação, cancelamento, abstenção e parada sem transmissão nem efeitos externos.

## O que já existe

- Firmware C com contratos de semântica, memória, interação, confiança, recuperação e ameaças.
- Simulador C com mundo controlado, distância e adversários.
- Protótipos Python para Semantic IR, raciocínio finito, Symbiont e avaliação de utilidade.
- Holdouts históricos e uma campanha causal isolada para efeitos ocultos.
- Manifestos de hardware e proveniência local.
- Documentação histórica desde o comunicador semântico original até a auditoria ASA.

## O que ainda não existe

Ainda não existe prova de autonomia, alcance de rádio, consumo, temperatura, ergonomia, segurança física, benefício social, acessibilidade populacional, adaptação a qualquer dispositivo, linguagem aberta, execução externa ou simbiose geral.

## Verificação

```bash
./prove.sh --quiet
make -C research test
PYTHONPATH=research python3 -m research.bridge_causal
PYTHONPATH=research python3 -m research.bridge_product
python3 tools/provenance_audit.py --strict research/software_provenance_manifest.json
```

Na última verificação local, a pesquisa passou com **148 testes e um skip**, e o proof integral confirmou as invariantes host-only e de simulação. Isso é uma regressão interna, não uma prova de produto ou de campo.

## Documentos de decisão

| Documento | Papel |
|---|---|
| [Documento mestre](docs/00-HERUS-MASTER.md) | Tese histórica do comunicador semântico e da arquitetura física. |
| [Arquitetura finita](docs/48-ARQUITETURA-FINITA-E-LINGUAGEM.md) | Semântica controlada e linguagem subordinada. |
| [Semantic IR](docs/50-INTENT-COMPILER-E-SEMANTIC-IR.md) | Compilação de intenção e representação intermediária. |
| [API Symbiont v2](docs/51-API-SIMBIONTE-V2.md) | Fronteira host-independent e transferência finita. |
| [Simbiose útil](docs/52-DEFINICAO-SIMBIOSE-UTIL.md) | Contrato congelado de benefício e segurança. |
| [Holdout adversarial](docs/53-BENCHMARK-HOLDOUT-ADVERSARIAL.md) | Contraexemplos e falsificação do mecanismo. |
| [Observabilidade checked](docs/54-CORRECAO-CONTRATO-OBSERVABILIDADE.md) | Ledger e decisão fail-closed. |
| [Etapas finais](docs/55-ETAPAS-FINAIS-EXECUTOR-HUMANO.md) | Executor sintético, holdout estendido e protocolo social. |
| [Rodada decisiva](docs/56-RODADA-DECISIVA-HERUS-BRIDGE.md) | P0, campanha causal e HERUS Bridge. |
| [Direção soberana](docs/57-DIRECAO-SOBERANA-E-SIMBIONTE.md) | Soberania de dados, Symbiont transferível e futuro objetivo. |

## Código por papel

- [Firmware](firmware/README.md)
- [Pesquisa](research/README.md)
- [Simulador](sim/README.md)
- [Ferramentas](tools/README.md)
- [Índice completo de documentos](docs/README.md)

## Licença

Proprietary. Copyright © 2026 Gustavo Gonçalves. Todos os direitos reservados — veja [LICENSE](LICENSE).
