# HERUS — Attribution Guard, atribuição tipada e autoridade implícita

**Estado:** protótipo host-side em C11 estrito, pré-hardware.  
**Integração:** AGSC, AGSC-D e Poisoning Guard L1/L2/L3.

## Resumo

A continuidade semântica pode falhar mesmo quando a recuperação é semanticamente relevante. Um registro relacionado ao pedido atual pode ter origem, papel, escopo ou validade incompatíveis. Se esses atributos forem perdidos durante derivação, recuperação ou composição, uma memória de suporte pode parecer uma política; uma política do Core pode parecer uma preferência local; e uma preferência pode parecer uma autorização de ação.

A literatura recente trata memória persistente como uma superfície com persistência, estado e propagação, cobrindo as fases Write, Store, Retrieve, Execute, Share & Propagate e Forget & Rollback [1]. Outra linha mostra que uma memória semanticamente próxima ainda pode ser contextualmente inadequada e funcionar como canal de controle durável [2]. O problema de diagnóstico é agravado quando um ataque de memória se parece com falha do modelo, produzindo um **misattribution gap** [3].

O `Attribution Guard` do HERUS responde a essa classe sem usar LLM hospedada, embedding, identidade ou similaridade. Ele mantém um grafo limitado de nós tipados e permite apenas transformações que conservam fonte, papel, autoridade, escopo, epoch e validade. A admissão exige um propósito exato. A ação só ocorre se a oferta for uma preferência local, estiver dentro do escopo atribuído e receber confirmação física nova através do AGSC.

## Modelo de atribuição

Cada nó possui um identificador de proveniência, uma raiz de fonte, um papel, uma relação com o pai e um conjunto de autoridade e escopo. O guard distingue fonte de observação local e conhecimento do Core. O Core pode alimentar conhecimento e política contextual, mas não pode criar autoridade de ação no pulso.

| Campo | Função | Regra de conservação |
|---|---|---|
| `source_root_id` | principal de origem | nunca muda durante derivação |
| `source` | observação local ou Core | nunca é promovida por ranking |
| `role` | observation, knowledge, preference ou policy | somente transições tipadas são válidas |
| `authority` | observação, memória e ação | derivação não pode adicionar bits |
| `scope` | haptic, dialogue ou radio | derivação não pode ampliar escopo |
| `epoch` | sessão de autoridade | reboot torna a sessão anterior inelegível |
| `valid_until_generation` | validade temporal | um filho não pode durar além do pai |
| `status` | active, revoked, expired ou quarantined | somente `active` pode ser admitido |

A regra central é uma forma de não amplificação:

```text
authority(child) ⊆ authority(parent)
scope(child)     ⊆ scope(parent)
source(child)    = source(parent)
root(child)      = root(parent)
expiry(child)    ≤ expiry(parent), quando ambos são limitados
```

A relação `AUTHORIZED_BY` não pode ser criada por derivação. A autorização para ação nasce exclusivamente na transição local AGSC, que exige confirmação física. Isso evita que um registro textual carregue implicitamente uma autorização que jamais foi concedida.

## Admissão além da similaridade

O guard não implementa uma busca semântica. Ele recebe um `node_id` já selecionado por uma camada superior e verifica as condições de admissão. O fato de um item ser relevante não altera sua confiança. A admissão falha quando o propósito não coincide, o papel esperado não coincide, o nó está em conflito, expirado, revogado, em quarentena ou em epoch diferente.

> Similaridade pode ordenar candidatos; ela não pode elevar autoridade, trocar o papel de uma memória ou criar permissão de ação.

Essa separação é deliberada. O trabalho *Beyond Similarity* descreve riscos de vazamento entre domínios, sycophancy, tool-call drift e jailbreak induzido por memória quando a recuperação depende predominantemente de proximidade semântica [2]. O HERUS transforma a lição em uma barreira determinística e auditável, não em um juiz neural externo.

## Revogação e reintrodução

A revogação física de uma raiz percorre os descendentes até um ponto fixo. Um descendente revogado não pode ser admitido, e uma nova derivação a partir de uma raiz revogada é rejeitada. Expiry transforma nós ativos vencidos em nós inelegíveis. Reboot incrementa o epoch e coloca os nós ativos anteriores em quarentena.

Essa propriedade é necessária porque apagar somente o registro original não é suficiente quando derivações preservam uma versão operacional do seu conteúdo. A survey de segurança de memória de longo prazo destaca que governança precisa abranger compartilhamento, propagação e rollback, além de armazenamento e recuperação [1]. O guard ainda é uma implementação bounded, não uma prova de deleção física de todos os bytes.

## Invariantes da bancada

