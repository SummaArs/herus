# Calibração de cobertura e precisão

## Objetivo

A memória contextual exata cobria 6,22% do holdout S06 com precisão seletiva de 100%. Esta rodada testou uma extensão probabilística baseada em margem de Naive Bayes, calibrada em S05 e avaliada apenas em S06.

## Regra

- S04: ajuste do modelo;
- S05: calibração do limiar;
- S06: holdout temporal final;
- nenhuma decisão do limiar usa rótulos de S06.

## Resultado

| Meta de precisão seletiva na calibração | Cobertura em S06 | Precisão seletiva em S06 |
|---:|---:|---:|
| 80% | 0,00% | sem propostas |
| 60% | 0,52% | 0,00% |
| 50% | 30,83% | 60,50% |
| Memória exata | 6,22% | 100,00% |

## Conclusão

A extensão aumenta a cobertura, mas não preserva a garantia de precisão. O resultado não autoriza incorporá-la ao núcleo soberano como política padrão.

A fronteira observada é:

> aumentar cobertura por similaridade textual simples transfere risco para falsos aceites.

A política correta permanece fail-closed. A próxima melhoria precisa aprender uma representação de contexto/efeito mais informativa, não apenas diminuir o limiar probabilístico.

A versão atual não teve aumento de score de conclusão. A implementação foi preservada como experimento reproduzível e não como vitória de desempenho.
