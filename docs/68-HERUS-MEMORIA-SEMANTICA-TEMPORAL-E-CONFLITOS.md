# HERUS — Memória semântica temporal e conflitos explícitos

**Estado:** host-only, C11, bounded e sem conteúdo bruto.  
**Princípio:** o HERUS pode aprender relações úteis, mas não pode transformar evidência conflitante em certeza.

## 1. O que foi acrescentado

A camada `memory_semantic_evidence` recebe somente um `memory_vault_card_t` já autorizado e um fato simbólico composto por handles. Ela não captura áudio, interpreta texto, acessa rede, lê a chave do vault ou decide sozinha o que deve ser persistido. Sua função é transformar cartões revisados em evidência consultável pelo reasoner local, preservando provenance mínima e estado temporal baseado em gerações monotônicas.

Cada evidência retém apenas o fato, `card_id`, `review_receipt_id`, geração observada, geração de validade e estado. Não há timestamp civil, identidade, localização ou texto. A validade por geração permite expiração deterministicamente testável e não depende de relógio de parede não verificado.

## 2. Estados da evidência

| Estado | Semântica |
|---|---|
| `ACTIVE` | evidência corrente e elegível para consulta |
| `SUPERSEDED` | evidência substituída por uma observação mais nova do mesmo fato |
| `CONFLICTED` | fato incompatível com outra evidência; não pode ser escolhido |
| `EXPIRED` | validade bounded ultrapassada; não aparece como conhecimento corrente |

A supersessão preserva o registro antigo para provenance. Uma sequência menor é rejeitada como rollback, uma duplicata da mesma geração é idempotente e uma evidência nova não apaga silenciosamente sua antecessora.

## 3. Conflito e abstenção

O índice permite registrar uma política opcional de predicados funcionais. Quando o mesmo sujeito e predicado recebem objetos diferentes em um predicado funcional, ambas as evidências são marcadas como conflitadas. A mesma regra vale para afirmações e negações opostas do mesmo fato.

Na consulta, um conflito retorna `MSE_QUERY_CONTRADICTED`, sem vencedor. Quando existem duas alternativas compatíveis para um predicado não funcional, a consulta retorna `MSE_QUERY_AMBIGUOUS`. Uma consulta com três variáveis é rejeitada para impedir que a memória privada se torne um endpoint de enumeração.

> A temporalidade ajuda a organizar evidência; ela não resolve verdade. “Mais novo” não significa “mais correto”.

## 4. Evidência observada

A nova suíte passa **19/19**. Ela cobre entrada de cartão revisado, provenance, duplicata idempotente, rejeição de rollback, supersessão preservada, conflitos funcionais, contradição, alternativas ambíguas, expiração, entradas inválidas, janela temporal impossível, zeroing de saída, saturação de capacidade sem inserção parcial e corrupção estrutural do índice.

O pipeline global passa com **58 suítes**, **111 invariantes de sistema simulado** e mutação adversarial **7/7**. Esses resultados demonstram os contratos host executados; não medem qualidade de memória humana, acurácia semântica em linguagem aberta, energia, latência no ESP32-S3 ou comportamento físico.

## 5. Limites deliberados

A camada não grava automaticamente no vault. O cartão precisa existir antes e carregar `review_receipt_id`, porque a inteligência não pode converter uma inferência local em memória pessoal sem a fronteira de revisão humana já existente. A camada também não promove conflito, não escolhe a observação mais nova como verdade e não envia nenhuma ação ao Core, rádio ou atuador háptico.

O próximo avanço seguro é compor esta evidência com consultas simbólicas explícitas, mantendo o contrato de prova, ambiguidade, contradição e confirmação física. A integração não deve introduzir busca livre, ranking opaco ou fallback para o Core.

## Referências internas

[1]: docs/65-HERUS-CONTRATO-CORE-FEED-E-CEREBRO-NO-PULSO.md "Contrato do Core feed"

[2]: docs/66-HERUS-MIGRACAO-HANDLES-E-KNOWLEDGE-FEED.md "Migração collision-aware e feed"

[3]: docs/67-HERUS-AUTENTICACAO-E-CURSOR-PERSISTENTE-CORE.md "Autenticação e cursor persistente"
