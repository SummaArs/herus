# Gate de ledger pareado e custo

**Progresso algorítmico:** 94,0%

## Decisão atual

O MIntRec possui um ledger completo do transformer, mas a análise pareada publicada contém apenas métricas agregadas dos três métodos. Também faltam medições de treino e inferência do HERUS e dos baselines no mesmo ambiente experimental.

O gate retorna:

```text
BLOCK
paired_prediction_ledger_not_archived
mintrec_costs_missing
```

## Por que isso importa

Uma diferença média de acurácia não basta para uma comparação pareada rigorosa. É necessário preservar, para cada índice do mesmo holdout:

- rótulo verdadeiro;
- previsão de cada método;
- aceitação ou abstenção;
- correção seletiva;
- versão e seed.

Da mesma forma, não é permitido afirmar que o HERUS é mais eficiente sem medir com a mesma definição:

- tempo de treino;
- tempo de inferência;
- memória de pico;
- ambiente e versão;
- orçamento computacional.

## Próximo experimento autorizado

O gate será desbloqueado somente quando um artefato contiver os três métodos alinhados no mesmo ledger de 386 exemplos do S06 e um protocolo de custo congelado. Até lá, claims de significância pareada completa e eficiência ficam bloqueados.

O resultado não diminui o valor dos experimentos existentes. Ele separa claramente:

- comparação agregada já disponível;
- comparação pareada completa ainda não arquivada;
- eficiência medida em outros conjuntos, mas não demonstrada de forma comparável no MIntRec.
