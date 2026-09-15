# Symbiotic Intelligence Model v1

## Decisão

O HERUS não ficará preso ao simbolismo puro. O SIM v1 introduz um componente neural local pequeno, determinístico e inspecionável, conectado a uma camada simbólica finita e a um seletor de orçamento do hospedeiro.

O modelo neural classifica um vetor de quatro características com pesos inteiros quantizados. A camada simbólica restringe a saída a `ARRIVE`, `HELP`, `CANCEL` e `UNKNOWN`. A camada simbiótica escolhe entre `SIM-INT8`, `SIM-HDC8` e `SIM-RULES` conforme representação declarada, bytes e passos disponíveis no hospedeiro.

## Fluxo

```text
características locais
        ↓
micro-rede fixa quantizada
        ↓
classe finita + confiança
        ↓
limiar simbólico e proveniência
        ↓
representação que cabe no hospedeiro
        ↓
PROPOSE ou ABSTAIN
        ↓
execução permanece ABSTAIN
```

O SIM não é uma LLM, não é treinamento profundo completo e não é uma alegação de inteligência geral. Ele é o primeiro contrato executável para combinar aprendizado local, significado finito e adaptação ao orçamento do hospedeiro.

## Aprendizado local

`train_local_delta` calcula estatísticas de observação sem alterar os pesos do modelo. Isso é deliberado. A primeira versão não permite que uma amostra altere silenciosamente o classificador usado para autoridade ou execução. Atualização de pesos exigirá um protocolo posterior com versão, evidência, rollback e validação física.

## Limites

O SIM não concede autoridade, não executa efeitos, não aceita labels fora do vocabulário finito e retorna `ABSTAIN` quando falta representação, orçamento, proveniência ou confiança. O desempenho medido no host não é desempenho do ESP32-S3.

No hardware, a primeira tarefa será substituir o vetor de fixture por características reais de microfone, gesto ou sensor, medir RAM, latência, energia e estabilidade, e comparar `SIM-INT8`, `SIM-HDC8` e `SIM-RULES` sob o mesmo protocolo.

## Ponte C11 para o primeiro hospedeiro

O núcleo de inferência foi portado para `firmware/core/sim.c` e `sim.h`. A implementação usa pesos constantes, aritmética inteira, buffers na pilha do chamador e nenhuma alocação dinâmica. O teste `make sim` compila com `-Wall -Wextra -std=c11` e confirma a classificação determinística, a seleção por orçamento, a exigência de proveniência e o bloqueio de execução.

Isso prova apenas compilação e comportamento host-side da ponte C11. O bring-up do ESP32-S3 ainda precisa medir tamanho final, RAM, ciclos, latência, energia e entradas físicas depois de B1/B2.
