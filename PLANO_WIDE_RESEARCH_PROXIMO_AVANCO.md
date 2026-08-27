# Plano Wide Research — próximo avanço do HERUS

## Objetivo

Avançar o HERUS por processamento paralelo, investigando e implementando a próxima camada de um sistema semântico seguro: entradas multimodais convergindo para uma Semantic IR tipada, raciocínio finito explicável, memória seletiva e autoridade fail-closed. O ciclo deve produzir apenas resultados verificáveis, com testes reais que procurem brechas, correções, métricas brutas e publicação no GitHub.

O objetivo científico não é declarar que o raciocínio aberto e generativo foi resolvido. O objetivo é construir uma cadeia experimental que possa testar, de modo falsificável, quanta generalização composicional e semântica útil pode ser obtida com vocabulário finito, álgebra, memória explicável e aprendizado local mínimo.

## Método de execução paralelo

Depois da aprovação, quatro frentes serão executadas em paralelo sempre que não houver dependência entre elas. Cada frente deverá produzir artefatos versionados, testes reproduzíveis e uma lista explícita de falhas encontradas. A integração só ocorrerá depois que cada frente passar seus próprios gates.

### Frente A — Contrato e Semantic IR

Será revisado o schema v0.1 e projetada a evolução mínima necessária para cartões de contexto, eventos, evidências, hipóteses, validade temporal, intenção de ação e origem da percepção. O contrato deverá continuar fechado, versionado, sem coerção silenciosa e sem autoridade operacional.

Critérios de aceite:

- entradas desconhecidas, campos extras, versões incompatíveis, scores incoerentes e hipóteses sem evidência são rejeitados;
- `TRUE`, `FALSE`, `BOTH` e `NEITHER` permanecem semanticamente coerentes com evidências positivas e negativas;
- nenhuma Semantic IR aceita produz diretamente envio, gravação no cofre ou alteração de trust;
- o schema e o validador Python não divergem em tipos, limites ou propriedades obrigatórias;
- casos válidos têm serialização canônica estável e hash reproduzível.

### Frente B — Parser real e convergência multimodal

Será ampliado o bridge real entre parser C, comando tipado e Semantic IR. O corpus será composto por fixtures de regressão do firmware, entradas reais fornecidas pelo usuário quando disponíveis e entradas adversariais derivadas das falhas observadas. O teste principal será equivalência semântica, não igualdade textual.

Critérios de aceite:

- frases e comandos equivalentes convergem para a mesma chave semântica;
- frases conflitantes, negadas, truncadas, com Unicode não suportado, números ambíguos e intenções múltiplas falham fechado;
- a cobertura do parser é medida por caso e por classe de erro;
- qualquer mudança de gramática atualiza corpus, oráculo e documentação;
- o teste roda no caminho oficial do firmware, com warnings e sanitizadores.

### Frente C — Raciocínio finito, memória e aprendizado local mínimo

Será avaliado o protótipo host-only de operads/assinaturas coloridas, e-graph limitado e lógica paraconsistente. A frente deverá separar novidade combinatória de geração aberta e definir uma interface para memória seletiva explicável. Aprendizado local mínimo será tratado como hipótese experimental: ajustar associações, pesos ou limiares sobre símbolos existentes, sem criar autoridade ou vocabulário não aprovado.

Critérios de aceite:

- terminação é garantida por orçamento de nós, profundidade, combustível e tempo;
- inserções excedentes são atômicas e deixam o estado intacto;
- contradições são preservadas sem explosão e bloqueiam ação;
- toda recuperação de memória retorna regra, evidência, versão e motivo de seleção;
- nenhum símbolo novo é usado sem processo explícito de descoberta, validação e versionamento;
- custo de memória, latência e número de candidatos são medidos, não estimados por narrativa.

### Frente D — Segurança, integração e adversarial testing

