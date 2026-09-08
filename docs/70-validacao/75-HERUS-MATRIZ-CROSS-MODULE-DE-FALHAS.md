# HERUS — Matriz cross-module de falhas emergentes

**Estado:** verificado no host; nenhuma conclusão física foi promovida.  
**Objetivo:** testar se falhas em uma camada podem reviver autoridade em outra.

## Resultado

A matriz foi conectada ao runtime real de interação, assurance, Core Link e voz local. O cenário construtor exige que uma autoridade válida seja a interseção de condições já existentes; o cenário sabotador remove uma barreira por vez e recompila o mesmo grafo de produção. O resultado foi:

| Gate | Resultado |
|---|---:|
| Matriz cross-module | **15/15** |
| Redteam cross-module | **3/3 mutantes mortos** |
| Regressão global | **70 suítes** |
| Invariantes da simulação | **111/111** |
| Proveniência | **válida; entradas locais unsigned, sem atestação externa** |

## Ataques compostos

A primeira classe removeu individualmente as quatro pré-condições do handoff direto do Core: sessão física atual, intenção aceita, confirmação física e handoff ainda não consumido. Cada ausência foi observada como bloqueio, nunca como valor parcialmente autorizado.

A segunda classe atacou o caminho Nucleus–Core. A matriz removeu confiança ativa, autenticação do control link e frescor do envelope, isoladamente, e exigiu bloqueio em cada caso. Em seguida, marcou uma sessão anteriormente válida como revogada. A revogação continuou dominante, mesmo com todas as outras condições corretas.

A terceira classe testou a fronteira do modelo local. Habilitar o modelo sem evidência de aceitação A9 e sem a marca display-only bloqueou o handoff. Quando ambas as marcas canônicas foram apresentadas, a decisão foi permitida apenas porque as demais condições já estavam satisfeitas; nenhuma autoridade nova foi criada pelo modelo.

A quarta classe usou uma interação real: push-to-talk, transcript de teste permitido apenas no host e draft em `INTERACTION_AWAIT_CONFIRM`. A perda da fonte ASR forçou `INTERACTION_LINK_LOST`, limpou o draft e rejeitou uma confirmação posterior. Um snapshot de assurance perfeitamente seguro não conseguiu reviver o handoff depois que o runtime entrou em perda de fonte.

A quinta classe confirmou uma propriedade deliberadamente sutil: uma tentativa de handoff assured bloqueada por revogação não consumiu o draft confirmado. O draft só pôde ser consumido pela via local ordinária, depois de a camada de assurance recusar o caminho assured. Isso mantém a separação entre decisão de assurance e consumo one-shot.

## Mutantes mortos

O redteam removeu a precedência terminal de revogação em `assurance.c`, removeu a limpeza do draft durante perda de fonte em `interaction.c` e removeu a barreira de assurance em `interaction_take_send_assured`. Os três mutantes fizeram o teste composto falhar e foram classificados como mortos.

| Mutante | Controle removido | Oráculo que detectou |
|---|---|---|
| `assurance-revocation-precedence` | Revogação terminal | Decisão Nucleus revogada bloqueada |
| `interaction-source-loss-scrub` | Limpeza de draft em perda de fonte | Estado `LINK_LOST`, draft zero e confirmação impossível |
| `interaction-assurance-handoff-barrier` | Verificação de assurance | Snapshot revogado não poderia consumir handoff |

## Limites honestos

A matriz não prova a qualidade de um transceptor, o comportamento de uma bateria, a atomicidade elétrica de uma gravação de flash, a confiabilidade de um GPIO, a presença de um DRV2605L ou o funcionamento de uma antena. Ela prova apenas que os estados tipados simulados não devem cruzar a fronteira de autoridade quando as condições compostas são incoerentes.

Também não prova inteligência geral equivalente a uma LLM. Prova uma arquitetura local que pode rejeitar ambiguidades, manter estado privado, limitar autoridade e impedir que o modelo ou o Core externo se torne executor. O próximo gate físico precisa repetir os estados relevantes com reset real, brownout observado, mídia real e instrumentos identificados.

## Reprodução

```bash
cd /home/ubuntu/herus-semantic-compiler-pr
python3 tools/test_cross_failure_redteam.py
cd firmware
make cross-failure-matrix cross-failure-redteam
cd ..
./prove.sh --quiet
python3 tools/provenance_audit.py --strict research/software_provenance_manifest.json
```

A frase `ALL INVARIANTS HOLD` deve ser interpretada como: contratos host-side passaram e gates físicos continuam pendentes.
