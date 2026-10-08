# Resultado do meta-algoritmo universal no MInDS-14

## Desenho

O `UniversalSymbioticLearner` recebeu dois candidatos independentes por exemplo:

- `supervised`: Naive Bayes multinomial;
- `unsupervised`: centróide de representação lexical.

A calibração usou 89 exemplos separados, gerando 178 candidatos (dois paradigmas por exemplo). O holdout final continha 98 exemplos. Os rótulos do holdout não foram fornecidos ao roteador.

## Resultado

| Método | Acurácia | Cobertura | Macro-F1 |
|---|---:|---:|---:|
| Roteador universal | **0,9286** | 1,0000 | **0,9119** |
| Naive Bayes | 0,9184 | 1,0000 | 0,8929 |
| Centróide | 0,8776 | 1,0000 | 0,8762 |

## Comparação pareada universal vs Naive Bayes

- vitórias do universal: 1;
- vitórias do Naive Bayes: 0;
- empates: 97;
- diferença observada: +1,02 pontos percentuais;
- bootstrap pareado, seed 20261007, 20.000 reamostragens: IC95% `[0,00; 3,06]` pontos percentuais;
- fração bootstrap não positiva: 0,3716.

## Veredito

O resultado é **promissor, mas não estatisticamente conclusivo**. O roteador não perdeu nenhum exemplo para Naive Bayes neste holdout e ganhou um, mas a amostra pareada é pequena e o intervalo inclui zero.

Este é o primeiro resultado que mostra uma melhoria mensurável do meta-algoritmo sobre o melhor candidato individual no protocolo. Não prova SOTA geral, nem prova substituição de todos os paradigmas.

## Limites

- apenas dois paradigmas foram fornecidos ao roteador;
- o transformer fine-tuned e o gpt-5.5 zero-shot foram mantidos como referências externas, pois não havia calibração pareada equivalente para eles;
- o dataset é um proxy de classificação de intenção, não um conjunto de eventos HERUS;
- não há ainda replicação em um segundo holdout independente;
- a diferença é pequena e não significativa neste bootstrap.

## Próximo experimento necessário

Replicar o protocolo em MIntRec e em pelo menos um terceiro dataset, adicionando candidatos transformer e LLM com calibração própria, sempre preservando o mesmo contrato: fit, calibração, holdout e ledger por exemplo.
