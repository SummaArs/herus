# H0 como primeiro hospedeiro

## Correção conceitual

H0 não é apenas um simulador. É o primeiro hospedeiro real do HERUS: um computador onde o núcleo reside, observa um ambiente lógico, aprende dentro de limites, propõe e mantém sua identidade. O computador fornece recursos amplos para validar correção, mas continua sendo um corpo hospedeiro, não a identidade do HERUS.

O pulso é o próximo corpo físico e o objetivo do projeto. Ele não é um acessório conceitual: é a futura **interface corporal pela qual a pessoa entra em simbiose com o sistema**. H0 existe para permitir que o HERUS seja validado antes de enfrentar as restrições do ESP32-S3, sem confundir validação de lógica com validação física.

## Saída governada

Quando o HERUS deixa H0, o runtime gera um `herus.exit-bundle.v1`. O bundle transporta apenas a identidade, a revisão do modelo e Skills previamente comprovadas. Ele declara explicitamente que não transporta contexto do mundo, segredos ou autoridade de execução.

O fluxo é:

```text
HERUS residente no computador
        ↓
observação e proposta limitada
        ↓
preparar saída
        ↓
detach e quarentena do H0
        ↓
bind a novo hospedeiro
        ↓
descoberta do novo perfil
        ↓
autoridade NONE
```

O novo hospedeiro não recebe uma cópia cega do estado do computador. Ele recebe uma identidade contínua e um contrato mínimo. Seu mundo, seus sensores, seus leases e seu contexto são novos.

## Hierarquia

```text
H0  computador: primeiro hospedeiro real, correção e residência
H1  ESP32-S3:   equivalência sob memória, energia e latência
H2  pulso:      corpo físico, sensores, LRA, rádio e interação
H3  migração:   pulso, computador, robô, veículo e outros hospedeiros
```

Assim, “sair do computador” não significa copiar um processo sem controle. Significa executar uma migração governada: o corpo muda, a identidade continua e a autoridade não é herdada automaticamente.
