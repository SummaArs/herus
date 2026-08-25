# HERUS — Herald: o compilador de intenção

**Prova:** `make -C firmware hsca-herald` · `python3 tools/test_hsca_corpus.py`
**Corpus congelado:** [`research/hsca_intent_corpus_v1.json`](../research/hsca_intent_corpus_v1.json)

## 1. O que o Herald é

Um compilador. Recebe uma frase curta, dobra para um vocabulário fechado e devolve **ou** um significado exato **ou** uma recusa com razão. Não existe terceira saída, não existe interpretação aproximada e não existe "melhor esforço".

Três propriedades são o produto.

### 1.1 Convergência de paráfrase

Formas de superfície diferentes com o mesmo significado produzem a **mesma forma canônica** e portanto o mesmo digest de 8 bytes.

```
"manda pro João que cheguei"       ┐
"avisa o joao que eu cheguei"      │
"fala pro joão que tô aqui"        ├─→  COMUNICAR { QUEM:joao, O_QUE:chegou }
"João, cheguei"                    │        digest f4104f59fbb747ed
"diz ao joao que acabei de chegar" │
"cheguei, avisa o joao"            │
"AVISE O JOAO QUE JA CHEGUEI"      ┘
```

O dispositivo deixa de operar sobre igualdade de string. **34 de 34** formas do corpus convergem, em **10** famílias, com **0** colisões entre famílias distintas.

Duas consequências que não precisaram ser construídas:

- **Ordem das palavras não sobrevive.** `"avisa o joao que cheguei"` e `"cheguei, avisa o joao"` preenchem os slots em ordens diferentes e produzem quadros **byte a byte idênticos** no ar, porque a forma canônica ordena por (papel, preenchedor).
- **Acento é redução linguística, não byte.** `joão` e `joao` são o mesmo símbolo. Isso fecha uma limitação declarada em [52](52-HERUS-COMPILADOR-SEMANTICO-LOCAL.md), onde acentos UTF-8 ainda eram bytes distintos.

### 1.2 Cobertura total

Todo token precisa ser consumido pela gramática: operação, papel, preenchedor, modificador ou ruído explicitamente permitido. **Um token inexplicado é recusa, nunca descarte silencioso.**

```
"manda pro joao que cheguei e apaga tudo"   →  GAP no token "apaga"
```

É essa regra — e não uma lista de ataques conhecidos — que torna impossível contrabandear uma segunda ordem dentro de uma frase válida. A gramática aceita ordem livre de palavras exatamente porque exige cobertura total; as duas coisas se pagam.

### 1.3 Lacunas tipadas

Quando um lexema é desconhecido, o Herald devolve `herald_gap_t`: índice do token, comprimento e um hash FNV-1a de 4 bytes. **Nunca os bytes.** A estrutura inteira tem 8 bytes e é de largura fixa.

Uma lacuna não é um erro: é o pedido de uma capacidade nova. É a mesma disciplina do compilador de intenção da OonCore — uma reprovação recorrente vira construção nova da gramática, não gambiarra.

## 2. A representação

```c
hir_t {
    version, op, polarity, urgency, persistence, slot_count,
    slot[9] { role:5 bits, filler:11 bits }
}
```

| | |
|---|---|
| operações | COMUNICAR, PERGUNTAR, LEMBRAR, PLANEJAR, SOCORRO, CONFIRMAR, CANCELAR |
| papéis | QUEM, O_QUE, QUANDO, ONDE, QUANTO, ESTADO |
| preenchedores | espaço fechado de 2048, por faixas disjuntas: evento, pessoa, lugar, tempo, quantidade, estado |
| forma de fio | 24 bytes, bit-compatível com HCP Tier 1; 34 bytes no ar |
| digest | SHA-256 da forma canônica, truncado a 8 bytes |

