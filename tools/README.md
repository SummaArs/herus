# Ferramentas HERUS

## Papel

`tools/` contém scripts que verificam budgets, proveniência, prontidão, interação, evidência de bancada, estudos e mutações. Cada script tem um escopo limitado; nenhum transforma um resultado local em atestação externa.

## Ferramentas centrais

| Ferramenta | Função |
|---|---|
| `provenance_audit.py` | Verificar digests declarados no manifesto. |
| `budget.py` | Recalcular budgets do desenho físico. |
| `readiness_audit.py` | Avaliar gates e manifesto de hardware. |
| `bench_evidence_validate.py` | Validar artefatos de bancada. |
| `interactionstudy.py` | Validar logs e estudos de interação. |
| `studyplan.py` | Validar planos de estudo. |
| `test_proof_fire_mutations.py` | Verificar se mutações importantes falham. |

## Regra de uso

Leia o código e o schema antes de interpretar a saída. Registre o comando, a versão do commit, o ambiente e o resultado bruto. Um `PASS` de ferramenta é uma conclusão local sobre o input que ela recebeu.
