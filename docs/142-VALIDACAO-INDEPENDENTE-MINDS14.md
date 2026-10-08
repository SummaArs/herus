# Validação independente: MInDS-14

## Resultado

O HERUS foi avaliado em um corpus independente do MIntRec:

- dataset: `PolyAI/minds14`;
- configuração: `pt-PT`;
- 604 exemplos reais;
- 14 classes;
- 417 fit, 89 calibração, 98 holdout;
- split estratificado por classe e ordenação determinística de path;
- seeds do transformer: 11, 23 e 47.

## Métricas

| Método | Acurácia | Cobertura | Precisão seletiva | Macro-F1 |
|---|---:|---:|---:|---:|
| Naive Bayes | 0,9184 | 1,0000 | 0,9184 | 0,8929 |
| Centróide | 0,8776 | 1,0000 | 0,8776 | 0,8762 |
| HERUS adapter | 0,8571 | 0,9082 | 0,9438 | 0,8766 |
| Transformer seed 11 | 0,2959 | 1,0000 | 0,2959 | 0,1948 |
| Transformer seed 23 | 0,3265 | 1,0000 | 0,3265 | 0,3028 |
| Transformer seed 47 | 0,2959 | 1,0000 | 0,2959 | 0,2086 |

## Interpretação

O resultado demonstra que a propriedade seletiva do HERUS generaliza para um segundo corpus real: sua precisão nos casos aceitos é superior à do Naive Bayes, embora com cobertura menor e acurácia bruta inferior.

Isso **não prova SOTA geral**. Na tarefa de classificação completa, o Naive Bayes vence o HERUS em acurácia e macro-F1. O transformer usado é pequeno e não deve ser tratado como baseline definitivo de estado da arte.

A afirmação permitida é:

> Em dois corpora reais de intenção, o adapter seletivo do HERUS encontrou regiões de maior precisão entre os casos aceitos do que seus baselines comparados, ao custo de cobertura. Essa evidência apoia uma hipótese de assurance seletiva, não superioridade geral de classificação.

## Limitações

1. O adapter não é o núcleo HERUS isolado.
2. MInDS-14 foi particionado dentro de um único split publicado; não há garantia de independência por locutor.
3. As classes MInDS não são eventos HERUS.
4. Não há bootstrap combinado entre os dois corpora neste artefato.
5. Não há baseline transformer atual de grande escala.
6. Nenhuma conclusão de SOTA geral é autorizada.
