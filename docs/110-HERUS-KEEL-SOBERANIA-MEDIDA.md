# HERUS — Keel: soberania medida, não prometida

**Prova:** `make -C firmware hsca-keel`

## 1. Por que existe

[64](64-HERUS-SOBERANIA-ON-WRIST-E-FRONTEIRA-CORE.md) enuncia a regra de produto: **o vestível é o cérebro; o Core é estação de apoio — carregador, antena e alimentador autorizado de conhecimento.** Uma regra que só vive num documento é uma preferência. A Keel a transforma em falha de build.

## 2. Orçamento medido

Cada módulo declara sua pegada como o `sizeof` da estrutura que realmente existe. O total é medido, não estimado; um módulo que cresce muda a tabela sozinho e o portão percebe.

| Módulo | RAM de estado | Cognitivo | Exige Core |
|---|---:|:---:|:---:|
| `hir` | 42 B | sim | não |
| `herald` | 72 B | sim | não |
| `ladder` | 34 B | sim | não |
| `drift` | 556 B | sim | não |
| `aura` | 324 B | sim | não |
| **total** | **1.028 B** | | |

O orçamento permitido é 65.536 B; a SRAM interna do ESP32-S3 é 524.288 B. **Toda a via cognitiva ocupa 0,2% do silício.** A suíte assere que o total cabe no orçamento e que fica abaixo de 2% da SRAM.

Isso é estado de runtime, não tamanho de código, e é o que se pretende que seja: a inteligência do HERUS é pequena porque a representação é pequena.

## 3. Matriz de papéis do Core

Produto cartesiano completo, 3 papéis × 8 ações. **4 permitidas, 20 recusadas.**

| | carregar | retransmitir cifra | uplink sat | propor conhecimento | executar | confirmar | ler memória | escrever memória |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| **energia** | ✔ | — | — | — | — | — | — | — |
| **antena** | — | ✔ | ✔ | — | — | — | — | — |
| **conhecimento** | — | — | — | ✔ | — | — | — | — |

Executar, confirmar, ler memória pessoal e escrever memória pessoal **não estão na lista, para papel nenhum, nunca** (WRIST-02/10). Papel desconhecido não pode nada; ação desconhecida é recusada em vez de assumir um padrão. Um papel não vaza para a ação do papel vizinho.

## 4. Portão de conhecimento

Tabela-verdade exaustiva de cinco condições: **1 de 32 combinações é admissível.**

```
produtor conhecido  ∧  versão de registry compatível  ∧  digest confere
∧  namespace NÃO pessoal  ∧  confirmação local
```

- **WRIST-03/06** — identidade, versão e digest, ou o pacote não entra.
- **WRIST-05** — pacote externo nunca aterrissa no namespace pessoal.
- **WRIST-04** — sem confirmação local, continua sendo proposta.

## 5. Ausência do Core

`keel_core_absent_complete()` prova WRIST-01/08: nenhum módulo cognitivo exige o Core presente. A suíte composta [`test_hsca_finale.c`](../firmware/core/test_hsca_finale.c) executa a cadeia inteira — fala, significado, confirmação, escolha de portador, custódia, chegada, renderização — **com o Core desligado**, e isola a fronteira do Core da fronteira de linha de visada: céu limpo com o Core desligado continua sem uplink, e devolver o Core é a única coisa que traz o degrau de volta.

## 6. O que isto não é

- Não é medição de energia, de tempo de execução ou de tamanho de flash. É orçamento de estado.
- Não é a migração de identidade para handles collision-aware, que continua pendente conforme [64 §5](64-HERUS-SOBERANIA-ON-WRIST-E-FRONTEIRA-CORE.md).
- Não prova que o Core real obedece: prova que o contrato local recusa. O portão físico e o `knowledge_feed` continuam sendo onde isso encosta no mundo.
