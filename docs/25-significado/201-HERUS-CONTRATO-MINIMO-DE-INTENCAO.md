# HERUS — Contrato Mínimo de Intenção

## Status

Este documento define o contrato conceitual mínimo da infraestrutura HERUS. Ele é uma especificação de trabalho do ciclo `meaning-hardening-2026-09`; não afirma que o contrato já esteja implementado em todos os caminhos do firmware, nem que interoperabilidade entre implementações independentes já tenha sido demonstrada.

## Tese

O HERUS não transporta apenas dados. Ele transporta uma proposta de ação limitada por significado, autoridade, contexto, condições, efeitos permitidos e evidência exigida.

> Uma intenção só pode adquirir capacidade de ação quando sua representação canônica e seu contrato de autoridade forem verificáveis no domínio em que ela será usada.

O contrato é universal apenas no formato de coordenação. O vocabulário, a semântica concreta, os efeitos e as provas continuam pertencendo a um domínio finito e versionado.

## Unidade mínima

Uma intenção HERUS é uma tupla canônica:

```text
Intent = (identity, domain, proposal, purpose, authority,
          context, conditions, effects, delegation, evidence)
```

A serialização deve ser determinística. Campos ausentes não podem ser preenchidos silenciosamente por inferência. Quando um campo obrigatório não puder ser resolvido, o resultado é `ABSTAIN` ou `BLOCKED`, nunca uma intenção parcialmente autorizada.

| Campo | Conteúdo mínimo | Regra de segurança |
|---|---|---|
| `identity` | Identificador do envelope, versão e emissor | O identificador não concede autoridade |
| `domain` | Vocabulário e versão que definem a proposta | Domínio desconhecido bloqueia a interpretação |
| `proposal` | Ação e argumentos tipados | Texto livre não é efeito executável |
| `purpose` | Finalidade declarada | A finalidade limita a delegação |
| `authority` | Principal, papel, escopo e referência de autorização | Autoridade deve ser verificável e não ampliável |
| `context` | Objeto, sessão, validade e nonce/replay boundary | Contexto expirado ou incompatível bloqueia |
| `conditions` | Pré-condições necessárias | Condição desconhecida não é condição satisfeita |
| `effects` | Efeitos permitidos e proibidos | Sink não declarado não pode ser alcançado |
| `delegation` | Origem, escopo, prazo e consumo da delegação | Delegação não pode exceder o pai |
| `evidence` | Provas exigidas para aceitação e conclusão | Sucesso sem evidência é `UNPROVEN` |

## Estados da intenção

A intenção não possui apenas `válida` ou `inválida`. O núcleo precisa distinguir a fase do compromisso:

| Estado | Significado | Pode executar efeito? |
|---|---|---:|
| `PROPOSED` | Uma entidade apresentou uma proposta | Não |
| `INTERPRETED` | A proposta foi mapeada para o vocabulário do domínio | Não |
| `AUTHORIZED` | A autoridade foi verificada para o propósito e escopo | Não necessariamente |
| `DELEGATED` | A autoridade foi transferida dentro dos limites permitidos | Não necessariamente |
| `CONDITIONED` | As pré-condições foram verificadas | Ainda depende do sink |
| `COMMITTED` | O principal confirmou o efeito previsto | Apenas se o contrato exigir confirmação e ela for válida |
| `EXECUTING` | O executor iniciou o efeito autorizado | Sim, dentro do sink autorizado |
| `PROVEN` | A evidência exigida comprovou o resultado | O efeito pode ser considerado concluído |
| `ABSTAINED` | O sistema não conseguiu resolver a intenção com segurança | Não |
| `BLOCKED` | Uma regra, autoridade, condição ou evidência falhou | Não |
| `REVOKED` | A autoridade ou condição deixou de valer | Não; descendentes também devem ser avaliados |

