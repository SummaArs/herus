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

A nova hipótese algorítmica é o [Symbiotic Learning v2](docs/84-ALGORITMO-SYMBIOTIC-LEARNING.md): indução contextual, bounded e reversível de efeitos observáveis para transferir uma Skill entre hospedeiros. O nome descreve uma linha de pesquisa proposta; ainda não é um campo científico estabelecido nem uma alegação de superioridade sobre os paradigmas existentes.

Sobre essa camada, o [Meta-Simbionte](docs/85-META-SIMBIONTE-E-HISTORICO-DE-SOLUCOES.md) mantém um histórico append-only de soluções verificadas. O histórico orienta novas sondagens e personaliza propostas, mas nunca substitui evidência fresca, autoridade ou confirmação humana.

## O que já existe

- Firmware C com contratos de semântica, memória, interação, confiança, recuperação e ameaças.
- Simulador C com mundo controlado, distância e adversários.
- Protótipos Python para Semantic IR, raciocínio finito, Symbiont e avaliação de utilidade.
- Holdouts históricos e uma campanha causal isolada para efeitos ocultos.
- Benchmark real MIntRec com holdout por temporada e comparação contra Naive Bayes, 1-NN, centróides e maioria.
- Comparação entre paradigmas supervisionado, não supervisionado, auto-supervisionado e reforço proxy no mesmo holdout real; nenhum resultado atual supera o supervisionado.
- Primeiro baseline transformer real: BERT pequeno, S04→S05→S06, 17,62% no holdout; comparação direta concluída sem alegação de vitória.
- Probe de atalhos em 386 utterances reais: prefixos superficiais mudaram até 55,18% das previsões dos baselines, revelando fragilidade que acurácia sozinha não mostra.
- Normalização canônica limitada recuperou 99,22%–100% das previsões após os fillers testados, sem apagar palavras internas; a correção permanece restrita ao vocabulário declarado.
- Habilidade de programação importável inspirada no Jev: decompõe tarefas tipadas e gera propostas de código/testes sem executar, escrever arquivos ou conceder autoridade.
- Ledger de programação inspirado no REA: propostas só fecham quando casos positivos, negativos e malformados têm evidência autenticada e autoridade comparável.
- Auditoria crítica de erros e contrato executável que bloqueia alegações de vitória contra transformers ou de simbiose geral sem evidência equivalente.
- Revisão acadêmica sobre simbolismo puro: núcleo constitucional 100% simbólico; aprendizado limitado nas camadas de percepção e proposta.
- Manifestos de hardware e proveniência local.
- Documentação histórica desde o comunicador semântico original até a auditoria ASA.

## Foco atual: algoritmo

O objetivo imediato é concluir e falsificar o algoritmo Symbiotic Learning e seu meta-aprendizado. A conformidade C11 e o protocolo externo são tratados como implementações de biblioteca e testes de interoperabilidade; não são uma solicitação para iniciar validação física.

A superfície importável está em [`herus_symbiotic`](docs/91-BIBLIOTECA-PUBLICA-SYMBIOTIC-LEARNING.md), versão `0.1.0`. A API é proposal-only: importar e inferir não executa ações, acessa rede ou concede autoridade.

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

Na última verificação local, a pesquisa passou com **198 testes e um skip**, e o proof integral confirmou as invariantes host-only e de simulação. A campanha ampla, o holdout local, o harness black-box, a API pública, a auto-supervisão experimental, o baseline transformer, o probe de atalhos, a normalização limitada, a habilidade de programação proposal-only, o ledger de obrigações e o protocolo entre processos também separam acerto seguro de falso aceite. Isso é uma regressão interna, não uma prova de produto ou de campo.

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
| [Symbiotic Learning](docs/84-ALGORITMO-SYMBIOTIC-LEARNING.md) | Algoritmo bounded de indução de efeitos e transferência de Skills. |
| [Meta-Simbionte](docs/85-META-SIMBIONTE-E-HISTORICO-DE-SOLUCOES.md) | Histórico verificável e adaptação personalizada sem reutilização cega. |
| [Campanha ampla](docs/86-CAMPANHA-WIDE-SYMBIOTIC-LEARNING.md) | 100 casos, baselines e falso aceite separado do acerto. |
| [Holdout independente](docs/87-HOLDOUT-INDEPENDENTE-SYMBIOTIC.md) | 60 fixtures separados e oracle não fornecido ao learner. |
| [Host black-box](docs/88-HOST-BLACK-BOX-E-ROTACAO.md) | Estado parcial, ações renomeadas e rotação de interface. |
| [Protocolo externo](docs/89-PROTOCOLO-EXTERNO-E-ISOLAMENTO.md) | Host em processo separado, JSONL, corrupção e timeout fail-closed. |
| [Host JSONL v1](docs/90-PROTOCOLO-HERUS-HOST-JSONL-V1.md) | Contrato congelado de interoperabilidade entre learner e hospedeiro. |
| [Biblioteca pública](docs/91-BIBLIOTECA-PUBLICA-SYMBIOTIC-LEARNING.md) | API importável v0.1.0, proposal-only e sem efeitos colaterais. |
| [Benchmark real](docs/92-BENCHMARK-DADOS-REAIS-E-BASELINES.md) | MIntRec S04/S05→S06 contra baselines clássicos; revela cobertura baixa da memória simbiótica. |
| [Paradigmas de ML](docs/94-COMPARACAO-PARADIGMAS-ML-REAIS.md) | Comparação real com supervisionado, K-means e bandit proxy, sem alegar equivalência indevida. |
| [Transformer real](docs/96-TRANSFORMER-REAL-MINTREC.md) | BERT pequeno executado no mesmo holdout temporal, com custo e limites registrados. |
| [Simbolismo puro](docs/97-SIMBOLISMO-PURO-E-SYMBIOTIC-LEARNING.md) | Decisão baseada em papers sobre o que deve ser simbólico e onde o aprendizado é necessário. |
| [Probe de atalhos](docs/98-PROBE-ATALHOS-MINTREC.md) | Teste de estabilidade contra fillers em holdout real; revela dependência de superfície. |
| [Habilidade de programação](docs/99-HABILIDADE-PROGRAMACAO-JEV-INSPIRED.md) | Decomposição tipada e propostas de código/testes sem execução ou autoridade. |
| [Auditoria REA](docs/100-AUDITORIA-REA-E-LEDGER-DE-PROGRAMACAO.md) | Evidência autenticada, obrigações abertas e autoridade comparável para propostas de código. |
| [Auditoria crítica](docs/95-AUDITORIA-CRITICA-ERROS-E-CORRECOES.md) | Erros metodológicos registrados e regras permanentes contra overclaiming. |

## Código por papel

- [Firmware](firmware/README.md)
- [Pesquisa](research/README.md)
- [Simulador](sim/README.md)
- [Ferramentas](tools/README.md)
- [Índice completo de documentos](docs/README.md)

## Licença

Proprietary. Copyright © 2026 Gustavo Gonçalves. Todos os direitos reservados — veja [LICENSE](LICENSE).
