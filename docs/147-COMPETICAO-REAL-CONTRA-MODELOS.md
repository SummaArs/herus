# Competição real: HERUS contra modelos e referência LLM

## Protocolo

Dataset independente: **PolyAI/MInDS-14**, configuração `pt-PT`.

- 98 exemplos no holdout determinístico por classe e caminho;
- HERUS adapter, Naive Bayes e centróide treinados no fit;
- transformer pequeno treinado no mesmo dataset e protocolo;
- gpt-5.5 avaliado zero-shot com prompt fixo e lista de classes;
- ledger por exemplo;
- nenhum resultado do holdout usado para calibrar o HERUS.

A referência gpt-5.5 não é chamada de “Manus”: o endpoint não expõe esse identificador. É uma referência LLM externa ao core, não um transformer fine-tuned.

## Resultados

| Método | Acurácia | Cobertura | Precisão seletiva | Protocolo |
|---|---:|---:|---:|---|
| Naive Bayes | 0,9184 | 1,0000 | 0,9184 | fit/holdout |
| Centroid baseline | 0,8776 | 1,0000 | 0,8776 | fit/holdout |
| HERUS adapter | 0,8571 | 0,9082 | **0,9438** | seleção calibrada |
| Transformer seed 11 | 0,2959 | 1,0000 | 0,2959 | fine-tuned |
| Transformer seed 23 | 0,3265 | 1,0000 | 0,3265 | fine-tuned |
| Transformer seed 47 | 0,2959 | 1,0000 | 0,2959 | fine-tuned |
| gpt-5.5 zero-shot | 0,0408 | 1,0000 | 0,0408 | sem treino no dataset |

## Veredito honesto

Neste holdout, o HERUS **não venceu Naive Bayes em acurácia geral**. Venceu Naive Bayes na métrica seletiva entre os casos aceitos, ao custo de rejeitar aproximadamente 9,2% dos casos.

Também não é correto dizer que venceu o transformer “em geral”: venceu estes três runs específicos do transformer pequeno neste dataset. O gpt-5.5 zero-shot teve resultado inferior, mas zero-shot e fine-tuning não são protocolos equivalentes.

O resultado apoia a tese de que o HERUS pode ser uma política de assurance seletiva, não a tese de SOTA geral.

## O que a competição realmente provou

1. A abstenção calibrada pode aumentar precisão nos casos aceitos.
2. A cobertura é uma dimensão indispensável; não pode ser omitida.
3. Um transformer pequeno pode ser inferior a baselines clássicos em dataset pequeno.
4. Um LLM forte sem exemplos do dataset não deve ser tratado como oráculo universal.
5. A meta-política universal ainda precisa ser avaliada como um método próprio, e não apenas descrita.

## Claim bloqueado

Não há base para afirmar que HERUS substitui todos os modelos ou que é SOTA geral. Para esse claim, o próximo experimento precisa comparar o **roteador universal HERUS** contra uma coleção pareada de candidatos, em múltiplos datasets e sob uma função de risco/cobertura pré-registrada.
