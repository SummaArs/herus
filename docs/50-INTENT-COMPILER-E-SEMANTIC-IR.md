# Intent Compiler, Semantic IR e a evolução do HERUS

**Status:** proposta de arquitetura e registro de contexto; não substitui os contratos normativos do firmware.

## Tese central

O material fornecido revela uma conexão importante: o **Intent Compiler** não é apenas um gerador de JSON para a OonCore. Ele é um primeiro exemplo de uma camada de compilação cognitiva: uma entrada ambígua e compacta é convertida em uma representação intermediária declarativa; um compilador determinístico expande a representação; validadores, referências cruzadas e testes rejeitam o que não pode ser aceito.

No HERUS, essa ideia deve ser transportada sem importar a promessa de NLU aberto. A Semantic IR deve ser uma **representação intermediária semântica finita, tipada e versionada**, que converte eventos de texto, voz, sensores, visão, código ou ambiente em cartões de contexto, propostas, estados e relações canônicas. Ela não é um espaço livre de embeddings nem uma autorização para qualquer modalidade escrever diretamente no cofre ou no rádio.

> **Percepção pode ser neural; significado aceito precisa ser tipado; autoridade precisa ser determinística.**

## O que a proposta do Intent Compiler prova

A proposta do OonCore fornece uma evidência experimental relevante, mas delimitada:

| Evidência | Leitura correta |
|---|---|
| A gramática de manifesto é pequena e fechada | Existe espaço para uma linguagem de spec compacta e um compilador determinístico. |
| O protótipo reproduz a maior parte de um UI manifest real | Estrutura repetitiva pode ser derivada em vez de ser escrita por um modelo. |
| Validadores retornam `issues[]` em milissegundos | Um laço gerar–validar–corrigir pode substituir parte do raciocínio caro. |
| Referências cruzadas exigem uma camada adicional | Validade local de documento não é validade sistêmica. |
| A spec não foi gerada por LLM no experimento | O resultado prova o compilador, não a compreensão da intenção em português. |
| A tazay usa lógica customizada fora do manifesto | A fidelidade estrutural não cobre automaticamente semântica de negócio. |
| Os números dependem de versões e fixtures declaradas | O corpus precisa carregar versão, proveniência, referência e método de comparação. |

O ponto forte é a separação entre **decisão de negócio** e **expansão mecânica**. O modelo ou humano declara o que é específico; o compilador deriva filtros, grupos, relações, widgets, invariantes e convenções. O portão não decide se o negócio foi modelado corretamente; ele decide se a proposta pode ser representada e executada dentro do contrato.

## Semantic IR do HERUS

A arquitetura ampliada deve ser organizada como uma sequência de fronteiras, não como uma rede monolítica:

```text
texto / áudio / visão / sensores / código / ambiente
                         |
                         v
              percepção neural, SSM, CNN ou atenção
                         |
                         v
                 Semantic IR tipada e versionada
                         |
          +--------------+---------------+
          |              |               |
          v              v               v
       HDC/VSA        memória        estado temporal
          |              |               |
          +--------------+---------------+
                         |
                         v
                 núcleo de raciocínio
                         |
                         v
       proposta de ação / resposta / pedido de confirmação
                         |
                         v
       política, autorização, invariantes e autoridade física
```

A representação intermediária precisa ter pelo menos: tipo de evento, origem, confiança, tempo, propósito, entidades referenciadas, relações, operação pretendida, escopo, validade, sensibilidade e evidências. Cada campo deve pertencer a um vocabulário finito ou ser marcado como desconhecido/incerto. O valor desconhecido não pode ser convertido silenciosamente em um valor válido.

### Contrato mínimo da Semantic IR

| Campo | Regra |
|---|---|
| `schemaVersion` | Obrigatório e compatível com o núcleo. |
| `eventKind` | Enum fechado; desconhecido gera rejeição ou revisão. |
| `source` | Origem enumerada e rastreável. |
| `entities` | Identificadores tipados; nenhuma entidade livre vira autoridade. |
| `slots` | Papéis canônicos, com sort e domínio validados. |
| `confidence` | Evidência auxiliar, nunca autorização isolada. |
| `evidence` | Lista de justificativas e observações de origem. |
| `hypothesis` | Pode coexistir com sua negação; estado `BOTH` bloqueia ação. |
| `requestedAction` | Somente proposta; passa por política e confirmação. |
| `ttl` | Expiração obrigatória para contexto transitório. |

## Como o Intent Compiler entra no HERUS

O Intent Compiler do OonCore oferece o padrão para a futura camada `Semantic IR Compiler`:

1. uma percepção ou um modelo propõe uma spec curta, não um manifesto operacional completo;
2. o compilador resolve aliases, tipos, relações, slots e convenções;
3. o validador checa o schema da IR e as referências cruzadas;
4. o motor de regras preserva ou rejeita hipóteses conforme o contexto;
5. o e-graph pode manter equivalências de formulação, sob orçamento;
6. o núcleo transforma somente uma IR aceita em cartão canônico ou pedido de confirmação;
7. a autoridade física decide se alguma ação é possível.

