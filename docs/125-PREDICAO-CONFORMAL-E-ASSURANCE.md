# Predição conformal como assurance

A rodada adiciona uma camada de **split conformal prediction** sobre o adaptador Naive Bayes usado como baseline do HERUS.

## Protocolo

- S04: ajuste do modelo;
- S05: calibração dos escores de não conformidade;
- S06: holdout temporal intocado;
- níveis testados: 80%, 90%, 95% e 99% de cobertura marginal.

## Resultado no holdout S06

| Cobertura alvo | Cobertura observada | Tamanho médio do conjunto | Cobertura singleton |
|---:|---:|---:|---:|
| 80% | 81,35% | 8,56 classes | 1,04% |
| 90% | 90,16% | 12,84 classes | 0% |
| 95% | 94,82% | 15,98 classes | 0% |
| 99% | 99,48% | 19,11 classes | 0% |

## Interpretação

A técnica resolve uma parte importante da deriva temporal: em vez de transformar uma previsão fraca em uma decisão falsa, o sistema devolve um **conjunto explícito de hipóteses** com cobertura mensurável.

Ela também revela o preço da honestidade: neste domínio, o conjunto é grande. Portanto, não é uma vitória de classificação e não prova raciocínio geral. É uma melhoria de assurance: o HERUS sabe representar incerteza sem escolher uma classe arbitrariamente.

O próximo refinamento é aprender representações que reduzam o tamanho dos conjuntos mantendo a cobertura, sempre calibradas fora do holdout.

## Ablação por classe

Também foi testada uma calibração com quantis separados por classe. Ela reduziu modestamente o tamanho médio dos conjuntos — de 15,98 para 15,37 no alvo nominal de 95% —, mas cobriu apenas 93,78% do S06. Portanto, foi **rejeitada como política principal**: uma compressão que perde a cobertura prometida não é melhoria de assurance.