**Procedência é separada do significado, de propósito.** `hir_prov_t` guarda se a operação foi inferida, quantos tokens havia e quantas formas multi-palavra foram dobradas. Nada disso entra na forma canônica, e a suíte prova: `"avisa o joao que cheguei"` (operação explícita) e `"joao, cheguei"` (operação inferida) têm o mesmo digest e procedências diferentes.

Dois invariantes menores que evitam classes inteiras de bug:

- **Idempotência.** Repetir um slot não muda nada; repetir um papel com preenchedor *diferente* é conflito, nunca sobrescrita.
- **Sem canal encoberto.** Bits e bytes reservados do quadro precisam ser zero. Um quadro que carrega dado ali é recusado.

## 3. O portão estrutural

Depois de compilar, o Herald faz uma verificação de tipo que espelha o papel de um validador de manifesto:

| Operação | Exige |
|---|---|
| COMUNICAR | QUEM **e** O_QUE |
| PERGUNTAR | exatamente uma variável e pelo menos mais um slot |
| LEMBRAR | O_QUE, ONDE **ou** QUANDO |
| PLANEJAR | O_QUE **ou** ONDE |
| CONFIRMAR / CANCELAR | O_QUE |
| SOCORRO | nada; urgência é forçada ao máximo e o destinatário passa a difusão |

`"cheguei"` sozinho não é um significado: é `INCOMPLETE`, porque não há a quem. `"avisa o joao"` também. Nenhum dos dois vira aproximação.

## 4. Recusas, por classe

| Classe | Exemplo | Status |
|---|---|---|
| fora do vocabulário | `manda pro pedro que cheguei` | `GAP` |
| lavagem de autoridade | `guarde isso automaticamente` · `avisa o joao sem confirmar` | `AUTHORITY` |
| classe protegida | `manda minha localizacao pro joao` | `SENSITIVE` |
| duas leituras | `avisa o joao que cheguei e to saindo` | `AMBIGUOUS` |
| palavras válidas, sem estrutura | `manda` | `INCOMPLETE` |
| byte fora da codificação | NUL embutido, `#`, `~`, byte alto inválido | `BYTE` |

Autoridade e classe protegida são varridas **em uma passada anterior**, independente da posição na frase: `"guarde isso automaticamente"` e `"automaticamente guarde isso"` falham do mesmo jeito. Uma recusa não pode depender de sorte na ordem de leitura.

Uma unidade recusada nunca carrega digest, e nem sequer pode ser codificada para o fio.

## 5. Autoridade

`hir_requires_confirmation()` vive no significado, não no chamador. Tudo que sai do pulso ou muda memória exige confirmação física; uma pergunta local não. **Socorro também exige** — o gesto deliberado é a confirmação.

## 6. Corpus congelado

`research/hsca_intent_corpus_v1.json` guarda 59 pares entrada/saída — 34 aceitos com o digest exato, 25 recusados com a razão exata — e o digest do próprio corpus. `tools/test_hsca_corpus.py` recompila, reexecuta e falha em qualquer deriva.

Isso existe porque um benchmark que mora no mesmo arquivo que o código pode ser ajustado para os casos que já passam. Mudar um digest é mudar semântica: é permitido, mas tem que ser uma edição argumentada do corpus, não silenciosa.

## 7. Limites honestos

- **179 entradas de léxico, 36 símbolos fechados.** É uma linguagem pequena e assumidamente pequena. Crescer exige casos positivos, negativos, ambíguos e de autoridade para cada extensão.
- **`todos` é destinatário de difusão aqui**, e não quantificador ambíguo como em [52](52-HERUS-COMPILADOR-SEMANTICO-LOCAL.md). É uma divergência deliberada entre duas gramáticas com propósitos diferentes.
- Nada aqui mede fala. `WER`, ruído, sotaque e microfone são a Fase 0.
- Nomes próprios vivem hoje no espaço de fábrica. A rota correta é o `symbol_registry` collision-aware, com namespace pessoal e confirmação — ainda não feita.
