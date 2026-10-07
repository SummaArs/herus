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

## Fase 9 — O algoritmo HERUS Symbiotic

**Segunda metade de setembro de 2026.** A direção principal deixa de ser o hardware e passa a ser a criação de um algoritmo de IA importável. O nome **Symbiotic Learning** é mantido como hipótese de uma família algorítmica: aprender uma competência condicionada ao hospedeiro, otimizar sob contratos e preservar a identidade verificável durante a adaptação.

Entram o núcleo Python, a fachada `from herus_symbiotic import Herus`, a indução contextual bounded, a memória episódica, o histórico de soluções, a explicação por IDs de evidência e a comparação com famílias reais de ML.

O resultado desta fase não é “HERUS venceu todos os modelos”. É a criação de um repertório experimental: Naive Bayes, SVM, Random Forest, regressão, k-NN, transformers, encoders multilíngues, Tensor-Train, conformal prediction e políticas seletivas passam a ser controles, componentes ou ablações explicitamente separados do HERUS-core.

## Fase 10 — Benchmark científico e limites

**Final de setembro e início de outubro de 2026.** O projeto executa benchmarks em MIntRec e MInDS-14, compara precisão total e seletiva, mede custo, cobertura, deriva e incerteza, e incorpora análise pareada, proveniência, manifesto de identidade e ledger por exemplo.

Os resultados são mistos e valiosos: algumas variantes do HERUS apresentam comportamento seletivo, mas baselines clássicos vencem em diversos regimes de cobertura total e seletiva. A transferência lexical não se sustenta automaticamente. A conclusão correta é que ainda não há SOTA geral.

Esta fase também revela problemas estruturais: o núcleo atual recebe `target_effect`, positivos são privilegiados na confiança, e o pipeline chamado HERUS não é sempre identificável como o mesmo objeto do núcleo. Esses problemas não invalidam o objetivo algorítmico; definem o trabalho necessário para transformar a semente em um algoritmo completo.

## Fase 11 — Crise científica e reorganização

**7 de outubro de 2026.** Uma Wide Research de crise revisa fundamentos, paradigmas, objeto, experimentos e trajetória prática. A decisão não é abandonar o HERUS Symbiotic, mas reorganizá-lo:

1. preservar todo o trabalho histórico;
2. declarar o HERUS Symbiotic como objetivo algorítmico principal;
3. separar core, adaptadores, controles e assurance;
4. definir entrada, função de decisão, função de otimização, memória e erro;
5. manter Object Lock, ledger e gates para impedir vazamento e claims infladas;
6. testar primeiro se a combinação de adaptação, evidência negativa, otimização sob restrições e abstenção produz uma vantagem real.

O Object Lock não substitui o algoritmo. Ele define as condições para que o algoritmo possa ser testado sem confundir uma tabela de consulta, um adaptador textual e o HERUS Symbiotic como se fossem a mesma coisa.

## Estado atual

O HERUS é hoje um **algoritmo de IA em evolução**, com um núcleo v2 implementado, uma fachada importável, um repertório de baselines reais e uma infraestrutura de verificação. A hipótese de Symbiotic Learning permanece aberta, mas ainda não foi provada como SOTA, nem como simbiose geral.

A pergunta atual é:

> Uma função de aprendizagem com memória, representação condicionada ao hospedeiro, otimização sob risco/custo/autoridade, feedback positivo e negativo e abstenção consegue adaptar-se melhor — ou de modo mais seguro e econômico — que os paradigmas existentes em um regime definido?

## Próximo futuro

O próximo ciclo deve fechar a função objetivo e a regra de atualização do HERUS Symbiotic, corrigir a semântica de feedback negativo e estado atual, testar hosts simulados com interfaces permutadas e comparar contra memória trivial, modelos supervisionados, bandits/RL proxy e transformers. O hardware físico permanece como host futuro, não como substituto da prova algorítmica.

## Referências

[1]: 00-HERUS-MASTER.md "Documento mestre histórico do HERUS"
[2]: 51-API-SIMBIONTE-V2.md "API do Symbiont v2"
[3]: 56-RODADA-DECISIVA-HERUS-BRIDGE.md "Rodada decisiva"
[4]: ../research/evidence/final_audit/00-synthesis.md "Síntese da auditoria final"
