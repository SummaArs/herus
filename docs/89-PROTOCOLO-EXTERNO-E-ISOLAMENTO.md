# Protocolo externo e isolamento entre processos

**Status:** evidência host-only com processo separado; ainda não é um host físico ou de terceiro.

## Arquitetura testada

```text
processo learner
      │ JSONL
      │ stdin/stdout
      ▼
processo host externo
```

O host externo agora é o programa standalone `research/independent_host_process.py`, sem imports do learner. Ele mantém em seu próprio processo:

- mapa de ações;
- estado privado;
- estado público parcial;
- rotação de interface;
- evolução temporal.

O learner recebe somente mensagens serializadas.

## Resultado

```text
processo isolado: sim
primeira ação encontrada: target_x
após rotação: target_new
mensagem corrompida: rejeitada
processo sobrevive à corrupção: sim
evidência fresca: obrigatória
autoridade concedida: não
```

Evidência: [`research/evidence/external_host_protocol_v1.json`](../research/evidence/external_host_protocol_v1.json).

## Falhas testadas

- JSON inválido retorna `ERROR/invalid_json`;
- o host continua respondendo depois da mensagem corrompida;
- timeout mata o processo externo;
- pipes são fechados sem vazamento;
- rotação altera o espaço de ações;
- a proposta antiga não é reutilizada sem nova sondagem.

## Interpretação

Este é um avanço sobre o black-box anterior porque o host e o learner não compartilham objetos Python, classes ou memória no caminho operacional. A comunicação é o protocolo congelado [HERUS Host JSONL v1](90-PROTOCOLO-HERUS-HOST-JSONL-V1.md).

Ainda não é prova de independência científica: ambos os processos são gerados pelo mesmo repositório e executados na mesma máquina. A próxima validação deve usar um host implementado separadamente, ou pelo menos um protocolo congelado antes do learner.
