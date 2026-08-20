# HERUS — Ponte memória–reasoner offline

**Estado:** host-only, C11, publicado como evolução incremental.  
**Princípio:** memória autorizada pode ampliar o raciocínio local; nunca pode conceder certeza ou autoridade por si só.

## 1. Composição no pulso

`memory_reasoning_bridge` compõe um `sr_reasoner_t` com a evidência semântica local em uma cópia `scratch` fornecida pelo chamador. O reasoner base e o índice de memória são somente leitura. O Core externo não aparece na API, não participa da consulta e não é necessário para derivação.

A ponte importa somente evidências `ACTIVE` que ainda estejam dentro da validade por geração. Evidências `SUPERSEDED`, `CONFLICTED` e `EXPIRED` não entram no scratch. O cartão e o recibo de revisão permanecem na metainformação da consulta, sem texto, áudio, embedding, identidade, localização ou chave.

## 2. Ordem segura da consulta

A consulta passa pelos seguintes gates:

| Ordem | Decisão |
|---|---|
| 1 | Rejeitar argumentos inválidos, geração zero, orçamento zero e alias entre base e scratch |
| 2 | Consultar diretamente a memória; ambiguidade ou contradição retornam abstenção |
| 3 | Copiar o reasoner base para scratch |
| 4 | Importar fatos ativos, bounded e não conflitantes |
| 5 | Saturar regras locais até ponto fixo; saturação parcial retorna `MRB_LIMIT` |
| 6 | Consultar o reasoner composto e preservar `DIRECT`, `DERIVED`, `ABSENT`, `CONTRADICTED` ou `AMBIGUOUS` |

A importação de todas as evidências ativas é deliberada: um fato lembrado pode ser uma premissa intermediária de uma regra, não somente a resposta final. O scratch é descartado pelo chamador depois da consulta e não altera o vault ou o índice.

## 3. Resultados demonstrados

A suíte `memory_reasoning_bridge` passa **14/14**. Ela demonstra uma derivação local sustentada por memória revisada, ausência do Core, preservação do reasoner base, rejeição de enumeração, rejeição de scratch aliasado, ambiguidade sem escolha, conflito sem importação, expiração, e falha fechada para saturação parcial.

O pipeline global passa com **58 suítes**, **111 invariantes de sistema simulado** e mutação adversarial **7/7**.

> Isso prova composição simbólica bounded no host. Não prova que uma linguagem aberta foi compreendida, que a memória representa toda a vida da pessoa, que o ESP32-S3 suporta a carga, nem que o comportamento físico ou energético foi validado.

## 4. Limites soberanos

A ponte não captura fala, não produz texto, não invoca LLM, não consulta o Core, não envia rádio e não aciona háptica. A memória só chega à ponte porque já passou pela fronteira de cartão revisado e recibo de confirmação. Conflitos não são resolvidos pela recência; alternativas ambíguas não são ranqueadas por uma pontuação opaca.

O próximo passo seguro é adicionar consultas compostas explicitamente tipadas, com evidência de derivação e orçamento de custo, mantendo o mesmo scratch fail-closed. A integração não deve transformar o reasoner em um agente executor.

## Referências internas

[1]: docs/68-HERUS-MEMORIA-SEMANTICA-TEMPORAL-E-CONFLITOS.md "Memória semântica temporal"

[2]: docs/66-HERUS-MIGRACAO-HANDLES-E-KNOWLEDGE-FEED.md "Handles collision-aware e Core feed"

[3]: docs/64-HERUS-SOBERANIA-ON-WRIST-E-FRONTEIRA-CORE.md "Soberania on-wrist"
