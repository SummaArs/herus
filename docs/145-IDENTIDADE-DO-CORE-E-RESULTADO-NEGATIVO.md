# Identidade do core: resultado negativo

## Objetivo

Testar se a vantagem observada nas curvas anteriores pertence ao núcleo `SymbioticLearner` ou apenas ao adapter que combina Naive Bayes e centróide.

O selector nativo `propose_action` recebe somente:

- estado atual derivado do texto;
- episódios de fit;
- contexto e contrato de custo/risco;
- passo temporal.

Ele **não recebe `target_effect`**, rótulo do holdout nem pontuação do Naive Bayes.

## Resultado no MInDS-14

| Métrica | Core nativo |
|---|---:|
| Acurácia geral | 0,0816 |
| Cobertura | 0,1939 |
| Precisão seletiva | 0,4211 |
| Abstenções | 79 de 98 |

## Veredito

O núcleo puro não reproduziu a vantagem do adapter. Isso não é uma falha de transparência; é a separação científica necessária:

> A evidência seletiva anterior pertence ao pipeline adapter NB–centróide. Ainda não há evidência de que o `SymbioticLearner` sozinho seja um classificador de linguagem ou um algoritmo SOTA.

O núcleo demonstrou corretamente propriedades de:

- indução de efeitos observados;
- propostas condicionadas;
- abstention por ambiguidade, deriva e contraevidência;
- limites de custo e risco;
- rastreamento de evidência;
- ausência de autoridade de execução.

Essas propriedades são diferentes de classificação semântica geral.

## Consequência para o claim

O claim permitido permanece:

> O HERUS possui um mecanismo de assurance seletiva no adapter NB–centróide. O núcleo SymbioticLearner ainda não demonstrou vantagem preditiva independente.

O claim **SOTA geral continua bloqueado**.

## Próxima direção legítima

Há duas opções cientificamente válidas:

1. definir HERUS como algoritmo de assurance sobre modelos hospedeiros, assumindo explicitamente o adapter como parte da arquitetura; ou
2. criar um encoder/aprendizagem de representação próprio para o core e submetê-lo a novos benchmarks, sem atribuir os resultados atuais a ele retroativamente.

Não é permitido chamar o core de SOTA com base no resultado do adapter.
