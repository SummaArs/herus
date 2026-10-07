# HERUS Symbiotic — função objetivo e regra de atualização

**Status:** especificação de pesquisa v0; ainda não é implementação completa nem claim de desempenho.

## Objetivo

O HERUS deve aprender uma política adaptada ao hospedeiro sem transformar aprendizagem em autoridade automática.

Para um hospedeiro `H`, observação `x_t`, memória `M_t` e contratos `C_t`:

```text
z_t = Encode(x_t, H, C_t)
a_t = Policy(z_t, M_t) ou ABSTAIN
```

A função de utilidade de uma proposta é:

```text
J_t = U(a_t, y_t)
      - λ Risk(a_t, C_t)
      - μ Cost(a_t, H)
      - ν AuthorityViolation(a_t, C_t)
      - ρ EvidenceDeficit(a_t, M_t)
```

A política deve otimizar `J`, mas somente dentro do conjunto admissível do hospedeiro:

```text
θ* = argmax_θ E[J_t | θ, H]
subject to θ ∈ Contract(H)
```

`ABSTAIN` é uma ação válida quando nenhuma proposta supera o limiar de utilidade e evidência.

## Atualização

A regra de atualização futura deve consumir feedback positivo e negativo:

```text
M_{t+1}, θ_{t+1} = Update(
    M_t,
    θ_t,
    z_t,
    a_t,
    y_t,
    provenance_t,
    C_t
)
```

Requisitos:

- feedback sem provenance não atualiza o estado;
- resultado negativo reduz suporte ou cria contraevidência;
- conflito nunca é resolvido silenciosamente;
- atualização excedendo orçamento é rejeitada;
- snapshot anterior permanece restaurável;
- nenhuma atualização concede autoridade de execução;
- mudanças de hospedeiro exigem revalidação das precondições.

## O que o v2 já implementa

- episódios tipados;
- cálculo de efeitos observados;
- indução contextual bounded;
- propostas condicionadas;
- IDs de evidência;
- explicação determinística;
- limites de risco/custo;
- abstention;
- snapshot/rollback.

## O que falta para chamar de algoritmo completo

- entrada que não receba o rótulo/efeito verdadeiro do holdout;
- estado atual e precondições na decisão;
- negativos usados como contraevidência real;
- função de utilidade executável e versionada;
- atualização de `θ` ou memória guiada por feedback;
- adapter de hospedeiro separado do core;
- benchmark de interfaces/hosts permutados;
- comparação com controles em orçamento equivalente;
- medição conjunta de utilidade, risco, custo, cobertura e latência.

## Critério de avanço

O HERUS não será chamado de SOTA por possuir uma função matemática. A função precisa produzir uma vantagem reproduzível em um regime definido e sobreviver a ablações, baselines fortes, três seeds e holdouts não vistos.
