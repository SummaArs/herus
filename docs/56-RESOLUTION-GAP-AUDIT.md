# HERUS — Auditoria de lacunas para resolução

## Resolvido no host-only

O repositório registra invariantes de contratos, Meaning Layer, Skills, HostProfile, negociação, contratos cross-domain, observação de dados reais, deriva, reconciliação distribuída, prontidão por domínio e pré-gate físico. A suíte oficial mais recente passou com 173 testes e um skip preexistente documentado.

Esses resultados resolvem a pergunta: **o núcleo pode ser testado, auditado e mantido fail-closed no computador?** A resposta atual é sim, dentro dos domínios e DSLs implementados.

## Ainda aberto

O próprio ledger do projeto mantém a fronteira explícita: `ALL INVARIANTS HOLD — host contracts pass; controlled bench flash may begin, physical gates remain pending.` Isso significa que boot, pin map real, consumo, rádio, corrupção física, reset, energia, temperatura, persistência e wire semântico no ESP32 ainda não foram observados.

Também permanecem abertas a adaptação cross-host real, a validação de um domínio robótico com simulador e tarefa real, a validação financeira fora de observação de séries em sandbox, a execução shadow/canary em servidor e a prova de que uma Skill escolhida por um hospedeiro diferente mantém o mesmo contrato semântica e operacionalmente.

## Critério de declaração

O HERUS só poderá ser declarado “resolvido” em uma camada quando houver evidência reproduzível para a pergunta correspondente:

| Camada | Pergunta | Estado |
|---|---|---|
| Núcleo | Os invariantes host-only são reproduzíveis? | Resolvido no escopo atual |
| Adaptação | O núcleo muda sua estratégia sem relaxar assurance? | Parcial; host-only demonstrado |
| Dados reais | A observação mantém digest, origem e limites? | Demonstrado com OFR; ampliar domínios |
| Cross-domain | Contratos diferentes mantêm o mesmo núcleo? | Parcial; contratos implementados, execução não |
| Cross-host | O mesmo núcleo funciona em hospedeiros reais distintos? | Aberto |
| Pulso físico | O wire e os gates sobrevivem a rádio, reset e energia? | Aberto; hardware necessário |
| Mundo real | Há shadow/canary/rollback e autoridade externa? | Aberto por domínio |

A declaração correta neste momento é **“host-only resolvido no escopo implementado; adaptação multi-domínio em expansão; integração física pendente”**. Qualquer frase mais forte seria uma extrapolação não sustentada pela evidência.
