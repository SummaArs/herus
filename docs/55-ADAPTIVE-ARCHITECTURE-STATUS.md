# HERUS — Estado da Adaptive Symbiotic Architecture

## O que o HERUS é agora

O HERUS é um núcleo de coordenação adaptativa com significado finito, Skills verificáveis, memória de observações, negociação de recursos e gates de autoridade. Ele pode representar um hospedeiro, escolher uma representação compatível, selecionar uma Skill verificada, detectar deriva e recusar adaptação quando há conflito, replay, falta de quórum, recurso ausente ou evidência insuficiente.

O hospedeiro de pulso continua sendo o primeiro teste físico, não a definição total do sistema. A mesma arquitetura foi preparada para classificar robótica, finanças em sandbox, servidores e sistemas críticos com vocabulários, riscos e tetos diferentes.

## Evidência acumulada

| Capacidade | Evidência atual |
|---|---|
| Skill verificável | Síntese finita, composição, generalização oculta e memória |
| Adaptação ao hospedeiro | HostProfile, negociação de representação e seleção proposal-only |
| Dados reais | Artefato OFR versionado, comprimido e protegido por SHA-256 |
| Deriva | Invalidação por mudança de métrica ou revisão do hospedeiro |
| Distribuição | Quórum, sequência monotônica, escopo comum e digest consistente |
| Multi-domínio | Contratos para pulso, robótica, finanças, servidores e críticos |
| Prontidão | Tetos explícitos para proposta, simulação, shadow, canary e humano-bound |
| Hardware | Pré-gate pronto; validação física ainda pendente |

## O que ainda não foi provado

O HERUS ainda não provou adaptação universal, raciocínio aberto, execução segura em robôs, operação financeira real, controle de infraestrutura crítica, autonomia geral ou desempenho em ESP32. Também não provou que um wire format compacto mantém todas as garantias sob rádio, reset, energia limitada e falhas físicas.

Essas lacunas não são escondidas pela arquitetura. Elas definem a sequência de trabalho: medir o pulso, ampliar para um segundo hospedeiro real, validar dados e simulações por domínio e só então estudar promoções específicas de efeito.

## Declaração pública permitida

> **HERUS é uma arquitetura simbiótica adaptativa: ela entra em um hospedeiro, descobre seus limites, escolhe capacidades verificadas, aprende observações finitas sobre o ambiente e permanece inerte quando não consegue provar que a adaptação é compatível e autorizada.**

A palavra “universal” descreve a ambição do contrato, não uma propriedade já comprovada. A palavra “simbiótica” descreve a relação operacional entre núcleo e hospedeiro, não uma afirmação biológica ou antropomórfica.

## Próximo marco

O próximo marco é físico e objetivo: duas placas T3-S3 com a mesma variante SX1262, identificadas por revisão e esquema, devem passar pelo bring-up, baseline a um metro, corrupção controlada, replay e wire semântico. O registro precisa ser validado pelo gate B1–B10. Se falhar, o resultado será `rejected`, com causa e evidência preservadas.
