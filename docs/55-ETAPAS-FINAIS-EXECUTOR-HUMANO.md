# Etapas finais — execução externa, holdout estendido e prova humana

## O que foi implementado

O HERUS agora possui um adaptador experimental de execução externa em `research/stage4/` com `ExecutionEnvelope`, `ExecutionReceipt`, `RecoveryAttestation`, `ExternalExecutorAdapter` e `AppendOnlyLedger`.

O adaptador exige autoridade externa, rejeita replay de nonce, verifica TTL e época de revogação, registra recibos por passo e distingue:

- `REJECTED`;
- `COMMITTED`;
- `FAILED_PARTIAL`;
- `UNKNOWN_OUTCOME`;
- `RECOVERY_REQUIRED`;
- `RECOVERY_RESOLVED`.

Falha parcial e ausência de ACK não são convertidas em sucesso nem em “zero efeitos”. A recuperação precisa vir de uma fonte diferente do executor original. Não existe retry automático.

## Holdout estendido

`research/extended_holdout.py` reserva famílias para:

- deriva de época e evidência stale;
- pré-condições e ordem não comutativa;
- recursos consumíveis e piso de recursos.

Os casos são repetidos três vezes, permanecem `UNSUPPORTED_BY_CONTRACT` e não executam efeitos. Eles não são apresentados como aprovação: o resultado continua `not_proven` até existirem contratos públicos completos, oráculo independente e executor autorizado.

## Protocolo de utilidade humana

Foi preparado `research/social/utility_protocol_v1.json` para uma futura tarefa pequena, reversível e não médica: seleção e confirmação local de respostas comunicacionais preparadas.

As condições previstas são:

- **CB:** baseline convencional;
- **NS:** condição sem simbionte;
- **H:** HERUS com decisão checked.

As métricas principais serão erro da tarefa, tempo/esforço e dependência de host/interface. O protocolo inclui consentimento, retirada, parada independente, minimização de dados, retenção limitada, acessibilidade e registro de intervenção do operador.

O protocolo está explicitamente em `protocol_ready_no_human_data`. Nenhum participante, resultado ou benefício foi inventado. A coleta real exige revisão externa, consentimento e operação fora deste sandbox.

## Resultado atual

Os 142 testes host-only passam, mas isso não transforma o sistema em simbiose útil. O estado continua limitado por dois fatos:

1. o holdout histórico preserva o contraexemplo de efeito oculto no caminho legado;
2. a prova humana ainda não foi coletada.

A etapa mecânica máxima possível no repositório foi implementada sem conceder autoridade ao runtime. A próxima ação legítima fora do código é revisão do protocolo e execução humana controlada, não uma alegação de sucesso.

## O que permanece não demonstrado

Ainda não há prova de segurança física, eficácia clínica, privacidade de produto, acessibilidade populacional, consciência, inteligência geral, utilidade social ampla ou equivalência ao impacto de um smartphone. Mesmo uma campanha humana positiva seria válida apenas para a tarefa, população, interfaces e condições pré-registradas.
