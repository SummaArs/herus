# Bootstrap combinado: efeito de seleção

## Protocolo

O transformer foi avaliado nos **mesmos exemplos aceitos pelo HERUS**. Foram usados 4.000 reamostragens bootstrap por domínio, seed 17.

Esse teste responde:

> Quando o HERUS aceita um exemplo, o transformer acertaria esse mesmo exemplo com a mesma frequência?

Ele não responde:

> O HERUS é um classificador geral melhor?

## Resultados

| Dataset/domínio | Cobertura HERUS | Exemplos aceitos | Delta de acurácia HERUS − transformer | IC95% |
|---|---:|---:|---:|---:|
| MIntRec S04 | 5,88% | 5 | +0,6032 | [+0,2000; +1,0000] |
| MIntRec S05 | 26,18% | 50 | +0,2986 | [+0,1600; +0,4400] |
| MIntRec S06 | 17,24% | 10 | +0,2027 | [0,0000; +0,5000] |
| MInDS-14 pt-PT | 90,82% | 89 | +0,6405 | [+0,5393; +0,7416] |

## Interpretação

O resultado é consistente com a hipótese de que o HERUS identifica uma região de exemplos mais fáceis ou mais confiáveis. Nos exemplos aceitos, o transformer tem desempenho inferior.

Isso é uma evidência de **assurance seletiva**, não de superioridade geral. O HERUS decide não aceitar muitos casos no MIntRec; comparar somente os aceitos inevitavelmente mede o efeito do filtro.

No MInDS-14 a cobertura é alta, o que fortalece a hipótese, mas ainda não elimina o problema: o Naive Bayes continua superior em acurácia geral no holdout.

## Claim permitido

> Em dois corpora reais, os exemplos aceitos pelo HERUS são significativamente mais bem resolvidos pelo pipeline HERUS do que pelo transformer pequeno pareado. O resultado apoia a capacidade de seleção de casos confiáveis, não um claim de SOTA geral.

## O que falta

- comparar curvas completas de risco–cobertura;
- usar baselines calibrados com o mesmo orçamento e mesmo espaço de decisão;
- isolar o núcleo HERUS do mecanismo NB/centróide;
- usar um transformer atual de maior escala;
- avaliar um terceiro corpus ou uma divisão por locutor;
- pré-registrar o protocolo antes da confirmação final.
