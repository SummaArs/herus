# Risco–cobertura em dados reais: MIntRec

## Protocolo

- Dataset: `THU-IAR/MIntRec`, via API pública do Hugging Face.
- Ajuste: temporada S04, 566 exemplos.
- Calibração: temporada S05, 1.272 exemplos.
- Holdout temporal: temporada S06, 386 exemplos.
- Modelo: Naive Bayes multinomial usado como adaptador de proposta.
- Seleção: escolher o maior coverage em S05 sob um limite mínimo de precisão seletiva; S06 é usado somente uma vez para avaliação.

## Resultado

| Precisão mínima na calibração | Cobertura no holdout | Precisão seletiva no holdout |
|---:|---:|---:|
| 0,50 | 30,83% | 60,50% |
| 0,60 | 0,52% | 0% |
| 0,70 | 0% | 0% |
| 0,80 | 0% | 0% |
| 0,90 | 0% | 0% |

## Interpretação científica

Uma correção metodológica foi necessária: a versão anterior usava S04+S05 para construir o escore do holdout depois de usar S05 na calibração. Isso permitia influência indevida da calibração. A versão atual pontua S06 somente com o modelo ajustado em S04. O resultado corrigido é 60,50% de precisão seletiva com 30,83% de cobertura no ponto de 0,50; para metas de 70% ou mais, não há cobertura. Este resultado substitui o anterior.

Isso não é vitória contra o Naive Bayes completo: o benchmark supervisionado obtém 49,22% de acurácia com 100% de cobertura, enquanto a política corrigida obtém 18,65% de acurácia global e 60,50% de precisão nos casos aceitos.

A contribuição do HERUS é a política explícita de proposta/abstenção e a medição de risco–cobertura. A evidência atual não demonstra superioridade geral nem garantia seletiva de 80%.

## Limites

MIntRec é um corpus textual de intenções externas, não um corpus de eventos HERUS. Não há mapeamento automático para comandos HERUS, nem áudio/vídeo neste ensaio. É necessário repetir a análise em outros datasets e tarefas com rótulos relevantes antes de aumentar alegações.

A evidência bruta está em `research/evidence/risk_coverage_mintrec_v1.json`.
