# Gate de admissão LLM no roteador universal

O ledger existente do gpt-5.5 contém apenas previsões do holdout MInDS-14. Ele foi produzido como referência zero-shot e não possui uma partição de calibração independente.

## Decisão atual

**Status: BLOCKED**

O gpt-5.5 não entra no roteador universal até existir:

1. um ledger separado de calibração;
2. IDs disjuntos entre calibração e holdout;
3. score de confiança ou regra de aceitação definida antes do holdout;
4. limiar calibrado sem observar o holdout;
5. teste de integridade pareado.

## Por que bloquear

Usar diretamente o resultado do holdout para decidir quando confiar no LLM seria leakage. Também seria incorreto comparar o zero-shot com candidatos treinados e depois usar os rótulos do mesmo holdout para calibrar a decisão.

O gate preserva o resultado já medido — 4,08% de acurácia zero-shot — mas impede que ele seja transformado artificialmente em um candidato roteável.

## Consequência

A referência LLM está registrada, auditada e disponível para a próxima campanha, mas permanece fora da decisão universal. Essa é uma limitação deliberada do HERUS, não uma omissão.
