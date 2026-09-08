# Aether — som e luz

> Dois degraus da escada do Ladder deixaram de ser declarados e passaram a ser
> codec provado. Duas pessoas com dois telefones quaisquer trocam um significado
> do HERUS hoje.

## Por que existe

`ladder.c` responde "quais canais do mundo são largos o bastante para 34 bytes?"
e a resposta é *todos*. Mas toda linha daquela tabela — alcance, latência,
energia — é **declarada de folha de dados**, e `ldr_unmeasured_count()` existe
justamente para que nenhum documento esqueça que nada ali foi medido.

O Aether ataca o outro lado. Dois degraus da escada — **som** e **luz** — não
precisam de rádio, de par, de conta, nem de hardware nenhum que a pessoa já não
tenha no bolso. Antes de qualquer bancada física, o HERUS passa a ser
experimentável.

## O quadro

```
     rádio  (HCP Tier 1):  2 (endereço efêmero) + 24 (significado) + 8 (tag AEAD) = 34 bytes
     aether (perfil aberto): 1 (cabeçalho) + 24 + 4 (CRC-32) + 4 (paridade RS)    = 33 bytes
```

Os dois cabem no **mesmo envelope de 34 bytes**. Um significado que atravessa o
ar de uma sala como som é o mesmo significado, byte a byte, que atravessaria 4 km
de LoRa — muda a roupa, não o conteúdo.

**O perfil aberto não é cifrado e não é autenticado, e isso é escolha
declarada.** Ele troca a tag AEAD por correção de erro, porque o problema do ar
de uma sala não é sigilo, é ruído. Quem quer sigilo usa o perfil de rádio, com
sessão, chave e vínculo. Quem quer *tocar* o HERUS hoje usa este.

## Falhar fechado

Três barreiras em série, e a ordem importa:

1. **Reed-Solomon RS(33,29)** sobre GF(256) corrige até **dois bytes** errados
   em qualquer posição. Padrão de erro fora do alcance é **recusa**, nunca chute.
2. **A reverificação de síndrome** depois da correção: se a correção proposta
   não zera as síndromes, ela é desfeita e o quadro é recusado.
3. **CRC-32** é a última palavra. Um quadro que não fecha o CRC nunca chega ao
   Babel.

O invariante é um só, e é o único que importa num portador:

> **o Aether nunca entrega um significado diferente do que foi enviado.**

Não entregar é aceitável — a pessoa repete o gesto. Entregar outra coisa seria a
pior falha possível do produto inteiro, porque quem recebe agiria sobre uma frase
que ninguém disse.

Medido, exaustivamente onde é finito:

| | |
|---|---|
| padrões de **um** byte errado (33 posições × 255 valores) | **8.415**, todos corrigidos |
| padrões de **dois** bytes errados (528 pares × 8 valores) | **4.224**, todos corrigidos |
| quadros com **3 a 8** bytes errados em posições distintas | **40.000** recusados, **0** entregues errados |

Não há entrelaçador, de propósito: há uma palavra-código e o RS corrige dois
bytes em qualquer posição. Entrelaçar não acrescentaria nada e acrescentaria
código — e código que não paga por si é dívida.

A decodificação também não usa Berlekamp-Massey. Com *t* = 2 o sistema é pequeno
o bastante para ser resolvido direto: uma equação para um erro, um sistema 2×2
para dois. Vinte linhas em vez de duzentas, e cada passo verificável a olho — o
que importa num módulo cuja falha silenciosa entregaria um significado que
ninguém disse.

## Som — 16-FSK alinhado à raia

A raia é a chave do desenho. Com 48 kHz e janela de 1024 amostras, o espaçamento
entre tons é exatamente `48000/1024 = 46,875 Hz`, que é **uma raia inteira da
DFT**. Tons alinhados à raia são **ortogonais na janela**, então o detector não
precisa de FFT nem de filtro: dezesseis Goertzel de 1024 amostras bastam, cada um
em memória constante. É por isso que este modem roda num relógio.

| | |
|---|---|
| taxa de amostragem | 48.000 Hz |
| símbolo | 1024 amostras = 21,33 ms |
| tons | 16 (4 bits/símbolo), raias 40 a 55 |
| faixa | 1875 a 2578 Hz |
| preâmbulo | 4 símbolos, padrão 15,0,15,0 |
| dados | 66 símbolos (33 bytes) |
| duração total | 1,493 s |
| ortogonalidade **medida** | pior vazamento entre raias: **1,3 × 10⁻¹⁷** |

A ortogonalidade é medida, não afirmada — e a primeira versão do invariante
estava **errada**: ela testava se a frequência é um número inteiro de Hz, e não
é (2578,125 Hz não é). A propriedade real é que um número inteiro de ciclos cabe
na janela, e a forma de saber é sintetizar o tom e olhar o vazamento.

A sincronia é uma busca em duas passadas — varredura grossa de 64 em 64 amostras
e refino de 4 em 4 — e o escore é a **margem média** entre a raia esperada e a
melhor rival, não a energia: energia premia ruído alto, margem premia tom limpo.

## O canal adversarial

Sete degradações, sorteadas e combinadas com escala em rampa: ruído branco,
ganho, componente contínua, recorte de saturação, deriva de relógio (±0,2%, por
reamostragem), eco de sala e um tom interferente **dentro da própria banda**.

