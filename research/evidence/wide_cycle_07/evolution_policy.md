# Política de evolução incremental do HERUS

**Versão:** `herus-evolution-policy-v1`

## Princípio operacional

O HERUS deve evoluir por **composição controlada**, não por crescimento indiscriminado nem por rewrite recorrente. A ordem padrão é:

> Construir com contratos explícitos → estabilizar com testes → observar evidência real → melhorar o menor ponto necessário → preservar invariantes e conhecimento acumulado.

Essa política preserva contratos e propriedades observáveis, não implementações internas. Uma substituição interna é permitida quando mantém ou migra explicitamente o contrato, não aumenta autoridade sem justificativa e possui rollback verificável. A política não impede uma quebra controlada quando manter o contrato antigo preservaria uma vulnerabilidade, uma semântica falsa ou uma migração impossível.

## Condições obrigatórias para qualquer mudança

Uma mudança só pode ser promovida ao núcleo se responder, no próprio registro da mudança:

| Pergunta | Evidência mínima |
|---|---|
| Qual problema real está sendo resolvido? | Bug reproduzido, requisito de contrato, dado auditado ou falha de gate |
| Por que o contrato atual não basta? | Limitação localizada e arquivo/módulo afetado |
| O que permanece estável? | Invariantes, versões, autoridade, comportamento de rejeição e compatibilidade declarados |
| A mudança é reversível? | Rollback, migração interrompida, timeout e estado seguro testados |
| Qual custo novo aparece? | Linhas/módulos, estados, dependências, memória, tempo de teste e superfície de autoridade |
| Como se impede regressão? | Contraexemplo, teste positivo, teste fail-closed e gate correspondente |

Uma alteração sem essas respostas é **experimental** ou **rejeitada**; não entra no núcleo por preferência estética ou pressão de novidade.

## Classificação

| Classe | Exemplo | Tratamento |
|---|---|---|
| `PATCH_NO_CONTRACT_CHANGE` | Correção de parsing, zeroização ou classificação de erro | Contraexemplo reproduzível, regressão, todos os gates e novo digest |
| `FINITE_ADDITIVE` | Novo papel, filler, frase controlada ou métrica | Limite finito, símbolo desconhecido rejeitado/ignorado conforme contrato, compatibilidade testada |
| `VERSIONED_MIGRATION` | Novo schema, persistência ou payload | Versão explícita, leitura da versão anterior, migração autenticada, rollback e corrupção testados |
| `BREAKING_SECURITY_FIX` | Remover autoridade indevida ou corrigir protocolo inseguro | Quebra declarada, migração/atualização coordenada, errata e revisão de segurança |
| `EXPERIMENTAL_HOST_ONLY` | LLM, aprendizado local, raciocínio generativo ou novo formalismo | Sem autoridade; manifesto, orçamento e critérios de falsificação; não é fallback de produto |

A classe não pode ser reduzida artificialmente para evitar uma revisão. Alteração de autoridade, confirmação física, semântica de evento, replay, autenticação ou formato persistente nunca é uma simples refatoração.

## Contratos que não podem mudar silenciosamente

O HCP preserva airtime, dwell, tier criptográfico, transmissão apenas de símbolos, endereço efêmero, replay/deduplicação e a regra P4 de ignorar papéis desconhecidos. O Core-Link preserva versão, comprimento, autenticação, par, direção, sequência, expiração e vínculo à sessão. A Semantic IR preserva `PROPOSAL_ONLY`, enumerações fechadas e rejeição de propriedades desconhecidas. Voz preserva gramática finita, rejeição de palavras embutidas, limite de transcript e confirmação posterior. Cofre/coleção preservam autorização humana, sessão física, geração/floor, capacidade declarada e bloqueio de ambiguidade.

Novos eventos, campos ou papéis devem ter comportamento definido para consumidores antigos. Onde o contrato atual é fechado (`additionalProperties=false`, enum fechado ou formato persistente versionado), a extensão exige nova versão e migração; não pode atravessar por coerção ou desconhecimento silencioso.

## Observação e evidência

“Observar” não significa coletar vida pessoal nem transformar teste host em telemetria de produção. Antes do hardware, observação válida é bug reproduzido, teste adversarial, auditoria de fonte real, métrica de execução ou falha de proveniência. Medições de RF, energia, temperatura, UX e ASR embarcado só podem ser adicionadas após a Fase 0 física e devem preservar privacidade e consentimento.

Dados de datasets externos podem medir integridade ou rejeição quando a fonte, identidade, licença e modalidade forem declaradas. Rótulos externos não têm autoridade HERUS. Dados pareados não podem ser criados por ordem, label, basename, similaridade ou conveniência.

## Reversibilidade e migração

Toda migração deve ser monotônica em segurança: uma falha não pode promover estado parcial, perder o floor de geração, reativar sessão, repetir transmissão, aceitar replay ou apagar evidência necessária à recuperação. O caminho de leitura deve distinguir versão suportada, versão desconhecida, registro truncado, registro autenticado inválido, conflito e backend indisponível. Ambiguidade vira bloqueio, não escolha automática.

A compatibilidade de interface não autoriza compatibilidade com uma vulnerabilidade. Se uma correção de segurança quebra wire, schema ou persistência, o projeto deve publicar a razão, a versão mínima, o procedimento de migração e o teste de recusa do formato antigo inseguro.

## Orçamento de complexidade

Cada mudança deve registrar diferença antes/depois em linhas e arquivos tocados, dependências, estados, caminhos de autoridade, tempo da suíte e memória host quando aplicável. O custo de teste faz parte do custo arquitetural. Reduzir linhas removendo uma barreira não é simplificação; é regressão.

A mudança deve ser rejeitada ou isolada como experimento se aumentar estados irreversíveis, superfície de autoridade, dependências externas ou custo de migração sem benefício demonstrado. “Mais capacidade” não é benefício suficiente.

## Proibições mantidas

A política não autoriza NLU/NLG aberto, ontologia geral, formalismo adicional no firmware, LLM hospedada, LLM local com autoridade, mapeamento de rótulo externo, persistência de transcrição/áudio, fallback generativo ou aprendizado que invente símbolos fora do vocabulário finito. VSA/HDC permanece limitado a `bind`, `unbind` e `bundle` sobre cartões tipados. Os estados `OTHER`, `AMBIGUOUS` e `CONFLICT` são abstenção não operacional e nunca atravessam a bridge.

## Checklist de promoção

Uma mudança candidata só recebe promoção quando: possui problema real declarado; pertence a uma classe; preserva ou versiona o contrato; tem orçamento; tem regressões válidas e inválidas; testa rollback ou justifica por que não se aplica; não cria caminho de autoridade novo; passa os gates; atualiza proveniência; e documenta limitações. Se qualquer item faltar, o estado é `BLOCKED` ou `EXPERIMENTAL`.

## Decisão deste ciclo

O princípio do “relógio suíço” é adotado como **política de preservação de contratos e evolução incremental**, não como proibição de mudança interna ou de correção quebradora. O inventário do ciclo mostra que o HERUS já possui seams adequados para composição, mas não possui compatibilidade universal: Semantic IR e persistência são migráveis; HCP possui partes imutáveis e uma extensão forward-compatible específica; voz e interação admitem apenas extensões limitadas. “Não reescrever” não é um valor absoluto quando uma correção de segurança ou uma migração impossível exige quebra controlada. O próximo ensaio deve testar essas regras contra versões, migrações e interrupções reais do próprio contrato.
