# HERUS — Soberania da inteligência no pulso e fronteira do Core

**Escopo:** auditoria arquitetural da branch `feat/herus-semantic-compiler`.  
**Regra de produto:** o HERUS vestível deve continuar funcional quando o Core externo estiver desligado, ausente, sem rádio ou sem conhecimento novo.

## 1. Decisão arquitetural

> O **HERUS no pulso é o cérebro**. O Core externo é uma estação de suporte: carregador, antena, transporte e alimentador autorizado de conhecimento. Ele não é o executor do raciocínio cotidiano, não é a autoridade do usuário e não pode ser um ponto único de disponibilidade.

Essa decisão muda o critério de evolução. Um algoritmo só pertence ao HERUS se puder operar no vestível com memória, tempo e energia bounded, sem uma chamada obrigatória ao Core. O Core pode preparar uma proposta de conhecimento, mas o HERUS deve verificar sua identidade, versão, digest, compatibilidade, limites e política antes de incorporá-la. A ausência do Core deve produzir degradação de capacidade externa, não perda da inteligência local.

## 2. Estado executável observado

| Fronteira | Estado atual | Evidência | Lacuna |
|---|---|---|---|
| Registry collision-aware | handles 32-bit com namespace, versão e slot; personal limitado e confirmado | `symbol_registry.{h,c}`, invariantes 15/15 | consumidores principais ainda usam ABI 16-bit |
| Compilador com registry | rota opt-in resolve handle e projeta para `uint16_t` legado | `sc_compile_with_registry()` | projeção pode perder namespace/versão se for usada como identidade primária |
| Reasoner | fatos, padrões, regras e abdução usam `uint16_t` | `symbolic_reasoner.h`, 26/26 | colisão e migração não são representadas no tipo do raciocínio |
| Planner | ações, estados e objetivos usam a mesma superfície simbólica legada | `symbolic_planner.h`, 9/9 | custo de ABI e compatibilidade ainda não medidos com handles 32-bit |
| Diálogo local | reasoner, memória e proposta VSA operam localmente; autoridade explícita | `symbolic_dialogue.{h,c}`, 20/20 | adapter de modelo local ainda é callback não promovido por medição target |
| Modelo/UX | callback `generate_local`, display-only e sem endpoint de rede | `dialogue.h`, `assurance.{h,c}` | contrato de alimentação do Core ainda não é um pacote tipado completo |
| Protocolo de comunicação | HCP transporta IDs simbólicos de 16 bits em wire format legado | `hcp.{h,c}` | não deve ser confundido com identidade collision-aware interna |
| Core externo | existe como fronteira documentada e control-link, não como requisito do reasoner | contratos de protocolo/assurance | falta provar offline, ausência, pacote rejeitado e fonte remota sem autoridade |

A conclusão dessa auditoria é precisa: a inteligência já está majoritariamente modelada como módulos locais, mas a identidade collision-aware ainda termina na borda do compilador. O próximo avanço lógico é migrar os consumidores internos para um tipo de símbolo que não possa ser confundido com o ABI legado.

## 3. Invariantes soberanos propostos

| ID | Invariante | Falha se |
|---|---|---|
| WRIST-01 | Consulta, derivação, abdução e plano básico funcionam sem Core | qualquer caminho local exige pacote remoto |
| WRIST-02 | O Core não pode executar ação no pulso | pacote externo chama executor ou concede confirmação |
| WRIST-03 | Conhecimento externo entra como proposta versionada, nunca como fato implícito | digest/versão/namespace inválidos são aceitos |
| WRIST-04 | Personalização exige confirmação local explícita | o Core cria memória pessoal sem confirmação |
| WRIST-05 | Fábrica e pessoal permanecem namespaces disjuntos | pacote converte um nome pessoal em factory por colisão |
| WRIST-06 | Handles incompatíveis abstêm | reboot, versão ou migração silenciosa muda identidade |
| WRIST-07 | HCP pode continuar legado, mas não define a identidade cognitiva interna | wire ID de 16 bits é usado como prova de identidade global |
| WRIST-08 | Ausência do Core é observável como `CORE_UNAVAILABLE`, não como erro de inteligência local | desligar o Core impede perguntas, memória ou planos locais |
| WRIST-09 | Logs não contêm texto, áudio, embedding, identidade, localização ou chave | ingestão externa vaza conteúdo de produto |
| WRIST-10 | O Core pode alimentar, mas nunca possui autoridade | uma resposta do Core é tratada como confirmação física |

## 4. Separação de responsabilidades

| Função | HERUS no pulso | Core externo |
|---|---|---|
| Compilar significado | obrigatório | opcionalmente prepara sugestões |
| Resolver símbolo | obrigatório, com registry versionado | pode fornecer pacote de registry aprovado |
| Raciocinar e provar | obrigatório | pode produzir evidência proposta, nunca autoridade |
| Memória pessoal | guardar, consultar, atualizar sob política local | sugerir cartões ou conhecimento novo |
| Planejamento | gerar plano bounded | sugerir catálogo ou dados, sem executar |
| Comunicação | decidir semântica, autenticar, apresentar e exigir confirmação | oferecer antena/transporte |
| Carregamento | receber energia | fornecer energia |
| Linguagem háptica | decodificar/apresentar localmente | não substitui confirmação física |
| Modelo aberto | não obrigatório para a operação local | pode alimentar conhecimento, mas não é dependência |

## 5. Próxima etapa

A etapa seguinte deve definir um pacote `knowledge_feed` com identidade do produtor, versão de registry, digest, namespace, limites, lista de fatos/regras/provas e estado de confirmação. Depois disso, a migração de `sr_fact_t`, `sr_pattern_t`, `sr_rule_t`, planner, diálogo e abdução para handles collision-aware poderá ser feita em rota paralela à ABI legada.

Não está sendo alegado que a migração já ocorreu, nem que o Core atual já esteja impedido por todos esses gates. Este documento é a linha de base que evita que o HERUS volte a tratar o suporte externo como seu cérebro.
