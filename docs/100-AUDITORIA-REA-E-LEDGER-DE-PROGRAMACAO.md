# Auditoria do REA e ledger de programação

## Decisão

O REA não foi copiado como ferramenta de reverse engineering. O que foi incorporado foi o seu método de assurance:

- separar observação de inferência;
- preservar desconhecidos e resultados parciais;
- exigir evidência autenticada;
- exigir autoridade comparável entre observação, fixture e verificador;
- manter obrigações abertas quando falta um caso;
- usar digest do fechamento para tornar a decisão auditável;
- não usar fallback silencioso quando um provider ou recurso está indisponível.

## Aplicação à programação

Uma proposta de código não é considerada correta porque parece plausível ou porque um teste unitário isolado passou. Ela recebe obrigações explícitas, por exemplo:

```text
parser.v1
  positive
  negative
  malformed
  autoridade: test
```

O `build_programming_ledger` só fecha essa obrigação quando todos os casos exigidos têm evidência autenticada e a autoridade da evidência é comparável à autoridade exigida. Caso contrário, o resultado permanece `open` e registra os desconhecidos.

## O que não foi incorporado

- Hopper, Ghidra ou IDA;
- execução de binários;
- acesso ao sistema de arquivos do usuário;
- shell ou subprocessos;
- ferramentas externas de reverse engineering;
- qualquer mecanismo que transforme uma proposta em execução automática.

Esses componentes não pertencem à superfície proposal-only da biblioteca.

## Por que isso aumenta a qualidade

A habilidade de programação do HERUS agora tem uma distinção operacional entre:

```text
código proposto
código sintaticamente válido
código testado
código comprovado sob autoridade comparável
código autorizado para execução
```

Esses estados não podem ser colapsados em um único `success`.

## Limite honesto

O ledger é um mecanismo de fechamento de evidência, não um provador de correção universal. Ele não garante que os testes sejam completos, que a especificação esteja correta ou que uma implementação seja segura em todos os ambientes. Ele impede uma alegação mais forte do que a evidência disponível.

A próxima etapa é usar o ledger em um benchmark real de programação com repositórios, testes ocultos e execução isolada fora da biblioteca.
