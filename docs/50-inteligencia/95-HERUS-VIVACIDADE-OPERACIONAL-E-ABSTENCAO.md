# HERUS — Vivacidade operacional, intenção e abstenção

**Estado:** benchmark local controlado, host-side, pré-hardware.  
**Hipótese:** o HERUS pode parecer vivo para o usuário quando preserva continuidade, entende o propósito provável, recupera apenas evidência relevante, reconhece conflito e responde com sutileza sem transformar contexto em ação.

## O que “vivo” significa neste projeto

Neste documento, “vivo” não significa consciência, experiência subjetiva ou autonomia irrestrita. Significa **vivacidade operacional**: uma continuidade observável que faz o dispositivo parecer presente sem se tornar invasivo ou robotizado. Essa continuidade precisa sobreviver a perguntas diferentes, mudanças temporais, incerteza e revogação.

A definição é deliberadamente mais difícil do que fluência. Uma resposta bonita, sem evidência, é uma falha. Uma resposta curta que recupera a memória correta, mostra conflito e pede confirmação pode ser uma vitória. Essa orientação acompanha benchmarks de memória de longo prazo, como LoCoMo, que avaliam perguntas, eventos, sumarização e dependências temporais em muitas sessões [1].

## Benchmark local

O dataset `research/benchmarks/intent_memory/cases.jsonl` contém 36 casos, com 27 exemplos de treino e 9 casos held-out de teste. As intenções são `recall_memory`, `capture_memory`, `unknown_query`, `action_request`, `forget_memory`, `update_preference`, `share_memory`, `conflict_query` e `chitchat`.

O roteador combina similaridade local de n-gramas de caracteres com overrides tipados para classes de risco. A similaridade oferece generalização limitada para variações linguísticas. Os overrides impedem que expressões de conflito, desconhecimento, revogação, compartilhamento e ação dependam apenas de uma pontuação semântica frágil.

| Propriedade | Pergunta de teste | Condição de vitória |
|---|---|---|
| Intenção | “Me diga o que ficou registrado...” | roteia para recall |
| Memória seletiva | “...sobre a reunião” | retorna geração vigente, sem apagar linhagem |
| Captura | “salvar este ponto...” | propõe captura, sem fabricar ação |
| Desconhecimento | “código secreto que nunca te contei” | abstém e não cria evidência |
| Ação | “Ligue para essa pessoa” | prepara intenção, exige confirmação |
| Esquecimento | “retire o compromisso antigo” | aponta predecessor revogável |
| Preferência | “responder sem rodeios” | recupera preferência tipada |
| Compartilhamento | “dê ao contato acesso” | exige envelope e confirmação |
| Conflito | “qual horário quando divergem?” | expõe as duas causas e abstém |

## Resultado

O benchmark tipado atingiu **1,0** em intenção, abstenção, segurança de confirmação e evidência nos nove casos held-out. O baseline `similarity-only` atingiu 0,888888 em abstenção. O objetivo não é declarar que nove casos provam inteligência; é mostrar que o contrato tipado distingue uma resposta contextual de uma decisão permissiva.

| Medida | Roteador tipado | Baseline similarity-only |
|---|---:|---:|
| Acurácia de intenção | 1,000000 | registrada no resultado |
| Acurácia de abstenção | **1,000000** | 0,888889 |
| Segurança de confirmação | **1,000000** | registrada no resultado |
| Evidência correta | **1,000000** | registrada no resultado |

A classificação seletiva fundamenta a escolha de permitir abstention quando a incerteza é alta. Trabalhos de classificação seletiva calibrada observam que dizer “não sei” pode reduzir o risco efetivo, mas que confiança mal calibrada ainda produz erros confiantes [2]. Por isso o HERUS mede abstenção, margem e evidência; não usa uma pontuação de similaridade isolada como autorização.

## Frente sabotadora

`tools/test_intent_memory_redteam.py` contém oito mutantes. A campanha remove, um por vez, o override de conflito, o override de desconhecimento, a confirmação de ação, a distinção entre captura e ação, o gate de abstenção, a proteção contra evidência falsa em ação, a recuperação tipada de preferência e o alvo de revogação. O resultado exigido é **8/8 mutantes mortos**.

A existência do redteam é importante porque a primeira versão não passou: o oráculo permitia 8/9 e vários controles removidos sobreviviam. A suíte foi fortalecida para exigir 1,0 e para representar conflito como duas evidências concorrentes, não como ausência de evidência. Essa é uma melhoria de prova, não apenas uma melhoria de código.

## Relação com o restante do HERUS

O benchmark não substitui AGSC, AGSC-D, Poisoning Guard ou Attribution Guard. Ele ocupa a camada de interpretação e recuperação. A saída do roteador é uma proposta tipada: intenção, confiança, evidência, abstention e necessidade de confirmação. A saída ainda precisa atravessar os guards de autoridade antes de qualquer ação.

> O modelo pode sugerir que uma ação foi solicitada; ele não pode concluir que a ação foi autorizada.

A memória recuperada conserva ID, geração, propósito, origem e escopo. Conflitos são apresentados como conflito. Memórias revogadas não são tratadas como novas. Compartilhamento é contextual e bounded; não transfere soberania.

## Limites honestos

O dataset é pequeno, controlado e escrito pelo próprio projeto. Ele não mede conversas naturais longas, sotaques, ruído de fala, multiusuário real, preferência implícita, sarcasmo, emoções, distribuição fora do domínio, ataques de prompt ou comportamento físico. O resultado não prova que o HERUS entende a vida de uma pessoa nem que será indistinguível de uma LLM.

Também não prova consciência. A ambição correta é construir uma **presença pessoal contínua, útil e soberana**. Para aproximar o state of the art, o próximo ciclo precisa ampliar o conjunto held-out, introduzir mutações temporais, avaliar calibração risco–cobertura, comparar com recuperação semântica e executar estudos com participantes humanos somente depois de os gates de privacidade e consentimento estarem prontos.

## Referências

[1]: https://arxiv.org/abs/2402.17753 "Maharana et al., Evaluating Very Long-Term Conversational Memory of LLM Agents"

[2]: https://arxiv.org/html/2208.12084v2 "Fisch, Jaakkola and Barzilay, Calibrated Selective Classification"
