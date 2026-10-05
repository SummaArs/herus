# Holdout independente do Symbiotic Learning

**Status:** holdout local preregistrado, com oracle separado; ainda não é validação externa.

## Desenho

O holdout usa um gerador separado da campanha ampla e produz 60 casos:

- 12 seguros com vocabulário de ações novo;
- 12 com contexto parcial incompatível;
- 12 com evidência temporalmente revertida/obsoleta;
- 12 com risco acima do limite;
- 12 com alias de ação.

O learner recebe apenas `Problem` e candidatos. O oracle permanece no avaliador e não é passado para a biblioteca.

## Resultado

```text
casos totais: 60
seguros corretos: 12 / 12
falsos aceites inseguros: 0 / 48
```

Evidência bruta: [`research/evidence/holdout_symbiotic_learning_v1.json`](../research/evidence/holdout_symbiotic_learning_v1.json).

## Falha encontrada e corrigida

A primeira execução falhou em 6 dos 12 casos seguros. A causa foi uma incompatibilidade real de representação: o oracle do fixture tratava `armed=0` como parte explícita do efeito, enquanto o learner canônico representa zero como ausência de delta.

A correção foi feita no fixture, removendo zero explícito do objetivo. O teste passou depois disso. O episódio é importante: o holdout encontrou uma divergência semântica que a campanha anterior não cobria.

## Interpretação

O resultado aumenta a evidência de robustez do contrato, mas não prova generalização mundial. O holdout continua:

- determinístico;
- local;
- sintético;
- criado pelo próprio projeto;
- sem usuário real;
- sem hardware;
- sem replicação independente.

A próxima versão deve incluir um oracle externo ao repositório ou um conjunto congelado produzido por outra implementação.
