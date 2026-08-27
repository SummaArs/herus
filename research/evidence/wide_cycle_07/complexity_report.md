# Complexidade e decisão de mudança mínima — Wide Research 07

## Baseline

No commit-base havia 365 arquivos rastreados, 97 fontes C, 65 headers, 49 fontes Python, 18.259 linhas C de firmware, 5.283 linhas de headers de firmware, 4.065 linhas nos módulos Python de pesquisa e 68 alvos públicos de Make.

## Delta medido

| Item | Delta do working tree |
|---|---:|
| Código de runtime alterado | 0 linhas |
| Teste C Core-Link | +14 linhas |
| Makefile | +1 linha líquida; inclusão de 1 teste |
| Teste host de política | 50 linhas novas |
| Teste host de compatibilidade | 55 linhas novas |
| Imports novos nos testes | `json`, `re`, `unittest`, `pathlib` — todos stdlib |
| Dependências externas novas | 0 |
| Caminhos novos de autoridade | 0 |
| Arquivos de evidência | 19, todos texto/JSON/Markdown |

O teste Core-Link adiciona somente casos de wire truncado/estendido e versão futura. Não modifica `core_link.c`, o wire format ou a ponte de autoridade. Os testes host verificam os contratos publicados; não são bibliotecas de runtime e não introduzem fallback.

## Custo observado

A suíte de pesquisa com a política e os testes de compatibilidade passou com **101 testes** em aproximadamente **0,18 s** de tempo real no host. `make -C firmware all` passou em aproximadamente **30,15 s** de tempo real no host. Esses tempos são custos de CI/host, não latência ou energia do ESP32-S3.

## Autoridade e reversibilidade

A análise de imports não encontrou biblioteca externa nova. O diff não adicionou `link_send`, escrita de cofre, inserção de coleção, confirmação ou transporte aos testes host. A análise de contratos confirmou que Semantic IR, Core-Link e persistência continuam fechados/versionados onde necessário; a única extensão forward-compatible automática permanece a regra P4 do HCP.

## Decisão

A mudança mínima justificada é manter o patch de 14 linhas no teste C, a inclusão de dois testes host e os artefatos normativos/evidenciais. Não há evidência para alterar o núcleo, criar uma camada de abstração nova, adicionar mecanismo de migração inexistente ou introduzir uma implementação de raciocínio generativo. O princípio do relógio suíço foi operacionalizado por política e regressões, não por código ornamental.

A ausência de uma migração persistente entre versões concretas permanece uma limitação registrada. Antes de qualquer mudança futura de formato, deverá existir um ensaio específico com fixtures da versão anterior, migração autenticada, interrupção e rollback. A ausência desse ensaio bloqueia a promoção, em vez de ser mascarada como compatibilidade.
