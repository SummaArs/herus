# HERUS — a camada do significado

> O HERUS deixou de ser um aparelho que transmite significado e passou a ser a
> **camada** onde o significado existe. O relógio é o primeiro corpo dela, não a
> definição dela.

## 1. O que estava pequeno

O HERUS chegou a este ponto com uma catedral de provas em cima de um vocabulário
de brinquedo. Cento e seis documentos, 206 invariantes, cinco camadas HSCA, um
corpus congelado — e, embaixo de tudo, uma tabela escrita à mão em C com **doze
eventos, em um idioma**, e três nomes de contato de demonstração.

A tese sempre esteve certa: *se a unidade de comunicação tem 34 bytes, o canal
deixa de ser requisito e vira escolha, e o alcance deixa de ser função da
potência para virar função do tempo.* Mas a tese estava presa em três lugares:

1. **Um idioma.** O compilador de intenção aceitava português controlado. Um
   HERUS de português é um HERUS brasileiro, e nenhuma pessoa fora daqui tem
   motivo para querer um.
2. **Nenhum canal real.** A escada do Ladder declara onze portadores a partir de
   folha de dados, e `ldr_unmeasured_count()` existe justamente para que nenhum
   documento esqueça que **nenhum deles foi medido**. Ninguém, em nenhum lugar
   do mundo, podia experimentar o HERUS.
3. **Nenhuma extensão possível.** Acrescentar um domínio — aviação, marítimo,
   defesa civil, logística — significava editar C e refazer as provas à mão.
   Uma plataforma que só o autor consegue estender não é plataforma.

Três limites, e o mesmo padrão nos três: **o dado estava dentro do código.**

## 2. A redefinição

O HERUS é a camada que faltava na pilha de comunicação:

```
   pessoa                     (qualquer língua: fala, toque, glifo)
      |  Babel.compile
   HIR — 24 bytes canônicos de SIGNIFICADO         <- a camada
      |  Aether / Ladder      (som, luz, BLE, LoRa, satélite, custódia)
   HIR — os mesmos 24 bytes
      |  Babel.render
   pessoa                     (outra língua: voz, háptica, glifo)
```

Acima do transporte, abaixo do idioma. TCP move bytes e não sabe o que eles
querem dizer. Um mensageiro move texto e obriga as duas pontas a falarem a mesma
língua. Entre os dois faltava uma camada que movesse **intenção** — pequena o
bastante para caber em qualquer canal, tipada o bastante para ser verificada, e
neutra o bastante para que cada ponta a vista com a própria língua.

Três consequências que não são funcionalidades, são propriedades:

- **Tradução é subproduto, não serviço.** Ninguém traduz nada. O significado
  nunca foi texto; o texto é a roupa que ele veste na entrada e na saída, e cada
  lado escolhe a sua. Oito idiomas, ~4,3 bilhões de falantes, zero nuvem.
- **O canal virou escolha.** 24 bytes cabem no ar de uma sala como som, na tela
  como selo, no bolso de quem passa como custódia. Não há requisito de operadora.
- **O vocabulário virou dado.** Acrescentar um idioma ou um domínio é escrever
  JSON e passar num portão, não editar firmware.

## 3. Por que isto funciona onde dois campos falharam

Não é ideia nova querer transmitir significado em vez de bits. **Duas tradições
acadêmicas inteiras tentaram, e as duas travaram no mesmo lugar.**

**Comunicação semântica** é hoje uma das linhas centrais de pesquisa de 6G:
transmitir apenas o que é semanticamente relevante para a tarefa, gastando menos
banda, energia e latência. A literatura é grande e a direção está certa. Mas
quase toda implementação é **neural** — codificador e decodificador aprendidos
em conjunto, que só se entendem se compartilharem os mesmos pesos, e cujo
comportamento não se verifica: prova-se estatisticamente, sobre um conjunto de
teste, e o que acontece fora dele é conjectura.

**Interlíngua** é mais antiga. A UNL, da Universidade das Nações Unidas, e o AMR
tentaram uma representação de significado independente de língua para tradução
automática. Trinta anos depois, o veredito da própria literatura é que uma
interlíngua universal que expresse com precisão todo o significado das línguas
naturais seria o método ideal de tradução — e ninguém conseguiu construí-la.

Os dois campos falham pelo mesmo motivo: **tentaram ser totais.**

O HERUS faz o contrário, e é a única razão pela qual ele fecha:

| | comunicação semântica neural | interlíngua clássica | HERUS |
|---|---|---|---|
| representação | vetor aprendido | grafo aberto | **24 bytes fechados e tipados** |
| cobertura | domínio de treino | toda a linguagem (meta) | **143 conceitos essenciais** |
| o que não cabe | é aproximado | é aproximado | **vira lacuna tipada** |
| verificação | estatística, em teste | manual | **340.728 idas-e-voltas + busca adversarial** |
| custo de execução | GPU ou acelerador | analisador pesado | **6 KB de código, roda numa bateria de moeda** |
| as duas pontas precisam | dos mesmos pesos | do mesmo analisador | **do mesmo pacote de 80 KB** |

