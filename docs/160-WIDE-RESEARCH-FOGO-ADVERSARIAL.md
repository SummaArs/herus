# Wide Research: fogo adversarial e expansão de datasets

## Decisão

O HERUS não pode receber um claim de SOTA geral apenas por melhorar em MIntRec, MInDS-14 e Banking77. O próximo ciclo deve separar quatro propriedades:

1. **qualidade limpa**;
2. **seletividade e abstention**;
3. **robustez adversarial/OOD**;
4. **custo controlado**.

A Wide Research recomendou começar por um experimento real, reprodutível e barato: uma bateria adversarial no Banking77, preservando o test oficial e sem usar o test para calibrar thresholds.

## Bateria inicial executável

A primeira implementação usa perturbações determinísticas no texto real do Banking77:

- `case_punctuation`: caixa e pontuação normalizadas;
- `typo`: transposição determinística em um token elegível;
- `deletion`: remoção determinística de um caractere;
- `irrelevant_prefix`: prefixo conversacional neutro, tratado como stress test.

Essas transformações são **stress tests textuais**, não prova de invariância semântica. Sem revisão humana cega, não é permitido chamá-las de ataques label-preserving certificados.

As métricas registradas são:

- accuracy limpa e atacada;
- coverage;
- selective risk entre casos aceitos;
- taxa de abstention;
- `wrong_label_flip_rate` entre exemplos limpos aceitos e corretos;
- `abstention_induced_rate` quando o ataque transforma uma decisão limpa correta em abstention;
- divergência entre saída limpa e atacada.

## Expansão de datasets — ordem recomendada

| Prioridade | Dataset | O que testa | Bloqueio antes de executar |
|---|---|---|---|
| P0 | CLINC150 | intenção in-scope + OOS | resolver discrepância CC BY 3.0/4.0 e congelar versão |
| P0 | MASSIVE 1.1 | 52 idiomas e baixa disponibilidade | fixar commit 1.1 e bloquear leakage entre traduções paralelas |
| P0 | CivilComments-WILDS | toxicidade e pior grupo | preservar metadata de identidade e revisar risco ético |
| P1 | MultiNLI | matched/mismatched e mudança de gênero | não chamar NLI de intent classification; fixar licença/checkpoint |
| P1 | TweetEval | ruído social e tarefas heterogêneas | respeitar regras de tweets e tarefas individuais |
| P1 | HWU64 | 64 intenções e outro artefato | resolver versão e licença não confirmada |
| P2 | SNIPS | sanity check barato | não usar como evidência de universalidade |

## Fontes públicas

- CLINC150: https://github.com/clinc/oos-eval e https://archive.ics.uci.edu/dataset/570/clinc150
- MASSIVE: https://aclanthology.org/2023.acl-long.235/ e https://huggingface.co/datasets/AmazonScience/massive
- CivilComments/WILDS: https://wilds.stanford.edu/datasets/ e https://github.com/p-lambda/wilds
- MultiNLI: https://cims.nyu.edu/~sbowman/multinli/ e https://aclanthology.org/N18-1101/
- TweetEval: https://github.com/cardiffnlp/tweeteval e https://aclanthology.org/2020.findings-emnlp.148/
- HWU64: https://github.com/jianguoz/Few-Shot-Intent-Detection e https://huggingface.co/datasets/DeepPavlov/hwu64
- SNIPS: https://github.com/sonos/nlu-benchmark e https://arxiv.org/abs/1805.10190

## Claims proibidos neste ciclo

- SOTA geral;
- robustez multimodal inferida de texto;
- robustez OOD sem gold OOD independente;
- invariância sem revisão semântica;
- economia energética sem medição de energia;
- universalidade entre ontologias incompatíveis.
