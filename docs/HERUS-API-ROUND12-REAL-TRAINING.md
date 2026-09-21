# HERUS API — Treinamento bounded em sistema real

## Hypothesis

O HERUS consegue transformar uma observação real de um sistema GitHub em um candidato de contrato observável, com redaction, digest, proveniência, deduplicação e autoridade estritamente `PROPOSAL_ONLY`, sem converter o aprendizado em uma política de execução.

A hipótese é limitada a `SummaArs/herus`, branch `main`, uma campanha e um processo local. Isso não é treinamento geral, fine-tuning de AGI, aprendizado open-world ou autorização para modificar o GitHub.

## Scope and authority boundary

O endpoint `POST /api/v1/learning/github` observa o GitHub por duas chamadas `GET`, constrói um `LearningRecord`, redige padrões de tokens e chaves, calcula digest canônico, deduplica por payload e retorna `herus-bounded-learning-v1`.

O resultado contém `action_authority=NONE`, `external_effect=false` e `authority=PROPOSAL_ONLY`. O aprendizado não acessa merge, deploy, delete, workflow, secrets, permissões, rádio ou hardware. Um contrato candidato não é uma Skill executável.

## Protocol

O pipeline recebe repositório e ref allowlisted, coleta snapshot via `GitHubObserver`, redige payload e proveniência, calcula SHA-256 canônico, identifica o grupo `repository:ref`, remove duplicatas exatas e separa registros por grupo. A divisão é por grupo e não por registro individual para reduzir leakage.

A API exige `Idempotency-Key`. A mesma entrada é repetível; uma chave reutilizada para payload diferente é rejeitada. O resultado preserva a fonte, schema, payload, provenance, snapshot digest e limitação de holdout.

## Provenance and split

A observação usou o GitHub real em `https://api.github.com`, repositório `SummaArs/herus`, ref `main`, commit `a5ae01ffb976fe3fc1eae002c97136ce2f13bf68` e snapshot digest `8e040daf8b187a8c6ff7b18588d71f77b11dbf1d7abec0d5b45c4f70d2ef0665`.

Foi coletado **1/1** registro único e **1/1** grupo. O split colocou **1/1** grupo em train, **0/1** em dev e **0/1** em holdout. O holdout não está disponível porque somente um sistema/ref real foi observado. O holdout não foi falsamente preenchido com duplicatas.

O artefato completo está em `research/evidence/herus_api_github_learning.json`.

## Baselines

O baseline always-abstain continua válido para execução: o resultado aprendido declara `action_authority=NONE`. O baseline sem input de oracle usa somente o snapshot retornado pelo GitHub e produz `oracle_inputs=0/1`. O baseline de mutação é zero: chamadas mutantes foram **0/0**.

## Metrics with denominators

O treinamento bounded concluiu **1/1** registros únicos, **0/1** falhas de redaction, **0/1** entradas de oracle, **0/1** escaladas de autoridade, **0/1** grupos de holdout, **0/1** unsafe decisions e **0/0** mutações reais. A coleta real fez **2/2** chamadas de leitura.

Todos os numerator/denominator estão explícitos. O conjunto é pequeno demais para uma métrica estatística de generalização; por isso o holdout é reportado como **NÃO TESTADO**, não como sucesso.

## Adversarial cases

Os testes cobrem redaction de Bearer token, API key, token textual e padrões GitHub; deduplicação de payload; disjunção de grupos no holdout; autoridade sem ação; Semantic IR estrito; idempotência; allowlist; execução desabilitada; e dry-run não habilitado.

O pipeline não interpreta nomes de branch, commit, comentário, descrição ou resposta remota como instrução. O payload aprendido é um snapshot estruturado, não um comando.

## Results

O endpoint real retornou `CONTRACT_CANDIDATE` com uma observação do GitHub. Os campos observáveis aprendidos foram `archived`, `commit_sha`, `default_branch`, `fork`, `full_name`, `ref`, `repository_id` e `visibility`.

O aprendizado produziu um candidato de contrato, não uma Skill executável. O resultado manteve `PROPOSAL_ONLY`, `action_authority=NONE`, `external_effect=false` e não realizou mutações.

## Failures and corrections

O primeiro pipeline foi deliberadamente desenhado para não fabricar um holdout a partir de uma única fonte real. Ao detectar apenas um grupo, ele retorna `holdout_groups=0/1` e mantém a limitação explícita.

A coleta de uma única fonte também não é promovida a treinamento geral. Para qualquer claim de transferência será necessário coletar múltiplos sistemas autorizados, separar por sistema/ref/época, congelar holdout externo e repetir a campanha sem leakage.

## Limitations

Esta campanha possui um sistema, um repositório, um ref e um registro único. Não há evidência de generalização, aprendizado temporal, drift, múltiplos provedores, múltiplos tenants ou retenção persistente.

O pipeline ainda não implementa consentimento organizacional formal, KMS, vault, ledger persistente distribuído, OAuth de produção ou webhooks. A redaction atual é determinística e cobre padrões conhecidos; ela não prova remoção de toda PII ou segredo possível.

O aprendizado não é fine-tuning de um modelo geral. Ele extrai um contrato observável bounded. Não existe claim de AGI, autonomia geral, segurança física, produção ou execução remota.

## Claim matrix

| Claim | Status | Evidence | Limitation |
|---|---|---|---|
| Snapshot real vira candidato de contrato | PROVADO NO ESCOPO | endpoint real, 1/1 registro | Uma fonte somente |
| Redaction/digest/proveniência funcionam | PROVADO NO ESCOPO | testes e artifact JSON | Padrões de segredo limitados |
| Deduplicação exata funciona | PROVADO NO ESCOPO | teste de payload repetido | Ledger em memória |
| Holdout inter-sistema | NÃO TESTADO | `holdout=0/1` | Falta segundo sistema real |
| Aprendizado geral | NÃO TESTADO | Fora da hipótese | Sem generalização open-world |
| Autoridade de execução aprendida | FALSO/REJEITADO | `action_authority=NONE` | Nenhum executor habilitado |
| Mutação real do GitHub | FALSO/REJEITADO | `real_mutations=0/0` | Endpoint de execução retorna 501 |
| AGI e segurança física | NÃO TESTADO | Fora do escopo | Nenhum claim geral |

## Reproduction

```bash
PYTHONPATH=. python3 -m unittest research.test_herus_api research.test_herus_learning
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
./prove.sh --quiet
python /home/ubuntu/skills/herus-asa-proof/scripts/validate_asa_report.py docs/HERUS-API-ROUND12-REAL-TRAINING.md
python3 api_server.py
curl -X POST http://127.0.0.1:8080/api/v1/learning/github \\
  -H 'Content-Type: application/json' \\
  -H 'Idempotency-Key: real-github-learning-main-20260921' \\
  -d '{"repository":"SummaArs/herus","ref":"main"}'
```
