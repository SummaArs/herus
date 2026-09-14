# HERUS — Lista de compra física v1

## Objetivo

Esta lista cobre apenas a primeira bancada do HERUS. Ela não compra componentes para uma versão final, PCB própria, produto comercial ou claims de alcance. A prioridade é obter duas unidades idênticas, medir a cadeia real e poder abortar cedo.

## Compra mínima

| Item | Quantidade | Necessidade | Critério de compra |
|---|---:|---|---|
| LilyGO T3-S3 com ESP32-S3 + SX1262, versão 915 MHz | 2 | Dois nós de rádio para B1–B6 | A mesma revisão e o mesmo perfil de frequência; confirmar o esquema antes de energizar |
| Antenas 915 MHz fornecidas com as placas | 2 | Manter o experimento comparável | Não substituir na primeira rodada |
| Cabo USB-C de dados | 2 | Flash, serial e coleta | Confirmar que cada cabo transmite dados, não apenas energia |
| LiPo de aproximadamente 150 mAh com proteção | 1 | Teste de volume e futura energia | Não conectar antes de confirmar polaridade, conector e circuito de carga |
| Piezo bender de aproximadamente 20 mm e espessura até 0,6 mm | 1 | Teste mecânico e interação mínima | Usar apenas depois do teste elétrico básico |
| Shell impresso de 30 × 30 × 10 mm | 1 | Gate mecânico | Imprimir com revisão identificada |
| Peça dummy de 28 × 28 mm e 1 mm | 1 | Simular PCB no empilhamento | Pode ser acrílico, madeira ou impressão rígida |
| Fita métrica e rota marcada | 1 | Distância e PDR | Registrar rota, pontos e condições |
| Cronômetro | 1 | Duração e latência manual | O telefone é suficiente |

## Instrumentação recomendada

| Instrumento | Prioridade | Por quê |
|---|---|---|
| Paquímetro digital | Alta | Medir altura fechada e confirmar o gate mecânico |
| Multímetro | Alta | Verificar tensão, continuidade, polaridade e consumo estático básico |
| Analisador de energia USB ou medidor de corrente | Alta | Medir workload declarado sem transformar estimativa em fato |
| Adaptador USB-UART adicional | Média | Manter logs separados e evitar depender de um único cabo |
| Computador com duas portas USB | Alta | Flash e coleta simultâneos |
| Estação de solda e consumíveis | Não comprar para B1 | A primeira fase usa placas prontas; solda só entra se o protocolo exigir |
| Osciloscópio | Não necessário para B1 | Pode ser alugado ou adiado até uma falha temporal concreta |

## Não comprar ainda

Não comprar PCB própria, componentes RF avulsos, bateria para produção, sensores adicionais, microfone, display, módulo criptográfico ou peças para uma carcaça final antes de B1–B6. Esses itens aumentam o investimento sem responder à primeira pergunta: **duas placas da revisão correta conseguem inicializar, trocar um frame e recusar corrupção?**

## Ordem de aquisição e uso

A compra deve começar pelas duas placas, antenas e cabos. Ao receber, fotografar a revisão, a marcação do MCU, o rádio, os conectores e o estado físico antes de energizar. Em seguida, executar B1 e B2. Somente depois do mapa de pinos confirmado devem ser conectadas bateria, piezo ou qualquer acessório.

O primeiro orçamento físico é deliberadamente mínimo: duas placas, um frame válido, um frame corrompido, logs seriais e o registro canônico do bench schema. Um resultado `blocked_by_missing_evidence` é aceitável e preferível a uma passagem baseada em medição incompleta.
