# Competição pareada com transformer

## Protocolo

Foram usadas as mesmas 386 amostras do holdout temporal S06 do MIntRec para:

- Naive Bayes multinomial;
- memória finita contextual do HERUS;
- DistilBERT multilíngue fine-tuned em S04, selecionado em S05.

Cada modelo agora possui previsão por exemplo. O bootstrap reamostra os mesmos índices para comparar diferenças diretamente.

## Resultado

| Comparação | Diferença observada | IC 95% bootstrap |
|---|---:|---:|
| HERUS − DistilBERT | −40,41 pp | [−45,60; −35,23] pp |
| DistilBERT − Naive Bayes | −2,59 pp | [−7,77; +2,59] pp |
| HERUS − Naive Bayes | −43,01 pp | [−47,93; −37,82] pp |

Métricas absolutas:

| Método | Acurácia | Cobertura | Precisão seletiva |
|---|---:|---:|---:|
| Naive Bayes | 49,22% | 100% | 49,22% |
| DistilBERT multilíngue | 46,63% | 100% | 46,63% |
| HERUS memória finita | 6,22% | 6,22% | 100% |

## Conclusão sem maquiagem

O HERUS **não venceu** o transformer ou o Naive Bayes nesta tarefa de classificação aberta. O intervalo do DistilBERT contra Naive Bayes inclui zero, portanto não há evidência suficiente de diferença estatisticamente conclusiva entre esses dois métodos neste único holdout.

A propriedade comprovada do HERUS é diferente: quando a memória aceita uma decisão, ela foi correta neste holdout. Isso é assurance seletiva, não SOTA de classificação.

## O que falta para SOTA

- múltiplas seeds do transformer;
- mais datasets e domínios;
- vetores de previsão do Tensor-Train;
- custos e memória por método;
- tarefa nativa de assurance com rótulos e consequências próprios;
- comparação independente contra sistemas gerais, incluindo Manus, mediante saídas observáveis no mesmo protocolo.
