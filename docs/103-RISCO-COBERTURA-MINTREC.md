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
| 0,50 | 39,38% | 74,34% |
| 0,60 | 0,78% | 100% |
| 0,70 | 0,26% | 100% |
| 0,80 | 0,26% | 100% |
| 0,90 | 0,26% | 100% |

## Interpretação científica

O ponto de 0,50 é mais útil do que o ponto anterior de 0,80: mantém precisão seletiva de 74,34% no holdout e cobre 39,38% dos exemplos. Ainda assim, não satisfaz uma garantia de 80% fora da calibração. Os pontos de 100% são seguros apenas porque quase sempre se abstêm.

Isso não é vitória contra o Naive Bayes completo: o benchmark supervisionado obtém 49,22% de acurácia com 100% de cobertura, enquanto a política seletiva obtém 29,27% de acurácia global e 74,34% de precisão nos casos aceitos.

A contribuição do HERUS é a política explícita de proposta/abstenção e a medição de risco–cobertura, não superioridade geral em classificação.

## Limites

MIntRec é um corpus textual de intenções externas, não um corpus de eventos HERUS. Não há mapeamento automático para comandos HERUS, nem áudio/vídeo neste ensaio. É necessário repetir a análise em outros datasets e tarefas com rótulos relevantes antes de aumentar alegações.

A evidência bruta está em `research/evidence/risk_coverage_mintrec_v1.json`.
