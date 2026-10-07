# Transformer forte e comparação com Manus

## Experimento executado

Foi repetido o holdout temporal do MIntRec com `distilbert-base-multilingual-cased`, um encoder multilíngue substancialmente maior que o `bert-tiny` usado anteriormente.

| Modelo | Acurácia S06 | Macro-F1 | Cobertura | Tempo CPU |
|---|---:|---:|---:|---:|
| BERT tiny | 17,62% | 3,96% | 100% | 16,9 s |
| DistilBERT multilíngue | **46,63%** | **33,88%** | 100% | **340,7 s** |
| Naive Bayes | 49,22% | 37,44% | 100% | medido no benchmark clássico |
| HERUS seletivo | — | — | 22,80% | proposta seletiva |

## Leitura

O transformer forte melhorou muito sobre o BERT tiny: +29,02 pontos percentuais de acurácia. Ainda ficou abaixo do Naive Bayes neste protocolo específico e não venceu o sistema seletivo do HERUS no eixo de precisão entre decisões aceitas.

Isso não significa que o HERUS venceu transformers em geral. Foram testados apenas dois checkpoints e um holdout temporal.

## Sobre comparar com Manus

Não é cientificamente válido declarar uma comparação direta com Manus nesta sessão. O HERUS não tem acesso aos pesos, ao ambiente interno, aos prompts proprietários, às ferramentas ou ao endpoint de avaliação do Manus. Portanto, não inventamos uma métrica.

A comparação correta exigiria:

1. tarefa e dataset congelados;
2. mesmo conjunto de entradas e rótulos;
3. protocolo cego;
4. saída exportada por ambos os sistemas;
5. métricas pré-registradas;
6. custo, latência e taxa de abstenção medidos;
7. avaliação independente por terceiro.

Podemos comparar o HERUS contra qualquer modelo externo quando suas previsões forem fornecidas nesse formato. Sem isso, a afirmação correta é: **o benchmark contra Manus permanece não executado**.

## Critério de vitória do HERUS

O HERUS não deve tentar vencer um modelo geral em toda linguagem aberta apenas com um dataset de classificação. Seu território natural é uma decisão verificável: detectar risco, explicar evidência, abster-se quando necessário e impedir execução sem autoridade.
