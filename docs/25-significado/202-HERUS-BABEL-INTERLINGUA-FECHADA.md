# Babel — o significado atravessa a língua

> Eu falo português, o outro lê japonês, e o que viajou foram 24 bytes. Não há
> tradução acontecendo: o significado nunca foi texto.

## O que é

Duas funções e um teorema.

```c
babel_status_t babel_compile(const char *texto, size_t len, uint8_t idioma,
                             babel_unit_t *out);
babel_status_t babel_render(const hir_t *h, uint8_t idioma,
                            char *out, size_t cap, size_t *n);
```

> para todo significado alcançável *h* e todo par de idiomas *L₁, L₂*:
> `compile(render(h, L₁), L₁) = h = compile(render(h, L₂), L₂)`

Concretamente, as oito frases abaixo produzem o **mesmo** digest
`c893750cf238857b` e os **mesmos** 24 bytes:

| | |
|---|---|
| pt | avisa maria urgente nao cheguei em casa agora |
| en | tell maria urgent not arrived at home now |
| es | avisa maria urgente no llegue en casa ahora |
| fr | dis a maria urgent pas arrive a maison maintenant |
| it | avvisa maria urgente non arrivato a casa adesso |
| de | sag maria dringend nicht angekommen in zuhause jetzt |
| ja | マリアに今家で緊急に着いていないと伝えて |
| zh | 告诉玛丽亚紧急的现在家里没到 |

## Interlíngua **fechada** — a palavra que importa é "fechada"

A UNL e o AMR tentaram representar todo significado possível e não fecharam em
trinta anos. A comunicação semântica neural de 6G resolve o problema com pesos
aprendidos que só se entendem entre si e cujo comportamento não se verifica.

Babel representa **143 conceitos essenciais**, prova a canonicidade deles, e o
que não cabe vira **lacuna tipada** em vez de aproximação. É a única razão pela
qual ele fecha. Recusar é a capacidade central, não o modo de falha.

## Diferença de método em relação ao Herald

O `herald.c` casa da esquerda para a direita, o mais longo primeiro, e aceita a
primeira leitura que fecha. Babel **enumera todas as leituras** (com orçamento) e
só aceita se exatamente uma produzir significado.

Onde o Herald escolheria em silêncio, Babel recusa. E o Herald continua no
repositório, congelado: o portão do Loom prova que o corpus dele sai idêntico,
com dois deltas declarados e argumentados. **Babel é generalização, não
substituição.**

### Uma fronteira, duas escritas

O casamento é uma busca em profundidade sobre bytes, com um predicado de
fronteira por escrita:

- **idioma com espaço**: começo e fim de palavra. `casa` nunca casa dentro de
  `casaco` — e `casaco` é lacuna, não `casa` mais lixo.
- **idioma sem espaço** (ja, zh): a fronteira é a **transição de escrita**. Um
  numeral ou palavra latina embutida é **atômico**, então `10` nunca se lê como
  `1` seguido de `0`.

É por isso que japonês não precisa de um analisador próprio: precisa de um
predicado de fronteira próprio. O mesmo laço serve as oito línguas.

O que a atomicidade do numeral compra de verdade: `15分钟` (quinze minutos)
contém `1` e `5分钟` (cinco minutos). Sem tratá-lo como atômico, a frase tem duas
leituras válidas com QUANDO diferente e viraria `AMBIGUOUS`.

### Dominância — a única regra de preferência, e ela é teorema

Quando duas leituras produzem significado, quase sempre a resposta certa é
recusar as duas. Mas existe uma classe em que recusar está errado, e ela tem
forma exata:

> **ruído nunca acrescenta significado.**

Uma palavra classificada como ruído não contribui papel nem preenchimento. Logo,
se duas leituras diferem apenas porque uma explicou como **ruído** o que a outra
explicou como **significado**, a segunda contém a primeira e a primeira não tem
nada que a segunda perca. Não há o que escolher: uma domina.

`emergencia estou aqui` tem duas leituras — `SOCORRO` sozinho, lendo *estou
aqui* como duas palavras de ruído, e `SOCORRO + CHEGUEI`, lendo a frase como o
evento. A segunda domina, e é obviamente o que a pessoa disse.

Quando as leituras se **contradizem** — mesmo papel com preenchimento diferente,
operação diferente, polaridade diferente — não há dominância e as duas são
recusadas. `avisa joao maria cheguei` continua `AMBIGUOUS`. O alemão `abbrechen
komme spater` também: *komme spater* é ATRASADO e também *komme* + *spater*, dois
eventos diferentes, e nenhum contém o outro.

A ordem é `≼` sobre (operação, polaridade, urgência, persistência, slots), e o
critério é o **máximo único**. Dois máximos distintos são impossíveis — os
candidatos passam por `hir_equal` antes de entrar no vetor, e a ordem é
antissimétrica — e a campanha de mutantes declara esse ramo **excluído por
inalcançabilidade** em vez de fingir que a suíte o mata.

