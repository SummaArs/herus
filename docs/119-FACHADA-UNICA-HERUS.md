# Fachada única da biblioteca HERUS

A biblioteca agora pode ser usada por uma única importação:

```python
from herus_symbiotic import Herus
h = Herus()
```

## Quatro operações principais

```python
h.data(records, label="label", objective="classify intent")
h.program("write a parser", language="python")
h.observe(before, "action", after)
h.propose(target_effect, context={}, cost_budget=4)
h.inspect()
```

### Para iniciantes

- `data` recebe registros e explica o que deve ser validado antes do ML;
- `program` transforma uma ideia em plano, perguntas e testes;
- `observe` registra evidência observada;
- `propose` sugere uma ação aprendida, mas não a executa;
- `inspect` mostra versão, observações, habilidades e autoridade.

### Para usuários seniores

A fachada preserva os contratos tipados internos e delega para:

- `DataScienceSkill`;
- `ProgrammingSkill`;
- `SymbioticLearner`;
- `MetaSymbioticLearner`.

A simplicidade é de entrada, não de perda de rigor. Todas as operações continuam locais, proposal-only e sem autoridade implícita.

## Garantias

- importar não abre rede nem executa código;
- propostas não são comandos;
- dados insuficientes geram `ABSTAIN`;
- estado observacional pode ser inspecionado;
- APIs avançadas continuam disponíveis para pesquisa e auditoria.
