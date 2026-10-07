# HERUS Symbiotic

**HERUS Symbiotic é um algoritmo de inteligência artificial em evolução.**

O projeto investiga como um sistema pode aprender uma competência a partir de observações, adaptar sua representação ao hospedeiro, otimizar decisões sob restrições e preservar contratos de evidência, risco, custo e autoridade.

> **Estado atual:** `research / algorithm-v2 / not-SOTA / proposal-only`

O HERUS ainda não provou superioridade geral, simbiose geral, AGI, causalidade, transferência universal ou segurança de produção. Essas são direções de pesquisa, não claims atuais.

## Objetivo atual

Construir e validar uma nova família de algoritmos — **Symbiotic Learning** — com:

- representação adaptável ao hospedeiro;
- memória episódica e evidência positiva/negativa;
- função de decisão condicionada a contexto, estado e restrições;
- função de otimização de utilidade sob risco, custo e autoridade;
- atualização limitada, reversível e auditável;
- abstenção como ação válida;
- explicação determinística de cada proposta;
- comparação rigorosa contra algoritmos existentes.

A definição evolui com a pesquisa. O núcleo atual é a semente verificável; não é o limite final do HERUS.

## Comece aqui

| Ordem | Documento | Finalidade |
|---:|---|---|
| 1 | [Repertório do algoritmo](docs/135-REPERTORIO-ALGORITMO-HERUS-SYMBIOTIC.md) | Mapa open source das famílias de IA e do lugar do HERUS |
| 2 | [Algoritmo Symbiotic Learning](docs/84-ALGORITMO-SYMBIOTIC-LEARNING.md) | Definição, mecanismo v2 e falsificação |
| 3 | [Função objetivo e atualização](docs/136-FUNCAO-OBJETIVO-E-ATUALIZACAO-HERUS.md) | Matemática que orienta a evolução do algoritmo |
| 4 | [Linha do tempo](docs/01-LINHA-DO-TEMPO.md) | Evolução completa, incluindo as mudanças de tese |
| 5 | [Mapa do código](docs/03-MAPA-DO-CODIGO.md) | Onde cada parte vive |
| 6 | [Mapa da evidência](docs/04-MAPA-DA-EVIDENCIA.md) | O que foi medido e o que permanece bloqueado |
| 7 | [Object Lock](object-lock/v1.yaml) | Contrato científico antes de qualquer benchmark |
| 8 | [Roadmap e gates](docs/06-ROADMAP-E-GATES.md) | Sequência legítima de validação |

## Como importar

A superfície pública é didática e proposal-only:

```python
from herus_symbiotic import Herus

herus = Herus()
print(herus.inspect())
```

O núcleo de pesquisa pode ser importado diretamente:

```python
from symbiotic_learning import SymbioticLearner

learner = SymbioticLearner()
print(learner.export())
```

Importar, observar ou propor não executa ações externas, acessa rede ou concede autoridade.

## Repertório científico

O repositório mantém implementações, baselines e análises de várias famílias:

- aprendizado supervisionado;
- aprendizado não supervisionado;
- auto-supervisão;
- bandits e reinforcement learning proxy;
- memória episódica e lookup;
- transformers e encoders multilíngues;
- redes tensoriais Tensor-Train;
- calibração seletiva e conformal prediction;
- análise causal e bootstrap agrupado;
- síntese enumerativa e programação proposal-only;
- contratos fail-closed, proveniência e ledger de evidência.

Essas famílias são **controles, componentes ou linhas de comparação**. Nenhuma delas é automaticamente atribuída ao HERUS-core.

## O que já foi construído

- `research/symbiotic_learning.py`: núcleo importável de indução contextual bounded;
- `herus_symbiotic/`: fachada pública para iniciantes e pesquisadores;
- memória de episódios, hipóteses, IDs de evidência e explicações determinísticas;
- snapshot/rollback e abstention;
- benchmarks reais em MIntRec e MInDS-14;
- comparação com Naive Bayes, SVM, Random Forest, regressão, k-NN, transformers e Tensor-Train;
- conformal prediction, risco–cobertura e análise de incerteza;
- ledger pareado por `example_id`, seeds e modelos;
- amostragem causal agrupada pré-registrada;
- proveniência local e gates fail-closed;
- firmware, simulador e contratos de hardware preservados como infraestrutura histórica e futura.

Os resultados negativos são parte do repertório. Até agora, nenhum resultado autoriza dizer que HERUS é SOTA geral.

## Organização do repositório

```text
herus_symbiotic/       # fachada pública importável
research/               # núcleo, baselines, benchmarks e análises
object-lock/            # contrato científico do objeto em avaliação
docs/                   # especificações, papers e decisões
archive/                # histórico preservado e claims revogadas
firmware/               # host físico e contratos C, fora do foco atual
```

## Regra científica

O HERUS deve evoluir até a definição de algoritmo, mas cada evolução precisa deixar claro:

1. qual é a entrada;
2. qual é a função de decisão;
3. qual é a função de otimização;
4. qual é o estado/memória;
5. qual é o erro ou custo;
6. contra qual baseline foi comparado;
7. qual evidência permite ou bloqueia a claim.

Não usamos uma porcentagem administrativa como prova de progresso. O algoritmo só avança quando uma hipótese reproduzível sobrevive a dados reais, baselines fortes e testes adversariais.

## Verificação

```bash
./prove.sh --quiet
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
python3 object-lock/validate_lock.py
python3 tools/provenance_audit.py --strict research/software_provenance_manifest.json
```

## Licença e colaboração

Consulte [CONTRIBUTING.md](CONTRIBUTING.md), [SECURITY.md](SECURITY.md) e o [índice documental](docs/README.md). O histórico não é apagado quando uma hipótese é revogada; ele é arquivado, datado e rotulado.