Será feita uma revisão cruzada do caminho inteiro: parser → Semantic IR → memória/raciocínio → proposta → gate de intenção → confirmação → trust → transporte. Serão usadas mutações de código e de dados para retirar barreiras, inverter condições, introduzir estado stale, forjar propostas e provocar replay, overflow, ambiguidade e downgrade.

Critérios de aceite:

- mutações que removem controles críticos são detectadas pelo proof-fire;
- AddressSanitizer, UndefinedBehaviorSanitizer e GCC analyzer passam no caminho alterado;
- regressões de sessão, confiança, replay, confirmação e autoridade não passam;
- mensagens rejeitadas não deixam estado parcialmente atualizado;
- a auditoria de proveniência permanece válida e fail-closed;
- warnings novos são classificados: defeito corrigido, falso positivo justificado ou dívida explícita.

## Integração das frentes

A integração seguirá esta ordem:

1. congelar uma versão de schema, corpus e vocabulário;
2. validar cada frente isoladamente;
3. executar os testes cruzados de convergência;
4. executar fuzzing e mutações depois da integração;
5. medir custos no host e, quando houver hardware, medir SRAM, flash, energia, latência e temperatura reais;
6. executar o proof completo do firmware, sanitizadores, análise estática e auditoria de proveniência;
7. documentar resultados brutos, falhas e limitações;
8. commit e push para `origin/main`.

## Métricas do ciclo

| Métrica | Regra de medição |
|---|---|
| Convergência | Percentual de entradas equivalentes que produzem a mesma chave semântica. |
| Rejeição segura | Percentual de casos inválidos rejeitados sem saída operacional. |
| Falsos consensos | Casos semanticamente diferentes que convergem indevidamente. Meta: zero nos fixtures conhecidos. |
| Cobertura adversarial | Número de mutadores, casos por mutador e classes de erro detectadas. |
| Explicabilidade | Proporção de decisões com regra, evidência, versão e custo recuperáveis. |
| Custo | Tempo, alocações, nós, bytes e limites de execução por caso. |
| Segurança | Controles removidos por mutação e regressões detectadas. |
| Reprodutibilidade | Hash do corpus, seed, versão do schema, commit e comando de execução. |

## Regras de decisão

Uma frente que falhar não será mascarada por média agregada. O artefato será marcado como falho, o contraexemplo será preservado e a correção será testada novamente. Nenhum resultado de simulação será tratado como evidência física. Nenhum protótipo host-only será tratado como firmware pronto.

Se a complexidade dos formalismos propostos exceder os limites medidos, a implementação será reduzida ao subconjunto finito que preserve a propriedade testada. HoTT permanece fundação de pesquisa até existir um núcleo formal implementado; e-graphs permanecem busca limitada; lógica paraconsistente permanece contenção de contradição; LLM local permanece adaptador de linguagem sem autoridade.

## Entregáveis previstos

- versão atualizada do contrato Semantic IR;
- corpus versionado e oráculos de convergência;
- bridge real parser C → semântica canônica;
- campanha adversarial e logs brutos;
- melhorias no protótipo finito e na memória explicável;
- relatório de falhas e correções;
- execução completa dos gates;
- commit publicado em `SummaArs/herus`.

## Assunções e riscos

Assume-se que as fixtures atuais do firmware são a primeira fonte de dados e que novas amostras reais poderão ser fornecidas pelo usuário ou coletadas sob autorização. Não se deve fabricar uma taxa de compreensão de linguagem aberta a partir de frases manuais. O maior risco técnico é confundir convergência para uma representação criada pelo próprio teste com entendimento real; por isso os testes precisam incluir contraexemplos, classes novas, holdout e avaliação humana quando a semântica não for decidível automaticamente.

O segundo risco é importar formalismos matemáticos sem benefício operacional. Cada componente só permanecerá no caminho experimental se tiver uma propriedade medida e um custo conhecido. O firmware continuará menor e mais restrito que a trilha de pesquisa.
