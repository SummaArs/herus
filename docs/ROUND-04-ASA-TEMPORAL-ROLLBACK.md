# Rodada 4 — Composição temporal, causalidade local e rollback

## Conclusão

A Rodada 4 ampliou o teste do ASA de efeitos isolados para uma capacidade composta: **armar antes de comprometer**. O contrato não é uma coleção de ações independentes; ele exige uma sequência temporal e uma pré-condição.

O ASA aprendeu o contrato por resultados observados em um host-fonte. Depois transferiu o contrato para um host-alvo com nomes de ações diferentes. A sequência transferida foi aceita somente porque o primeiro efeito ocorreu em estado inicial, o segundo estava bloqueado no estado inicial e passou a ocorrer depois da primeira ação.

A execução válida terminou em `committed=1`. O ASA rejeitou a sequência adversarial `commit` antes de `arm`, rejeitou um host que escondia a pré-condição, e restaurou o estado inicial depois de uma falha que já havia mutado parcialmente o estado.

Esta rodada fornece evidência de **composição temporal verificável e rollback local**. Não prova planejamento aberto, causalidade física ou segurança de atuadores reais.

## Hipótese falsificável

> **H4:** Para transferir uma habilidade composta, o ASA precisa provar as pré-condições e os efeitos de cada transição na ordem correta. Se qualquer pré-condição não puder ser confirmada, o plano deve ser rejeitado. Se a execução falhar depois de uma mutação parcial, o estado deve voltar ao snapshot anterior.

A hipótese seria falsificada se o ASA aceitasse `commit` antes de `arm`, transferisse o contrato para um host que não impõe a pré-condição, ou deixasse `armed=1` ou `committed=1` após uma falha transacional.

## Protocolo

O host temporal possui três ações com identificadores arbitrários. Uma ação prepara o estado, outra conclui o objetivo e uma terceira cancela a preparação. O estado possui os campos `armed` e `committed`.

A ação de preparação só pode ser executada quando o sistema está desarmado. A ação de compromisso só pode ser executada depois da preparação. Ao completar, ela retorna o estado a desarmado e incrementa `committed`. A ação de cancelamento também exige preparação e retorna a estado desarmado.

O algoritmo de descoberta não usa os nomes para deduzir o contrato. Ele testa as ações no estado inicial, separa ações inicialmente bloqueadas, executa cada ação desbloqueada e testa novamente as ações bloqueadas. Uma composição só é aprendida quando uma ação inicialmente bloqueada passa a produzir o objetivo depois de uma ação observada.

A transferência procura no host-alvo uma ação com o primeiro efeito, verifica que a segunda ação continua bloqueada no estado inicial, executa a primeira e procura o segundo efeito. Se houver ambiguidade, ausência de pré-condição ou ausência do efeito, a transferência é rejeitada.

A execução é transacional. Antes do primeiro passo, o host salva um snapshot de estado, sequência e última ação. Qualquer exceção restaura o snapshot completo.

## Resultados

| Prova | Resultado |
|---|---|
| Contrato temporal aprendido no host-fonte | **Sim** |
| Transferência para nomes renomeados | **Aceita** |
| Plano transferido | `stage_9 → commit_blue` |
| Execução válida | **Completou** |
| Estado final válido | `armed=0, committed=1` |
| `commit` antes de `arm` | **Rejeitado** |
| Estado após tentativa inválida | `armed=0, committed=0` |
| Host sem pré-condição confirmável | **Rejeitado** |
| Falha após mutação parcial | **Rollback completo** |
| Estado após falha parcial | `armed=0, committed=0` |

A prova adversarial de falha parcial usa um host que incrementa `committed` e depois lança uma exceção. O rollback restaura tanto `committed` quanto `armed`, mostrando que o mecanismo não depende de a exceção ocorrer antes de qualquer mutação.

## Por que esta rodada é diferente

As Rodadas 1 a 3 mostraram que efeitos podem ser transferidos, filtrados por observabilidade e recuperados por consenso. Isso ainda permitiria uma arquitetura que reconhecesse cada ação isoladamente e as executasse em ordem errada.

A Rodada 4 testa uma propriedade mais forte: uma ação não é válida apenas por seu efeito final. Sua validade depende do **estado causal anterior**. O ASA precisa provar a transição, não somente reconhecer a assinatura local.

O cenário `missing_precondition` é especialmente importante. Nele, o host permite que a ação final seja executada sem a preparação. Uma política baseada apenas em observar o efeito final poderia aceitar esse host. A transferência atual recusa-o porque não consegue confirmar a pré-condição temporal exigida pelo contrato aprendido.

## Limitações

O ambiente ainda é discreto, pequeno e determinístico. A pré-condição é representada por um único campo de estado. Não há concorrência, relógio, timeout, múltiplos agentes ou efeitos irreversíveis.

O rollback restaura o estado interno do simulador. Em hardware real, efeitos externos como rádio transmitido, movimento físico ou desgaste não podem ser desfeitos apenas restaurando memória. Por isso, este mecanismo não autoriza atuadores físicos.

A descoberta de contrato usa exploração controlada do host sintético. Em ambientes reais, sondar ações pode ter custo ou risco. O próximo estágio deverá diferenciar ações puramente observacionais de ações com efeito externo e exigir uma política de autorização separada.

O corpus textual local aparece apenas como resumo de proveniência nesta rodada. A prova principal é temporal e sintética; ela não demonstra que linguagem aberta consiga especificar corretamente uma sequência causal.

## Reprodução

```bash
PYTHONPATH=. python3 -m research.asa_round4
PYTHONPATH=. python3 -m unittest research.test_asa_round4
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
./prove.sh --quiet
```

A validação passou com **134 testes Python**, um teste previamente ignorado e nenhum erro. O gate oficial do firmware também passou com todas as invariantes.

## Próxima hipótese

A Rodada 5 deve testar **contrafactuais e generalização estrutural**: hosts com múltiplos caminhos para o mesmo objetivo, efeitos colaterais, ações irreversíveis simuladas e tarefas holdout. O critério deverá exigir que o ASA escolha apenas planos cujo caminho causal esteja provado e rejeite planos que atingem o objetivo por um efeito colateral não autorizado.