A analogia tem um limite essencial: na OonCore o produto final são manifestos de uma Central; no HERUS, o produto final intermediário é **um evento semântico ou cartão de contexto**, não uma ação. A compilação não pula memória, política, consentimento, trust ou rádio.

## Três gerações

### HERUS 1 — Semantic Computer

Primeiro, demonstrar que diferentes entradas controladas — texto curto, botão, evento de sensor e comando vocal limitado — convergem para a mesma Semantic IR canônica. A meta não é cobertura universal; é equivalência verificável entre modalidades dentro de um vocabulário finito.

O Intent Compiler é o modelo de processo para esta fase: uma entrada compacta é revisável; a expansão é determinística; o portão explica falhas; o artefato aceito pode ser reproduzido byte a byte.

### HERUS 2 — Cognitive Architecture

Depois, conectar IR, memória seletiva, HDC/VSA, estado temporal, e-graph limitado e aprendizado local mínimo. O aprendizado deve ajustar associações e limiares sobre símbolos existentes antes de tentar ampliar o vocabulário. A expansão de vocabulário exige evidência, versão, testes e aprovação.

Nesta fase, a lógica paraconsistente é útil para conservar evidências conflitantes sem explosão. Ela não escolhe a verdade por mágica: o estado `BOTH` deve bloquear ação e solicitar resolução ou confirmação.

### HERUS 3 — General Architecture

Somente depois de medir as duas primeiras gerações faz sentido comparar a arquitetura com Transformers, SSMs, híbridos e outros sistemas em linguagem, visão, áudio, vídeo, código, robótica e agentes. A hipótese científica é desempenho comparável ou superior em famílias definidas, com custo menor e melhor verificabilidade; não basta um diagrama universal.

## Decisões de fronteira

| Questão | Decisão |
|---|---|
| Atenção | Pode ser componente de percepção quando útil; não é proibida nem elevada a princípio universal. |
| SSM/Mamba/CNN/ViT | São frontends substituíveis; o contrato comum é a Semantic IR. |
| VSA/HDC | Opera no vocabulário finito e nos papéis dos cartões; não resolve linguagem aberta. |
| Memória | Continua seletiva, explicável, autorizada e fail-closed. |
| E-graph | Preserva equivalências e hipóteses sob combustível, memória, profundidade e custo limitados. |
| HoTT | Linha de pesquisa para especificação/verificação; não alegar implementação sem um núcleo formal correspondente. |
| LLM local | Futuro frontend/adaptador de linguagem; somente propõe IR tipada, sem autoridade. |
| Código customizado | Sinal de lacuna de gramática, validador ou runtime; deve virar item de capacidade permanente. |
| Ação | Só ocorre após política, autorização, invariantes, validade temporal e gates físicos. |

## Métricas que tornam a ideia científica

A evolução não deve ser avaliada por expressividade retórica. Cada geração precisa medir:

| Métrica | Pergunta |
|---|---|
| Cobertura de convergência | Entradas equivalentes chegam à mesma IR? |
| Rejeição segura | Entradas desconhecidas ou ambíguas são recusadas ou encaminhadas para revisão? |
| Generalização composicional | Novas combinações de símbolos conhecidos funcionam sem exemplos exatos? |
| Fidelidade de compilação | A IR aceita produz a mesma saída canônica em execuções repetidas? |
| Explicabilidade | É possível recuperar regra, evidência, versão e custo? |
| Custo físico | SRAM, flash, energia, latência e temperatura cabem no hardware real? |
| Segurança de autoridade | Alguma hipótese ou saída não validada consegue agir? |
| Evolução | Cada falha recorrente fecha uma lacuna reutilizável de gramática ou validador? |

## Conclusão

O compilador de intenção é a peça que faltava para ligar a visão do Semantic State à disciplina do HERUS. Ele mostra como transformar uma proposta compacta em artefatos determinísticos e verificáveis. A grande oportunidade é generalizar **o padrão do compilador e do portão**, não copiar uma solução de NLU aberta.

A tese para avançar é, portanto:

> **HERUS não precisa ser uma rede que contém todo o significado. Precisa ser uma arquitetura em que diferentes percepções possam propor a mesma representação semântica, e em que somente o núcleo tipado, verificável e autorizado possa transformá-la em memória ou ação.**

## Materiais de origem

- [Proposta do Compilador de Intenção para a OonCore](../research/source_materials/proposta_compilador_intencao_ooncore.docx)
- [Pesquisa sobre o Compilador de Intenção no HERUS](../research/source_materials/herus_pesquisa.pdf)
- [Visão Semantic IR / arquitetura cognitiva](../research/source_materials/visao_semantic_ir_herus.txt)
- [Protótipo de raciocínio finito](../research/FINITE-REASONER-README.md)
- [Desafio do raciocínio generativo](49-DESAFIO-RACIOCINIO-GENERATIVO.md)
- [Baseline bruto da arquitetura](../research/evidence/baseline_program.txt)
- [Baseline bruto do contrato de intenção](../research/evidence/intent_contract_baseline.txt)
