# Proveniência local de build — inventário limitado, digests canônicos e gates pendentes

**Passo 5 pré-hardware · HERUS-T13-001 · controle local unsigned, não atestação de release**

> O HERUS agora pode recusar uma alteração não refletida nos insumos locais declarados. Isso não prova quem alterou o manifesto, quem construiu um artefato, qual ambiente executou a build ou se um binário entregue ao usuário corresponde a esta árvore.

Este passo reduz uma lacuna específica de TM-09: antes dele, o repositório não possuía um inventário verificável nem uma referência de integridade para os insumos que executam a prova host. Agora, `research/software_provenance_manifest.json` declara esses insumos e `tools/provenance_audit.py` os valida com Python 3 stdlib. O mecanismo é propositalmente pequeno, offline e fail-closed. Ele não carrega áudio, transcrição, embedding, identidade, localização, chave, token, credencial, conteúdo de produto, artefato de memória, rádio ou rede.

O NIST SSDF trata proteção de componentes e coleta/compartilhamento de proveniência de release como práticas de desenvolvimento seguro, mas enfatiza que a adoção deve ser baseada em risco e resultado, não um checklist mágico [1]. A especificação SLSA distingue a simples existência de proveniência da sua autenticidade e da resistência à falsificação; artefato identificado por digest não equivale a atestação assinada do builder [2]. A CISA descreve SBOM como inventário detalhado de componentes para transparência e análise de risco; sua página de elementos mínimos de 2025 registra um documento em consulta pública, não uma certificação de cobertura [3]. O HERUS aplica apenas a parte que pode provar localmente: inventário direto declarado, formato estrito e detecção de divergência contra uma referência revisada.

| Artefato | Responsabilidade | Deliberadamente não faz |
|---|---|---|
| `research/software_provenance_manifest.json` | Declara insumos protegidos, componentes diretos, estado de confiança e gates futuros. | Assinar, atestar, fixar um commit, enumerar dependências transitivas, declarar CVEs ou aprovar release. |
| `tools/provenance_audit.py` | Valida schema, campos proibidos, paths, IDs, estados, gates e SHA-256 de arquivo/árvore. | Acessar rede, GitHub, segredo, token, chave, lockfile, builder, binário, firmware alvo ou hardware. |
| `tools/test_provenance_audit.py` | Falsifica alteração, escape de path, token, duplicidade, formato e escalada de confiança. | Provar que o manifesto não foi alterado junto com os arquivos, ou que uma plataforma CI é confiável. |
| `prove.sh` | Executa a 26ª suíte da sequência global e expõe duas invariantes de proveniência. | Converter o resultado em assinatura, SLSA ou aprovação de segurança física. |
| `threat_model.[ch]` | Exige flag de integridade local para TM-09, mas conserva classificação `PENDING_TARGET`. | Emitir `MITIGATED_HOST` para supply chain ou estimar risco residual. |

## 1. Contrato de confiança

O estado obrigatório do manifesto é `local_unattested`. Qualquer outro estado — inclusive `slsa_build_l2`, `signed`, `trusted`, `reproducible` ou campo não conhecido — é recusado. O auditor também exige as quatro declarações abaixo, porque omitir seus limites seria uma alegação implícita.

| Declaração | Valor obrigatório agora | Leitura correta |
|---|---|---|
| `external_attestation` | `pending` | Não existe assinatura verificável nem identidade autenticada de builder. |
| `sbom_coverage` | `declared_direct_inputs_only` | O arquivo é um inventário direto e incompleto; não cobre transitividade, componentes resolvidos de artefato ou vulnerabilidades. |
| `reproducible_build` | `not_claimed` | Não houve rebuild independente nem comparação de digest de artefato alvo. |
| `build_isolation` | `not_claimed` | Não há evidência de ambiente efêmero, cache isolado, segredo protegido ou ausência de influência externa. |

A SLSA requer que uma proveniência existente identifique inequivocamente o pacote por digest e descreva sua produção; para autenticidade, o consumidor deve conseguir verificar uma atestação assinada [2]. O manifesto HERUS não satisfaz esses requisitos: ele é um arquivo versionado e unsigned que pode, em princípio, ser alterado junto com os hashes. Portanto ele só detecta **drift de insumo contra a referência que o revisor escolheu confiar naquele checkout**. Essa limitação é estrutural, não uma pendência editorial.

## 2. Insumos, algoritmo e inventário mínimo

O manifest protege a árvore `firmware/`, a árvore `tools/`, `prove.sh`, o workflow de prova e o manifesto de readiness físico. Para arquivo, o digest é SHA-256 dos bytes. Para árvore, o auditor percorre nomes relativos normalizados e bytes de cada arquivo regular em ordem lexicográfica; cada nome UTF-8 e conteúdo são separados por byte NUL. `build/`, `__pycache__/` e `.git/` são excluídos porque são produtos efêmeros, não fontes. Symlinks e arquivos especiais não são seguidos nem representados.

