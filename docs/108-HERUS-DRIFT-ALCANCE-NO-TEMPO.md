# HERUS — Drift: alcance como função do tempo

**Prova:** `make -C firmware hsca-drift`

## 1. A troca

O alcance de rádio do HERUS é limitado por regulação: 400 ms de dwell por canal fazem de SF9 o teto, e o teto urbano são 650 m capsula a capsula ([00-HERUS-MASTER §6](00-HERUS-MASTER.md)). Não há firmware que mude isso.

Mas 34 bytes são baratos o bastante para entregar ao aparelho de um desconhecido, que vai embora, e entrega adiante. **O alcance deixa de ser função da potência e vira função do tempo decorrido.**

## 2. Custódia não é leitura

Quem carrega um pacote tem: um texto cifrado de 24 bytes, um endereço rotativo de 2 bytes que não consegue atribuir, e um identificador de 8 bytes que não consegue inverter.

| Não consegue | Prova |
|---|---|
| ler o significado | abrir com chave alheia falha, e o buffer sai zerado |
| alterar sem destruir | **272 de 272** inversões de um bit rejeitadas |
| saber quem enviou | não existe campo de origem no pacote |
| ligar dois envios da mesma pessoa | endereço e identificador derivam de HMAC por contador |
| transformar custódia em autoridade | abrir falha e o quadro não decodifica para significado |

Em 512 envios consecutivos o endereço de 16 bits repetiu **1 vez** — a taxa de aniversário esperada, não um padrão.

## 3. Três limites, todos duros

| Limite | Valor | Papel |
|---|---:|---|
| saltos | 10 | uma cadeia de portadores parou em 9 repasses na suíte |
| fanout por custódia | 3 | uma malha totalmente conectada de 24 nós saturou e parou |
| expiração | por pacote | um TTL de meia hora **não** cruza 40 km, e a suíte prova isso |

Além disso: capacidade de custódia finita (recusa em vez de descartar o pacote de outra pessoa), entrega consumida uma única vez, e identificador consumido lembrado para que o mesmo significado não volte.

## 4. A simulação

Modelo declarado, determinístico, unidimensional:

```
corredor         40.000 m          origem fixa em 0, destino fixo em 40.000
portadores       N móveis          posição inicial uniforme
velocidade       1 a 13 m/s        sinal alternado, refletindo nas pontas
raio de contato  650 m             o número urbano já orçado, não um novo
passo            60 s              horizonte 12 h
sementes         12 populações     independentes, por ponto
```

| Portadores | Entregas | Tempo médio | Transmissões (semente 0) |
|---:|---:|---:|---:|
| 0 | 0/12 | — | 0 |
| 8 | **12/12** | 7.965 s | 11 |
| 16 | 12/12 | 7.500 s | 21 |
| 32 | 12/12 | 8.640 s | 43 |
| 64 | 12/12 | 8.725 s | 106 |

**Dez saltos de 650 m somam 6,5 km. O corredor tem 40 km. A diferença são pernas.**

## 5. O achado que contraria a intuição

Multiplicar por oito os portadores mudou a média em **9%** e as transmissões em cerca de **dez vezes**. Replicação cega gasta orçamento perto da origem sem comprar distância; quem entrega é o tempo. É por isso que o fanout é limitado por projeto.

O caminho até esse número também corrigiu um defeito real. A primeira versão marcava como "vista" toda cópia aceita — então o primeiro portador a receber uma réplica sem saltos restantes ficava envenenado para o resto da vida do pacote. A regra virou **ficar com a melhor réplica**, e a memória permanente ficou só para o que foi consumido.

## 6. Critério de morte

Se, em encontros reais, a mediana de entrega a 5 km ficar acima de 6 h com dez portadores, a custódia é recurso de nicho e deve ser apresentada como tal — não como o alcance do produto.

## 7. O que isto não é

- Não é medição de campo. É um modelo de mobilidade declarado; demonstra o mecanismo, não prevê uma cidade.
- Não é anonimato de rádio. Impressão digital de camada física é questão de hardware e continua aberta.
- Não é entrega garantida. É entrega provável, com limites explícitos e falha visível.
