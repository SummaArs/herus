# HERUS — Núcleo Generativo Híbrido e Composição Fundamentada

**Estado:** concluído em host; hardware físico ainda não validado.

**Ramo:** `feat/herus-semantic-compiler`

**Autoria:** Gustavo — `4llbluu3@gmail.com`

**Objetivo desta evolução:** transformar prova simbólica, memória semântica, contrafactuais e planejamento bounded em candidatos locais de resposta, sem confundir geração com autoridade.

## Resumo

O HERUS recebeu um primeiro núcleo generativo híbrido em [`firmware/core/generative_core.c`][5], com contrato público em [`firmware/core/generative_core.h`][6]. A camada recebe uma consulta tipada, um reasoner local, um léxico explicitamente registrado e, opcionalmente, o índice de memória semântica já revisado. A saída pode ser uma resposta direta, uma resposta derivada, uma explicação, uma hipótese contrafactual ou um plano candidato.

A principal mudança conceitual é que o HERUS deixa de tratar “gerar” como produzir uma sequência verbal plausível. Geração, nesta etapa, é **compor componentes conhecidos em uma estrutura nova, limitada e verificável**. A verbalização é uma camada posterior e só ocorre quando a composição tem suporte suficiente e todos os símbolos necessários estão registrados.

A suíte C11 passou em **21/21 casos**. O redteam matou **6/6 mutantes críticos**. A regressão global preservou **276 invariantes de simulação** e terminou com `ALL INVARIANTS HOLD`. Esses números são resultados do host e da bancada virtual determinística; não são acurácia de LLM, WER, latência de MCU, autonomia ou prova física.

> **Resultado central:** o HERUS agora pode produzir novidade sem inventar autoridade, desde que a novidade seja uma composição derivada de fatos, regras, memória revisada ou um plano explicitamente hipotético.

## Por que uma arquitetura híbrida

A literatura recente separa problemas que frequentemente são reunidos sob a palavra “inteligência”. Estudos de generalização sistemática mostram que uma saída nova precisa ser avaliada em combinações e distribuições fora do treino, não apenas em frases parecidas com as observadas [1]. Trabalhos sobre composição mostram que rigidez simbólica e flexibilidade não são a mesma propriedade; um sistema útil deve combinar regras corretas com comportamento não catastrófico quando a situação é ambígua [2].

A literatura de groundedness também alerta que uma resposta correta pode conter sentenças sem suporte verificável [3]. Por isso, o núcleo não aceita fluência como evidência. Finalmente, a pesquisa sobre abstenção organiza a decisão em respondibilidade da consulta, confiança e valores humanos [4]. No HERUS, essa ideia aparece como estados separados para evidência ausente, ambiguidade, conflito, limite, política e símbolos não registrados.

A hipótese de engenharia desta etapa é, portanto, **híbrida**:

| Capacidade | Mecanismo local atual | Limite assumido |
|---|---|---|
| Exatidão relacional | Reasoner simbólico com fatos, regras e prova | Requer símbolos e regras compilados |
| Novidade composicional | Regras de múltiplas premissas e abdução read-only | Não é geração aberta de domínio geral |
| Continuidade pessoal | Ponte entre reasoner e índice semântico temporal | O card já precisa ter passado pela política de memória |
| Planejamento | Busca finita sobre estado tipado | Plano não é execução |
| Linguagem | Linearizador com léxico registrado | Não há léxico livre nem tokenização aberta |
| Segurança | Abstenção, conflito, limite, política e confirmação | A camada superior ainda deve controlar entrada, apresentação e gesto físico |

## Contrato de entrada e saída

O contrato não recebe áudio, transcript, embedding ou prompt arbitrário. Ele recebe `gc_request_t`, que contém um modo de geração, uma consulta simbólica, um orçamento e, no modo de plano, um problema finito. O chamador também pode fornecer um `mse_index_t` de memória semântica revisada e uma geração atual.

O gerador nunca modifica o reasoner do chamador. Para responder, satura uma cópia em `scratch`; para contrafactuais, calcula uma hipótese em estado temporário; para memória, chama `mrb_query`, que importa apenas evidência corrente e não conflitada. O índice original, a memória persistente e as regras do chamador permanecem inalterados.

