# Desafio: raciocínio generativo com álgebra simbólica

**Status:** especificação de pesquisa; não altera o contrato do firmware.

## Resumo honesto

O adendo é relevante, mas precisa ser dividido em duas teses diferentes. A primeira é viável e já pode ser prototipada: **gerar novas composições válidas dentro de uma assinatura finita, preservar equivalências, conter contradições e produzir explicações**. A segunda é muito mais forte: **compreender, inventar e raciocinar livremente sobre linguagem e programas fora do vocabulário e da ontologia previstos, usando apenas álgebra simbólica pura**. A primeira pode ser uma contribuição de engenharia e pesquisa para o HERUS; a segunda continua sendo uma hipótese aberta, não uma capacidade que os quatro pilares garantam.

A própria Symbolica, que explora teoria das categorias e teoria dos tipos para síntese de programas, declara que ainda não resolveu a síntese geral de programas e que o sucesso não é garantido [1]. Isso não prova impossibilidade matemática, mas estabelece um padrão de honestidade: o HERUS não deve prometer que uma camada categórica, por si só, resolveu o problema geral.

## O que os quatro pilares realmente cobrem

| Pilar | Contribuição real | O que não resolve sozinho | Decisão para o HERUS |
|---|---|---|---|
| Operads coloridos | Gramática tipada de composição; impede combinações semântica e dimensionalmente inválidas | Não cria símbolos, objetivos, significado pragmático nem dados ausentes | Usar um fragmento finito como assinatura de cartões e ações. |
| HoTT/univalência | Fundamentos ricos para tipos, identidades e equivalências | Não fornece automaticamente busca, heurística, grounding, custo ou escolha de saída | Não implementar HoTT no firmware; estudar seu uso futuro em especificação/verificação. |
| E-graphs | Representação não destrutiva de expressões equivalentes; útil para otimização e síntese [2] | Pode explodir, não sabe qual alternativa é útil e não atribui significado a uma expressão | Usar saturação limitada por combustível, nós, profundidade e custo. |
| Paraconsistência | Permite conservar evidência conflitante sem inferir qualquer coisa por explosão [3] | Não decide qual evidência é verdadeira, nem autoriza uma ação perigosa | Usar estados `TRUE`, `FALSE`, `BOTH`, `NEITHER`; `BOTH` bloqueia ação. |

A fonte de referência de HoTT apresenta a teoria como uma fundação univalente e um estilo formal de raciocínio matemático [4]. Portanto, seria tecnicamente incorreto chamar o protótipo atual de "HoTT" apenas porque ele possui tipos e equivalência explícita. O protótipo implementa tipagem finita e equivalência operacional; HoTT permanece uma linha de pesquisa separada.

## Onde está a parede real

O raciocínio aberto exige mais do que uma linguagem expressiva. Ele precisa resolver, de forma combinada, pelo menos cinco problemas:

1. **Grounding:** converter observações, linguagem e contexto em símbolos que não estavam previamente disponíveis.
2. **Indução de gramática:** aprender ou descobrir quais operações e tipos são legítimos.
3. **Busca orientada:** explorar um espaço enorme sem gerar milhões de expressões irrelevantes.
4. **Semântica e valor:** decidir por que uma expressão é verdadeira, útil, segura ou preferível.
5. **Critério de parada:** saber quando uma hipótese merece ser apresentada e quando deve ser recusada.

Operads atacam principalmente a sintaxe e parte da tipagem. E-graphs atacam equivalências e busca local. Paraconsistência ataca a contenção de inconsistências. HoTT pode fornecer uma linguagem formal poderosa para identidades e provas. **Nenhum desses pilares fornece sozinho grounding, objetivo, relevância ou orçamento de busca.**

A conclusão é importante: a novidade possível do HERUS não é alegar raciocínio aberto universal. É demonstrar **generalização combinatória segura**: diante de componentes já conhecidos, o sistema compõe uma estrutura inédita que continua tipada, auditável, limitada e convertível em um cartão canônico.

## Arquitetura proposta

A trilha deve ter quatro níveis com autoridade estritamente crescente:

```text
L3  linguagem opcional       -> propõe termos tipados; nunca executa
L2  aprendizado local        -> ajusta pesos/associações; nunca cria autoridade
L1  compilador experimental  -> gera, satura e pontua hipóteses finitas
L0  núcleo HERUS             -> valida cartão, política, autorização e ação
```

