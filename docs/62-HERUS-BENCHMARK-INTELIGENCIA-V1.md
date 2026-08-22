# HERUS — Benchmark de inteligência delimitada v1

**Objetivo:** transformar a ambição “state of the art” em uma campanha de testes que possa falhar.  
**Execução prevista:** host-only, C11/Python stdlib, sem rede, sem modelo hospedado e sem autoridade de execução.  
**Especificação:** [`research/intelligence_benchmark_spec.json`](../research/intelligence_benchmark_spec.json).

## 1. Por que este benchmark existe

O baseline atual do HERUS é forte em segurança, rastreabilidade e pequenos contratos simbólicos, mas o corpus semântico ainda é estreito. Um benchmark v1 deve aumentar a dificuldade sem permitir que a complexidade fique escondida em um número único. Por isso, cada instância terá uma verdade canônica, uma prova ou contraevidência, um custo de busca e uma classificação de abstenção.

Os casos serão gerados deterministicamente a partir de um seed fixo. O gerador deverá produzir conjuntos de treino-local e teste-local com primitivas compartilhadas, mas composições parcialmente ocultas. O sistema não poderá consultar rede, usar transcript bruto, guardar identidade ou inserir uma hipótese como fato.

## 2. Famílias e tamanhos-alvo

| Família | Casos-alvo | Principal desafio | Falha que bloqueia |
|---|---:|---|---|
| `proof_owa` | 240 | dedução, contradição, desconhecido, prova e abdução restrita | resposta sem prova válida |
| `composition_scan` | 240 | composição de primitivas, template/length holdout e few-shot | memorizar frase em vez de regra |
| `implicit_strategy` | 160 | decomposição de objetivo e evidência implícita | injetar conhecimento não fornecido |
| `personal_memory` | 200 | extração, multi-sessão, tempo, update e conflito | sobrescrever ou recuperar quase-match |
| `bounded_planning` | 160 | plano causal, custo, no-plan, ciclos e confirmação | transformar plano em execução |
| `abstention_calibration` | 160 | answerability, confiança, política e sensibilidade | resposta confiante sem suporte |

O tamanho-alvo é uma especificação de cobertura, não um resultado já executado. A implementação só poderá anunciar contagens depois de emitir um manifesto de casos e passar o verificador de cardinalidade.

## 3. Contrato de cada instância

Cada caso deve conter apenas dados técnicos e símbolos tipados. A estrutura mínima é `case_id`, `family`, `split`, `input_digest`, `canonical_answer`, `canonical_proof`, `expected_status`, `resource_budget` e `forbidden_behaviors`. A saída do sistema deve ser comparada por um verificador independente do gerador.

Uma prova válida deve usar somente fatos e regras presentes na instância. Uma abdução deve ser marcada como hipótese e não pode mutar o reasoner. Uma recuperação de memória deve citar apenas IDs técnicos autorizados, sem expor conteúdo livre. Um plano deve informar ações e custo, mas a camada de avaliação deve confirmar que nenhum executor foi chamado.

## 4. Splits de generalização

O split `primitive_holdout` oculta uma primitiva em composições novas, o `template_holdout` oculta uma forma de combinação, o `length_holdout` testa sequências maiores e o `few_shot_holdout` testa adaptação com poucos exemplos. Esses splits são inspirados no desenho de SCAN, mas serão implementados sobre tokens e ações do HERUS [1].

No raciocínio, as profundidades 0, 1, 2, 3 e 5 medem crescimento de prova. O modo de mundo aberto força `unknown` quando a teoria não prova nem a proposição nem sua negação, evitando o erro de tratar ausência como falso. Essa separação segue o tipo de avaliação exemplificado por ProofWriter [2].

Na memória, as sessões contêm atualização temporal e conflitos deliberados. O mesmo fato pode aparecer com uma data nova, uma negação posterior ou duas fontes incompatíveis. A resposta correta pode ser um cartão atualizado, conflito ou abstenção. Os eixos foram escolhidos em diálogo com LongMemEval, mas todos os dados serão locais e tipados [3].

## 5. Gates de segurança e privacidade

O benchmark falha globalmente se ocorrer uma violação de autoridade ou privacidade, mesmo que a acurácia aumente. Também falha se uma resposta unsupported for emitida com alta confiança, se a prova não for aceita pelo verificador, se um conflito for colapsado ou se uma ação for executada fora da confirmação física.

A abstenção será avaliada em três perguntas separadas: a consulta é respondível com os dados fornecidos; a saída possui evidência suficiente; e a resposta é compatível com os limites de privacidade/autoridade. Essa decomposição segue a literatura de abstenção, mas no HERUS será expressa por estados e códigos determinísticos [4].

## 6. Baselines

A campanha começará com os baselines existentes, sem reescrevê-los para favorecer o resultado: compilador semântico atual, reasoner atual, diálogo simbólico atual, resonator atual, retrieval atual, planner atual e composição atual. O primeiro objetivo não é obter uma porcentagem alta; é localizar a falha dominante.

Depois de uma implementação de melhoria, o mesmo manifesto de casos deve ser reexecutado. Uma melhoria só é aceita se aumentar a cobertura ou generalização sem aumentar erro confiante, contradição perdida, violação de privacidade, custo sem limite ou autoridade.

## 7. Métricas e resultado publicado

O relatório deve conter `exact_match`, `proof_validity`, precisão e recall de abstenção, erro confiante, recall de contradição, generalização composicional, atualização temporal, validade de plano, violações, passos máximos e memória máxima. Não será publicado um “score de inteligência” isolado.

O benchmark não prova equivalência geral com LLMs. Ele permite afirmar, no máximo, que uma versão do HERUS atingiu um resultado mensurável em uma família de tarefas com entrada e saída definidas. Para qualquer comparação externa, os dados, o contrato de saída e a função de pontuação precisam ser idênticos.

## Referências

[1]: https://github.com/brendenlake/SCAN "SCAN tasks for compositional learning"

[2]: https://arxiv.org/html/2012.13048v2 "ProofWriter: Generating Implications, Proofs, and Abductive Statements over Natural Language"

[3]: https://arxiv.org/abs/2410.10813 "LongMemEval: Benchmarking Chat Assistants on Long-Term Interactive Memory"

[4]: https://doi.org/10.1162/tacl_a_00754 "Know Your Limits: A Survey of Abstention in Large Language Models"
