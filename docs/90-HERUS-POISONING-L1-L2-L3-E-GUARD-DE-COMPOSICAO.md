# HERUS — Redteam de poisoning persistente L1/L2/L3

**Estado:** protótipo host-side C11, pré-hardware.  
**Relação:** AGSC-D e Continuidade Semântica Governada por Autoridade.

## Motivação

Memória persistente cria uma superfície de ataque temporal. A taxonomia do MemPoison separa três níveis: **L1**, corrupção direta de um registro; **L2**, corrupção composicional de múltiplos registros; e **L3**, corrupção dormente ativada por contexto futuro [1]. O ponto crítico é que um filtro aplicado somente no momento da escrita pode não detectar uma combinação posterior de registros que pareciam benignos individualmente.

O HERUS não usa uma memória textual ou uma LLM hospedada, mas o risco estrutural permanece: uma sequência de fatos tipados pode ser interpretada como autorização, e uma oferta pode ser confundida com ação. Por isso o teste foi formulado contra as fronteiras de proveniência e autoridade, não contra uma taxa de “acerto” de linguagem.

## Guard de composição

O módulo `sim/poisoning_guard.{h,c}` implementa um bundle limitado de cápsulas de memória. Ele mantém os IDs de proveniência, o epoch e a validade mínima. Ao adicionar uma cápsula, a autoridade é calculada por **interseção**, nunca por união:

```text
autoridade(bundle) = autoridade(m1) ∩ autoridade(m2) ∩ ...
```

O bundle pode produzir apenas uma oferta contextual. O gatilho não concede `AT_AUTH_ACTION` e não marca confirmação física. A ação local continua exigindo a transição AGSC própria e contato físico separado. Conflito, expiração, token contextual incorreto e epoch divergente produzem rejeição ou abstention.

## Matriz de ataques

| Classe | Ataque | Gate esperado |
|---|---|---|
| L1 | Registro Core tenta se promover sem contato | `AT_E_AUTH` |
| L2 | Duas memórias compõem uma autoridade maior | Interseção de máscaras |
| L2 | Oferta composta executa diretamente | Ausência de `AT_AUTH_ACTION` |
| L2 | Ação composta sem novo contato | `AT_E_AUTH` |
| L2 | Memórias em conflito ativam contexto | `PG_E_CONFLICT` |
| L3 | Token de contexto errado | `PG_E_CONTEXT` |
| L3 | Bundle após expiry | `PG_E_EXPIRED` |
| L3 | Bundle após reboot/epoch novo | `PG_E_EXPIRED` ou `PG_E_EPOCH` |
| Replay | Conveniência tenta converter bundle antigo em ação | Rejeição fail-closed |

## Descoberta de cobertura

A primeira versão do cenário tinha duas memórias com a mesma máscara `AT_AUTH_OBSERVATION | AT_AUTH_MEMORY`. Assim, uma mutação que trocava interseção por união não era observável: união e interseção produziam o mesmo valor. Isso não foi contado como mutante morto.

O oráculo foi fortalecido com uma memória derivada portando somente `AT_AUTH_MEMORY`. A composição correta passou a produzir `AT_AUTH_MEMORY`; a mutação de união produziu uma máscara maior e foi morta. Essa descoberta é importante porque demonstra que **um redteam válido precisa variar autoridade, não apenas variar conteúdo**.

## Resultado host-side

| Evidência | Resultado |
|---|---:|
| Cenário L1/L2/L3 | **15/15** |
| Redteam de poisoning | **8/8 mutantes mortos** |
| Mutator global anterior | **7/7 mutantes mortos** |
| Redteam AGSC-D | **8/8 mutantes mortos** |
| Proveniência | Deve ser recalculada após esta extensão |

## Limites

O cenário não representa um agente linguístico completo e não mede ataque contra um modelo de linguagem. Ele prova apenas que a cadeia simbólica local não soma autoridade ao combinar cápsulas e que um gatilho contextual não cria ação.

Também não é possível inferir segurança contra todos os canais de poisoning de um sistema físico sem definir os adaptadores reais de percepção, Core e armazenamento. No hardware, a matriz deverá ser repetida com interrupção de energia, recuperação NVS, replay de pacotes e logs de diagnóstico privados.

## Referências

[1]: https://arxiv.org/abs/2607.14651 "MemPoison: Uncovering Persistent Memory Threats and Structural Blind Spots in LLM Agents"

[2]: https://arxiv.org/abs/2605.15338 "Hidden in Memory: Sleeper Memory Poisoning in LLM Agents"
