# HERUS

**A camada do significado.** Acima do transporte, abaixo do idioma.

O HERUS transmite **intenção**, não texto. A unidade é um significado canônico
de **24 bytes** — tipado, fechado e verificável — e essa escolha tem três
consequências que não são funcionalidades, são propriedades:

- **Quem escreve e quem lê não precisam da mesma língua.** Você digita
  `avisa maria cheguei em casa agora`; do outro lado aparece
  `マリアに今家で着いたと伝えて`. Ninguém traduziu nada: o que viajou nunca foi
  texto. Oito idiomas, ~4,3 bilhões de falantes, zero nuvem.
- **O canal virou escolha, não requisito.** 24 bytes cabem no ar de uma sala
  como som, na tela como selo, num toque NFC, numa dwell de LoRa, numa rajada de
  satélite, ou no bolso de quem passa andando na outra direção.
- **O que não cabe vira lacuna tipada, nunca aproximação.** Um compilador que
  adivinha não é um compilador. Recusar é a capacidade central.

> **Experimente agora, sem hardware:** [HERUS Aberto](https://claude.ai/code/artifact/1fe7f409-baf4-4c8f-ab17-138469e3a057)
> — dois telefones, um toca, o outro ouve. Sem internet, sem operadora, sem
> conta, sem servidor. A página reproduz **1.637 vetores do firmware** na
> própria carga e diz em vermelho se um único divergir.

## Por que isto fecha onde dois campos travaram

Não é ideia nova querer transmitir significado em vez de bits. **Comunicação
semântica** é linha central de pesquisa de 6G, mas quase toda implementação é
neural: codificador e decodificador aprendidos juntos, que só se entendem se
compartilharem os mesmos pesos, e cujo comportamento não se verifica.
**Interlíngua** é mais antiga — UNL, AMR — e o veredito da própria literatura é
que a interlíngua universal seria o método ideal de tradução, e ninguém
conseguiu construí-la.

Os dois falham pelo mesmo motivo: **tentaram ser totais.**

O HERUS é o oposto. Fecha o vocabulário em **143 conceitos essenciais**, prova a
canonicidade deles, e transforma o que não cabe em pedido de capacidade com
endereço. **O que faz funcionar é exatamente aquilo que os antecessores se
recusaram a fazer — recusar.**

Resultado: 6 KB de código de compilador, roda numa bateria de moeda, e as duas
pontas precisam do mesmo pacote de 80 KB em vez dos mesmos pesos de rede.

## A pilha

```
   pessoa                     (qualquer língua: fala, toque, glifo)
      |  Babel.compile         143 conceitos × 8 idiomas, recusa tipada
   HIR — 24 bytes canônicos de SIGNIFICADO
      |  Aether / Ladder       som, luz, BLE, LoRa, malha, satélite, custódia
   HIR — os mesmos 24 bytes
      |  Babel.render          telegráfico, natural, e reconhecível de volta
   pessoa                     (outra língua: voz, háptica, glifo)
```

| camada | o que resolve |
|---|---|
| **[Loom](docs/25-significado/201-HERUS-LOOM-O-TEAR.md)** | o tear: uma especificação `.hlx.json`, quatro artefatos gerados, dez invariantes de portão. Acrescentar um idioma ou um domínio é escrever dado, não editar firmware. |
| **[Babel](docs/25-significado/202-HERUS-BABEL-INTERLINGUA-FECHADA.md)** | a interlíngua fechada. Frases diferentes, línguas diferentes, o **mesmo** digest byte a byte. |
| **[Aether](docs/25-significado/203-HERUS-AETHER-SOM-E-LUZ.md)** | 33 bytes que atravessam o ar como 16-FSK e a tela como selo 18×18. Reed-Solomon conserta dois bytes; o CRC-32 é a última palavra. |
| **[Herald](docs/25-significado/106-HERUS-HERALD-COMPILADOR-DE-INTENCAO.md)** | o compilador de intenção original, congelado, cujo corpus Babel reproduz idêntico. |
| **[Ladder](docs/25-significado/107-HERUS-LADDER-ESCADA-DE-PORTADORES.md)** | onze portadores; um significado cabe em 11 de 11, quatro segundos de fala cabem em 8. |
| **[Drift](docs/25-significado/108-HERUS-DRIFT-ALCANCE-NO-TEMPO.md)** | quando não há canal agora, o significado viaja com quem passa. Custódia sem leitura. |
| **[Aura](docs/25-significado/109-HERUS-AURA-PRESENCA-PRIVADA.md)** | quem está por perto, sem servidor, sem conta, sem localização, sem identificador estável. |
| **[Keel](docs/25-significado/110-HERUS-KEEL-SOBERANIA-MEDIDA.md)** | 1.028 bytes de estado para a via cognitiva inteira, e exatamente quatro ações para o Core. |

**A pessoa continua no comando.** Nenhuma camada pode persistir memória, enviar
significado ou criar HCP sem confirmação física. Isso valia antes e continua
valendo palavra por palavra depois de tudo o que foi acrescentado.

## O balanço

**Provado, com o número ao lado:**

| evidência | número |
|---|---|
| ida-e-volta: todo significado alcançável × todo idioma | **340.728** |
| C do firmware × referência executável | **681.530** comparações, 0 divergências |
| busca adversarial, 11 invariantes, semente reproduzível | **350.000** tentativas |
| invariantes em C (Babel + Aether) | **59 + 23** |
| mutantes mortos (Babel + Aether) | **16/16 + 12/12** |
| erros de 1 e 2 bytes, exaustivo | **8.415 + 4.224**, todos corrigidos |
| quadros com 3 a 8 bytes errados | **40.000** recusados, **0** entregues errados |
| canal adversarial (ruído, ganho, recorte, deriva, eco, tom) | **2.000** ensaios, **0** entregas erradas |
| vetores do firmware reproduzidos no navegador | **1.637**, 0 divergências |
| extensão de terceiro: um pacote de domínio no mesmo portão | **19** invariantes, 6 recusas tipadas |
| suítes independentemente falsificáveis no `prove.sh` | **99** |

**Não provado, e não vou dizer que foi:**

- **Canal simulado não é ar.** Alcance real, reverberação de sala, resposta de
  alto-falante e AGC de microfone seguem pendentes da Fase 0.
- **O perfil aberto da página não é cifrado nem autenticado** — escolha
  declarada. Sigilo é o perfil de rádio, com sessão e chave.
- **Achar o selo numa foto** é outro problema: está provado o codec, não a câmera.
- **143 conceitos** cobrem coordenação essencial, não toda fala humana.
- **Fala, háptica em hardware e bateria** são medições que ninguém fez.
- **A qualidade dos oito idiomas** foi escrita e revisada por uma pessoa. Um
  falante nativo vai achar coisa a corrigir, e o `.hlx.json` existe para que
  isso seja um *pull request* de dados.

## Rodar

```
./prove.sh                 as 99 suítes; sai 1 se qualquer número regredir
./prove.sh --quiet         só os veredictos

python3 tools/loom.py --check          o portão do pacote semântico (L1-L10)
cd firmware && make babel aether       invariantes em C
python3 tools/test_babel_cross.py      o C e a referência são a mesma função
python3 tools/build_web.py             monta a página do navegador
```

**Um número asserido é rumor; um número medido ao lado da sua forma fechada é
resultado.** Rode `prove.sh` antes de confiar em qualquer figura deste
repositório — inclusive nas de cima.

## Onde está o quê

| | |
|---|---|
| [**docs/INDEX.md**](docs/INDEX.md) | 110 documentos em 8 seções, com índice gerado e links provados |
| [a camada do significado](docs/25-significado/200-HERUS-A-CAMADA-DO-SIGNIFICADO.md) | a redefinição, a comparação com o estado da arte, e os onze defeitos que o portão encontrou |
| [especificação do sistema](docs/10-arquitetura/00-HERUS-MASTER.md) | arquitetura geral, protocolo, segurança, energia e limites conhecidos |
| [visão do produto](docs/20-produto/04-PRODUCT.md) | propósito, proposta de valor e direção |
| [guia de construção](docs/60-hardware/03-BUILD-GUIDE.md) | próximos passos de hardware e critérios para interromper ou prosseguir |
| [segurança](SECURITY.md) | o que a criptografia protege hoje e o que depende de integração física |
| [contribuir](CONTRIBUTING.md) | como acrescentar sem quebrar as garantias |

## Estado

**Release candidate pré-hardware.** A arquitetura, os contratos de privacidade,
a confirmação física, a interlíngua fechada, os portadores acústico e óptico, a
memória seletiva cifrada e o modelo de ameaças executável estão implementados e
verificados em host.

A próxima etapa física é a Fase 0, e ela tem **duas** frentes: rádio e energia
em bancada, e a curva de entrega acústica medida em sala contra a curva simulada.
A segunda é nova, e é a que muda o risco do projeto — até aqui toda alegação de
campo dependia de fabricar algo; agora existe um caminho em que a primeira
evidência de campo custa dois telefones e uma tarde.