Nenhuma transição pode pular diretamente de `PROPOSED`, `INTERPRETED` ou `AUTHORIZED` para um sink físico ou persistente. A transição exata dependerá do tipo de efeito e do risco do domínio, mas todos os saltos precisam ser explícitos no perfil HCAE.

## Separação de papéis

O HERUS separa quatro atos que sistemas convencionais frequentemente confundem:

1. **Propor**: declarar o que se deseja fazer.
2. **Autorizar**: conceder poder limitado para uma finalidade.
3. **Delegar**: transferir parte desse poder sem ampliar escopo, prazo ou efeito.
4. **Executar**: produzir uma mudança no mundo sob as condições verificadas.

Uma mesma entidade pode ocupar mais de um papel somente quando o domínio declarar essa composição e o certificado puder verificá-la. Uma frase plausível de uma IA é uma proposta; ela não é, por si só, autorização ou confirmação humana.

## Efeitos e sinks

Todo efeito que possa alterar comunicação, memória, credencial, dispositivo, estado físico ou estado persistente deve possuir um identificador de sink. O perfil do sink declara:

- a classe finita do efeito;
- o propósito compatível;
- os guards que devem dominar o sink;
- a autoridade necessária;
- a condição de validade temporal e de sessão;
- a evidência de conclusão;
- a política de falha e revogação.

Um efeito não perfilado é `REVIEW_REQUIRED`. Um sink anotado sem perfil, um perfil sem implementação ou uma chamada sensível sem disposição exata impede a promoção do caso para `ASSURED`.

## Evidência

A evidência deve provar uma afirmação específica, não apenas registrar que uma função retornou sucesso. Exemplos de afirmações são:

- a intenção foi interpretada no vocabulário e na versão declarados;
- a autoridade correspondia ao principal e ao propósito;
- a condição de sessão era a mesma que a do sink;
- a delegação não ampliou o escopo;
- o efeito autorizado foi o efeito realmente tentado;
- a operação persistente foi confirmada pelo backend exigido;
- a revogação não deixou um descendente acionável.

Evidência ausente, ambígua, expirada ou semanticamente incompatível produz `UNPROVEN`, `ABSTAINED` ou `BLOCKED`, conforme o perfil do domínio.

## Limite de escopo

O contrato não promete compreender toda intenção humana. Uma intenção só entra no caminho de execução quando pode ser formalizada dentro de:

1. um domínio conhecido;
2. um vocabulário finito e versionado;
3. um conjunto de tipos e argumentos permitido;
4. um perfil de autoridade;
5. um conjunto de efeitos e sinks declarado;
6. uma política de evidência.

Fora dessa fronteira, o HERUS pode preservar a entrada como proposta não executável, pedir esclarecimento ou recusar. Completar silenciosamente uma lacuna semântica é uma violação do contrato.

## Relação com as camadas existentes

| Camada | Responsabilidade no contrato |
|---|---|
| Babel | Compilar proposta para significado tipado ou recusar |
| Loom | Versionar vocabulários, extensões e limites finitos |
| Aether | Transportar representação sem corromper ou inventar significado |
| HSCA | Verificar estados, autoridade, contexto, condições, efeitos e evidência |
| Firmware C11 | Impor a fronteira fail-closed nos sinks reais |
| Certificado | Promover somente a cadeia de evidência efetivamente coberta |

## Critério de maturidade

Este contrato será considerado implementado somente quando o repositório demonstrar, em testes adversariais e numa cadeia ponta a ponta:

- duas representações compatíveis do mesmo envelope;
- rejeição de domínio, versão, autoridade, condição, efeito e evidência incompatíveis;
- delegação que nunca amplifica autoridade;
- revogação que não deixa descendentes executáveis;
- transporte corrompido que não vira significado válido;
- execução que não ocorre sem o sink e os guards correspondentes;
- evidência de conclusão distinta de uma simples mensagem de sucesso.

Até esse momento, esta especificação é uma direção congelada do projeto, não uma alegação de interoperabilidade universal já alcançada.
