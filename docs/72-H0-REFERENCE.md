# H0 — Reference

## Objetivo

H0 é o executor de referência do HERUS no computador. Ele prioriza correção sem tratar memória, consumo, tamanho, latência ou temperatura como restrições de produção. H0 não é benchmark de desempenho e não concede autoridade.

## Contrato

A entrada canônica contém:

- identificador do caso;
- vetor finito de características;
- fonte e digest de proveniência;
- representações disponíveis;
- orçamento do hospedeiro;
- autoridade declarada;
- limiar de confiança.

A saída canônica contém:

- classe;
- representação selecionada;
- confiança;
- `PROPOSE` ou `ABSTAIN`;
- `ABSTAIN` fixo para execução;
- motivo determinístico;
- autoridade observada.

Os vetores dourados estão em `research/evidence/h0_golden_vectors.json`. Eles serão reutilizados no H1 para equivalência semântica. Diferenças numéricas pequenas podem ser toleradas; diferenças em autoridade, conflito, replay, orçamento, proveniência ou decisão não podem.

## Relação com o pulso

O computador é o laboratório de referência. O objetivo final continua sendo H2: o pulso como **interface corporal pela qual a pessoa entra em simbiose com o sistema**. O H0 existe para separar erro lógico de erro de quantização, firmware, sensor ou energia.

```text
H0 Reference → H1 Embedded → H2 Physical Symbiont → H3 Host Migration
```

O pulso permanece o primeiro hospedeiro físico. H0 apenas fornece a verdade comportamental contra a qual a implementação embarcada será comparada.