| Campo de resultado | Significado |
|---|---|
| `kind` | Direto, derivado, composto, contrafactual, plano, desconhecido, ambíguo, contradito, não suportado ou limitado |
| `grounded` | Indica que a resposta percorreu o caminho de memória semântica corrente |
| `evidence[]` | Índices bounded de fatos usados como suporte local |
| `composition` | Status e proveniência retornados pela ponte memória–reasoner |
| `derivation_digest` | Digest numérico da composição observada; não é texto nem segredo |
| `abstain_reason` | Ausência, ambiguidade, conflito, não suporte, orçamento, política ou ausência de plano |
| `authority` | Nenhuma, somente apresentação ou confirmação necessária |
| `requires_confirmation` | Exigência para uma etapa posterior; não é confirmação e não executa nada |
| `response` | No máximo 255 bytes de linguagem realizada a partir de símbolos registrados |

## Capacidades demonstradas

### Resposta direta e derivada

Com os fatos `alice pai bob` e `bob pai cara`, e uma regra de duas premissas, o reasoner produz a relação nova `alice avo cara`. O gerador retorna a forma textual registrada, preserva a evidência e calcula um digest da derivação. A saída é nova em relação aos fatos de entrada, mas não é uma invenção: sua validade depende da regra e das premissas.

### Explicação local

O modo `GC_MODE_EXPLAIN` produz uma apresentação distinta, como `porque alice avo cara`. A palavra “porque” não é uma explicação livre de modelo; ela é um envelope de apresentação para uma conclusão cuja prova continua disponível no resultado. Se o símbolo, a prova ou a evidência não estiverem disponíveis, a camada abstém-se.

### Contrafactual contido

O modo `GC_MODE_COUNTERFACTUAL` usa abdução para propor uma condição faltante, como `se bob pai cara, entao alice avo cara`. A hipótese é marcada como contrafactual, permanece read-only e não entra no reasoner. Isso cria uma forma inicial de raciocínio “e se?” sem transformar imaginação em memória ou autoridade.

### Plano candidato

O modo `GC_MODE_PLAN` chama o planner finito existente. Um plano de ação retorna IDs de ação, custo, nós explorados e contagem de confirmações. Quando uma ação requer confirmação, a saída torna-se `GC_AUTH_CONFIRMATION_REQUIRED`; a geração não pode converter o plano em execução.

### Memória pessoal fundamentada

Quando `request.memory` é fornecido, a resposta passa por `mrb_query`. O teste registra card e receipt opacos, geração atual, expiração e conflito funcional. Uma memória corrente pode fundamentar a resposta e carregar sua proveniência. Uma memória expirada retorna desconhecimento; duas alternativas funcionais conflitantes retornam contradição; nenhum dos dois casos produz texto assertivo.

## Verificação adversarial

A suíte em [`firmware/core/test_generative_core.c`][7] cobre os casos normais e adversariais. A campanha em [`tools/test_generative_core_redteam.py`][8] recompila mutantes contra a mesma suíte oficial.

| Mutante | Controle removido | Resultado |
|---|---|---:|
| `policy-bypass` | Política deixa de bloquear a geração | Morto |
| `unknown-abstention-bypass` | Ausência de evidência deixa de abster | Morto |
| `conflict-abstention-bypass` | Contradição deixa de ser explícita | Morto |
| `unknown-lexeme-invention` | Símbolo não registrado pode ser verbalizado | Morto |
| `plan-confirmation-bypass` | Plano perde exigência de confirmação | Morto |
| `plan-authority-bypass` | Plano parece apresentação sem confirmação | Morto |

O resultado foi:

```text
GEN CORE: 21 pass, 0 fail
GENERATIVE CORE REDTEAM: 6/6 critical mutants killed
```

Esses seis mutantes demonstram que os testes detectam a remoção dos controles selecionados. Não demonstram que todos os possíveis ataques, expressões linguísticas ou combinações de módulos foram cobertos.

## Orçamento host-side

