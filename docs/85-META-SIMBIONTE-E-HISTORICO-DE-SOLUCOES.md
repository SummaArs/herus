# Meta-Simbionte — aprendizado de como adaptar

**Status:** protocolo de pesquisa v1, importável e host-only.

## A ideia

O Symbiotic Learning aprende uma competência transferível. O **Meta-Symbiotic Learning** aprende também quais referências e estratégias ajudam a resolver um novo problema.

A diferença é:

```text
aprendizado comum:
  problema → solução

Symbiotic Learning:
  hospedeiro + observações → Skill transferível

Meta-Symbiotic Learning:
  histórico de soluções verificadas + problema novo
  → estratégia de investigação + nova proposta personalizada
```

O histórico funciona como um **Norte**, não como uma resposta automática. Uma solução antiga nunca é copiada diretamente para um hospedeiro novo.

## Contrato de segurança

Uma referência histórica só pode:

- ser lida;
- influenciar a escolha de estratégia de sondagem;
- ordenar candidatos observados por contexto e compatibilidade histórica;
- sugerir uma hipótese de correspondência;
- aparecer no trace para auditoria.

Ela não pode:

- conceder autoridade;
- executar uma ação;
- substituir observação nova;
- transportar contexto privado de outro hospedeiro;
- ignorar risco, orçamento, deriva ou ambiguidade;
- ser promovida se não tiver prova marcada como `verified`.

A memória é append-only nesta versão. Não há edição destrutiva nem exclusão silenciosa de histórico.

## Ciclo meta-simbiótico

```text
problema novo
    ↓
consultar referências compatíveis
    ↓
escolher estratégia de exploração
    ↓
sondar o hospedeiro atual
    ↓
comparar efeito, contexto e risco
    ↓
propor ou abster-se
    ↓
verificação externa
    ↓
adicionar solução ao histórico somente se verificada
```

A regra mais importante é:

> **Histórico pode orientar a busca; somente evidência fresca pode sustentar a proposta.**

O ranking meta-simbiótico não remove candidatos. Ele apenas altera a ordem de investigação. O verificador continua recebendo o conjunto completo e pode abster-se por alias, deriva, orçamento ou risco.

## Personalização

A personalização não significa alterar a constituição do HERUS para cada usuário. Ela significa adaptar a solução ao problema atual usando:

- objetivo observável;
- contexto local;
- tipo de hospedeiro;
- risco permitido;
- orçamento;
- referências verificadas anteriores.

O núcleo de segurança permanece fixo. O repertório de hipóteses pode crescer, ser versionado e ser revertido.

## Biblioteca

`research/meta_symbiotic_learning.py` expõe:

- `Problem` — objetivo, contexto, hospedeiro e risco;
- `VerifiedSolution` — registro imutável de solução comprovada;
- `MetaProposal` — proposta com estratégia, referência e evidência fresca;
- `MetaSymbioticLearner` — histórico, recuperação, adaptação e exportação.

O benchmark `research/meta_symbiotic_benchmark.py` mede se uma referência verificada coloca o candidato correto mais cedo sem reduzir o conjunto de candidatos, sem dispensar evidência fresca e sem conceder autoridade. A versão atual controla o contexto e o custo dos candidatos: sem histórico o alvo fica em posição 3; com referência do efeito compatível, sobe para posição 1. Assim o ganho atribuído ao histórico não é apenas um efeito de contexto.

Exemplo:

```python
from meta_symbiotic_learning import MetaSymbioticLearner, Problem
from symbiotic_learning import Episode

meta = MetaSymbioticLearner()
problem = Problem.from_maps({'mode': 1}, context={'channel': 1}, host_kind='host-b')
evidence = [Episode.from_maps({'mode': 0}, 'gesture_double', {'mode': 1}, context={'channel': 1})]
proposal = meta.adapt(problem, evidence)

if proposal.status == 'PROPOSE':
    print(proposal.action, proposal.strategy, proposal.fresh_evidence)
```

## O que isso ainda não prova

A biblioteca ainda não prova:

- autoaprendizado aberto;
- solução para qualquer problema;
- generalização fora do vocabulário observável;
- memória soberana em produção;
- benefício humano;
- execução segura;
- inteligência geral.

O próximo experimento deve medir se a memória verificada reduz sondagens, custo ou tempo **sem aumentar falsos aceites**, comparada a um learner sem histórico.

O estado científico correto é:

```text
meta-simbionte implementado
histórico verificável implementado
personalização bounded implementada
vantagem empírica ainda não demonstrada
```