A frase que resume: **o que faz o HERUS funcionar é exatamente aquilo que os
antecessores se recusaram a fazer — recusar.** Uma lacuna tipada não é uma
falha; é um pedido de capacidade, com endereço. Um vocabulário fechado não é uma
limitação; é a condição de possibilidade da prova.

## 4. As três camadas novas

Herald, Ladder, Drift, Aura e Keel continuam de pé. Três camadas se juntam a
elas, e cada uma resolve um dos três limites da seção 1.

### Loom — o tear
[docs/25-significado/201](201-HERUS-LOOM-O-TEAR.md)

Uma especificação `.hlx.json`, quatro artefatos gerados. O espaço semântico
deixou de morar no C e passou a morar em dados versionados, com um **portão de
dez invariantes** que um pacote precisa passar antes de existir: colisão de
forma, forma canônica, unicidade de símbolo, cobertura por idioma, **ida-e-volta
total**, **convergência translíngue**, congelados intactos, corpus reproduzido,
orçamento de leituras e envelope de tabela.

O portão não é decoração: nesta revisão ele **encontrou e travou onze defeitos
reais** nos próprios dados — inclusive uma ambiguidade genuína do espanhol
(*mañana* é amanhã e manhã) e uma do japonês (a forma de ESPERA continha a forma
de POUCO). Ver a seção 6.

### Babel — o significado atravessa a língua
[docs/25-significado/202](202-HERUS-BABEL-INTERLINGUA-FECHADA.md)

`babel_compile(texto, idioma)` e `babel_render(significado, idioma)`. Oito
idiomas, duas escritas, duas formas de segmentação. O teorema:

> para todo significado alcançável *h* e todo par de idiomas *L₁, L₂*:
> `compile(render(h, L₁), L₁) = h = compile(render(h, L₂), L₂)`

`avisa maria cheguei em casa agora` e `マリアに今家で着いたと伝えて` produzem o
**mesmo digest**, `a427ad61e9e6f589`, e os mesmos 24 bytes.

### Aether — som e luz
[docs/25-significado/203](203-HERUS-AETHER-SOM-E-LUZ.md)

Dois degraus da escada do Ladder deixaram de ser declarados e passaram a ser
codec provado. Um quadro de 33 bytes — 1 de cabeçalho, 24 de significado, 4 de
CRC-32, 4 de paridade Reed-Solomon — que cabe **dentro do mesmo envelope de 34
bytes** do quadro cifrado de rádio, e atravessa o ar como 16-FSK ou a tela como
selo 18×18. Duas pessoas com dois telefones quaisquer trocam um significado do
HERUS **hoje**, sem internet, sem operadora, sem conta e sem servidor.

## 5. O que é o produto agora

O HERUS One e o Núcleo/Dock continuam sendo o corpo. O que mudou é que o HERUS
deixou de precisar deles para existir.

| público | o que ele compra |
|---|---|
| **pessoa comum** | manda "cheguei" para a mãe, ela lê na língua dela, e funciona no meio do nada. Um botão, uma vibração, nenhuma conta. |
| **profissional de campo** | coordenação curta e inequívoca entre pessoas que não compartilham idioma, sem cobertura móvel e sem rádio de voz. |
| **empresa** | um domínio semântico próprio, compilado com portão, que roda em qualquer canal e cuja recusa é auditável. Um `.hlx.json` é um ativo versionável. |

O que segura a promessa não é a lista de recursos: é que **nenhuma camada pode
persistir memória, enviar significado ou criar HCP sem confirmação física**, e
isso continua valendo palavra por palavra depois de tudo o que foi acrescentado.

## 6. O que o portão encontrou nesta revisão

Um portão que nunca acusa nada não está provando; está decorando. Este acusou.
Onze defeitos reais, e o valor de cada um está no que ele revela:

1. **`generative_core.text = 4110 > 4096`**, uma falha *pré-existente* que
   deixava o `prove.sh` vermelho. Causa raiz: o orçamento compilava com `-O2` —
   otimização de *velocidade* — e comparava contra um orçamento de *memória*.
   Com `-Os`, o mesmo arquivo dá 3442 bytes. O número era rumor.
2. **Quatro colisões de forma** entre idiomas (`zahlung`, `annule`, `para`, `是`).
3. **`mañana`**: em espanhol, *manhã* e *amanhã*. A saída não foi escolher uma —
   foi tirar `la` da lista de ruído, porque em espanhol o artigo é
   semanticamente **carregador** nessa distinção.
4. **Urgência e negação se perdiam** em seis das sete operações: só `COMUNICAR`
   tinha os marcadores no template. A ida-e-volta acusou.
