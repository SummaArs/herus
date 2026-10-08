# Replicação do meta-algoritmo universal em MIntRec

## Protocolo

Segundo dataset real: **THU-IAR/MIntRec**, com divisão temporal sem embaralhamento:

- fit: temporada S04;
- calibração: temporada S05;
- holdout: temporada S06;
- 386 exemplos no holdout;
- candidatos: Naive Bayes supervisionado e centróide não supervisionado;
- decisão sem acesso aos rótulos do holdout;
- ledger por identidade `season/episode/clip`.

## Resultado

| Método | Acurácia | Macro-F1 | Cobertura |
|---|---:|---:|---:|
| **Roteador universal HERUS** | **38,34%** | 23,42% | 100% |
| Naive Bayes | 36,53% | 18,30% | 100% |
| Centróide | 36,01% | **30,34%** | 100% |

## Comparação pareada com Naive Bayes

- exemplos: 386;
- vitórias do universal: 7;
- vitórias do Naive Bayes: 0;
- empates: 379;
- diferença: +1,81 pontos percentuais;
- bootstrap pareado, seed 20261008, 20.000 reamostragens;
- IC95%: `[0,52; 3,37]` pontos percentuais;
- fração bootstrap não positiva: 0,105%.

## Veredito

A melhora do roteador universal sobre Naive Bayes foi replicada em um segundo dataset real e temporalmente separado. Ao contrário do MInDS-14, onde a diferença não foi conclusiva, neste holdout MIntRec o intervalo bootstrap ficou acima de zero.

Isso é evidência de **generalização inicial da coordenação entre candidatos**, não prova de SOTA geral. O roteador ainda usa apenas dois paradigmas e a vantagem em macro-F1 não supera o centróide.

## Limites

- MIntRec é uma tarefa de classificação de intenção, não um benchmark de eventos simbióticos abertos;
- somente dois candidatos entraram no roteador;
- o LLM e o transformer permanecem fora por falta de calibração/admissão;
- faltam replicações com seeds, datasets e tarefas adicionais;
- custo, latência e memória ainda não foram comparados.

O resultado eleva o HERUS de uma demonstração isolada para uma **hipótese de meta-algoritmo replicável**, ainda não para um claim SOTA.
