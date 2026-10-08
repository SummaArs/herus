# HERUS como nova era do aprendizado de máquina

## Escolha estratégica

O HERUS não deve tentar ser um quinto modelo de classificação que substitui aprendizado supervisionado, não supervisionado, auto-supervisionado ou por reforço. Essa disputa favoreceria os modelos existentes em seus próprios territórios.

A proposta mais forte e testável é uma camada ortogonal:

> **Aprendizado simbiótico é o aprendizado de uma política de coordenação entre um modelo, um hospedeiro e um regime de autoridade.**

O objeto de aprendizagem deixa de ser apenas `y = f(x)` e passa a ser:

```text
(host, candidate, context, evidence, risk, cost, authority)
    -> ACCEPT(candidate) ou ABSTAIN(reason)
```

## Relação com os paradigmas existentes

| Paradigma | Pergunta principal | Papel possível no HERUS |
|---|---|---|
| Supervisionado | Qual saída corresponde ao exemplo? | produzir candidatos |
| Não supervisionado | Que estrutura existe nos dados? | descobrir contexto e drift |
| Auto-supervisionado | Como aprender representação sem rótulo explícito? | produzir embeddings/estado |
| Reforço | Qual ação maximiza retorno acumulado? | produzir políticas de ação |
| **Simbiótico** | Quando este candidato pode ser aceito neste hospedeiro, com quais garantias? | verificar, calibrar, abster e adaptar |

O HERUS pode envolver qualquer um dos quatro paradigmas, mas não é reduzido a nenhum deles.

## Função objetivo

Para um candidato `c` e hospedeiro `H`:

```text
J(accept | c, H) =
    Utility(c)
    - λ Risk(c, H)
    - μ Cost(c, H)
    - ρ EvidenceDeficit(c)
    - ν AuthorityViolation(c, H)
```

A ação válida pode ser `ABSTAIN`. A política é ajustada em calibração e congelada no holdout.

## Implementação v0

`research/symbiotic_policy.py` implementa:

- limiar calibrado apenas no conjunto de calibração;
- restrições explícitas do hospedeiro;
- déficit de evidência;
- abstention determinística;
- IDs de evidência;
- feedback verificado com provenance;
- orçamento finito de atualizações;
- ausência de autoridade de execução.

## Estado científico

A política v0 é uma **especificação executável**, não uma prova de SOTA. O benchmark anterior demonstrou assurance seletiva no adapter NB–centróide; o teste do core puro mostrou que essa vantagem ainda não pertence ao `SymbioticLearner` isolado.

Para elevar a tese a um novo campo de ML, ainda será necessário demonstrar em datasets reais que a política simbiótica melhora uma métrica definida — por exemplo, precisão sob cobertura fixa e risco limitado — contra baselines calibrados dos quatro paradigmas.

## Claim permitido

> HERUS propõe uma quinta dimensão de aprendizado: a coordenação verificável entre candidatos de modelos, hospedeiros, evidências e autoridade. A implementação inicial é testável e fail-closed; sua superioridade geral ainda não foi provada.
