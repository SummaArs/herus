# Loom — o tear do espaço semântico

> Uma especificação, quatro artefatos, dez invariantes. O vocabulário deixou de
> ser código e passou a ser dado versionado com portão.

## Por que existe

O léxico do Herald é uma tabela `static const` em `herald.c`: forma de
superfície, classe, papel, símbolo. Escrita à mão, funciona muito bem — **para
um idioma**. Para oito, ela para de funcionar por três motivos independentes:

1. Nenhuma pessoa revisa duas mil formas em C sem errar.
2. Um falante nativo de japonês que quisesse corrigir uma forma teria de abrir o
   firmware, e não vai.
3. As garantias que interessam são **relações entre formas** — nenhuma colide,
   toda ida-e-volta fecha, todo conceito existe em todo idioma. Relação entre
   dados se verifica sobre os dados, não sobre o código que os contém.

Então a fonte da verdade virou `research/loom/<pacote>/*.hlx.json`, e tudo o que
consome significado é **gerado**:

```
research/loom/core/*.hlx.json          <- a fonte da verdade, escrita à mão
        |
        |  tools/loom.py --emit
        |
        +-- firmware/core/loom_core.{h,c}      tabelas C11 do aparelho
        +-- research/loom/core.compiled.json   pacote do navegador
        +-- (tools/babel_ref.py lê o pacote direto, sem gerar)
```

`prove.sh` verifica que os arquivos gerados **são exatamente o que o tear
gera** — não que "não mudaram desde o último commit", que é outra coisa e mais
fraca. Gera de novo, compara o digest, e falha se divergir.

## O formato

Um pacote é um diretório de partes. Uma parte é `grammar` ou `concepts`.

**`grammar`** declara os idiomas (etiqueta, endônimo, escrita, segmentação,
direção, falantes), as sete operações com os verbos de cada idioma, as palavras
de pergunta por papel, os dois níveis de urgência, a negação, os templates de
renderização, e as três listas que definem o que **não** é significado: ruído
permitido, classe protegida e lavagem de autoridade.

**`concepts`** declara os símbolos de um namespace. Cada conceito tem posição
fixa (`n`), identificador, glosa, e as formas de superfície por idioma — a
primeira é a forma de renderização, as outras são paráfrases aceitas.

```json
{ "n": 0, "id": "CHEGUEI", "gloss": "cheguei ao lugar", "frozen": true,
  "lex": {
    "pt": ["cheguei", "to aqui", "estou aqui", "acabei de chegar"],
    "en": ["arrived", "i arrived", "im here"],
    "ja": ["着いた", "到着"] },
  "neg": { "ja": "着いていない", "zh": "没到" } }
```

O campo `n` é explícito e imutável de propósito: **um símbolo nunca muda de
significado.** Um pacote v2 pode acrescentar 60, nunca redefinir 7 — senão dois
aparelhos com versões diferentes concordariam nos bytes e discordariam no que
eles querem dizer, que é a pior falha possível de um protocolo semântico.

### Templates

Um template é uma lista de segmentos. Segmento sem `%PAPEL%` é literal e sempre
sai; segmento com `%PAPEL%` só sai quando aquele papel está presente. Os
segmentos são unidos pelo separador do idioma (`" "` ou `""`) e normalizados.

```json
"pt": { "COMUNICAR": ["%OP%","%QUEM%","%URG%","%NEG%","%O_QUE%",
                      "em %ONDE%","%QUANDO%","%QUANTO%","%ESTADO%"] },
"ja": { "COMUNICAR": ["%QUEM%に","%URG%","%QUANDO%","%ONDE%で","%QUANTO%",
                      "%ESTADO%","%O_QUE%","%NEG%","と","%OP%"] }
```

Repare no `em %ONDE%`: a cola literal viaja *dentro* do segmento, então ela
desaparece junto com o papel ausente. É por isso que o desenho é uma lista de
segmentos e não uma string com marcadores — uma string exigiria regra de
"apagar a preposição órfã", e regra desse tipo é onde bug mora.

### Negação lexical

Em português, inglês e alemão a negação é um marcador solto: *não cheguei*,
*not arrived*, *nicht angekommen*. Em japonês e chinês ela é **morfológica**:
`着いた` + `ない` produz `着いたない`, que nenhum japonês escreveria.

