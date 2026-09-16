# Equivalência H0 → H1

## Resultado

A ponte C11 do SIM foi comparada com os quatro vetores H0 protegidos: caso válido, proveniência ausente, orçamento insuficiente e autoridade indisponível. A comparação verifica label, representação, confiança, proposta e execução.

O caso de proveniência ausente encontrou uma divergência real durante a implementação. O C11 bloqueava cedo demais e retornava representação vazia, enquanto o contrato H0 mantém a representação e a confiança observadas, mas remove o label não comprovado e bloqueia a proposta. A implementação foi corrigida para preservar essa semântica.

O alvo `make -C firmware sim-equivalence` agora compila com C11, executa os casos e informa `SIM H0/H1 EQUIVALENCE PASS`.

## Limite da evidência

Esta é equivalência host-side da implementação C11. Ela prova que a ponte compilada no computador coincide nos casos protegidos; não prova ainda execução no ESP32-S3, consumo, RAM, temperatura, latência física ou operação no pulso. Esses atributos pertencem ao H1/H2 físico e exigem as placas.

A regra de segurança permanece:

```text
H0 = ABSTAIN  ⇒  H1 não pode PROPOSE ou EXECUTE
```

A equivalência é um pré-requisito da simbiose física, não sua conclusão.
