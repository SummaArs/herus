# Híbrido: encoder multilíngue + cabeça tensorial

## Hipótese

A rede tensorial hash alcançou 89,80% com 4.608 parâmetros, enquanto o encoder multilíngue congelado transferiu informação cross-language. A hipótese era combinar os dois: usar embeddings multilíngues congelados e comprimir somente a cabeça de decisão com uma camada Tensor-Train.

## Resultado

| Modelo | Parâmetros treináveis | Acurácia | Inferência |
|---|---:|---:|---:|
| Tensor-Train sobre hash, rank 32 | 4.608 | 89,80% | 0,43 ms |
| Encoder congelado + Tensor-Train, rank 32 | 7.424 | 87,76% | 0,55 ms |
| Linear SVM | não comparável diretamente | 95,92% | 6,70 ms |

O híbrido ficou abaixo do tensorial hash puro e abaixo dos baselines clássicos.

## Decisão

A hipótese foi rejeitada nesta configuração. Um encoder multilíngue congelado não garante que uma cabeça Tensor-Train pequena preserve a geometria útil para classificação local. Isso não invalida redes tensoriais; mostra que a escolha da representação e a forma de fatoração importam.

Não há aumento de progresso por desempenho. O resultado é mantido como ablação negativa para impedir seleção pós-hoc da configuração.

Limites: um encoder, um rank, um corpus, uma cabeça de dois núcleos e treinamento supervisionado local.

Evidência: `research/evidence/tensor_encoder_minds14_v1.json`.