A saída não foi forçar a língua a caber no template. Foi deixar o **dado**
fornecer a forma negativa, com uma classe lexical própria (`LOOM_FILLNEG`) que
carrega preenchimento *e* polaridade — e calar o marcador quando ela é usada,
para não haver negação dupla. Cinquenta e um eventos têm forma negativa em
japonês e chinês; onde ela não existe, o marcador continua valendo e a
ida-e-volta é provada nos dois caminhos.

Isto é uma decisão de arquitetura, não um remendo: **algumas línguas negam por
morfologia, então uma interlíngua precisa deixar os dados fornecerem a
superfície negativa.** O código segue fechado.

## O portão

Um pacote só compila se provar, sobre os próprios dados:

| | invariante |
|---|---|
| **L1** | nenhuma forma de superfície tem dois significados no mesmo idioma |
| **L2** | toda forma está na forma dobrada canônica |
| **L3** | todo símbolo é único e cabe no namespace de 11 bits |
| **L4** | todo conceito existe em todo idioma declarado, ou declara a lacuna |
| **L5** | **ida-e-volta**: `compile(render(h, L), L) = h` |
| **L6** | **convergência translíngue**: renderizar em L₁ e L₂ e compilar cada um dá o mesmo *h* |
| **L7** | os conceitos congelados guardam exatamente as formas pt do `herald.c` |
| **L8** | o corpus HSCA v1 sai idêntico — status e digest |
| **L9** | nenhuma frase renderizada estoura o orçamento de leituras |
| **L10** | as tabelas cabem no envelope declarado |

**L5 é o teorema. Todo o resto existe para que L5 possa ser verdade.**

### Como L5 é verificado

Não por amostragem. Por **enumeração onde é finito e busca adversarial onde não
é** — a mesma doutrina do resto do repositório:

- **símbolos** (1.358 significados): todo conceito, em todo papel legal, em toda
  operação que o aceita. Garante que nenhum símbolo fica sem renderizador nem
  sem forma de volta, em nenhum idioma.
- **estruturas** (1.672): toda combinação de papéis presentes, toda polaridade,
  toda urgência. Garante que nenhum template perde um marcador.
- **pares** (39.561): todo par de papéis, toda combinação de preenchimentos, em
  duas variantes — a **operação mínima**, que deixa os outros papéis vazios, e
  `COMUNICAR` completando o que falta.

Total: **340.728 idas-e-voltas**, e o critério que fez a enumeração crescer duas
vezes é explícito: **se a busca adversarial acha uma classe de defeito, a classe
vira enumeração** — a busca fica livre para procurar a próxima em vez de
reencontrar a mesma.

### Congelados e deltas declarados

O corpus `hsca_intent_corpus_v1.json` está congelado e diz, na própria capa, que
mudar um status é uma mudança semântica que precisa ser **argumentada, nunca
absorvida**. Ampliar o vocabulário mudou duas linhas: `aeroporto` deixou de ser
lacuna porque `LOC.AEROPORTO` passou a existir, e `cheguei ontem` mudou de
`GAP` para `INCOMPLETE` — não falta mais vocabulário, falta destinatário.

O argumento vive em `research/loom/core/frozen_deltas.json`, e o portão falha de
**três** formas: divergência não declarada, delta declarado que não se confirma,
e delta declarado que não acontece mais (entrada morta). Não dá para esconder
regressão ali dentro.

## Estender

Acrescentar **um idioma** é acrescentar uma entrada em `languages`, uma coluna
de formas em cada conceito, um bloco de templates, e as três listas de não-
significado. Zero linhas de C. O portão dirá exatamente onde falta forma e onde
há colisão.

Acrescentar **um domínio** é escrever um pacote de extensão com namespace
próprio dentro da faixa 1536-2047 e rodar o mesmo portão com os dois pacotes.
`research/loom/campo` é o exemplo funcionando, e a seção seguinte mostra o que o
portão aceita e o que ele recusa. Um `.hlx.json` que passa no portão é um ativo
versionável: uma empresa pode manter o próprio vocabulário, com o próprio ciclo
de revisão, sem tocar no firmware e sem poder quebrar a canonicidade de ninguém.

