# Capacidade e economia computacional

A contribuição `capacity_check.py` foi integrada como uma ferramenta analítica separada do classificador HERUS. Ela calcula o limiar de aceitação de bundles binários e verifica se um limite de capacidade é compatível com o modelo estatístico declarado.

## Resultado auditado

Parâmetros:

- `D = 10240` bits independentes;
- `L = 512` comparações no union bound;
- `epsilon = 1e-3`;
- limiar derivado `theta = 0,52280945`;
- capacidade reivindicada: `K = 308`.

O modelo exige `K` ímpar para maioria estrita. Portanto, `K=308` é inválido para a própria especificação. Além disso, o limite fechado com `z_miss=0` é aproximadamente `305,908`; o maior valor ímpar abaixo dele é `305`.

Os valores analíticos de miss rate são:

| K | Miss rate aproximado |
|---:|---:|
| 305 | 0,4957 |
| 307 | 0,5018 |

O modo `--strict` rejeita a reivindicação `K=308` em vez de arredondar silenciosamente ou declarar segurança.

## Relação com a economia do HERUS

A ferramenta é barata em CPU: depois de definidos `D`, `L` e `epsilon`, o cálculo é constante e não executa treino, inferência ou busca sobre datasets. Ela pode servir como gate de capacidade antes de ativar uma representação comprimida.

Ela não prova que o algoritmo de aprendizado HERUS é mais econômico que um transformer. Para essa afirmação ainda são necessários benchmarks reais de tempo, memória, energia e qualidade contra baselines equivalentes.

## Veredito

A contribuição é útil como contrato analítico e proteção contra overclaim, mas a reivindicação `K=308` deve permanecer bloqueada até ser corrigida para uma capacidade ímpar e para um orçamento de miss rate explicitamente escolhido.
