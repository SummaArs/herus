# Habilidade de programação — Jev-inspired

## O que foi incorporado

A biblioteca pública agora inclui `ProgrammingSkill`, uma habilidade de programação que transforma uma solicitação tipada em um artefato revisável:

- decomposição do problema;
- perguntas de escopo e restrições;
- perguntas de autoridade para efeitos externos;
- esqueleto de código;
- lista de testes adversariais;
- pressupostos explícitos;
- status `PROPOSE`, `PROPOSE_WITH_QUESTIONS` ou `ABSTAIN`.

A inspiração vem do padrão Jev/System One estudado anteriormente: uma tarefa é dividida em perguntas estruturadas, e a solução é tratada como uma sequência de decisões observáveis, não como uma resposta textual única.

## Limites constitucionais

A habilidade não:

- escreve arquivos;
- executa código;
- abre shell;
- instala dependências;
- acessa rede;
- publica alterações;
- concede autoridade.

Ela produz uma proposta. Aplicação, execução e publicação continuam sendo etapas externas, sujeitas a revisão e aos contratos do HERUS.

## Exemplo

```python
from herus_symbiotic import ProgrammingRequest, ProgrammingSkill

skill = ProgrammingSkill()
proposal = skill.propose(
    ProgrammingRequest(
        goal="add bounded JSON parsing",
        language="python",
        constraints=("stdlib only",),
    )
)
```

A proposta contém código, plano e testes. O código gerado é um esqueleto deliberadamente incompleto. Isso é intencional: gerar código plausível não é o mesmo que provar código correto.

## Por que isso ainda não aumenta o score

A habilidade foi validada como API proposal-only, mas ainda não foi comparada em um benchmark real de programação contra modelos ou ferramentas existentes. Portanto, ela aumenta o repertório do HERUS, mas não aumenta automaticamente o score científico de conclusão.

O próximo benchmark deve usar tarefas reais versionadas, teste oculto, execução isolada fora da biblioteca e métricas de correção, cobertura, custo e segurança. O executor não deve ser incorporado ao núcleo proposal-only.
