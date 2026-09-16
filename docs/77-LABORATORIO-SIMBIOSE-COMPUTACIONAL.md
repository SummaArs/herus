# Laboratório de simbiose computacional

## Objetivo

Este laboratório prova a simbiose antes da quantização física. O HERUS recebe apenas um canal finito de probes de um hospedeiro oculto. Ele não recebe o `HostProfile` verdadeiro. A partir das evidências, descobre capacidades, compila um plano de adaptação, constrói um perfil limitado e opera dentro desse escopo.

## Ciclo

```text
host oracle oculto
        ↓
probes autenticadas e orçamentadas
        ↓
hipótese de capacidades
        ↓
plano de adaptação
        ↓
perfil compilado somente da evidência
        ↓
bind do HERUS
        ↓
proposta limitada
        ↓
execução ABSTAIN
```

O laboratório executa o mesmo núcleo em três hosts estruturalmente diferentes: um host com rádio e haptic, um host com display e sensor e um host restrito. O plano muda conforme as capacidades observadas. O HERUS não inventa interfaces, não importa a memória verdadeira do oracle e não recebe autoridade.

## Resultado observado

Cada host produz uma representação e conjunto de Skills diferentes. Em todos os casos, o perfil compilado é identificado pela sessão de descoberta, inicia com mundo vazio, permanece válido, pode produzir uma proposta limitada e retorna `ABSTAIN` para execução.

O caso de orçamento reduzido permanece limitado e não transforma capacidades desconhecidas em capacidades disponíveis. Probes insuficientes significam adaptação parcial, não generalização por imaginação.

## O que foi comprovado

A evidência demonstra **simbiose computacional bounded**: o mesmo núcleo descobre, adapta sua representação e muda seu repertório conforme um hospedeiro heterogêneo, preservando identidade, isolamento e autoridade fail-closed.

## O que ainda não foi comprovado

O laboratório não demonstra sensores físicos, energia, rádio real, LRA, latência de microcontrolador, operação do pulso ou adaptação a um hospedeiro arbitrário fora do vocabulário de probes. A quantização para o pulso continua sendo uma etapa posterior de engenharia e medição.

A saída reproduzível está em `research/evidence/computational_symbiosis_lab.json`.
