# HERUS — Adaptação Pessoal Local e Esquecimento Verificável

**Estado:** concluído em host; sem validação física.

**Ramo:** `feat/herus-semantic-compiler`

**Autoria:** Gustavo — `4llbluu3@gmail.com`

**Escopo:** aprender uma preferência tipada de apresentação, com consentimento, margem, revogação e quarentena de reboot.

## Resumo

O HERUS agora possui um adaptador pessoal local em [`firmware/core/personal_adapter.c`][1]. A primeira capacidade aprendida é deliberadamente pequena: escolher entre estilos de apresentação (`neutro`, `conciso`, `detalhado` e `técnico`) para uma feature tipada. O perfil armazena somente contadores numéricos, versões, estado ativo e tombstones. Ele não recebe texto, áudio, transcript, embedding, identidade ou localização.

A adaptação foi conectada ao núcleo generativo híbrido. Quando existe uma preferência técnica suficientemente estável, a explicação local pode ser apresentada como `derivacao alice avo cara`; quando a adaptação está em quarentena, o gerador volta à forma canônica `porque alice avo cara`. A preferência altera somente a superfície de apresentação. Ela não altera a prova, a memória, a seleção da evidência ou o estado de autoridade.

Os resultados observados foram **12/12 casos do adaptador**, **5/5 mutantes críticos mortos** e **24/24 casos do gerador híbrido**, incluindo os casos de adaptação e grounded memory. Isso ainda não prova aprendizado aberto, entendimento de voz ou equivalência com uma LLM.

> **Alegação permitida:** o HERUS consegue adaptar uma preferência de apresentação tipada sob consentimento explícito e pode apagá-la ou colocá-la em quarentena sem reintrodução silenciosa.

> **Alegação proibida:** o HERUS já aprendeu a personalidade completa, a linguagem da pessoa ou a vida da pessoa.

## Contrato de atualização

A atualização exige simultaneamente uma feature não nula, um estilo válido, origem local e consentimento explícito. Cada sample acrescenta um voto bounded à entrada correspondente; ele não substitui silenciosamente uma preferência anterior. A confiança do sample é apenas uma condição de validade da entrada, não uma prova estatística de verdade.

| Gate | Regra implementada | Falha observável |
|---|---|---|
| Consentimento | `explicit_consent == 1` | A atualização é rejeitada |
| Origem | `local_origin == 1` | Um card externo não se promove a preferência pessoal |
| Capacidade | Máximo de 16 features | O perfil rejeita crescimento sem expulsar entradas |
| Estabilidade | Pelo menos 2 votos e margem mínima de 1 | Predição empatada abstém |
| Revogação | Tombstone persistente na entrada | Novo sample não reintroduz a feature revogada |
| Reboot | Epoch cresce e entradas ativas ficam inativas | O perfil retorna à quarentena |
| Autoridade | Nenhuma | O adaptador só influencia apresentação local |

A escolha de usar votos e margem em vez de uma única observação segue a mesma disciplina do restante do HERUS: uma impressão inicial pode ser útil como evidência, mas não deve virar certeza operacional sem repetição e separação suficiente entre alternativas. A literatura de abstenção também trata confiança e respondibilidade como dimensões separadas, e não como uma única probabilidade verbal [4].

## Esquecimento e reboot

`pa_forget` não apenas zera uma contagem. Ele registra uma entrada tombstone, marca-a como inativa e incrementa uma versão monotônica. Uma atualização posterior para a mesma feature retorna `PA_E_REVOKED`. Isso impede que um caminho de ingestão posterior reintroduza automaticamente uma preferência que a pessoa acabou de remover.

`pa_reboot_quarantine` incrementa o epoch e inativa todas as entradas. O perfil preserva apenas a fronteira de época e os dados necessários para auditoria local; nenhuma preferência ativa volta a operar simplesmente porque o processo reiniciou. A reativação, quando existir, deverá ser uma operação separada e autorizada, vinculada ao ciclo de recuperação do HERUS.

## Integração com geração

O gerador híbrido recebe um ponteiro opcional para `pa_profile_t`. Quando a predição passa os gates, `gc_result_t` informa `adapted = 1` e carrega a predição tipada. A composição e a proveniência permanecem as mesmas. Se a predição abstém, o gerador continua podendo apresentar sua forma canônica, sem fingir que aprendeu uma preferência.

A integração foi testada com a seguinte sequência:

1. Duas amostras locais consentidas treinam o estilo técnico.
2. Uma explicação derivada é realizada com o prefixo `derivacao`.
3. O perfil entra em quarentena por reboot.
4. A mesma prova passa a ser apresentada na forma canônica `porque`.
5. A autoridade permanece `GC_AUTH_PRESENTATION_ONLY` em ambos os casos.

Essa separação é importante para o objetivo do segundo cérebro: personalização pode tornar a interação sutil e agradável, mas não pode reescrever a realidade, transformar uma hipótese em memória ou autorizar um ato.

## Verificação adversarial

O teste normal está em [`firmware/core/test_personal_adapter.c`][2], e o redteam em [`tools/test_personal_adapter_redteam.py`][3]. Os cinco mutantes críticos foram mortos:

| Mutante | Controle atacado | Resultado |
|---|---|---:|
| `consent-bypass` | Remoção do consentimento explícito | Morto |
| `origin-bypass` | Aceitação de origem externa como preferência pessoal | Morto |
| `revocation-bypass` | Reintrodução após tombstone | Morto |
| `tie-confidence-bypass` | Escolha silenciosa em empate | Morto |
| `reboot-quarantine-bypass` | Reativação automática após reboot | Morto |

O resultado registrado foi:

```text
PERSONAL ADAPTER: 12 pass, 0 fail
PERSONAL ADAPTER REDTEAM: 5/5 critical mutants killed
GEN CORE: 24 pass, 0 fail
```

Esses números provam a cobertura dos casos e mutantes selecionados. Eles não provam que toda forma possível de envenenamento, inferência de identidade ou aprendizagem indesejada foi eliminada.

## Limitações atuais

O adaptador ainda não aprende conteúdo semântico, fatos, preferências complexas, estilo linguístico aberto, voz, humor, objetivos ou relações temporais. Ele não transforma exemplos em pesos neurais e não substitui o transfer learning local já medido no repositório. A feature de estilo é um passo de infraestrutura: cria uma superfície segura onde a personalização pode começar sem misturar aprendizagem e autoridade.

O próximo salto necessário é uma adaptação composta, mas ainda tipada: preferências de granularidade de resposta, prioridade de memória, forma de confirmação e tolerância a interrupção. Cada dimensão deverá possuir seu próprio consentimento, versão, revogação e benchmark. A progressão correta é aumentar a expressividade somente depois de provar que o esquecimento continua funcionando.

## Referências

[1]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/firmware/core/personal_adapter.c "Implementação C11 do adaptador pessoal"
[2]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/firmware/core/test_personal_adapter.c "Testes do adaptador pessoal"
[3]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/tools/test_personal_adapter_redteam.py "Redteam do adaptador pessoal"
[4]: https://direct.mit.edu/tacl/article/doi/10.1162/tacl_a_00754/131566 "Wen et al. — Know Your Limits: A Survey of Abstention in Large Language Models"
