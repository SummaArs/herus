# HERUS API v1

A API HERUS v1 fornece uma fronteira HTTP para **propostas declarativas** e observação read-only do repositório GitHub allowlisted `SummaArs/herus`. O modo padrão é `proposal-only`; não existe executor remoto habilitado.

## Executar localmente

```bash
python3 api_server.py
```

A API ficará disponível em `http://localhost:8080`. A especificação OpenAPI é exposta em `/openapi.json` e a documentação interativa em `/docs`.

## Health

```bash
curl http://localhost:8080/api/v1/health
```

A resposta declara `mode=proposal-only`.

## Criar proposta

Toda criação exige `Idempotency-Key`. O Semantic IR é validado com tipos estritos, campos fechados, `authority=PROPOSAL_ONLY` e `confidencePct >= runnerUpPct`.

```bash
curl -X POST http://localhost:8080/api/v1/proposals \
  -H 'Content-Type: application/json' \
  -H 'Idempotency-Key: example-proposal-1' \
  -d '{
    "ir": {
      "schemaVersion": 1,
      "eventKind": "HELP",
      "source": "TEXT",
      "confidencePct": 90,
      "runnerUpPct": 10,
      "slots": {"minutes": null},
      "evidence": [{"kind":"OBSERVATION","ref":"demo-1","polarity":"POSITIVE","weight":90}],
      "hypothesisStatus": "TRUE",
      "authority": "PROPOSAL_ONLY"
    }
  }'
```

Repetir a mesma chave e payload devolve a resposta registrada com `Idempotency-Replayed: true`; reutilizar a chave com payload diferente é rejeitado.

## Observação real do GitHub

O endpoint `/api/v1/connectors/github/observe` faz somente `GET` para o repositório `SummaArs/herus` e a ref solicitada. É obrigatório fornecer `Idempotency-Key`. Se `HERUS_GITHUB_TOKEN` estiver configurado, ele será usado apenas como bearer token para leitura; sem token, a API pública pode ser consultada conforme os limites do GitHub.

```bash
curl -X POST http://localhost:8080/api/v1/connectors/github/observe \
  -H 'Content-Type: application/json' \
  -H 'Idempotency-Key: github-observation-main-1' \
  -d '{"repository":"SummaArs/herus","ref":"main"}'
```

A resposta contém commit SHA, branch padrão, visibilidade, estado de fork/arquivamento e `snapshot_digest`. O conteúdo recebido é tratado como observação, não como instrução.

## Limites atuais

O endpoint `/api/v1/executions` está permanentemente desabilitado nesta fase. `/api/v1/executions/dry-run` também retorna `501` até que exista um adaptador com transport instrumentado que prove zero chamadas mutantes. Não são permitidos merge, deploy, delete, alteração de branch, workflow, secrets, permissões, segurança ou qualquer operação física.

A API ainda não implementa OAuth completo, webhooks, ledger persistente distribuído, tenant isolation de produção, KMS/secret vault ou rollback remoto. Portanto, esta versão é adequada para desenvolvimento local e observação controlada; não deve ser exposta publicamente nem tratada como autorização de produção.

## Verificação

```bash
PYTHONPATH=. python3 -m unittest research.test_herus_api
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
```