### Recusas tipadas

| status | quando |
|---|---|
| `GAP` | lexema fora do vocabulário. Vem com posição e hash — nunca os bytes. **É um pedido de capacidade.** |
| `SENSITIVE` | classe protegida nomeada, em qualquer posição, **antes** de construir significado |
| `AUTHORITY` | a frase tentou se conceder autoridade |
| `AMBIGUOUS` | duas leituras com significado que se contradizem |
| `INCOMPLETE` | palavras válidas que não formam significado; diz **qual papel** falta |
| `OVERFLOW` | texto, leitura ou orçamento de arestas estourou |
| `BYTE` | byte fora da codificação aceita |
| `LANG` | idioma não existe neste pacote |

A pré-varredura de classe protegida existe para um caso específico: a palavra é
recusada **mesmo quando nenhuma leitura completa existe**. Sem ela,
`avisa joao senha zzqq` sairia como lacuna e a classe protegida passaria batida.

E o orçamento de leituras falha fechado: `avisa joao to aqui to aqui to aqui to
aqui to aqui to aqui` tem 64 leituras, **todas concordando**, e ainda assim
recusa — dentro do orçamento não dá para *provar* unicidade. Falhar fechado vale
mais que estar quase certo.

## A cadeia de confiança

Três elos, e nenhum sozinho basta:

1. **`tools/loom.py --check`** prova as propriedades sobre os dados:
   340.728 idas-e-voltas, corpus 59/59.
2. **`tools/test_babel_cross.py`** prova que o C do firmware e a referência
   executável em Python são a **mesma função**: 681.530 comparações, status,
   digest e 24 bytes, zero divergências.
3. **`tools/test_babel_prova.py`** prova as propriedades sob busca hostil:
   11 invariantes, 350.000 tentativas com semente reproduzível, mais 20.000
   casos aleatórios enviados ao C real.

O elo 2 é o que impede a armadilha clássica: um espelho pode concordar consigo
mesmo e discordar do artefato. Nesta revisão ele pegou exatamente isso — o
espelho recusava `cheguei cheguei` como ambíguo, e o C está certo: repetir o
mesmo preenchimento no mesmo papel não é ambiguidade, é idempotência.

### Os invariantes adversariais

| | |
|---|---|
| ida-e-volta translíngue | 60.000 |
| convergência entre dois idiomas | 40.000 |
| ruído em fronteira de lexema nunca muda o significado | 40.000 |
| corrupção dentro do lexema falha fechado, nunca HIR inválido | 30.000 |
| classe protegida é inescapável em qualquer posição | 20.000 |
| autoridade não se lava em qualquer posição | 20.000 |
| nunca adivinha: recusa tipada ou significado estável | 40.000 |
| byte arbitrário nunca produz HIR inválido | 20.000 |
| compilação é determinística e sem estado | 20.000 |
| todo significado cabe em exatamente 24 bytes | 20.000 |
| dobramento é idempotente e sem espaço solto | 20.000 |

O invariante de ruído merece nota, porque a **primeira versão dele estava
errada**. Ela inseria ruído em posição aleatória e exigia que o significado não
mudasse. Num idioma com espaço isso é verdade. Num idioma sem espaço é falso e
não há conserto: enfiar uma partícula no meio de `15` produz `1` e `5分`, que é
outro número de verdade. Não é ruído, é **corrupção de lexema**, e nenhum
compilador do mundo desfaz isso porque a informação se foi.

O enunciado correto é sobre **fronteira** de lexema, e a corrupção dentro do
lexema tem invariante próprio, mais fraco e honesto: pode virar outro
significado, mas nunca um HIR inválido e nunca um significado que não sobreviva à
própria ida-e-volta.

## Custo

| | |
|---|---|
| código (`babel.c`, `cc -Os`) | 6.010 bytes de `.text` |
| tabelas (`loom_core.c`, 8 idiomas) | ~204 KB de flash |
| pilha por compilação | ~3 KB, sem alocação |
| orçamentos | 160 B de texto, 24 tokens, 64 leituras, 48 lexemas, 4 significados |

## Ferramentas

```
cd firmware && make babel              invariantes em C
python3 tools/test_babel_cross.py      C x referência executável
python3 tools/test_babel_prova.py      invariante + busca adversarial
python3 tools/test_babel_redteam.py    um controle removido por vez
```

Ver também: [Loom](201-HERUS-LOOM-O-TEAR.md),
[Aether](203-HERUS-AETHER-SOM-E-LUZ.md),
[a camada do significado](200-HERUS-A-CAMADA-DO-SIGNIFICADO.md),
[Herald](106-HERUS-HERALD-COMPILADOR-DE-INTENCAO.md),
[HIR e o paradigma HSCA](105-HERUS-HSCA-PARADIGMA.md).