| Classe declarada | Exemplos no manifesto | Nível de integridade declarado |
|---|---|---|
| Fonte/prova local | `firmware/`, `tools/`, `prove.sh` | `local_digest_declared`: hash com referência local, sem assinatura. |
| Ambiente de execução | compilador C11, `make`, Python 3 stdlib | `environment_unattested`: necessário ao comando, mas sem versão fixada ou cadeia autenticada. |
| Ações de CI | `actions/checkout` e `actions/upload-artifact` por SHA de commit | `revision_pinned`: reduz deriva de tag; não autentica checkout, builder, artefato ou infraestrutura de CI. |
| Gate físico | `hardware_readiness_manifest.json` | Referência de declaração, não evidência de silício. |

O auditor não aceita caminhos absolutos, `..`, path não normalizado, digest que não seja SHA-256 em minúsculas, tipo de input fora de `file`/`tree`, componente duplicado, componente sem versão/fonte, `direct` não booleano, gate ativo sem arquivo de evidência ou gate pendente contendo evidência física. Ele rejeita também chaves de metadados como `token`, `secret`, `password`, `private_key`, `credential`, `key`, `pair_key`, `audio`, `transcript`, `embedding`, `identity`, `location` e `message_content`.

> O hash deliberadamente não inclui o próprio manifesto. Auto-hash não criaria uma âncora independente: introduziria circularidade ou exigiria excluir algum campo. A âncora real deverá ser uma atestação externa assinada sobre o manifesto e o artefato de release.

## 3. Contraprovas

T13 usa uma fixture temporária, sem rede e sem dados de produto. Cada caso precisa produzir erro observável para a prova passar.

| Contraprova | Recusa exigida |
|---|---|
| Arquivo ou árvore protegida alterada | `digest mismatch`; nenhuma normalização do conteúdo é aceita. |
| Cache `build/` dentro da árvore | Excluído de propósito; produto efêmero não é apresentado como fonte. |
| Mesmo ID de componente duas vezes | `duplicate component id`; não há substituição silenciosa. |
| Campo `token` | Campo não suportado e sensível; nenhum segredo pode entrar no inventário. |
| `trust_state = slsa_build_l2` | Rejeitado; o manifesto não pode autoelevar sua confiança. |
| Caminho `../outside.txt` | Rejeitado antes de leitura; nenhum arquivo externo vira insumo confiado. |
| Gate ativo com evidência ausente | Rejeitado; status ativo exige arquivo versionado existente. |
| Tipo de input não suportado | Rejeitado; não há fallback para blob, URL ou backend. |

No modelo de ameaças, `supply_chain_local_integrity=1` comprova somente que esse controle local foi executado. TM-09 ainda retorna `PENDING_TARGET` com `THREAT_MODEL_FAIL_TARGET_PENDING`. Se a flag estiver ausente, também retorna `THREAT_MODEL_FAIL_SUPPLY_INTEGRITY`. Assim, o próprio classificador impede que a nova suíte seja transformada em “supply chain mitigada”.

## 4. Gates para evidência posterior

A plataforma é aberta: os gates não pressupõem ESP32-S3, GitHub Actions, fornecedor de CI, secure element ou sistema operacional. Eles devem ser satisfeitos por evidência adequada ao release e ao alvo escolhidos.

| Gate pendente | Evidência mínima futura | O que permitiria dizer — e o que ainda não |
|---|---|---|
| `signed-build-provenance` | Atestação assinada, digest de artefato, fronteira do builder e procedimento de verificação. | Permite avaliar uma proveniência autenticada daquela release; não torna código livre de vulnerabilidades. |
| `complete-component-inventory` | Inventário de artefato, método de cobertura, identidades de componentes e revisão. | Permite declarar a cobertura documentada; não garante ausência de CVE ou de componente oculto. |
| `reproducible-target-build` | Toolchain pinada, receita, artefato alvo e digest comparado em ambientes independentes. | Permite declarar repetição observada naquele escopo; não prova isolamento universal ou hardware íntegro. |
| Integridade de plataforma | Secure boot, debug, raiz, storage, cortes controlados e evidências de alvo. | Continua em gates físicos próprios; proveniência de software não substitui proteção de silício. |

## 5. Reprodução

```bash
python3 tools/provenance_audit.py research/software_provenance_manifest.json --strict
python3 tools/test_provenance_audit.py
./prove.sh --quiet
```

O pipeline passa a executar **30 suítes**, **69 invariantes de prova** e mantém **74 invariantes de sistema simulado**. O resultado positivo afirma apenas que o inventário local atual é canônico, os insumos declarados batem com seus digests e os ataques de fixture são recusados. Não afirma que o checkout é autêntico, que o manifest não foi adulterado, que dependências estão completas, que a CI está isolada, que há assinaturas, que o release é reprodutível, que o firmware alvo deriva desta árvore, ou que hardware e supply chain física são confiáveis.

## Referências

[1] National Institute of Standards and Technology, *Secure Software Development Framework (SSDF) / SP 800-218 v1.1*. [Página oficial](https://csrc.nist.gov/projects/ssdf).

[2] SLSA, *Build: Requirements for producing artifacts, v1.2*. [Especificação oficial](https://slsa.dev/spec/v1.2/build-requirements).

[3] Cybersecurity and Infrastructure Security Agency, *2025 Minimum Elements for a Software Bill of Materials (SBOM)*. [Página oficial](https://www.cisa.gov/resources-tools/resources/2025-minimum-elements-software-bill-materials-sbom).