## Provado, não afirmado: um pacote de domínio

A frase acima — "um `.hlx.json` que passa no portão é um ativo versionável" —
era afirmação até esta revisão. Agora existe `research/loom/campo`, um pacote de
domínio para equipe de campo e resgate, e ele prova as duas metades.

**Metade positiva.** Quatorze conceitos num namespace próprio (`CAMPO`, base
1536), **zero linhas de C**, e o mesmo portão:

```
python3 tools/loom.py --pack research/loom/core --pack research/loom/campo --check
LOOM core+campo: PASS L1-L10 — 157 conceitos, 8 idiomas, 370.128 idas-e-voltas, 59/59 corpus
  extensoes    campo
  lacunas      29.400
```

O corpus congelado do núcleo continua 59/59 — instalar um domínio não mexe no
que já estava no ar.

**As 29.400 lacunas são o achado.** O pacote de campo existe em **quatro**
idiomas, não em oito, e **diz isso** em vez de inventar vocabulário técnico de
resgate em japonês que ninguém revisou. Esse caminho — `"gap": ["it","de","ja","zh"]`
— existia no esquema desde o início e **nunca tinha sido exercitado**. Exercitá-lo
expôs um defeito real no C: `babel_render` devolvia `BABEL_E_ARG` (erro genérico
de argumento) onde a resposta certa é `BABEL_E_GAP` (lacuna tipada). Corrigido, e
com invariante próprio.

E o caso que interessa em campo é ainda mais forte: **um aparelho que não
instalou o pacote de domínio** recebe 24 bytes perfeitamente válidos e não
consegue vesti-los em nenhuma língua. A resposta é lacuna com endereço — *qual*
símbolo — e nunca a palavra mais parecida.

**Metade negativa, que é a que importa.** Um mecanismo de extensão que aceita
tudo não é plataforma, é buraco. `tools/test_loom_extension.py` prova 19
invariantes, e seis deles são recusas:

| a extensão que… | é recusada porque |
|---|---|
| redefine `EV` | mudaria o significado de símbolos já no ar |
| põe `CAMPO` na base 600 | invade a faixa de `LOC` |
| usa a forma `cheguei` | colide com o núcleo |
| esquece de declarar a lacuna de `it`, `de`, `zh` | forma faltando que ninguém declarou é defeito de dado, não decisão de produto |
| põe dois conceitos no símbolo 0 | duas coisas no mesmo símbolo |
| traz a forma `Evacuar` | não está na forma dobrada canônica |

### O limite, declarado

O preenchimento tem **11 bits**. O núcleo ocupa 1 a 1535; a faixa **1536-2047**
é a faixa de domínio, e cabem **dois** namespaces de 256. Não há negociação de
pacote na versão 1: dois aparelhos que trocam símbolo de domínio precisam estar
rodando o **mesmo** pacote de domínio.

Isso é uma limitação, não um recurso, e está escrita aqui em vez de ser
descoberta em campo. O que a torna suportável é o comportamento acima: quando o
pacote não coincide, o resultado é lacuna tipada — não silêncio e não
aproximação.

## Custo medido

| | |
|---|---|
| conceitos no núcleo | 143 |
| idiomas | 8 (~4,3 bi de falantes) |
| formas de superfície somadas | 2.232 |
| tabelas C geradas | ~204 KB de flash |
| código do compilador (`babel.c`, `-Os`) | 6.010 bytes |
| pacote do navegador | 80 KB |

Seis quilobytes de código para oito idiomas. O custo está no dado, e dado mora
em flash, que é a coisa mais barata do orçamento.

## Ferramentas

```
python3 tools/loom.py --check          o portão (L1-L10)
python3 tools/loom.py --emit           o portão + gera os quatro artefatos
python3 tools/loom.py --pack research/loom/outro --emit
```

Ver também: [Babel](202-HERUS-BABEL-INTERLINGUA-FECHADA.md),
[Aether](203-HERUS-AETHER-SOM-E-LUZ.md),
[a camada do significado](200-HERUS-A-CAMADA-DO-SIGNIFICADO.md),
[Herald](106-HERUS-HERALD-COMPILADOR-DE-INTENCAO.md).
