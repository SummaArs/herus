# Probe de atalhos em dados reais

## Protocolo

Usamos as 386 utterances reais do MIntRec S06 como holdout. Para cada uma, adicionamos um prefixo de polidez/filler:

- `please`;
- `if you can`;
- `thanks`.

Os rótulos originais não foram alterados. A hipótese testada é restrita: o prefixo não deveria mudar a intenção principal. Isso não prova equivalência semântica geral.

## Taxa de mudança de previsão

| Método | `please` | `if you can` | `thanks` |
|---|---:|---:|---:|
| Naive Bayes | 22,54% | 26,17% | 19,95% |
| 1-NN cosseno | 9,33% | 50,78% | 22,28% |
| Centróide cosseno | 4,66% | 55,18% | 8,55% |
| Memória contextual HERUS | 0% | 0% | 25,00% |

## Falha principal

Os métodos clássicos mudam muitas previsões por uma transformação superficial. O caso mais grave é `if you can`, que muda aproximadamente metade das previsões do 1-NN e do centróide.

O HERUS não muda com `please` ou `if you can` porque a memória exata abstém-se. Isso é uma proteção, mas não uma solução de generalização. Com `thanks`, 25% das previsões comparáveis mudaram e a acurácia transformada caiu para 0,78%.

A leitura correta é:

> abstenção reduz falsos compromissos, mas uma memória contextual frágil ainda pode ser afetada por pequenas mudanças de superfície.

## O que o probe não prova

- não prova que os fillers preservam semântica em todos os casos;
- não mede um transformer porque o checkpoint não foi persistido neste probe;
- não prova robustez linguística aberta;
- não transforma baixa taxa de mudança em acerto correto.

## Consequência para o HERUS

A representação do HERUS deve separar conteúdo de intenção de marcadores de polidez, sem simplesmente apagar palavras que podem ser relevantes em outros domínios. A próxima versão precisa aprender uma normalização com evidência e testar tanto invariâncias desejadas quanto mudanças semânticas mínimas.
