# Arquivo histórico do HERUS

Este diretório preserva linhas de pesquisa, claims revogadas, protótipos e decisões que não representam necessariamente o objeto atual. Nada é apagado para esconder resultados negativos ou mudanças de tese.

## Regra de leitura

- `active`: parte do objetivo atual do algoritmo HERUS Symbiotic;
- `historical`: implementação ou documentação preservada para contexto;
- `exploratory`: experimento útil, mas não validado como claim;
- `revoked`: hipótese ou linguagem explicitamente retirada;
- `infrastructure`: firmware, simuladores e contratos que permanecem reutilizáveis.

## Inventário inicial

| Área | Estado | Onde consultar |
|---|---|---|
| Comunicador semântico, rádio e interação | historical/infrastructure | `docs/00-HERUS-MASTER.md` até `docs/47-*` e `firmware/` |
| ASA e Symbiont v2 | historical/research | `docs/48-*` até `docs/91-*`, `research/symbiont_v2/` |
| Benchmarks MIntRec/MInDS-14 | exploratory/evidence | `docs/92-*` até `docs/125-*`, `research/evidence/` |
| Conformal e assurance | infrastructure/limited-claim | `docs/125-PREDICAO-CONFORMAL-E-ASSURANCE.md` |
| Gates de identidade, causalidade e ledger | active/assurance | `docs/127-*` até `docs/133-*`, `research/*gate*`, `research/paired_ledger.py` |
| Crise de objeto e reestruturação | active/decision record | `docs/134-SINTESE-CRISE-E-REESTRUTURACAO.md` |
| Object Lock | active/scientific contract | `object-lock/` |

## Claims revogadas, não apagadas

As seguintes frases podem aparecer em documentos históricos, mas não são claims atuais:

- HERUS é SOTA geral;
- HERUS supera todos os transformers;
- HERUS é AGI ou simbiose geral;
- conformal prediction garante ações no mundo;
- precisão seletiva é acurácia geral;
- testes de firmware provam eficácia do algoritmo;
- bootstrap ou seeds provam causalidade.

## Princípio

O repertório histórico continua valioso porque registra decisões, erros, negativos e componentes que podem ser reutilizados. A precedência, porém, é:

1. Object Lock e contratos atuais;
2. evidência versionada;
3. testes reproduzíveis;
4. documentação histórica, quando não conflitante.
