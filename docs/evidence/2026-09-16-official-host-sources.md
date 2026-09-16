# Fontes oficiais consultadas — 2026-09-16

## LILYGO T3-S3 V1.3

URL: https://wiki.lilygo.cc/products/t3-series/t3-s3-v1.3/

A página oficial descreve a T3-S3 V1.3 como baseada no ESP32-S3FH4R2 dual-core LX7, com variantes opcionais SX1262/SX1276/SX1278 em 433/868/915 MHz ou SX1280 em 2,4 GHz. Lista OLED SSD1306 128x64, slot TF, USB-C, 4 MB Flash, 2 MB QSPI PSRAM, Wi-Fi, Bluetooth 5.0 e bateria Li-Po 3,7 V. A página também fornece instruções de PlatformIO/Arduino e alerta que a variante e a revisão reais devem ser confirmadas.

Para SX1262, a tabela oficial lista SCK GPIO5, MISO GPIO3, MOSI GPIO6, RESET GPIO8, DIO1 GPIO33, BUSY GPIO34 e CS GPIO7. A documentação informa que GPIO18/17 são SDA/SCL do I2C, mas aparecem como não disponíveis no mapa SX1262; isso deve ser confirmado na unidade física antes de conectar acessórios.

## Espressif ESP32-S3 datasheet

URL: https://documentation.espressif.com/esp32-s3_datasheet_en.pdf

O datasheet oficial descreve o ESP32-S3 como SoC dual-core Xtensa LX7 de até 240 MHz com Wi-Fi 2,4 GHz, Bluetooth LE 5, USB Serial/JTAG, GPIO, UART, I2C, SPI, ADC e modos de baixo consumo. A tabela de baixo consumo mostra valores típicos dependentes do modo e dos periféricos; esses valores não substituem medição da placa T3-S3 montada.

## Uso no HERUS

As fontes foram aceitas pelo hospedeiro Internet com HTTPS, domínio allowlisted, claims explícitos e digest. Foram usadas para preparar hipóteses H1/H2 e o mapa de hosts; não executaram código, não alteraram firmware e não concederam autoridade. B1/B2 físicos continuam obrigatórios.
