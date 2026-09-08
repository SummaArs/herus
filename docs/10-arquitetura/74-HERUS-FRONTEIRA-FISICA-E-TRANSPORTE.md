# HERUS — Fronteira física, energia e transporte

**Estado:** contratos host-side verificados; evidência física ainda pendente.  
**Escopo:** recuperação após falha de energia, journal persistente, corrupção de bits, DRV2605L, porta I²C ESP32-S3 e SX1262.

## 1. Objetivo

A inteligência soberana do HERUS só é útil se a fronteira física não transformar um erro de barramento em uma conclusão falsa. Nesta etapa, a frente construtora manteve os adaptadores pequenos, injetáveis e limitados; a frente sabotadora removeu controles de estado, ordem, BUSY, potência, timer, fiação e diagnóstico. Nenhum teste abriu SPI, I²C, GPIO ou rádio real.

A regressão final de `./prove.sh --quiet` terminou com `ALL INVARIANTS HOLD`. O ledger passou a conter **68 suítes**, preservando **111 invariantes do simulador**, **7/7** mutantes de autonomia, **5/5** mutantes de magia/memória, **7/7** mutantes do Core e **8/8** mutantes de transporte.

| Área | Resultado host-side | Evidência exercitada |
|---|---:|---|
| Journal power-fail | **7/7** | Cada prefixo parcial de escrita rejeitado; RAM inalterada; sucessor autenticado recuperável |
| Corrupção persistente | **496/496** | Cada bit dos 62 bytes do registro ativo rejeitado durante reboot |
| Adaptador DRV2605L | **19/19** | Ordem de escrita, busy, abort, falha I²C, overtemperature e não-retry |
| Porta haptics ESP32-S3 | **4/4 + 4/4** | Caminho não verificado bloqueado; override explícito continua separado |
| Driver SX1262 | **passou** | Packet type, frequência, PA, sync privado, BUSY, duty-cycle, TX power e selftest |
| Transporte redteam | **8/8** | Mutantes hápticos, target e rádio mortos pelos mocks correspondentes |
| Regressão global | **68 suítes** | Checks de código, proveniência, simulação e mutação sem falha |

## 2. Power-fail e persistência

A suíte `test_knowledge_feed_cursor_powerfail.c` modela uma mídia apagada como `0xFF`, injeta interrupção depois de cada prefixo de 0 a 61 bytes e tenta recuperar o journal após reboot. O registro incompleto nunca é promovido, o estado em RAM não é alterado por uma promoção que falhou e um sucessor completo continua recuperável quando a energia é considerada perdida depois da escrita autenticada.

A mesma suíte percorre os **62 bytes × 8 bits = 496** corrupções unitárias do slot mais novo. Cada corrupção faz o carregamento falhar fechado ou permanecer sem cursor inicializado; o slot antigo não é usado silenciosamente para esconder a adulteração do registro mais recente.

> Isso prova o comportamento do journal perante o modelo de falha injetado. Não prova que uma escrita física do ESP32-S3 é atomicamente interrompível nos mesmos pontos, nem que flash, NVS, brownout detector ou secure element tenham sido medidos.

## 3. DRV2605L e autoridade de apresentação

O adaptador háptico permanece uma camada de apresentação, não uma fonte de autoridade. O frame semântico é validado antes da primeira escrita; modo, waveform e GO são enviados em ordem; uma reprodução concorrente retorna busy; falhas de leitura ou escrita entram em `HA_STATE_FAULT`; overtemperature, overcurrent e diagnóstico não são confundidos com conclusão; abort escreve standby e uma falha não habilita retry automático.

A porta ESP32-S3 continua bloqueada por padrão. `BOARD_HAS_HAPTIC_I2C=0` e `PIN_HAPTIC_ENABLE_VALID=0` impedem que um pin map não verificado seja tratado como presente. O caminho de override existe apenas para o teste explícito e não muda o estado default do produto.

## 4. SX1262 e autoridade de transmissão

O driver de rádio continua sujeito à mesma regra soberana: o transporte não cria conteúdo, confirmação ou autoridade de ação. O mock verifica que `SetPacketType(LORA)` precede parâmetros de modulação, que o BUSY é aguardado antes de cada comando, que a frequência, o PA e a potência vêm de configurações coerentes, que o sync word privado é escrito e que o duty-cycle não é alterado silenciosamente por um timer de preâmbulo.

A campanha também removeu cada um desses controles individualmente. Todos os mutantes foram detectados. O selftest continua apenas diagnóstico: encontrar um problema no mock ou na placa futura não autoriza transmissão, não libera o pin map e não transforma um ruído plausível em enlace confiável.

## 5. Limites e próximo gate

O resultado não demonstra energia, alcance, interferência, temperatura, sensibilidade, latência, ruído I²C, estabilidade de TCXO, comportamento de antena, vida útil de flash, confiabilidade de botão, qualidade háptica ou desempenho do ESP32-S3. O `SX1262 COMMAND SEQUENCES VERIFIED` é um resultado do mock SPI. O caminho de haptics com override é um stub de host, não presença elétrica do DRV2605L.

Antes de qualquer alegação física, o próximo gate deve registrar revisão da placa, marcação dos componentes, pin map verificado, instrumento identificado, fonte limitada, antena correta e evidência privada conforme os schemas já definidos. Só então a matriz de interrupção deve ser reproduzida na placa, com reset e mídia reais observados, sem habilitar o atuador por conveniência.

## 6. Reprodução

```bash
cd /home/ubuntu/herus-semantic-compiler-pr
python3 tools/test_transport_redteam.py
cd firmware
make knowledge-feed-cursor-powerfail
make haptic-adapter radio haptic-target
cd ..
./prove.sh --quiet
python3 tools/provenance_audit.py --strict research/software_provenance_manifest.json
```

Um resultado positivo autoriza somente avançar para a preparação controlada de bancada. Ele não autoriza afirmar que o HERUS já funciona fisicamente ou que a inteligência local atingiu desempenho de uma LLM geral.
