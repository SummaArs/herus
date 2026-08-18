# Proposta tipada e invariantes para a inteligência local do HERUS

**Status:** implementado em host; adaptação de hardware ainda pendente.
**Revisão:** proposal-01
**Data:** 17 de agosto de 2026

## 1. Origem e decisão

Dois documentos fornecidos para esta revisão trouxeram duas ideias úteis. O primeiro propõe uma cadeia **especificação compacta → compilador determinístico → validadores → correção dirigida**. O segundo propõe separar observação, representação estrutural, hipótese, evidência e política, com uma restrição de que simulação não seja promovida automaticamente como conhecimento do mundo.

A aplicação correta ao HERUS não é copiar um compilador de manifestos de outro produto nem instalar uma GNN no wearable. A aplicação correta é introduzir uma fronteira explícita entre **saída de um modelo local** e **semântica que o HERUS está autorizado a considerar**:

```text
modelo local não confiável
        │ proposta tipada, versionada e sem texto
        ▼
memory_proposal_compile()
        │ candidato transitório ou ABSTAIN
        ▼
memory_policy_assess()
        │ descarte, revisão ou elegibilidade advisory
        ▼
lifecycle existente com sessão, proveniência e controle físico
        │
        ├── persistência somente por gate humano
        └── comunicação/HCP somente pelo caminho físico já existente
```

Essa é uma adaptação de menor alcance e maior segurança. Ela reaproveita a ideia de compilação, mas não concede ao modelo nenhuma autoridade nova.

## 2. O contrato novo

`firmware/core/memory_proposal.h` define `memory_model_proposal_t`. O registro contém apenas versão do schema, abstention, tipo de memória, escopo, sensibilidade e quatro scores limitados a 0–100. Não contém áudio, transcrição, embedding, identidade, localização, chave, prompt, conteúdo de memória, HCP, comando ou ação.

O resultado é o `memory_candidate_t` já existente. O compilador define `session_authorized` apenas porque recebeu uma sessão de captura ativa e define `explicit_remember` como zero. A confirmação explícita não pode ser fabricada por um modelo. Ela continua sendo uma decisão humana em uma camada posterior.

| Elemento | Regra do compilador |
|---|---|
| Schema | Somente `MEMORY_PROPOSAL_SCHEMA_VERSION` é aceito |
| Sessão | O identificador deve corresponder à sessão de captura ativa |
| Booleano | `abstain` aceita somente 0 ou 1 |
| Abstention | `abstain == 1` limpa a saída e não cria candidato |
| Enumerações | `kind`, `scope` e `sensitivity` devem estar nos domínios fechados |
| Scores | confiança, novidade, valor futuro e consequência devem ser ≤ 100 |
| Confirmação | `explicit_remember` sempre sai como 0 |
| Origem | candidato de modelo recebe `CONTROLLED_INFERENCE`, não `EXPLICIT` |
| Persistência | não existe API de escrita neste módulo |
| Comunicação | não existe API de rádio, HCP ou envio neste módulo |

O compilador limpa a saída antes de cada tentativa e também em todos os caminhos de rejeição. Isso evita que um candidato válido anterior sobreviva a uma proposta inválida ou a uma abstenção.

## 3. O que foi incorporado dos anexos

### 3.1 Compilação em vez de confiança cega

A proposta de um modelo é análoga a uma spec compacta: pode ser produzida por um modelo local no Dock, por um modelo maior no celular ou por um classificador determinístico. O compilador C11 faz somente validações estruturais e transporta o resultado para o contrato de candidato já existente.

O modelo não escreve um cartão de memória, não escolhe um HCP e não chama uma política de transmissão. Ele produz uma hipótese limitada. A política existente decide se a hipótese deve ser descartada ou encaminhada para revisão. A persistência continua separada.

### 3.2 Correção dirigida e fail-closed

A lógica segue o princípio “erro estruturado antes de retentativa”. Schema desconhecido, sessão vencida, enum inválido, score fora da faixa e booleano não canônico são rejeitados sem saída parcial. No futuro, um adaptador de modelo pode receber esse tipo de erro para corrigir a proposta, mas nenhum laço de correção foi autorizado neste módulo.

### 3.3 Perguntar ou abster-se em vez de inventar

`ABSTAIN` é uma saída de primeira classe. Quando o modelo não consegue classificar com segurança, ele não deve escolher a classe mais próxima. A proposta é limpa e nenhum candidato chega ao cofre. Uma baixa confiança pode gerar um candidato marcado como ambíguo, mas a `memory_policy` existente o descarta abaixo do piso de confiança.

