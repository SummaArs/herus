# Competição final de ML — scorecard real

> Este relatório agrega experimentos já executados. Não treina novamente nem transforma um dataset em prova universal.

## Resultado honesto

O HERUS **ainda não vence todos os algoritmos**. No MIntRec, o melhor baseline de cobertura total é o Naive Bayes (49,22% de acurácia) e o Tensor-Train rank 64 chega a 42,23%. O HERUS seletivo atinge 93,18% de precisão, mas aceita apenas 22,80% dos casos. São objetivos diferentes.

| Dataset | Método | Família | Acurácia | Macro-F1 | Cobertura | Precisão seletiva |
|---|---|---|---:|---:|---:|---:|
| MInDS-14 pt-PT | tfidf_linear_svm | classical supervised | 0.959184 | 0.955328 | 1.0 | 0.959184 |
| MInDS-14 pt-PT | tfidf_random_forest | classical supervised | 0.959184 | 0.953565 | 1.0 | 0.959184 |
| MInDS-14 pt-PT | tfidf_logistic_regression | classical supervised | 0.94898 | 0.94637 | 1.0 | 0.94898 |
| MInDS-14 pt-PT | tfidf_knn | classical supervised | 0.938776 | 0.934785 | 1.0 | 0.938776 |
| MIntRec S06 | multinomial_naive_bayes | supervised / HERUS adapter | 0.492228 | 0.374427 | 1.0 | 0.492228 |
| MIntRec S06 | tensor_train_rank_64 | tensor network | 0.42228 | 0.370328 | 1.0 | 0.42228 |
| MIntRec S06 | 1nn_cosine | supervised / HERUS adapter | 0.398964 | 0.322042 | 1.0 | 0.398964 |
| MIntRec S06 | tensor_train_rank_32 | tensor network | 0.398964 | 0.354478 | 1.0 | 0.398964 |
| MIntRec S06 | centroid_cosine | supervised / HERUS adapter | 0.373057 | 0.340202 | 1.0 | 0.373057 |
| MIntRec S06 | unsupervised_kmeans_cluster_then_calibrate | unsupervised / self-supervised / RL proxy | 0.313472 | 0.15986 | 1.0 | 0.313472 |
| MIntRec S06 | self_supervised_cooccurrence_frozen_prototype | unsupervised / self-supervised / RL proxy | 0.178756 | 0.091169 | 1.0 | 0.178756 |
| MIntRec S06 | bert_tiny | transformer | 0.176166 | 0.039633 | 1.0 | 0.176166 |
| MIntRec S06 | majority | supervised / HERUS adapter | 0.119171 | 0.010648 | 1.0 | 0.119171 |
| MIntRec S06 | reinforcement_contextual_bandit_proxy | unsupervised / self-supervised / RL proxy | 0.119171 | 0.10936 | 0.994819 | 0.119792 |
| MIntRec S06 | symbiotic_finite_context_memory | supervised / HERUS adapter | 0.062176 | 0.11877 | 0.062176 | 1.0 |
| MIntRec S06 | symbiotic_calibrated_prototype | supervised / HERUS adapter | 0.0 | 0.0 | 0.0 | 0.0 |
| MIntRec S06 | herus_selective_target_95 | HERUS selective assurance | - | - | 0.227979 | 0.931818 |

## Interpretação

- **Cobertura total:** HERUS ainda perde para baselines supervisionados no MIntRec.
- **Assurance seletiva:** HERUS reduz risco ao abster-se; isso não é vitória de classificação geral.
- **Transformer:** o checkpoint pequeno testado ficou abaixo dos baselines clássicos; isso não representa todos os transformers.
- **Reforço:** o resultado é apenas proxy contextual; não houve ambiente com transições reais.
- **MInDS-14:** SVM linear e floresta atingiram 95,92%; HERUS não foi adaptado a esse corpus nesta rodada.

## Próximo critério de 100%

Só considerar avanço para 100% após: múltiplos datasets independentes, seeds repetidas, intervalos de confiança, teste estatístico pareado, custo medido e comparação com modelos fortes ajustados de forma justa.
