# Primeiro transformer real no MIntRec

## Protocolo

- modelo: `google/bert_uncased_L-2_H-128_A-2`;
- arquitetura: BERT pequeno, 2 camadas, 128 dimensões;
- seed: 17;
- ajuste: S04;
- seleção de checkpoint: S05;
- holdout intocado: S06;
- 5 épocas;
- batch 16;
- comprimento máximo 96;
- taxa de aprendizado `5e-5`;
- execução em CPU;
- tempo medido: 16,943 s.

O classificador foi inicializado para as 20 classes do MIntRec. A cabeça de classificação foi treinada no S04; o S05 escolheu o melhor checkpoint; o S06 foi avaliado uma única vez.

## Resultado

| Método | Acurácia S06 | Macro-F1 | Cobertura |
|---|---:|---:|---:|
| Naive Bayes supervisionado | 49,22% | 37,44% | 100% |
| K-means + calibração | 31,35% | 15,99% | 100% |
| HERUS memória exata | 6,22% | 11,88% | 6,22% |
| BERT pequeno real | **17,62%** | 3,96% | 100% |
| HERUS PPMI auto-supervisionado | 17,88% | 9,12% | 100% |

## Leitura crítica

O transformer real não venceu os baselines clássicos neste protocolo. O HERUS PPMI ficou 0,26 ponto percentual acima em acurácia, mas com diferença pequena e sem significância estatística estabelecida; isso não é uma vitória geral contra transformers.

O modelo também não é estado da arte. É um baseline pequeno e controlado, usado para fechar a comparação arquitetural inicial.

## O que foi provado

1. O protocolo do HERUS consegue executar um transformer real.
2. A comparação usa o mesmo holdout temporal S06.
3. O HERUS não precisa inventar uma vitória: o resultado foi registrado como 17,62%.
4. A auto-supervisão PPMI ainda não demonstrou vantagem robusta.

## O que continua pendente

- transformer maior ou fine-tuning mais amplo;
- múltiplas seeds e intervalo de confiança;
- segundo dataset real;
- custo de memória e inferência para todos os métodos;
- teste de transferência para domínio novo;
- comparação em uma tarefa HERUS de proposta, abstenção e autoridade, não apenas classificação MIntRec.
