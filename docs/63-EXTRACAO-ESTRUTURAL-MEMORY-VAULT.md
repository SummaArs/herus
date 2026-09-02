# Extração estrutural do memory vault

## Objetivo

O HERUS agora compara automaticamente um subconjunto estrutural de `firmware/core/memory_vault.c` com o caso declarativo real de assurance. O extrator é host-only e nunca executa, compila ou interpreta o C11 como autoridade de runtime.

O comando reproduzível é:

```text
make -C research memory-vault-extraction
```

A saída é `research/evidence/memory_vault_structural_extraction.result.json`.

## Subconjunto observado

O parser reconhece apenas funções públicas específicas, corpos com chaves balanceadas, atribuições diretas a `v->state`, retornos com códigos nomeados, chamadas de armazenamento conhecidas e a ordem lexical de `auth_valid`, `card_valid` e dos sinks de persistência.

| Construção | Tratamento |
|---|---|
| Funções `memory_vault_init`, `seal`, `open`, `erase` | Observadas com linha de origem |
| `v->state = MEMORY_VAULT_*` | Observado |
| `return MEMORY_VAULT_*` | Observado |
| `store_sealed` e `commit_generation_floor` | Observados como sinks |
| Guardas de autorização e cartão | Observadas e comparadas com os sinks |
| Macro não permitida | `UNKNOWN` |
| Corpo desbalanceado ou função absorvida | `UNKNOWN` |
| Função, guarda ou sink ausente | `DIVERGENCE` |
| Sink antes da guarda | `DIVERGENCE` |

## Resultado atual

O fonte real produz:

```text
verdict = EXTRACTED_MATCH
reason  = declared_obligations_observed
```

O resultado inclui SHA-256 do fonte, observações com arquivo implícito e linha, funções observadas, retornos, transições de estado, sinks e guardas. O digest atual do fonte analisado é `3fe1eaaa189ad01b2d0eac6ce383940ab1de189b412cc8526a1b91b3d81d1807`.

## Falsificação

Os testes mutantes exercem remoção da guarda de autorização, sink antes da guarda, função ausente, corpo desbalanceado, macro não suportada e schema inválido. Os resultados esperados são, respectivamente, `DIVERGENCE`, `DIVERGENCE`, `DIVERGENCE`, `UNKNOWN`, `UNKNOWN` e `UNKNOWN`.

## Limites

`EXTRACTED_MATCH` significa que as obrigações declaradas pelo extrator foram observadas no subconjunto estrutural suportado. Não significa que todo C11 foi analisado, que todos os caminhos interprocedurais foram cobertos, que macros e ponteiros de função foram resolvidos, que não existe comportamento indefinido ou que o firmware está certificado.

Por essa razão, o resultado permanece separado do certificado composto. A promoção futura exigirá uma política explícita e testes demonstrando que a extração automática cobre todas as obrigações relevantes do caso. Até lá, a extração é evidência estrutural adicional, não autoridade.
