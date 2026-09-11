# HERUS — Contrato háptico LRA

## Decisão arquitetural

O primeiro hospedeiro físico do HERUS usará um atuador LRA controlado por um driver háptico dedicado, em vez de acionar o atuador diretamente por GPIO. O candidato inicial é um driver da família DRV2605L, que oferece controle para LRA por I2C, biblioteca de efeitos e controle em malha fechada baseado no back-EMF do atuador [1].

O ESP32-S3 possui controladores I2C e suporta os modos Standard e Fast, com limite de clock SCL de 400 kHz conforme a documentação do ESP-IDF [2]. A integração elétrica final, endereço I2C, alimentação, pull-ups e pinos disponíveis continuam condicionados à revisão física exata da placa LILYGO e não serão presumidos antes do B1.

## Separação de autoridade

O HERUS não transforma diretamente uma intenção em acionamento. O fluxo é:

```text
HIR / estado verificável
  → padrão háptico PROPOSAL_ONLY
  → política do hospedeiro
  → driver LRA
  → observação de execução
```

Um padrão háptico é uma saída de interface, não uma autorização. Uma mensagem inválida, uma Skill não verificada ou um estado sem autoridade não pode gerar vibração apenas porque o driver está conectado.

## Vocabulário inicial

| Estado semântico | Padrão inicial | Observação |
|---|---|---|
| `CONFIRMATION` | pulso curto único | confirmação de recepção, não de execução |
| `ATTENTION` | dois pulsos separados | requer leitura ou confirmação humana |
| `WAITING` | três pulsos curtos em baixa prioridade | operação pendente |
| `REFUSED` | pulso longo único | sistema recusou por falta de garantia |
| `BLOCKED` | dois pulsos longos | tentativa bloqueada por política |
| `EMERGENCY_PROPOSAL` | sequência reservada | nunca autoriza sozinha; exige política explícita |

Esses padrões são símbolos finitos. Sua semântica deve ser ensinada ao usuário e testada com pessoas; intensidade, duração e percepção não serão presumidas a partir do nome do padrão.

## Critérios de falha

O HERUS deve permanecer silencioso quando o driver não responde, quando o endereço I2C é inesperado, quando a alimentação está fora da faixa validada, quando a LRA não apresenta resposta compatível ou quando a política do hospedeiro não vincula o estado a um padrão permitido. Falha do atuador não pode ser interpretada como confirmação sem evidência separada.

## Referências

[1]: https://www.ti.com/product/DRV2605L "Texas Instruments — DRV2605L Haptic Driver"

[2]: https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/i2c.html "Espressif — ESP32-S3 I2C Driver"