**2.000 ensaios, 0 entregas erradas.** A curva de entrega:

| SNR | entrega |
|---|---|
| < 0 dB | 71,1% |
| 0 a 6 dB | 98,2% |
| 6 a 12 dB | 99,8% |
| 12 a 20 dB | 98,8% |
| > 20 dB | 100,0% |

E um resultado contraintuitivo que a medição confirmou:

| severidade de recorte | entrega |
|---|---|
| sem recorte | 88,2% |
| até 2× | 96,0% |
| 2× a 4× | 99,5% |
| acima de 4× | 100,0% |

**Recortar ajuda.** Tem explicação física: recortar uma senoide joga energia nos
harmônicos ímpares, e 3 × 1875 Hz = 5625 Hz cai bem fora do banco de tons, então
o limitador preserva — e realça — a dominância do fundamental. Na prática:
telefone pode tocar isso no volume máximo.

### O NaN que quase virou um número publicado

A **primeira** campanha reportou uma anomalia: a faixa ">20 dB" entregava 71,5%,
pior que a faixa de 6 a 12 dB. Investiguei o recorte, e a medição **refutou** a
hipótese. Só então o diagnóstico apareceu: `snr = nan`.

O `gauss()` do modelo de canal fazia `sqrt(-2·log(u1))`, e o `u1` podia ser
exatamente 1,0 — dando raiz de número negativo, NaN, e o NaN contaminava o sinal
inteiro. A faixa ">20 dB" nunca foi uma faixa de SNR: era o balde onde o NaN
caía, porque toda comparação com NaN é falsa.

**O modem estava certo. O modelo de canal estava quebrado.** Corrigido para o
intervalo aberto (0,1), a curva ficou monótona, e uma barreira contra NaN entrou
na campanha para que isso nunca mais passe calado. É o tipo de defeito que teria
me feito publicar um número falso sobre o produto.

### Resultado negativo registrado

Tolerar **um** símbolo de preâmbulo errado foi tentado e medido: quadros sem
sincronia caem de 127 para 0, e entregas certas sobem de 1840 para **1841**. A
tolerância não entrega mais quadro nenhum — só move a recusa do detector de
sincronia para o CRC, e no caminho destrói um diagnóstico que o produto usa:
*"não ouvi nada"* é uma mensagem diferente de *"ouvi e chegou quebrado,
repita"*. Um quadro em dois mil não paga por isso. Revertido, e o número ficou
no comentário para que a próxima pessoa não gaste a tarde de novo.

## Luz — o selo 18×18

O mesmo quadro como imagem. Quatro cantos levam um bloco 2×2; **três são cheios
e um é vazio**, e essa assimetria é a orientação: girar o selo move o canto
vazio, então o decodificador descobre a rotação sem precisar de marca de tempo
nem de máscara. Sobram 308 células para 264 bits; as 44 restantes ficam claras,
disponíveis para uma versão futura sem mexer no que já existe.

O que está provado é o **codec**: bits → células → bits, nas quatro rotações, com
um byte estragado por sujeira sendo corrigido pelo RS. **Achar o selo numa foto**
— perspectiva, iluminação, foco — é outro problema, continua fora do provado, e
está declarado como tal.

## As barreiras são carregadoras

`tools/test_aether_redteam.py` desliga uma barreira por vez e exige que
`test_aether.c` falhe: **12/12 mutantes detectados**.

Uma décima terceira mutação **sobreviveu** e a resposta foi remover código, não
inventar teste: o guard `if (e1 == 0 || e2 == 0)` no caminho de dois erros era
redundante, porque a reverificação de síndrome já recusa o mesmo caso. Código
defensivo que duplica uma barreira mais forte não é profundidade, é ruído.

E o preâmbulo virou carregador por outro caminho: ele **não é barreira de
segurança** — o CRC-32 é — mas carrega um diagnóstico que o produto precisa, e
agora existe um invariante que cobra exatamente isso (tons na banda sem preâmbulo
devem dar `NO_SYNC`, não lixo decodificado).

## Custo

| | |
|---|---|
| código (`aether.c`, `cc -Os`) | 4.441 bytes de `.text` + 841 de dados |
| tabelas GF(256) | 512 bytes, construídas na primeira chamada |
| quadro | 33 bytes |
| som por significado | 1,493 s a 48 kHz |

## O que segue pendente

- **Canal simulado não é ar.** Alcance real, reverberação de sala, resposta de
  alto-falante de telefone e AGC de microfone.
- **A câmera**, para o selo.
- **Consumo** de energia do modem em hardware.

A boa notícia é que a primeira frente da Fase 0 acústica **não precisa de
hardware nenhum**: dois telefones e a página publicada bastam.

## Ferramentas

```
cd firmware && make aether             invariantes do portador
cd firmware && make aether-channel     campanha adversarial de canal
python3 tools/test_aether_redteam.py   uma barreira desligada por vez
python3 tools/build_web.py             monta a página que roda isso no navegador
```

Ver também: [Loom](201-HERUS-LOOM-O-TEAR.md),
[Babel](202-HERUS-BABEL-INTERLINGUA-FECHADA.md),
[a camada do significado](200-HERUS-A-CAMADA-DO-SIGNIFICADO.md),
[Ladder](107-HERUS-LADDER-ESCADA-DE-PORTADORES.md).
