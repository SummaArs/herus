# HERUS — Descoberta física do primeiro hospedeiro

## Estado

Este documento é um protocolo de preparação. Nenhuma capacidade física está comprovada antes de a placa, sua revisão, o rádio, a alimentação e o self-test serem observados e registrados. O laboratório host-only não pode substituir essa evidência.

## Objetivo

Repetir no hospedeiro de pulso o ciclo que já foi medido no laboratório oculto: sondar capacidades permitidas, construir uma hipótese parcial, testar limites, negociar representação e manter `authority=NONE` até que uma autoridade externa e uma política de execução sejam fornecidas.

O pulso não receberá um `HostProfile` completo pronto. O firmware de ensaio exporá somente respostas limitadas sobre boot, revisão declarada, memória disponível para o ensaio, interfaces presentes, transporte, tamanho máximo de payload e driver háptico detectado. A descoberta não poderá ler chaves, identidade pessoal, conteúdo de mensagens ou permissões humanas.

## Sequência obrigatória

A primeira etapa é B1: fotografar e identificar a placa, confirmar a variante de frequência, a revisão, o MCU, o rádio, o esquema e o pin map. A segunda é B2: boot serial, self-test, versão do firmware e digest do build. Sem B1 e B2 aprovados, nenhuma sondagem de capacidade será considerada evidência física.

Depois, o ensaio seguirá a sequência abaixo.

| Etapa | Medição | Critério de bloqueio |
|---|---|---|
| P1 | Identidade e revisão da placa | Qualquer divergência ou campo ausente |
| P2 | Boot e self-test | Reset inesperado, erro não explicado ou digest ausente |
| P3 | Sondagem de representação e payload | Resposta fora do esquema ou capacidade não reproduzível |
| P4 | Sondagem de rádio e serial | Transporte não identificado ou parâmetro inconsistente |
| P5 | Sondagem I2C/DRV2605L | Driver ausente, endereço conflitante ou falha de leitura |
| P6 | Negociação de Skill sem efeito | Skill incompatível proposta ou efeito não vazio |
| P7 | Deriva e reset | Crença antiga reutilizada sem renegociação |
| P8 | Wire semântico | CRC, digest, replay ou round-trip incorreto |

## Evidência mínima

Cada observação precisa conter sessão, revisão da placa, sequência monotônica, sonda, argumento, valor, unidade, timestamp de ensaio, digest do firmware e digest do registro. Um registro sem qualquer desses campos permanece `PENDING` e não pode promover uma capacidade.

A evidência física deve ser específica ao dispositivo, revisão, antena, firmware, fonte de alimentação, instrumento e condições ambientais observadas. Um resultado no pulso não prova adaptação em robótica, servidores, finanças ou outro hospedeiro.

## Critérios de passagem da descoberta

A descoberta física passa somente se o HERUS identificar capacidades realmente presentes, não propor capacidades ausentes, marcar como desconhecidas as que não puder testar, respeitar o orçamento e renegociar depois de reset, alteração de revisão ou mudança de métrica. O resultado precisa permanecer com `authority=NONE` e `allowed_effects=[]` durante toda a fase de descoberta.

O primeiro sucesso físico esperado é limitado: o HERUS descobre o subconjunto de recursos observáveis do pulso, escolhe uma representação que cabe no orçamento medido e bloqueia uma Skill que exige recurso ausente. Isso prova descoberta nesse hospedeiro específico; não prova adaptação universal.

## Condição de parada

Se as placas não chegarem, o trabalho correto é preparar o protocolo, não fabricar resultados. Quando chegarem, devem ser enviadas fotografias nítidas da frente, verso, etiquetas e revisão antes de ligar bateria ou conectar o LRA. A campanha começa então em B1.
