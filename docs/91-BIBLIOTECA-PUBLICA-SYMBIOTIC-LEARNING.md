# Biblioteca pública HERUS Symbiotic Learning

## Importação

```python
import herus_symbiotic as herus

learner = herus.SymbioticLearner()
episode = herus.Episode.from_maps(
    {"mode": 0},
    "tap",
    {"mode": 1},
)
learner.observe(episode)
proposal = learner.propose(episode.effect, (episode,))
```

A API retorna `PROPOSE` ou `ABSTAIN`. Ela não executa a ação, não abre rede, não acessa hardware e não concede autoridade durante a importação ou inferência.

## Superfície estável v0.1.0

- `Episode`: observação finita de transição;
- `SymbioticLearner`: indução e proposta bounded;
- `SkillHypothesis`: hipótese observável;
- `Proposal`: decisão proposal-only;
- `Problem`: objetivo, contexto e risco;
- `MetaSymbioticLearner`: histórico verificado e ranking de sondas;
- `MetaProposal`: proposta com indicação explícita de evidência fresca.

## Limites

A biblioteca não é uma AGI, não é um executor de agentes, não substitui modelos de linguagem e não prova generalização aberta. Seu contrato é deliberadamente pequeno: aprender efeitos observáveis sob orçamento finito, reter referências verificadas e abster-se quando a evidência é insuficiente, ambígua, arriscada ou obsoleta.

O pacote `herus_symbiotic` é a superfície pública. Os módulos em `research` continuam sendo a implementação e o laboratório de pesquisa; consumidores não devem depender de funções privadas desses módulos.
