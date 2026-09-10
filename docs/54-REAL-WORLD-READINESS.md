# HERUS — Prontidão para o mundo real por domínio

O HERUS não terá um único selo de “pronto”. Cada domínio terá um teto de prontidão próprio, determinado por evidência, risco, reversibilidade e autoridade externa.

| Domínio | Teto host-only atual | Próximo gate | Produção automática |
|---|---|---|---|
| Pulso | Simulação e pré-gate | Identificar placa, boot, rádio, wire e falhas físicas | Proibida |
| Robótica | Contrato e simulação planejados | Mundo simulado com zona segura, parada e replay | Proibida no primeiro ciclo |
| Finanças | Observação real de séries OFR sem efeito | Sandbox reproduzível e análise com dados temporais separados | Proibida |
| Servidores | Contratos de canary e rollback | Shadow com artefato versionado e rollback testado | Apenas com autoridade externa |
| Sistemas críticos | Proposta/shadow conceitual | Dados de sensores, revisão humana e integração autorizada | Proibida sem governança específica |

## Gates comuns

Antes de subir de `PROPOSAL` para `SIMULATE`, o domínio precisa ter fonte de dados, esquema, digest, vocabulário finito e casos negativos. Antes de `SHADOW`, precisa ter testes ocultos, adversariais, política de deriva e reconciliação. Antes de `CANARY`, precisa haver reversibilidade, orçamento, observabilidade e rollback. Antes de `HUMAN_BOUND`, deve existir autoridade humana explícita vinculada ao contexto. `PRODUCTION` nunca é resultado automático da adaptação.

## Estado atual

A infraestrutura host-only já possui HostProfile, negociação, Skills verificáveis, contratos de domínio, observador de dados OFR, detecção de deriva, reconciliação distribuída e pré-gate físico. Isso permite pesquisar adaptação multi-domínio sem alegar que robótica, finanças ou infraestrutura crítica já foram validadas operacionalmente.

A próxima prova de pulso é material: confirmar a revisão da placa e o rádio, executar boot e self-test, estabelecer baseline a um metro e somente então conectar a camada semântica. A próxima prova financeira é analítica: separar janelas temporais, medir observações fora da amostra e preservar a regra de que nenhuma saída gera ordem ou transferência. A próxima prova robótica deve ser em simulador com estado e parada segura, não em um atuador real.

> **Pronto para o mundo real significa saber em qual mundo, sob quais condições, com quais evidências e com qual autoridade o HERUS pode operar.**