5. **`少し待って`** continha `少し` (POUCO): duas leituras válidas.
6. **`not alone`** em inglês colidia com negação + SOZINHO.
7. **`all good` + `morning`** também se lê como `all` + `good morning` — e
   estava mascarado no portão porque a enumeração preenchia QUEM. A correção foi
   enumerar cada par também na **operação mínima**, que deixa os outros papéis
   vazios.
8. **Meu espelho Python divergiu do C** em `hir_put`: o C é idempotente de
   propósito (repetir o mesmo preenchimento não é ambiguidade) e o espelho
   recusava. A prova diferencial pegou. **O errado era o espelho.**
9. **Numerais colados**: em escrita sem espaço, `grupo 1` seguido de `1 hora`
   renderizava `小组11小时`, e o `11` deixava de ser dois números.
10. **O modelo de canal gerava NaN.** A faixa ">20 dB" da primeira campanha não
    era uma faixa de SNR: era o balde onde o NaN caía. 28% de falha atribuída ao
    modem que era defeito do meu `gauss()`. **Isso teria me feito publicar um
    número falso.**
11. **Duas barreiras não eram carregadoras.** Um guard do Reed-Solomon era
    redundante (a reverificação de síndrome já o subsumia) e foi **removido**; e
    o preâmbulo do modem não é barreira de segurança — o CRC é — mas carrega um
    diagnóstico que o produto precisa ("não ouvi nada" ≠ "ouvi quebrado").

E dois **resultados negativos**, registrados de propósito para que ninguém gaste
a tarde de novo: tolerar um símbolo de preâmbulo errado leva os quadros sem
sincronia de 127 para 0 e as entregas certas de 1840 para **1841** — não entrega
mais nada, só move a recusa e destrói o diagnóstico. E recortar o áudio, ao
contrário do esperado, **ajuda**: entrega sobe de 88% para 100%, porque recortar
uma senoide joga energia nos harmônicos ímpares, fora do banco de tons.

## 7. O balanço

**Provado, com o número ao lado:**

| evidência | número |
|---|---|
| ida-e-volta, todo significado alcançável × todo idioma | **340.728** |
| C do firmware × referência executável | **681.530** comparações, 0 divergências |
| busca adversarial, 11 invariantes, semente reproduzível | **350.000** tentativas |
| invariantes em C (Babel + Aether) | **59 + 23** |
| mutantes mortos (Babel + Aether) | **16/16 + 12/12** |
| padrões de erro de 1 e 2 bytes, exaustivo | **8.415 + 4.224**, todos corrigidos |
| quadros com 3 a 8 bytes errados | **40.000** recusados, **0** entregues errados |
| canal adversarial (ruído, ganho, recorte, deriva, eco, tom) | **2.000** ensaios, **0** entregas erradas |
| vetores do firmware reproduzidos no navegador | **1.637**, 0 divergências |
| extensão de terceiro: pacote de domínio no mesmo portão | **19** invariantes, 6 recusas tipadas |
| núcleo + domínio: idas-e-voltas e lacunas tipadas | **370.128** e **29.400** |
| documentos, links, órfãos na hierarquia | 180, 444, **0** |
| suítes independentemente falsificáveis | **99** |

**Não provado, e não vou dizer que foi:**

- **Canal simulado não é ar.** Alcance real, reverberação de sala, resposta de
  alto-falante de telefone e AGC de microfone seguem pendentes da Fase 0.
- **O perfil aberto não é cifrado nem autenticado.** É escolha declarada, não
  esquecimento: sigilo é o perfil de rádio, com sessão, chave e vínculo.
- **Achar o selo numa foto** — perspectiva, iluminação, foco — é outro problema.
  Está provado o codec, não a câmera.
- **143 conceitos cobrem coordenação essencial**, não toda fala humana.
- **Reconhecimento de fala, háptica em hardware e consumo de bateria** são
  medições que ninguém fez.
- **A qualidade linguística dos oito idiomas** foi escrita por uma pessoa e
  revisada por uma pessoa. Um falante nativo de cada língua vai achar coisa a
  corrigir, e o `.hlx.json` existe exatamente para que corrigir seja um
  *pull request* de dados, não uma refatoração.

## 8. Próximo passo

A Fase 0 física continua sendo a próxima etapa, e agora ela tem **duas** frentes
em vez de uma:

1. **Rádio e energia** — dois devkits, bancada curta, medição de RF, consumo e
   interação, com critérios de interrupção definidos antes da coleta. Sem
   mudança. Ver [guia de construção](../60-hardware/03-BUILD-GUIDE.md).
2. **Ar de verdade** — a curva de entrega do Aether acústico medida em sala, em
   rua e em ambiente com gente falando, contra a curva simulada da seção 7. É a
   primeira medição do HERUS que **não precisa de hardware nenhum**: dois
   telefones e a página publicada bastam.

A segunda frente é nova, e é a que muda o risco do projeto. Até aqui, toda
alegação de campo dependia de fabricar algo. Agora existe um caminho em que a
primeira evidência de campo custa dois telefones e uma tarde.