A medição foi feita com `cc -O2 -Wall -Wextra -Werror -std=c11`. O objeto isolado do gerador apresentou **3624 bytes de text, 0 de data e 0 de bss**. O executável de teste, que inclui reasoner, planner, ponte de memória e harness, apresentou **26249 bytes de text, 808 de data e 8 de bss**.

| Medida | Valor | Escopo |
|---|---:|---|
| Objeto `generative_core.c` — text | 3624 bytes | Implementação isolada, relocável |
| Objeto `generative_core.c` — data | 0 bytes | Implementação isolada |
| Objeto `generative_core.c` — bss | 0 bytes | Implementação isolada |
| Executável de teste — text/data/bss | 26249/808/8 bytes | Inclui dependências e harness |
| Resposta | 255 bytes úteis | Limite do contrato |
| Evidência do gerador | 4 raízes | Limite do contrato |
| Nós de derivação declarados | 32 | Limite do contrato |

Os números acima não são orçamento final do ESP32-S3. É necessário repetir a medição no toolchain alvo, incluindo stack, concorrência, watchdog, linkagem, cache, radio task, driver háptico e caminho de áudio local. O tamanho do executável de teste não é o tamanho do firmware.

## O que foi provado

Foi provado no host que o HERUS pode compor relações novas a partir de regras tipadas, explicar uma conclusão, propor uma hipótese contrafactual sem mutar o estado, gerar um plano bounded e fundamentar respostas em memória revisada corrente. Foi provado que expiração e conflito impedem uma resposta assertiva e que seis controles críticos são visíveis para o redteam.

A regressão global, executada por [`prove.sh`][9], preservou os contratos anteriores:

```text
PASS  Generative core composes direct, derived and grounded answers without authority
PASS  Generative core redteam kills policy, grounding, language and confirmation mutants
PASS  276 system invariants hold in simulation (sim/build/herus-sim)
ALL INVARIANTS HOLD — host contracts pass; controlled bench flash may begin, physical gates remain pending.
```

A frase final deve ser interpretada no escopo do pipeline. Ela significa que os contratos host e a simulação determinística passaram. Não significa que a linguagem natural aberta está resolvida, que o HERUS equivale a uma LLM, que o microfone acertará as intenções, que o hardware terá autonomia aceitável ou que a resposta será sempre surpreendente para uma pessoa.

## Limitações e próximo problema

O gerador ainda depende de um léxico fornecido pelo chamador. Ele não recebe português livre e não aprende novos símbolos durante a geração. Não existe ainda uma superfície de diálogo aberta com memória contextual de vários turnos, geração longa, controle de estilo, recuperação distribuída ou comparação com uma LLM externa sob o mesmo contrato. A verbalização atual é deliberadamente pequena para impedir que a fluência esconda uma falha de grounding.

O próximo problema mensurável é **aprendizagem pessoal local sem perda de soberania**. O HERUS precisa aprender associações, preferências e formas de expressão a partir de exemplos autorizados, mas deve provar que uma adaptação não reintroduz evidência revogada, não grava transcriptes proibidos, não desloca a autoridade e não reduz a capacidade de abstenção. A adaptação deve ser um estado versionado, bounded e apagável, não um “treinamento mágico” sem auditoria.

## Referências acadêmicas

[1]: https://arxiv.org/html/2505.13089v1 "Wold et al. — Systematic Generalization in Language Models Scales with Information Entropy"
[2]: https://www.nature.com/articles/s41586-023-06668-3 "Lake e Baroni — Human-like systematic generalization through a meta-learning neural network"
[3]: https://aclanthology.org/2024.findings-naacl.100/ "Stolfo — Groundedness in Retrieval-augmented Long-form Generation: An Empirical Study"
[4]: https://direct.mit.edu/tacl/article/doi/10.1162/tacl_a_00754/131566 "Wen et al. — Know Your Limits: A Survey of Abstention in Large Language Models"
[5]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/firmware/core/generative_core.c "Implementação do núcleo generativo híbrido"
[6]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/firmware/core/generative_core.h "Contrato do núcleo generativo híbrido"
[7]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/firmware/core/test_generative_core.c "Suíte C11 do núcleo generativo"
[8]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/tools/test_generative_core_redteam.py "Redteam do núcleo generativo"
[9]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/prove.sh "Pipeline global de prova do HERUS"
