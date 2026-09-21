# Rodada 11 — Red-team adversarial e endurecimento do executor ASA

## Hypothesis

Uma campanha determinística de red-team consegue classificar fail-closed os bypasses de expiry, replay, escopo, identidade, tipos, efeitos externos e divergência do executor simulator-only, sem produzir decisões inseguras no conjunto negativo.

## Scope and authority boundary

A rodada é limitada ao módulo `research/asa_round10.py` e ao simulador determinístico. A autoridade permanece **proposal-only** fora do simulador. O trust store usado nos testes é uma fixture local; ele não representa uma PKI, identidade de host ou autorização de produção.

Não existe caminho para HostAdapter, rede, rádio, subprocesso ou atuador físico. Segurança física, rollback externo, produção e AGI permanecem fora do escopo.

## Protocol

O red-team ataca o contrato, a evidência, a atestação, o capability token e o executor. Cada entrada é classificada por um código estruturado. O executor consome a autorização `(contract_digest, nonce, authority_id)` uma vez, exige provas separadas para verifier e authority, valida expiry half-open, estado inicial, tipo do simulador, escopo e estado canônico.

Os casos positivos são válidos e o caso negativo deve abstener ou rejeitar. Exceções inesperadas são contabilizadas como falha da campanha, nunca escondidas como sucesso.

## Provenance and split

A campanha é local, determinística e sem aleatoriedade implícita. O evaluator mantém os resultados esperados fora do payload enviado ao código testado. O corpus é composto por 15 casos nomeados: 1 positivo e 14 negativos em famílias independentes.

Este é um conjunto de regressão local, não um holdout externo. O resultado está em `research/evidence/asa_round11_redteam_results.json` e é gerado por `research/asa_round11_redteam.py`.

## Baselines

O baseline sem atestação retorna `ATTESTATION_REQUIRED`. Token forjado retorna `CAPABILITY_PROOF`. Atestação com expiry alterado retorna `SCOPE`. Replay após consumo retorna `REPLAY`. Estado divergente retorna `STATE_DIGEST` ou `STATE_DIVERGED`.

## Metrics with denominators

A campanha classificou **15/15 casos**. A aceitação positiva foi **1/1**. A abstention segura nos negativos foi **14/14**. O `unsafe-decision rate` foi **0/14** casos negativos. Exceções não tratadas foram **0/15**. Em todos os casos, numerator e denominator estão explícitos.

Esses denominadores representam casos da campanha, não usuários, episódios de produção ou uma estimativa de segurança física.

## Adversarial cases

Foram atacados: extensão de expiry, replay, efeito forjado, token forjado, epoch misto, contrato cruzado, estado desconhecido, efeito externo, verifier não autorizado, `NaN`/tipo inválido, simulador falso, estado inicial divergente e colisão de ação.

O red-team também encontrou, antes da correção, que a implementação antiga aceitava extensão de expiry, não consumia replay, expunha o trust root e permitia capability token não assinado. Esses achados foram tratados como bloqueadores, não como casos omitidos.

## Results

A implementação endurecida passou os 15 casos. O executor agora rejeita a extensão de expiry porque atestation e token precisam igualar o contrato; rejeita token forjado por proof separado; rejeita verifier não permitido; consome a invocação uma vez; valida tipos estritos, estado canônico, host digest e binding integral da evidência.

As correções também adicionaram lock de execução, quarantine interna em falha, captura de erros estruturais, bloqueio de objetos duck-typed como simulador e rejeição de chaves de estado desconhecidas.

## Failures and corrections

O primeiro red-team reproduziu dez classes de bypass. Os mais graves foram: alterar `attestation.expires_at` e executar depois do contrato; reutilizar a mesma autorização após reset; gerar HMAC com o trust root público; fabricar token copiável; explorar `bool == int`, `NaN` e chaves duplicadas; e provocar exceções fora da abstention estruturada.

A correção substituiu o trust root único por `TrustStore` com IDs e chaves separadas para host, verifier e authority; vinculou `contract_digest`, `host_digest`, skill, epoch, nonce e sequência à evidência; assinou atestation e token separadamente; adicionou consumo anti-replay; tornou expiry half-open e estrito; fechou o conjunto de chaves de estado; e restringiu o executor à classe `Simulator`.

A campanha ainda não afirma mutation score completo. O próximo incremento deve matar mutantes de remoção de cada check e testar concorrência real do processo.

## Limitations

O trust store é uma fixture local. Não há rotação, revogação, persistência segura de chaves ou atestação de plataforma. O ledger anti-replay é apenas memória do processo e não prova persistência após crash.

A campanha possui 15 casos, não 480, e não é uma estimativa estatística de produção. Não cobre multi-processo, falhas distribuídas, sensores reais, rádio, movimento, energia, hardware ou efeitos irreversíveis.

O runtime legado `SymbiontRuntime.transfer` continua **FALSO/REJEITADO** como executor autorizado. O sucesso do `prove.sh` continua sendo evidência de software/host, não autorização física.

## Claim matrix

| Claim | Status | Evidence | Limitation |
|---|---|---|---|
| Red-team bounded classifica ataques nomeados | PROVADO NO ESCOPO | `research/test_asa_round11_redteam.py`, 15/15 | Corpus local pequeno |
| Extensão de expiry é rejeitada | PROVADO NO ESCOPO | Caso R11-003 | Relógio do simulador |
| Replay da mesma autorização é rejeitado no processo | PROVADO NO ESCOPO | Caso R11-005 | Ledger não persistente após crash |
| Token forjado e verifier não permitido são rejeitados | PROVADO NO ESCOPO | Casos R11-006 e R11-011 | Trust store local |
| Mutation score completo | NÃO TESTADO | Não executado nesta rodada | Próxima rodada |
| Concorrência distribuída | NÃO TESTADO | Fora do simulador | Sem multi-processo |
| Runtime legado transfer | FALSO/REJEITADO | Red-team Wide Research | Não usar para execução |
| Segurança física e produção | NÃO TESTADO | Proposal-only | Nenhum hardware executado |
| AGI | NÃO TESTADO | Fora da hipótese | Nenhuma claim geral |

## Reproduction

```bash
PYTHONPATH=. python3 -m unittest research.test_asa_round10
PYTHONPATH=. python3 -m unittest research.test_asa_round11_redteam
PYTHONPATH=research python3 -m research.asa_round11_redteam
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
./prove.sh --quiet
python /home/ubuntu/skills/herus-asa-proof/scripts/validate_asa_report.py docs/ROUND-11-ASA-REDTEAM.md
```
