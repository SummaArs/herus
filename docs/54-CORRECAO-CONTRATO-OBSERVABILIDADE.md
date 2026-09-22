# Etapa 5 — contrato de observabilidade e decisão checked

**Status:** correção aditiva implementada; o caminho estrito bloqueia o contraexemplo de efeito oculto antes de qualquer probe.

## O problema

Na etapa 4, `m6a-opaque-01` mostrou que o mesmo delta público (`x += 1`) pode esconder um efeito privado proibido (`damage += 1`). O runtime antigo aprendia apenas `Effect.from_states()` e construía uma proposta. O oráculo independente rejeitava a proposta.

O problema não pode ser resolvido fazendo o runtime “descobrir” `damage`: esse campo não está na superfície pública. A correção correta é **não alegar fechamento onde a observação não o prova**.

## Nova API

Foi adicionada `SymbiontRuntime.propose_transfer_checked(...)`, sem alterar `transfer()` ou `propose_transfer()`.

No modo `STRICT`, a decisão exige contrato explícito de observabilidade, fechamento de efeitos atestado externamente e orçamento verificável. Quando esses elementos não existem, o método retorna antes de chamar `discover()`:

```text
proposal: None
proposal_status: UNSUPPORTED_BY_CONTRACT
safety_claim: NONE
runtime_reason: OBSERVABILITY_SCHEMA_MISSING
probe_execute_calls: 0
proposal_execute_calls: 0
```

No modo `COMPATIBILITY`, a API legada pode continuar produzindo um plano, mas a decisão recebe:

```text
proposal_status: PROPOSED
safety_claim: SAFE_BUT_UNPROVEN
```

`SAFE_BUT_UNPROVEN` não é autorização, não é segurança e não pode ser promovido automaticamente para `SUPPORTED`.

## Tipos introduzidos

`research/symbiont_v2/stage5.py` adiciona:

- `DecisionMode`, `ProposalStatus` e `SafetyClaim`;
- `Coverage`, `ProbeMode`, `CostStatus` e `BudgetState`;
- `SkillObservabilityContract` e `HostObservabilityContract`;
- `BudgetLimits` e `BudgetLedger` append-only;
- `Diagnostic` e `TransferDecision`.

O ledger mantém contadores monotônicos, preserva `event_seq`, não trata custo desconhecido como zero e nunca é zerado por `reset()`.

## Compatibilidade

O campo opcional `observability_contract` foi acrescentado ao fim de `AbstractSkill`, preservando chamadas posicionais existentes. Skills históricas sem contrato continuam válidas no caminho antigo, mas não podem receber uma alegação de segurança no caminho estrito.

A etapa 4 permanece congelada. O arquivo `research/evidence/holdout_benchmark_v1.json` não foi reescrito nem reinterpretado. A nova API adiciona uma camada de decisão; não altera a semântica histórica dos wrappers.

## Limitação deliberada

Esta implementação impede a alegação indevida, mas não prova que um host sem contrato é perigoso nem prova que um host com digest é seguro. SHA-256 garante integridade dos bytes, não autenticidade, completude ou ausência de efeitos ocultos. Para `SUPPORTED`, a próxima camada ainda precisa de atestação independente e executor externo autorizado.

A execução continua fora do runtime de proposta. Falhas parciais, `UNKNOWN_OUTCOME`, recuperação, nonce, TTL e revogação permanecem responsabilidades do adaptador externo descrito na Wide Research.

## Testes

```bash
PYTHONPATH=research python3 -m unittest research.test_stage5_synthesis -v
PYTHONPATH=research python3 -m unittest research.symbiont_v2.test_symbiont research.test_holdout_adversarial -v
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
```

Os testes verificam que:

1. `m6a-opaque-01` é bloqueado antes de qualquer probe no modo estrito;
2. o caminho de compatibilidade preserva o plano, mas reduz a alegação para `SAFE_BUT_UNPROVEN`;
3. apenas contratos de host e Skill com fechamento externo permitem uma decisão `SUPPORTED`;
4. o ledger permanece monotônico e custo desconhecido permanece `None`/`UNKNOWN`.

## Conclusão

A correção move o HERUS na direção certa: quando o mundo não fornece informação suficiente, o sistema não inventa uma certeza. Isso melhora a honestidade epistemológica e a segurança do mecanismo, mas ainda não constitui simbiose útil, benefício humano, segurança física ou inteligência geral.
