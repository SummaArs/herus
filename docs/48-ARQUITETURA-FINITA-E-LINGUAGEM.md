# Arquitetura finita: semântica controlada e linguagem subordinada

**Status:** decisão arquitetural normativa para o HERUS.

## Decisão

O HERUS não pretende resolver NLU/NLG aberto. O domínio do produto é um vocabulário finito de **cartões de contexto**, intenções, tipos, origens, estados e preenchimentos previamente definidos. O VSA/HDC fica restrito a operações sobre esse domínio:

| Operação | Permitida | Limite |
|---|---:|---|
| `bind` | Sim | Somente símbolos do vocabulário finito e seus papéis tipados. |
| `unbind` | Sim | Somente recuperar componentes de um registro previamente composto. |
| `bundle` | Sim | Somente agregar sinais/candidatos do domínio conhecido. |
| Busca aberta em linguagem natural | Não | Fora do contrato do HERUS. |
| Geração livre de texto ou ação | Não | Não possui autoridade de memória, envio ou decisão. |
| Embeddings de uma vida inteira | Não | Não fazem parte do armazenamento ou do protocolo. |

As famílias AMR, CCG, OWL2/SROIQ, ProbLog, ASP e TAG não fazem parte do roadmap do HERUS. Elas são ferramentas para problemas gerais de representação, inferência ou geração que excedem deliberadamente o escopo finito, privado e embarcado do produto. Não devem ser introduzidas como dependências, camadas de execução, formato de persistência ou critério de maturidade.

## Memória seletiva explicável

A memória seletiva deve ser implementada por **política tipada, máquina de estados e motor leve de regras ou solver sobre domínio finito**. Cada decisão de elegibilidade, revisão, conflito, retenção, expiração, recuperação e remoção precisa apontar para sinais observáveis: tipo, origem, consentimento explícito, confiança, sensibilidade, prazo, identificador e estado de autorização.

O motor não precisa de ontologia geral nem de raciocínio aberto. A explicabilidade vem do conjunto fechado de regras e das razões de decisão, por exemplo: `REVIEW_LOW_CONFIDENCE`, `REVIEW_THIRD_PARTY`, `REJECT_NO_CONSENT`, `MATCH_TYPED_UNIQUE` e `BLOCK_AMBIGUOUS`. Uma regra desconhecida, um campo não canônico ou uma combinação contraditória deve falhar fechado.

## LLM local futura

Uma LLM local pode ser considerada posteriormente como **camada de linguagem no Núcleo**, responsável por interpretar ou verbalizar entradas e saídas já tipadas. Ela não substitui o pipeline finito de política, proposta, autorização, persistência, recuperação ou envio.

A entrada em hardware depende de evidência real, não de sizing host-only:

1. orçamento medido de SRAM, PSRAM, flash, latência, temperatura e energia;
2. pesos e tokenizer identificados, versionados e reproduzíveis;
3. avaliação de erro, ambiguidade, alucinação, prompt injection e degradação offline;
4. adaptador que converta a saída somente em propostas tipadas;
5. ausência de autoridade direta sobre cofre, coleção, rádio, confirmação física ou telemetria.

Até esses gates serem fechados, o sistema deve operar sem fallback generativo e manter a interface local limitada, determinística e explicável.

## Critério de revisão

Qualquer pull request que introduza representação aberta, ontologia geral, solver probabilístico não limitado, geração livre ou chamada direta da LLM a armazenamento/transmissão deve ser rejeitado por incompatibilidade arquitetural. A expansão válida é somente aquela que aumenta o vocabulário finito, preserva contratos canônicos, mantém o custo mensurável e adiciona testes de falha fechada.
