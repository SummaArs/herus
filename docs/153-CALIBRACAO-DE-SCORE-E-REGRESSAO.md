# Calibração de score por paradigma

Foi testada uma nova política que transforma o score bruto de cada paradigma em uma estimativa empírica de precisão usando exclusivamente a calibração.

| Dataset | Roteador atual | Score calibrado | Naive Bayes |
|---|---:|---:|---:|
| MInDS-14 | **92,86%** | 91,84% | 91,84% |
| MIntRec | 38,34% | **39,64%** | 36,53% |

## Veredito

A calibração de score melhora o desempenho sob drift temporal no MIntRec, mas piora o resultado no MInDS-14. Portanto, ela não substitui automaticamente o roteador atual.

Esse resultado revela que a escala de confiança e a política de seleção dependem do hospedeiro e do regime de dados. Uma única regra global pode ser subótima.

O método permanece como ablação experimental. O próximo passo é selecionar entre políticas usando apenas a partição de calibração, com uma regra pré-definida, e verificar se essa seleção generaliza no holdout sem leakage.

Não há claim de SOTA neste documento; há um resultado positivo e uma regressão explícita.
