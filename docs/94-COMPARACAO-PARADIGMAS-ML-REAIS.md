# Comparação entre paradigmas de ML em dados reais

## Dados e protocolo

Foi usado o MIntRec público, com 2.224 registros reais de texto e 20 intenções. O ajuste usa S04, a calibração de clusters usa S05 e o holdout temporal é S06.

Nenhum rótulo MIntRec foi transformado em evento HERUS.

## Resultados

| Paradigma/adaptador | Método | Acurácia | Macro-F1 | Cobertura |
|---|---|---:|---:|---:|
| Supervisionado | Naive Bayes multinomial | 49,22% | 37,44% | 100% |
| Supervisionado | 1-NN cosseno | 39,90% | 32,20% | 100% |
| Não supervisionado | K-means + mapeamento S05 | 31,35% | 15,99% | 100% |
| Auto-supervisionado | Coocorrência PPMI + protótipo congelado | 17,88% | 9,12% | 100% |
| Reforço | Bandit contextual proxy | 11,92% | 10,94% | 99,48% |
| Memória simbiótica exata | Contexto finito | 6,22% | 11,88% | 6,22% |

## Auto-supervisionado

Esta primeira versão usa coocorrência local e PPMI no S04 como tarefa de representação sem rótulos. A representação é congelada; apenas protótipos supervisionados são ajustados depois. Ela foi avaliada no S06 e obteve 17,88%. Isso é um resultado real, mas fraco.

O protocolo especifica:

1. tarefa pretexto sem rótulo;
2. divisão temporal;
3. representação aprendida;
4. congelamento antes da avaliação;
5. transferência para S06;
6. comparação contra os mesmos baselines.

O resultado não vence K-means, 1-NN ou Naive Bayes. Também não é um transformer.

## Limites do reforço

O bandit foi incluído como adaptador transparente. Seus “estados” são textos, suas “ações” são rótulos e sua recompensa é acerto de classificação. Isso não constitui um ambiente de reforço completo porque o MIntRec não possui transições causais, consequências de ação ou interação.

Logo, não é correto afirmar que o HERUS venceu ou perdeu o RL em geral. O resultado apenas mostra que esse proxy não é competitivo na tarefa textual escolhida.

## Estado científico

O HERUS ainda não vence os baselines supervisionados reais. O valor diferencial continua sendo a combinação de proposta, abstenção, memória verificável e controle de autoridade. Para competir em desempenho, ainda falta uma representação transferível que preserve essas garantias.

## Transformers

Não há uma vitória contra transformers nesta rodada porque nenhum transformer foi treinado e avaliado neste mesmo protocolo. A comparação correta exigirá fixar modelo, tokenizer, orçamento de parâmetros, dados permitidos, fine-tuning, seed, custo e holdout. “Vencer transformers” é uma hipótese futura falsificável, não um resultado atual.
