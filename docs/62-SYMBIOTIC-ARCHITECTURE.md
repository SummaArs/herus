# HERUS como Adaptive Symbiotic Architecture

## Definição operacional

O HERUS é uma infraestrutura adaptativa de simbiose entre uma pessoa, um núcleo computacional e um hospedeiro externo. O hospedeiro pode ser um dispositivo de pulso, um robô, um servidor, um sensor, uma rede local ou um conector de domínio. O HERUS não assume que conhece o hospedeiro. Ele observa, formula uma hipótese finita de capacidades, negocia uma representação, propõe uma Skill e só permite progressão quando as evidências exigidas estão presentes.

A metáfora de “dar alma” a um robô é útil para comunicação, mas não é uma afirmação técnica. O robô continua sendo o corpo físico; o HERUS acrescenta uma camada verificável de coordenação, memória de estado, descoberta de capacidades e limites de autoridade. Se uma capacidade não foi comprovada, ela não existe para o núcleo.

> **Descobrir não autoriza. Conectar não significa confiar. Propor não significa executar.**

## Quatro modos de simbiose

| Modo | Hospedeiro principal | Função | Limite inicial |
|---|---|---|---|
| Hospedeiro simbiótico | Robôs, máquinas, sensores | Descobrir capacidades e negociar Skills | Simulação e telemetria antes de controle |
| Guardião pessoal | Pessoa e dispositivo de pulso | Comunicar estados e solicitar decisões | O LRA nunca concede autoridade sozinho |
| Rede defensiva | BLE, LoRa, Wi-Fi e outros canais | Inventariar, verificar, detectar deriva e colocar em quarentena | Apenas dispositivos e ambientes autorizados |
| Conector de domínio | Servidores, finanças em sandbox e sistemas críticos | Adaptar contratos de domínio sem misturar autoridades | Observação, proposta, simulação ou shadow mode |

## Fronteira de autoridade

O transporte é separado da autoridade. USB, BLE, LoRa, Wi-Fi, UART, I2C, SPI, CAN e RS-485 possuem contratos diferentes de autenticação, tamanho de mensagem e efeitos transportáveis. Nenhum deles transforma uma mensagem em ordem válida automaticamente.

Uma ordem candidata precisa carregar, no mínimo, identidade do hospedeiro, digest da evidência, contexto, efeito pretendido e escopo de autoridade. A ausência de qualquer elemento produz recusa ou proposta não executável. Um canal não autenticado pode transportar telemetria local apenas quando a política permitir; ele não pode declarar que um motor, conta, processo ou intertravamento está sob autoridade do HERUS.

## Guardião defensivo

O guardião mantém um registro append-only lógico de observações autorizadas. Uma primeira observação autorizada pode produzir o estado `VERIFIED`. Uma mudança no digest de capacidade produz `DRIFTED` e exige renegociação. Uma mudança de identidade produz `QUARANTINED`. Um dispositivo não autorizado também entra em quarentena. A revogação humana produz `REVOKED` e continua valendo quando o dispositivo reaparece.

A rede Bluetooth do HERUS, portanto, é defensiva. O núcleo pode observar inventário autorizado, validar evidências, perceber pareamento inesperado, detectar mudança de identidade ou capacidade e solicitar bloqueio. Ele não deve explorar, invadir ou interferir em dispositivos sem autorização explícita e contrato correspondente.

## O pulso como canal de autoridade humana

O pulso não é somente uma tela mínima. Ele é um canal físico para tornar visível a fronteira entre fato, proposta e autorização. Os padrões hápticos são finitos e proposal-only.

| Estado lógico | Feedback atual | Significado |
|---|---|---|
| `VERIFIED` | Confirmação | Observação autorizada estável |
| `DRIFTED` | Espera | Capacidades mudaram; renegociação necessária |
| `QUARANTINED` | Bloqueio | Identidade ou autorização não é aceitável |
| `REVOKED` | Recusa | A pessoa revogou o dispositivo |
| Ausência de evidência | Silêncio | O HERUS não deve fabricar um estado |

O usuário poderá futuramente selecionar ações como permitir uma vez, permitir nesta sessão, negar, revogar e esquecer. A ação física precisa ser deliberada e separada do simples recebimento do alerta. Um pulso nunca deve ser interpretado como autorização acidental para movimento, transferência, alteração de dados ou bypass de intertravamento.

## Evidência atual

A implementação host-only contém contratos de modo, canais, autoridade, guardião defensivo, tradução para padrões hápticos e cenários em sandbox para robótica, Bluetooth e finanças. A suíte publicada no branch de pesquisa possui 243 testes passando, incluindo casos de identidade alterada, deriva de capacidade, revogação persistente, autenticação ausente e efeitos desconhecidos.

Esses resultados provam invariantes do software e cenários finitos. Eles não provam ainda a operação de rádio, a qualidade de um LRA real, o consumo, a latência, a interferência, o comportamento de um ESP32-S3, a segurança de um robô físico ou a compreensão humana dos padrões táteis. Essas alegações só podem entrar depois dos ensaios físicos correspondentes.

## Próximo gate físico

A primeira integração física deverá permanecer limitada a observação, telemetria, padrões hápticos e confirmação humana. O núcleo deve medir identidade, latência, memória, energia, perdas de comunicação, reinicialização, comportamento sob mensagens truncadas e recuperação após deriva. Motores, atuadores perigosos, contas financeiras e alterações irreversíveis ficam fora do primeiro gate.

O critério de sucesso não será “o HERUS conectou”. Será: **o HERUS recusou corretamente o que não conseguiu provar, comunicou o estado de forma reconhecível, preservou a revogação e não executou efeitos fora da autoridade autorizada**.

## Limite conceitual

Esta arquitetura é um caminho para computação adaptativa geral no sentido de adaptação de contratos e Skills dentro de ambientes definidos. Ela ainda não demonstra inteligência geral, raciocínio livre aberto ou substituição de modelos de linguagem. A linguagem pública deve preservar essa distinção. O avanço real está em tornar a adaptação verificável e limitada, não em declarar uma capacidade que os testes ainda não mediram.