### 3.4 Separação entre observação, hipótese e confirmação

O candidato compilado representa uma interpretação controlada, não uma verdade. Sua origem fica marcada como inferência controlada. A confirmação explícita é um fato externo e não é aceita no registro do modelo. A política pode marcar uma saída para revisão ou elegibilidade, mas isso não é persistência.

Esta separação é compatível com a regra de que resultados de simulação não devem ser promovidos como evidência física. Um teste host prova o contrato do compilador; ele não prova a qualidade do modelo, a compreensão da fala, a energia, o alcance ou a segurança de um dispositivo real.

## 4. O que foi deliberadamente rejeitado

O documento sobre SocietyBrain contém ideias interessantes, mas suas alegações de causalidade geral e estado da arte não são aceitas como fatos nesta base. O exemplo usa `sys.settrace` de Python, identificadores derivados de `hash`/`id`, captura incompleta e uma política que retorna ações como `patch_state` e `add_lock`; isso não é um fundamento seguro para autorizar mudanças no HERUS.

Por isso, esta PR não implementa GNN, treino online, autoedição, `patch_state`, `add_lock`, intervenção autônoma, planejamento ou promoção automática de invariantes. Essas funções seriam incompatíveis com o requisito do HERUS de controle humano permanente e, no wearable, seriam também desproporcionais ao orçamento de memória e energia.

Uma futura pesquisa pode estudar um catálogo de invariantes estruturais no Dock, mas ele deverá manter estados separados como hipótese, observação e confirmação, armazenar apenas evidência mínima e nunca executar uma ação sem o mesmo gate físico. Até haver dataset, protocolo e métricas, isso permanece backlog, não capacidade do produto.

## 5. Provas implementadas

A suíte `firmware/core/test_memory_proposal.c` verifica os seguintes caminhos:

| Caminho | Resultado esperado |
|---|---|
| sessão ausente ou cancelada | rejeição e saída limpa |
| schema desconhecido | rejeição antes da compilação |
| `abstain` canônico | nenhum candidato |
| booleano não canônico | rejeição fail-closed |
| tipo semântico inválido | rejeição sem candidato parcial |
| proposta válida | candidato advisory, `explicit_remember == 0` |
| escopo de terceiro ou sensibilidade alta | revisão obrigatória pela política existente |
| confiança baixa | marcação de ambiguidade e descarte pela política |
| métricas | somente contadores numéricos |

A execução é explícita e fica fora do `all`:

```sh
make -C firmware memory-proposal
```

Na execução atual, todos os casos passam e terminam com:

```text
MEMORY PROPOSAL INVARIANTS HOLD — model output is a bounded proposal, not authority.
```

Isso é uma prova host-only do contrato C11. Ainda não é medição de um modelo real nem de um microcontrolador.

## 6. Próximo passo seguro

O próximo trabalho não deve ser permitir que a LLM escreva texto de memória. Deve ser construir um adaptador isolado que converta uma saída estruturada de um modelo em `memory_model_proposal_t`, com digest de modelo, versão de tokenizer/configuração e casos golden. O adaptador deve ser aceito somente depois de passar por `model_lab`, abstention, testes adversariais e medição no alvo.

O celular pode fornecer esse adaptador durante a prototipagem; o Dock pode fornecê-lo localmente depois. O wearable pode permanecer responsável pelo botão, háptica, sessão e confirmação. Se o modelo não estiver disponível, o caminho C11 determinístico continua sendo a referência e o sistema falha para uma experiência mais limitada, nunca para autonomia escondida.

## 7. Limites de evidência

Esta implementação não demonstra acurácia, causalidade, generalização, qualidade de LLM, português, latência física, autonomia, consumo, temperatura, PDR, alcance, WER, segurança de rádio ou resistência a ataques no hardware. Ela demonstra somente uma nova fronteira de software: **uma proposta de modelo pode ser validada e transformada em candidato tipado sem ganhar autoridade sobre memória ou comunicação**.

## Referências de projeto

1. `Proposta-Compilador-de-Intencao-OonCore.docx`, documento fornecido pelo usuário nesta tarefa, versão 2, 14 de agosto de 2026.
2. `pesquisas2.docx`, documento fornecido pelo usuário nesta tarefa, contendo a proposta SocietyBrain vNext e seus limites declarados.
3. [Contrato de extração de candidatos do HERUS](../firmware/core/memory_extract.h).
4. [Política de relevância seletiva do HERUS](../firmware/core/memory_policy.h).
5. [Contrato de diálogo local display-only](../firmware/core/dialogue.h).