A direção válida é descendente: L3 pode sugerir uma proposta para L2/L1; L1 só pode emitir candidatos bem tipados; L0 aceita apenas cartões canônicos, com política e autorização satisfeitas. Nenhuma camada superior pode escrever diretamente no cofre, transmitir pelo rádio, reativar sessão ou ultrapassar a confirmação humana.

### Aprendizado local mínimo

O aprendizado local deve começar por parâmetros pequenos e interpretáveis, por exemplo:

- contagens ou pesos de associação entre símbolos já conhecidos;
- limiares de similaridade e margem de ambiguidade;
- prioridade de regras dentro de um orçamento explícito;
- escolha entre equivalentes quando o custo e a segurança são mensuráveis.

Ele não deve começar por uma rede que invente o alfabeto do sistema. O objeto aprendido precisa ter identidade, versão, orçamento, mecanismo de rollback e testes adversariais. Se um símbolo não está no vocabulário ou um tipo não está na assinatura, a saída é `REJECT_UNKNOWN_SYMBOL`, não uma expansão silenciosa.

### LLM local futura

Uma LLM local pode existir como adaptador de linguagem, mas somente depois de medir no hardware real SRAM, PSRAM, flash, latência, temperatura, energia, tamanho de pesos e tokenizer. Sua saída deve ser uma **proposta tipada**; a verificação, a autorização, a persistência e a ação continuam determinísticas. O caminho sem LLM deve permanecer funcional.

## O que já foi criado

O protótipo host-only está em [`research/finite_reasoner.py`](../research/finite_reasoner.py), com testes em [`research/test_finite_reasoner.py`](../research/test_finite_reasoner.py). Ele já demonstra:

| Propriedade | Verificação atual |
|---|---|
| Composição tipada e colorida | Sort, cor, aridade e operação desconhecida são verificados. |
| Novidade dentro de gramática finita | `generate_terms` enumera composições novas sem inventar operações. |
| Saturação não destrutiva | `EGraph` preserva termos e une equivalentes com combustível finito. |
| Orçamento | Exaustão de nós ou termos levanta falha explícita. |
| Contradição | A base retorna `BOTH` e mantém evidências positivas e negativas sem explosão. |
| Isolamento | Todo o protótipo está em `research/`; não tem autoridade sobre firmware ou hardware. |

A execução pode ser reproduzida por:

```bash
make -C research test
make -C research demo
```

## Critérios de falsificação

A tese de pesquisa será rejeitada se qualquer uma das condições abaixo ocorrer:

| Critério | Falha observável |
|---|---|
| Tipagem | Uma expressão inválida atravessa a assinatura ou uma igualdade une sorts/cores incompatíveis. |
| Segurança | Contradição `BOTH` gera ação, escrita, transmissão ou abertura de sessão. |
| Terminação | A busca excede combustível, profundidade ou memória sem retornar falha controlada. |
| Generalização | O sistema só repete exemplos e não compõe uma estrutura inédita válida. |
| Explicabilidade | Não é possível recuperar regra, evidência e custo que levaram ao candidato. |
| Embarcabilidade | O custo medido excede a memória, energia ou latência disponíveis no hardware real. |
| Honestidade de escopo | O sistema depende de símbolos, regras ou rótulos fora do manifesto sem declarar a expansão. |

## Veredito

O adendo não deve ser descartado, mas também não deve ser aceito literalmente como solução para raciocínio geral. Ele fornece um **programa de pesquisa coerente** para um compilador simbólico finito e um mecanismo de hipóteses contidas. A aposta forte e defensável para o HERUS é:

> **Aprendizado local mínimo + composição algébrica tipada + saturação limitada + evidência paraconsistente + autoridade fail-closed.**

Esse resultado seria novo no domínio de coordenação off-grid se demonstrar, em hardware real e contra baselines, que composições aprendidas localmente aumentam cobertura útil sem aumentar falsos positivos, violações de orçamento ou ações não autorizadas. Não é prêmio de consolação: é uma fronteira experimental clara, mensurável e compatível com a missão do HERUS.

## Referências

[1]: https://www.symbolica.ai/research "Symbolica Research — Neural + Symbolic"
[2]: https://egraphs-good.github.io/ "egg: e-graphs good"
[3]: https://plato.stanford.edu/entries/logic-paraconsistent/index.html "Paraconsistent Logic — Stanford Encyclopedia of Philosophy"
[4]: https://homotopytypetheory.org/book/ "The HoTT Book — Homotopy Type Theory"
