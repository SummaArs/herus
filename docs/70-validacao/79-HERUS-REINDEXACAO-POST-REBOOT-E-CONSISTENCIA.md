# HERUS — Reindexação pós-reboot e consistência semântica

**Estado:** host-side verificado; não é uma medição de desempenho nem uma prova de persistência física.  
**Objetivo:** garantir que o conhecimento semântico reintroduzido depois de um reboot obedeça ao floor, preserve supersession e expiração e se abstenha diante de conflito ou alternativas compatíveis.

## Resultado executável

| Controle | Resultado |
|---|---:|
| Matriz de reindexação pós-reboot | **11/11** |
| Redteam de reindexação | **5/5 mutantes mortos** |
| Matriz de reboot, floor, contexto e divergência | **17/17** |
| Redteam de reboot, floor, contexto e divergência | **11/11 mutantes mortos** |
| Regressão global | **76 suítes** |
| Invariantes da simulação | **111/111** |
| Proveniência | **válida; entradas locais continuam não atestadas externamente** |

## Contrato de reindexação

O reboot deixa o índice semântico vazio e instala um `generation_floor` somente depois de recuperar um floor de sessão autenticado. Uma reindexação na mesma geração ou em geração anterior é `MSE_E_ROLLBACK`. Somente uma geração estritamente posterior pode entrar no índice.

A reindexação não muda as regras epistemológicas do índice. Uma evidência exata mais nova supersede a anterior, mas não converte duas cópias em dois fatos independentes. Uma evidência expirada deixa de ser conhecimento corrente. Duas evidências incompatíveis para um predicado funcional são marcadas como conflito e a consulta retorna `MSE_QUERY_CONTRADICTED` sem escolher a mais nova. Alternativas compatíveis para um predicado não funcional permanecem `MSE_QUERY_AMBIGUOUS`, com o candidato selecionado zerado.

O floor de sessão e o floor semântico não podem ser tratados como valores intercambiáveis sem validação. Se o índice carrega floor semântico doze e a recuperação oferece floor de sessão oito, o HERUS bloqueia e limpa a fronteira. Ele não reduz o floor para oito, não reabre contexto antigo e não transforma a divergência em sucesso parcial.

## Ataques adversariais

A campanha removeu, um por vez, cinco controles críticos:

| Mutante | Controle removido | Falha que o oráculo exige detectar |
|---|---|---|
| `reindex-floor-gate` | Rejeição de geração stale | Evidência na geração do floor passa a entrar |
| `reindex-supersession` | Substituição exata | A consulta vê duplicidade em vez de apenas o sucessor |
| `reindex-expiry` | Expiração por geração | Fato vencido continua sendo apresentado |
| `reindex-conflict-marking` | Marcação de conflito funcional | O conflito é perdido e uma alternativa pode parecer certa |
| `reindex-conflict-abstention` | Retorno contradito | O reasoner escolhe ou esconde a contradição |

A campanha de reboot também removeu o gate de divergência entre floors e o detectou. No total, essa fronteira possui **11/11 mutantes mortos** quando considerados os ataques de scrub, contexto, floor, geração, argumentos e divergência.

## Descoberta importante

A propriedade de segurança não é apenas “não importar memória após reboot”. A propriedade mais forte é: **qualquer memória reimportada deve provar que é sucessora da fronteira atual e deve continuar sujeita às mesmas regras de conflito, expiração e ambiguidade**. Sem o primeiro gate, um card antigo poderia reaparecer; sem os demais, um card novo poderia substituir silenciosamente a verdade ou produzir uma resposta falsa por seleção arbitrária.

## Limites honestos

A prova não demonstra que o floor será durável em flash, que a geração sobreviverá a brownout ou que uma reindexação física ocorrerá atomicamente em NVS. Também não estabelece qualidade semântica, relevância ou utilidade prática dos cards. Ela demonstra apenas a ordem e o fechamento dos contratos C11 em host: floor inválido bloqueia, geração stale é rejeitada, sucessão exata é controlada, expiração é respeitada e conflito ou ambiguidade não viram certeza.

O resultado continua sem alegações de WER, acurácia, energia, alcance, latência física ou funcionamento do DRV2605L/SX1262 no hardware real.

## Reprodução

```bash
cd /home/ubuntu/herus-semantic-compiler-pr
python3 tools/test_post_reboot_reindex_redteam.py
cd firmware
make memory-post-reboot-reindex memory-post-reboot-reindex-redteam
cd ..
python3 tools/provenance_audit.py --strict research/software_provenance_manifest.json
./prove.sh --quiet
```

O próximo gate continua sendo a bancada controlada para confrontar a hipótese de consistência lógica com flash/NVS, reboot e energia reais.
