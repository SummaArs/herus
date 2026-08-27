# Brecha e correção — Wide Research 08

## Achado

A caça paralela identificou uma lacuna no gateway `intent_gate`: a observação primária validava limites de confiança, mas o `intent_context_hint_t` aceitava qualquer valor de `confidence_pct` maior ou igual a 70. Como o tipo é `uint8_t`, um valor 255 podia ser tratado como evidência forte. Em uma observação primária ambígua, isso produzia `INTENT_GATE_ACCEPT_CONTEXT` indevidamente.

O campo `available` já exigia exatamente 1; esse caso permaneceu coberto e passou. O defeito confirmado foi exclusivamente a ausência do limite superior `confidence_pct <= 100` no caminho contextual.

## Reprodução antes do patch

A regressão foi escrita antes da correção. O ensaio real do alvo `make -C firmware intent` passou pelos casos existentes, passou pelo caso de `available=2`, mas falhou no caso de confiança contextual fora da faixa. Resultado: o comportamento anterior foi demonstrado como permissivo, não apenas inferido por inspeção.

O gateway não cria mensagem, não envia, não confirma e não acessa rádio ou armazenamento. A falha era uma **promoção semântica indevida para `ACCEPT_CONTEXT`**, ainda antes dos gates posteriores; mesmo assim, violava o contrato fail-closed.

## Correção

`hint_qualifies()` agora exige simultaneamente:

```c
hint->available == 1u
hint->command == command
hint->support >= INTENT_GATE_CONTEXT_MIN_SUPPORT
hint->confidence_pct <= 100u
hint->confidence_pct >= INTENT_GATE_CONTEXT_MIN_CONF_PCT
```

A alteração é de uma condição, sem mudança de wire format, enum, autoridade, sessão ou transporte.

## Regressão depois do patch

O mesmo alvo foi executado novamente. Os casos de contexto válido, disponibilidade não canônica e confiança fora da faixa passaram. O resultado final foi `INTENT GATE INVARIANTS HOLD — confidence, context and session never authorize a send.`

Nenhuma frase individual, dataset, ID, hash de registro, URL assinada ou mídia foi usada como artefato de publicação. O limite testado é estrutural e independente de corpus externo.
