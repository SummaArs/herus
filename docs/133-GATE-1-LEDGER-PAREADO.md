# Gate 1 — ledger pareado por identidade e seed

**Status atual:** `BLOCK` por ausência de ledger executado.  
**Progresso algorítmico:** 94,0%.

## Contrato

O ledger futuro precisa conter exatamente:

```text
386 exemplos × 3 seeds × 3 métodos = 3.474 linhas
```

Métodos congelados no protocolo:

- `herus_full`;
- `multinomial_naive_bayes`;
- `distilbert_multilingual`.

Cada linha deve conter, no mínimo:

- `seed`;
- `example_id`;
- `model`;
- `y_true`;
- `prediction`;
- `accepted`;
- score ou margem quando disponível.

## Validações fail-closed

O validador rejeita:

- ID desconhecido ou ausente;
- rótulo divergente do manifesto S06;
- duplicata de `(seed, example_id, model)`;
- seed não pré-registrada;
- método não congelado;
- ausência de qualquer combinação esperada;
- flag `accepted` ausente ou não booleana;
- previsão ausente.

A análise não pode fazer join por posição.

## Resultado atual

```json
{
  "decision": "BLOCK",
  "rows": 0,
  "expected_rows": 3474,
  "expected_examples": 386,
  "expected_seeds": [17, 29, 43]
}
```

Esse bloqueio é correto. O HERUS ainda não executou o pipeline completo com o contrato de três seeds e não deve receber um claim de eficácia ou causalidade antes disso.

## Próxima execução

1. congelar `herus_full` como implementação concreta;
2. gerar previsões no S04/S05 → S06;
3. registrar as três seeds;
4. unir todos os resultados pelo manifesto `example_id`;
5. recalcular métricas e bootstrap por episódio;
6. só então permitir o Gate 2 de eficácia.