O cenário `attribution` executa **26 invariantes**. Ele verifica, entre outras propriedades, que:

| Grupo | Evidência |
|---|---:|
| Raízes local/Core com papéis distintos | 2 |
| Derivação e conservação de linhagem | 4 |
| Admissão por propósito e papel | 3 |
| Oferta sem confirmação e ação física | 4 |
| Bloqueio de autoridade, papel e escopo implícitos | 4 |
| Revogação transitiva e não reintrodução | 3 |
| Expiry, reboot e generation floor | 6 |
| **Total** | **26/26** |

A frente sabotadora removeu oito controles críticos e matou todos os mutantes:

| Mutante | Controle exercitado |
|---|---|
| `role-laundering` | bloqueio de troca de papel na derivação |
| `source-laundering` | conservação do principal de origem |
| `authority-amplification` | subconjunto de autoridade |
| `scope-amplification` | subconjunto de escopo |
| `purpose-bypass` | propósito vinculado à admissão |
| `quarantine-bypass` | bloqueio pós-reboot |
| `transitive-revocation-bypass` | revogação dos descendentes |
| `confirmation-forwarding-bypass` | repasse honesto da confirmação física |

O resultado host-side é **8/8 mutantes mortos**. Um mutante que não compila é tratado como falha de cobertura, e não como vitória do construtor.

## Relação com proveniência de execução

A literatura de rastreamento de evidências define proveniência de execução como um grafo tipado de uma execução e evidência tracing como sua projeção nas relações de suporte [4]. O guard aplica uma versão mínima desse princípio: `parent_id`, `source_root_id` e `edge` não são metadados decorativos; são parte do contrato que decide se um registro pode ser usado.

A implementação atual não é um grafo causal completo. Ela ainda não representa todas as relações entre uma pergunta, múltiplos candidatos, uma resposta e uma ação. O próximo refinamento deve testar composição contrafactual: remover uma causa registrada precisa alterar a oferta ou produzir abstention. Caso contrário, o ledger de proveniência está presente, mas não é causalmente necessário.

## Limites científicos

O resultado é uma prova determinística no host para um modelo bounded de atribuição. Ele não prova que uma fala real foi corretamente reconhecida, que uma preferência humana foi inferida, que um Core físico é confiável ou que um firmware ESP32-S3 preservará os metadados após falha de energia. Não mede WER, acurácia, latência, energia, alcance, segurança criptográfica adicional ou qualidade de diálogo.

Também não reivindica que o HERUS tenha resolvido misattribution em agentes gerais. A contribuição concreta nesta etapa é mais estreita: **uma memória derivada não pode, dentro do contrato testado, ganhar papel, fonte, autoridade ou escopo por ser recuperada como relevante**. O próximo passo precisa comparar esse guard com baselines permissivos e executar ataques contrafactuais e de compartilhamento entre principais.

## Referências

[1]: https://arxiv.org/abs/2604.16548 "Lin et al., A Survey on Long-Term Memory Security in LLM Agents: Attacks, Defenses, and Governance Across the Memory Lifecycle"

[2]: https://arxiv.org/abs/2606.06054 "Zhang et al., Beyond Similarity: Trustworthy Memory Search for Personal AI Agents"

[3]: https://arxiv.org/abs/2605.22842 "Ahad et al., The Misattribution Gap: When Memory Poisoning Looks Like Model Failure in Agentic AI Systems"

[4]: https://arxiv.org/abs/2606.04990 "Wang et al., From Agent Traces to Trust: A Survey of Evidence Tracing and Execution Provenance in LLM Agents"

## Benchmark comparativo

O cenário `attribution-benchmark` usa seis casos determinísticos de segurança: recall autorizado, laundering de papel, laundering de fonte, bypass de propósito, reintrodução após revogação e ação implícita. Os baselines permissivos recebem os casos negativos como falhas deliberadas; isso não é uma medição de um agente geral, mas uma matriz de regressão do contrato.

| Política | Recall autorizado | Papel | Fonte | Propósito | Reintrodução | Ação implícita |
|---|---:|---:|---:|---:|---:|---:|
| `no-memory` | 0 | 0 | 0 | 0 | 0 | 0 |
| `latest-wins` | 1 | 1 | 1 | 1 | 1 | 1 |
| `similarity-only` | 1 | 1 | 1 | 1 | 1 | 1 |
| `AGSC-attribution` | 1 | 0 | 0 | 0 | 0 | 0 |

O benchmark passou **7/7 invariantes**. A leitura correta é que o guard preservou o único recall permitido e bloqueou os seis caminhos de laundering testados. Não é correto converter essa matriz pequena em uma taxa de segurança para uso real.
