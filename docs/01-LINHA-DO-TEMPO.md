# HERUS — Linha do tempo didática

## Como ler a história

A história do HERUS não é uma linha reta de funcionalidades. Ela alterna entre três movimentos: formular uma tese, construir um mecanismo e reduzir a tese quando a evidência não acompanha a ambição. A redução não é fracasso de projeto; é o mecanismo de pesquisa funcionando.

## Fase 1 — O comunicador semântico

**12–13 de agosto de 2026.** O repositório nasce com uma tese de produto: enviar significado em vez de transportar áudio ou texto bruto. A ideia central é um comunicador pessoal de baixa largura de banda, com léxico compartilhado, protocolo simbólico, rádio LoRa, renderização local e interação curta.

Os primeiros marcos incluíram o documento mestre, a álgebra HERUS, o protocolo de fio, o guia de build, o firmware, o núcleo local, a voz controlada e o feedback háptico. O projeto já começou com uma preocupação correta: medir orçamento de bytes, airtime, energia e autoridade em vez de depender de slogans.

**O que esta fase deixou:** a intuição de que uma mensagem pode ser uma intenção estruturada, não apenas uma sequência de sons. **O que ela não provou:** que pessoas preferem ou compreendem o comunicador, que o rádio físico cumpre os números ou que significado simbólico é superior em uso real.

## Fase 2 — Núcleo, interação e confiança

**13–14 de agosto.** O projeto ganha um Core, um Núcleo, um enlace autenticado, um ciclo de confiança, gateway de intenção, diálogo local e laboratório de aceitação de modelo. A arquitetura passa a separar percepção, proposta, confirmação física e execução.

O princípio que permanece é: **o modelo pode sugerir; a autoridade vem de uma fronteira externa e explícita**. Essa separação aparece mais tarde no Symbiont como distinção entre proposta e execução.

## Fase 3 — Memória seletiva e recuperação

**13–14 de agosto.** A pesquisa se expande para memória seletiva: relevância, captura transitória, extração de candidatos, cofre cifrado, consolidação humana, recuperação tipada e apresentação física one-shot.

Depois vêm coleção multi-cartão, índice privado, recuperação transacional, sessões vinculadas a propósito, recuperação após reboot e quarentena de boot. O firmware passa a conter muitos contratos de falha fechada.

**O que esta fase deixou:** uma linguagem de engenharia para autorização, retenção mínima, recuperação e não apagamento silencioso. **O risco introduzido:** a quantidade de contratos pode produzir uma sensação de maturidade maior que a evidência de produto.

## Fase 4 — Pré-hardware e provas de fogo

**14–15 de agosto.** O projeto organiza ameaças, proveniência local, protocolos de bancada e guia de montagem. A tese física é preparada, mas o hardware ainda não é uma prova. O simulador e os testes host-only verificam invariantes, não ergonomia, autonomia, rádio real ou temperatura.

A regra correta desta fase é: `pre_hardware` significa **preparado para medir**, não **medido**.

## Fase 5 — Produto, LLM e inteligência própria

**17 de agosto.** O HERUS explora adoção, LLM local em ESP32, propostas tipadas, invariantes e aprendizados externos. Surge a tensão estratégica: a arquitetura quer ser uma infraestrutura pessoal de significado, enquanto a pressão por “inteligência própria” pode empurrá-la para competir com assistentes gerais.

O aprendizado durável é negativo: LLM, voz contínua, memória pessoal e “segundo cérebro” não devem entrar no caminho crítico antes de uma tarefa humana pequena demonstrar valor.

## Fase 6 — ASA e o Symbiont

**15–16 de setembro.** O projeto começa a investigar uma tese diferente da tese original de rádio: um núcleo persistente pode habitar hospedeiros diferentes, observar capacidades locais e transferir Skills finitas por contratos de efeito.

Entram o ciclo `observe → infer → collect evidence → learn model → compose → verify → transfer`, o modelo de hospedeiro, o mundo observado, identidade persistente, equivalência semântica entre hosts, descoberta e experimentos multi-host.

Este é o nascimento do **Symbiont v2** como pacote de pesquisa host-independent. Ele é importante porque transforma “simbiose” em uma pergunta testável. Ele não é uma AGI nem um sistema de controle geral.

## Fase 7 — ASA, auditoria e falsificação

**21–22 de setembro.** A pesquisa ASA cresce rapidamente: rounds de transferência, holdouts, autoridade, proveniência, drift, rollback e executor sintético. A auditoria profunda mostra que alguns nomes eram mais fortes que os mecanismos: `SUPPORTED` podia ser emitido por contratos apenas textuais; orçamento podia ser decorativo; oracles podiam depender de labels; e o executor podia confirmar passos não executados.

Esta é a virada mais importante do projeto. O HERUS deixa de perguntar apenas “o mecanismo funciona?” e passa a perguntar “o teste consegue detectar quando o mecanismo está mentindo?”.

## Fase 8 — A correção epistemológica

Ainda em **22 de setembro**, três correções são implementadas:

1. **P0:** o runtime não emite mais `SUPPORTED`; o máximo local é `SAFE_BUT_UNPROVEN`. Classificadores, protocolo social, CI e README tornam-se fail-closed.
2. **P1:** runtime e oracle da campanha causal são separados por subprocessos. Variantes com transcript público igual e efeito oculto diferente são bloqueadas.
3. **P2:** nasce o **HERUS Bridge**, uma experiência local em que a pessoa define uma rotina, troca de interface, revisa uma proposta e confirma ou cancela sem qualquer efeito externo.

## Estado atual

O HERUS é hoje um **candidato de mecanismo host-only com uma demonstração de produto local**, não uma simbiose geral. A pergunta imediata não é “como adicionar mais capacidades?”. É:

> Uma transferência finita pode ser demonstrada sem circularidade, e uma pessoa entende claramente o que o sistema propõe, o que ele não fez e como pará-lo?

## Próximo futuro

Se o Bridge passar por revisão independente e sessões formativas, o projeto pode preparar um piloto humano pareado. Se falhar, deve redesenhar ou encerrar essa linha de produto antes de investir em hardware, rádio, voz ou LLM. Essa possibilidade de parar é parte do desenho científico.

## Referências

[1]: 00-HERUS-MASTER.md "Documento mestre histórico do HERUS"
[2]: 51-API-SIMBIONTE-V2.md "API do Symbiont v2"
[3]: 56-RODADA-DECISIVA-HERUS-BRIDGE.md "Rodada decisiva"
[4]: ../research/evidence/final_audit/00-synthesis.md "Síntese da auditoria final"
