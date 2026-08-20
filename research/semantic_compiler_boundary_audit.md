# Auditoria de fronteiras do compilador semântico

## Objetivo

Medir propriedades do compilador local existente antes de ampliar sua linguagem, sem criar módulos por volume. A auditoria deve separar o que é provado no corpus atual do que continua sendo hipótese fora dele.

## Estado congelado auditado

A cadeia foi reproduzida com `prove.sh --quiet` e CI Ubuntu/macOS verdes na revisão `56ed2f6`. A evidência atual inclui reasoner 20/20, planner 9/9, diálogo 16/16, compilador 39/39, benchmark 32/32 exact, 16/16 abstentions, zero violações de autoridade, bancada simulada 111/111 e mutação 7/7.

## Generalização de caixa

A normalização ASCII foi provada: `Gustavo` e `GUSTAVO`, assim como `caderno` e `CADERNO`, produzem o mesmo ID. A limitação continua explícita: bytes UTF-8 acentuados não recebem redução linguística; portanto variantes acentuadas e não acentuadas só são equivalentes quando a gramática fornece aliases específicos.

## Colisão de símbolos

O contrato do reasoner fixa fatos, padrões e termos em `uint16_t`. O compilador usa FNV-1a de 32 bits dobrado para 16 bits. Uma busca determinística encontrou a colisão `gh` e `ne`, ambas com ID `14346`.

Uma medição separada sobre os 48 lexemas efetivamente presentes no corpus atual encontrou 48 IDs distintos e zero colisões. O gate intra-utterance agora rejeita lexemas diferentes que colidam na mesma frase; o corpus fechado atual permanece sem colisões, mas o domínio aberto de entidades não pode alegar identidade global collision-free entre utterances.

> Conclusão: trocar o hash por outro sem mudar o contrato não seria uma prova; apenas moveria a primeira colisão.

## Consequência de arquitetura

Antes de aceitar vocabulário aberto, o HERUS ainda precisa de um domínio de símbolos versionado e collision-aware entre utterances: por exemplo, um léxico fechado auditável, ou uma tabela de interning que preserve a identidade transitória e recuse colisões antes de emitir um ID persistível. A barreira intra-utterance atual é parcial. Isso deve ser tratado como etapa própria e medida contra os limites de RAM/ABI, não como aumento cosmético do hash.

Até essa etapa, a afirmação válida é: **IDs são estáveis e não colidentes no corpus auditado de 48 lexemas; não são uma identidade simbólica universal**.

## Próximo passo lógico

Especificar e testar o contrato de um léxico collision-aware entre utterances, começando por uma proposta host-only que possa rejeitar colisões de forma explícita, sem permitir que texto bruto ou autoridade entrem no reasoner. Só depois de provar esse contrato deve haver alteração adicional de firmware.
