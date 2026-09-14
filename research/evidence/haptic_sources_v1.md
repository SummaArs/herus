# Fontes técnicas da interface LRA

## Texas Instruments DRV2605L

Fonte oficial: https://www.ti.com/lit/ds/symlink/drv2605l.pdf

O DRV2605L é um driver háptico para LRA e ERM, com controle por I2C, reprodução de efeitos, modo de reprodução em tempo real, rastreamento automático de ressonância para LRA, diagnóstico do atuador e controle em malha aberta ou fechada. A folha de dados indica faixa de alimentação de 2 V a 5,2 V e saída diferencial para o atuador. A integração deve respeitar a tensão, o desacoplamento, os pinos e o modo de controle do driver.

## Adafruit DRV2605L breakout

Fonte: https://learn.adafruit.com/adafruit-drv2605-haptic-controller-breakout?view=all

O breakout expõe controle I2C, regulador de 3,3 V, conversão de nível lógico e conectores STEMMA QT; o cabo QT não é incluído. A documentação informa compatibilidade com LRA e ERM, mas o atuador LRA escolhido ainda precisa ser compatível com o modo e a frequência configurados.

## Vybronics LRA catalogue

Fonte: https://www.vybronics.com/products/coin-vibration-motors/lra

O catálogo mostra que LRAs diferem em tensão nominal, corrente, frequência de ressonância, dimensões e força. Exemplos listados incluem atuadores de 6 mm, 8 mm e 10 mm com frequências de ressonância de 170 Hz, 205 Hz, 210 Hz, 235 Hz e 260 Hz. Portanto, não se deve comprar uma LRA genérica sem especificação da frequência e tensão; o driver e o atuador precisam ser tratados como um conjunto.

## Espressif ESP32-S3 I2C

Fonte: https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/i2c.html

O ESP32-S3 possui dois controladores I2C e suporta Standard-mode e Fast-mode até 400 kHz. O barramento usa SDA e SCL open-drain com resistores de pull-up; a documentação recomenda resistores externos adequados e observa que pull-ups internos podem não ser suficientes em frequências maiores.
