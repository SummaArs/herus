# HERUS Host JSONL v1

Este documento congela o contrato mínimo entre o learner e um host externo.

## Transporte

- uma mensagem JSON por linha;
- uma resposta JSON por linha;
- nenhum estado compartilhado;
- stdout contém apenas respostas do protocolo;
- erro de parsing não encerra obrigatoriamente o host;
- timeout do cliente encerra a sessão fail-closed.

## Operações

### `ready`

Request:

```json
{"op":"ready"}
```

Response:

```json
{"status":"OK","protocol":"herus-host-jsonl-v1"}
```

### `describe`

Retorna apenas nomes públicos de ações:

```json
{"status":"OK","protocol":"herus-host-jsonl-v1","actions":["target_x","target_y"]}
```

### `probe`

Request:

```json
{"op":"probe","action":"target_x"}
```

Response:

```json
{"status":"OK","before":{"mode":0},"after":{"mode":1},"context":{"zone":1},"step":1,"risk":0}
```

### `reset` e `rotate`

São comandos de ensaio do fixture e não representam autoridade universal. Um host real poderá oferecer operações equivalentes com semântica própria.

### Erro

```json
{"status":"ERROR","reason":"invalid_json"}
```

## Garantias do contrato

O learner não pode presumir:

- que nomes de ações persistem;
- que o estado interno está disponível;
- que uma ação existe após a rotação;
- que um efeito antigo continua válido;
- que `PROPOSE` equivale a execução.

O host independente usado nesta versão importa apenas a biblioteca padrão. O learner importa o contrato de episódios separadamente e converte respostas de rede em evidência local.

## Status epistemológico

O protocolo está congelado apenas dentro deste repositório. A prova de interoperabilidade independente ainda requer uma implementação externa ao projeto.
