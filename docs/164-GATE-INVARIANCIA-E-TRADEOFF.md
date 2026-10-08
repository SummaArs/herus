# Gate de invariância de decisão — Banking77

## Veredito

**Ablação forte de segurança; não promovida por baixa cobertura.**

O gate compara a decisão do modelo em três visões determinísticas da mesma entrada: original, normalizada e com prefixo introdutório removido. O limiar foi definido somente com 1.519 exemplos de calibração; o holdout de 3.080 exemplos não participou da escolha.

| Ataque | Acurácia atacada | Precisão dos aceitos | Cobertura | Wrong-label flip |
|---|---:|---:|---:|---:|
| Caixa/pontuação | 74,22% | 74,22% | 100,00% | 2,81% |
| Typo | 19,35% | 73,58% | 26,30% | 2,68% |
| Deleção | 20,55% | 77,01% | 26,69% | 1,21% |
| Prefixo irrelevante | 61,43% | 70,62% | 86,98% | 11,23% |

## Interpretação

O gate reduz substancialmente os flips errados, especialmente em deleção e typo. Porém, faz isso abstendo-se de cerca de 73% desses casos. Isso caracteriza uma fronteira risco–cobertura, não uma vitória geral do classificador.

A implementação foi otimizada para uma única calibração e inferência em lote. A primeira implementação ingênua foi interrompida após consumir CPU excessivamente; a versão final pré-calcula 30.379 textos únicos e termina no tempo esperado.

## Decisão

O gate permanece como ablação. Não foi inserido na política universal porque ainda precisa de uma regra de cobertura mínima, validação cross-domain e comparação com conformal prediction.

**Claim permitido:** melhora de risco condicional em um stress test real do Banking77.

**Claims proibidos:** robustez certificada, SOTA geral, universalidade ou segurança operacional.
