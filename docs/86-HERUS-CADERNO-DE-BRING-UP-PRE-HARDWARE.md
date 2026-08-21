# HERUS — Caderno de bring-up pré-hardware

**Estado:** protocolo preparado; hardware ainda não testado.  
**Manifesto:** `pre_hardware`.  
**Baseline obrigatório:** `./prove.sh --quiet` com proveniência válida.

## 1. Finalidade

Este caderno define como o HERUS deverá sair da prova host-side para a bancada física sem confundir uma medição local com validação do sistema inteiro. Cada gate possui uma pergunta única, um adaptador delimitado, um instrumento, evidências mínimas e condições de abortamento. Um gate falho bloqueia os dependentes; nenhuma conclusão é promovida automaticamente.

O protocolo completo está em [`research/bench_bringup_protocol.json`](../research/bench_bringup_protocol.json). Ele contém **12 gates**, cada um com `abort_if`, evidência e dependências explícitas.

> A bancada não deve descobrir o significado de autoridade, quarentena ou conflito. Ela deve verificar se os adaptadores físicos respeitam contratos que já foram definidos e exercitados no host.

## 2. Ordem dos gates

| Ordem | Gate | Pergunta única | Dependências |
|---:|---|---|---|
| 1 | `G01 host-baseline` | O release host permanece íntegro antes do hardware? | Nenhuma |
| 2 | `G02 board-pin-map` | A placa real corresponde ao pin map declarado? | G01 |
| 3 | `G03 haptic-fixture` | DRV2605L e atuador reproduzem HAP-SEM no fixture? | G02 |
| 4 | `G04 energy-instrumentation` | O consumo é medido com unidade e workload declarados? | G02 |
| 5 | `G05 clock-and-reset` | Clock, reset e brownout preservam monotonicidade e quarentena? | G04 |
| 6 | `G06 nvs-crash-recovery` | A persistência recupera interrupções conforme a matriz? | G05 |
| 7 | `G07 physical-session-and-trust` | Pareamento, revogação, replay e quarentena funcionam no alvo? | G06 |
| 8 | `G08 local-interaction` | Contato, percepção local e haptic não criam envio implícito? | G03, G04 |
| 9 | `G09 radio-one-metre` | Dois alvos trocam frames autenticados no perfil mínimo? | G02 |
| 10 | `G10 urban-pdr-comparison` | A rota pré-registrada sustenta os resultados declarados? | G09 |
| 11 | `G11 local-model-profile` | O modelo local respeita memória, latência, energia e agência? | G07, G08 |
| 12 | `G12 human-contact` | Pessoas entendem e toleram o comportamento sutil? | G11 |

A ordem é deliberadamente conservadora. Pin map vem antes de periféricos; instrumentação de energia vem antes de medir autonomia; interrupção vem antes de confiar em persistência; interação vem antes do modelo completo; e contato humano é o último gate, porque só faz sentido quando o comportamento técnico está identificado.

## 3. Evidência e abortamento

Toda execução deve identificar fixture, revisão da placa, revisão do firmware, protocolo, run, instrumento e digest do artefato bruto. O log de produto continua proibido de conter áudio, transcrição, embedding, identidade, localização, chave ou conteúdo de mensagem. Evidência incompleta mantém o gate em `pending`; não existe estado “provavelmente passou”.

| Classe de abortamento | Exemplos | Consequência |
|---|---|---|
| Identidade física incerta | revisão desconhecida, pin map divergente | Interromper antes de energizar dependentes |
| Medição inválida | unidade ausente, saturação, workload alterado | Descartar resultado e repetir com instrumento válido |
| Autoridade indevida | replay aceito, confirmação implícita, Core executando ação | Gate falha e dependentes ficam bloqueados |
| Persistência ambígua | PREPARED promovido, floor rebaixado, cleanup sem prova | Falha de recuperação; não atualizar manifesto |
| Privacidade | payload proibido em log ou fixture | Interromper, isolar artefato e não publicar |
| Segurança física | safety stop falho, saída contínua, reclamação humana | Abortamento imediato e revisão do fixture |

## 4. O que não será alegado

O protocolo não autoriza alegar alcance, PDR, autonomia, consumo, WER, acurácia, compreensão ou aceitação antes do gate correspondente. Um resultado em fixture de haptic não prova que uma pessoa interpretou o sinal. Um resultado de rádio a um metro não prova alcance urbano. Um perfil de modelo no alvo não prova inteligência pessoal contínua. Uma sessão de interação não prova indispensabilidade.

O Core externo continua restrito a carregador, antena e alimentador autorizado de conhecimento. Nenhum gate físico pode promovê-lo a executor ou cérebro. A inteligência pessoal permanece no vestível; cada adaptação de hardware deve preservar essa direção.

## 5. Reversibilidade

Até o gate correspondente passar, cada ensaio deve ser isolado, reversível e limitado ao adaptador em questão. A sequência deve permitir voltar ao host sem carregar confiança física parcial para o produto. O manifesto só muda de `pending` para evidência específica após o artefato bruto, método, instrumento e revisão serem registrados.

A transição correta não é “o hardware funcionou”. É uma cadeia de afirmações delimitadas: **esta placa**, **este driver**, **este instrumento**, **este workload**, **esta rota**, **este perfil**, **este protocolo**. Fora desse conjunto, a conclusão permanece desconhecida.

## 6. Estado atual

O protocolo está preparado, mas todos os gates físicos permanecem pendentes. O host-side continua sendo a única evidência disponível para a lógica de autoridade, memória, conflito, reboot, presença e degradação. A próxima ação física, quando houver hardware e instrumentos, deve começar em `G01` e `G02`; não é seguro saltar diretamente para modelo completo ou teste humano.

## Reprodução do pré-requisito host

```bash
cd /home/ubuntu/herus-semantic-compiler-pr
python3 -m json.tool research/bench_bringup_protocol.json >/dev/null
python3 tools/provenance_audit.py --strict research/software_provenance_manifest.json
./prove.sh --quiet
```
