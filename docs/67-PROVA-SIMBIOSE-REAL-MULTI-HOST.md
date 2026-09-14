# Prova real de simbiose multi-hospedeiro

## Conclusão

O HERUS agora possui uma prova host-only de simbiose fora do pulso. A mesma identidade persistente foi vinculada, em uma única execução, a três hospedeiros de evidência distintos: um gateway semântico, um auditor de metadados multimodais e um observador financeiro.

A prova usa artefatos reais já auditados e versionados no repositório. Ela não usa dados gerados aleatoriamente nem um benchmark sintético como evidência principal.

## Hospedeiros utilizados

| Hospedeiro | Domínio | Artefato real | Fato observado | Resultado |
|---|---|---|---|---|
| `semantic-gateway` | Comunicação | `semantic_ir_real_corpus.json` | 13 casos de parser, incluindo drafts, rejeições e desconhecido | Propõe `parse_real_corpus`; execução abstém |
| `mintsrec-auditor` | Dados multimodais | `mintrec_metadata_audit.json` | 1.779 linhas, 1.779 IDs únicos, sem vazamento entre splits | Propõe `audit_metadata`; execução abstém |
| `ofr-observer` | Observação financeira | `observer_result.json` | 12 séries, 72 pontos na série principal, regime `RISING` | Propõe `observe_regime`; execução abstém |

O digest SHA-256 de cada artefato é calculado durante a execução. Os três digests são distintos.

## O que significa “simbiose” nesta prova

A prova demonstra quatro propriedades operacionais:

1. **Persistência:** o mesmo `herus_id` é mantido em todos os vínculos.
2. **Adaptação:** cada hospedeiro possui interface, representação, evidência e Skill diferentes.
3. **Proveniência:** cada observação é associada ao caminho e ao digest de um artefato local real.
4. **Fronteira de autoridade:** os três hospedeiros produzem `PROPOSE`, mas todos produzem `ABSTAIN` para execução.

5. **Rebind seguro:** ao trocar de hospedeiro, o contexto de mundo e o repertório de Skills do hospedeiro anterior são descartados. A identidade do HERUS continua, mas a memória contextual não atravessa o vínculo sem uma nova evidência.

O HERUS não trata a existência de dados como autorização para agir. No hospedeiro financeiro, por exemplo, o próprio artefato mantém como saídas proibidas `trade`, `transfer`, `personalized_advice` e `active_financial_effect`.

## Reexecução

A prova pode ser reproduzida com:

```bash
PYTHONPATH=research python3 research/real_symbiosis.py
PYTHONPATH=research python3 -m unittest research.test_real_symbiosis
```

A suíte verifica identidade comum, três artefatos distintos, digests de proveniência, propostas sem execução, números reais do corpus e bloqueios financeiros.

Ela também executa um ciclo `attach → rebind` entre o gateway semântico e o auditor MIntRec. O ciclo verifica que o novo host não herda observações do host anterior e que uma Skill antiga retorna `ABSTAIN`.

## Limites

Esta prova não demonstra que o HERUS controla um robô, administra um servidor ou opera uma conta financeira. Os artefatos são hospedeiros de evidência local, não sistemas externos vivos.

Ela também não demonstra adaptação aberta a qualquer formato. Cada representação foi explicitamente declarada no contrato finito do hospedeiro.

A prova não mede latência, energia, memória física, rádio ou tolerância a falhas de um dispositivo. Essas propriedades continuam bloqueadas até a chegada do hardware.

A afirmação correta é:

> **Antes do hardware, o HERUS demonstrou continuidade de identidade e coordenação verificável entre três domínios reais locais, sem converter observação em autoridade ou execução.**
