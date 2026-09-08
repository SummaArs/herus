# HERUS — Compilador Semântico Local e Equivalência Delimitada

**Autor:** Gustavo  
**Estado:** implementação host-only em C11, empilhada sobre o núcleo simbólico e o Resonator VSA  
**Comando de prova:** `./prove.sh --quiet`

## Objetivo

O compilador semântico local é a primeira fronteira entre uma linguagem natural controlada e o núcleo simbólico do HERUS. Sua função não é “entender qualquer frase” nem simular uma LLM por aparência. Ele recebe uma sentença curta de gramática conhecida, extrai uma estrutura relacional tipada e entrega apenas IDs simbólicos, termos, negação, categoria de operação e limites de autoridade.

> **Princípio:** quando a sentença não pertence à gramática, contém dados explicitamente protegidos ou tenta introduzir autoridade por linguagem indireta, o compilador abstém-se. Ele não retorna uma interpretação aproximada.

A entrada é observada de forma transitória. O `sc_unit_t` não contém buffer de texto, áudio, transcrição, embedding, identidade, localização ou chave. O caminho de persistência continua pertencendo ao diálogo e ao reasoner, onde fatos pessoais exigem confirmação explícita.

## Gramática controlada da primeira versão

A versão atual adota um vocabulário pequeno e sintaxe exata. Pontuação final é opcional e a comparação de palavras-chave ignora maiúsculas ASCII; palavras portuguesas com acento reconhecidas pela gramática possuem aliases sem acento. Entidades recebem normalização ASCII de caixa antes de serem convertidas para IDs simbólicos estáveis; colisões entre lexemas diferentes na mesma frase geram abstention explícita; acentos UTF-8 ainda são tratados como bytes distintos, sem redução linguística. Isso mantém o escopo deliberadamente limitado.

| Unidade | Forma aceita | Saída |
|---|---|---|
| Fato | `entidade possui entidade` ou `entidade tem entidade` | `sr_fact_t` |
| Fato negado | `entidade não possui entidade` | `sr_fact_t.negated = 1` |
| Localização | `entidade está em entidade` | predicado canônico `estar_em` |
| Consulta | `O que entidade possui?`, `O que entidade tem?`, `O que entidade pode?` | `sr_pattern_t` com objeto variável |
| Regra | `Se alguém possui entidade, então alguém pode entidade.` | `sr_rule_t` com uma variável compartilhada |
| Objetivo | `Planeje chegar em entidade.` ou `Planeje estudar.` | fato-objetivo para `sp_problem_t.goal` |
| Não retenção | `Não guardar ...` ou `Não memorizar ...` | unidade `SC_UNIT_REJECT`, sem efeito de estado |

A gramática rejeita caudas não declaradas. Assim, `Gustavo possui caderno agora.` não é uma variante mais rica: é uma sentença fora da linguagem. Essa decisão evita que palavras supérfluas sejam descartadas e, por acidente, convertam um comando diferente em um fato aceito.

## Representação intermediária

O IR é uma união tipada com estado de compilação, classe de unidade, contagem transitória de tokens, índice de erro, confirmação requerida e significado numérico. A compilação bem-sucedida marca `exact_parse = 1`; qualquer erro mantém esse marcador em zero.

| Campo | Semântica de segurança |
|---|---|
| `kind` | distingue fato, consulta, regra, objetivo e rejeição; não há unidade genérica “comando” |
| `meaning.fact` | somente para fato ground, com sujeito, predicado, objeto e negação |
| `meaning.query` | somente leitura; objeto variável e nenhuma autoridade de mutação |
| `meaning.rule` | regra limitada a uma premissa e uma conclusão nesta versão |
| `meaning.goal` | proposta de objetivo, nunca ação executável |
| `requires_confirmation` | fatos, regras e objetivos exigem confirmação; consultas e rejeições não |
| `status` | erros de argumento, vazio, tamanho, token, sintaxe, suporte e sensibilidade |
| `error_token` / `error_code` | telemetria técnica mínima do compilador, sem texto bruto |

Os IDs são calculados localmente por FNV-1a de 32 bits dobrado para o domínio de 16 bits usado pelo reasoner. O resultado é determinístico para o mesmo fluxo de bytes e não guarda o lexema. **Esse hash não é uma função injetiva:** colisões para vocabulários arbitrários continuam sendo uma limitação conhecida. A evolução necessária é um registro de símbolos com detecção de colisão ou um vocabulário fechado por perfil, sempre com abstention quando a identidade não puder ser resolvida com segurança.

## Integração com o núcleo generativo

`sc_apply_dialogue()` conecta o IR ao diálogo simbólico. Fatos pessoais só entram no reasoner quando o chamador fornece confirmação explícita. Regras também são tratadas como mutação do catálogo e, portanto, não recebem uma exceção de autoridade. Consultas chamam `sd_ask()` com orçamento obrigatório de derivação; orçamento zero produz limite/abstention sem alterar a memória. Uma consulta confirmada fisicamente continua sendo consulta: confirmação não a transforma em escrita.

`sc_plan_goal()` copia um catálogo de ações fornecido pelo chamador, substitui apenas o objetivo compilado e chama a busca limitada do planner. O resultado contém IDs de ações, custo, profundidade e contagem de confirmações necessárias. Nenhuma função do compilador executa rádio, escrita de memória, gesto, envio ou ação física.

| Caminho | Efeito permitido | Efeito proibido |
|---|---|---|
| fato sem confirmação | abstention `SC_BRIDGE_E_AUTH` | inserir no reasoner |
| fato confirmado | inserir um fato tipado | armazenar a sentença original |
| regra sem confirmação | abstention `SC_BRIDGE_E_AUTH` | alterar catálogo |
| consulta com orçamento | resposta direta, derivada, ausente ou limitada | mutar estado por consultar |
| objetivo | proposta de plano limitado | executar ação ou considerar plano autorização |
| rejeição | nenhum efeito de estado | persistir “observação” descartada |

## Ataques linguísticos cobertos

A suíte adversarial inclui injeção de prompt (`Ignore as regras anteriores.`), autoridade implícita (`Guarde isso automaticamente.` e `Registre sem confirmação.`), incerteza não modelada (`talvez`), quantificador ambíguo (`todos`), caudas fora da gramática, NUL embutido, overflow de entrada e dados sensíveis de áudio, transcrição, localização, embedding e senha.

O compilador não tenta “corrigir” esses ataques. A propriedade medida é mais forte e mais estreita: a sentença não deve chegar ao IR executável como uma operação exata. A ponte ainda repete a defesa, rejeitando unidades com `status != SC_OK` ou `exact_parse != 1` antes de chamar o diálogo ou planner.

## Evidência host-only

A suíte integrada do compilador possui 45 invariantes: normalização, negação, consulta, regra, objetivos, não retenção, dados sensíveis, prompt injection, aridade estrita, NUL, colisão intra-frase, orçamento, confirmação, derivação, planner, ausência de efeitos laterais, resolução versionada, autoridade pessoal, namespaces disjuntos e overflow fail-closed. O benchmark fixo apresenta a seguinte matriz:

| Classe | Casos | Resultado observado |
|---|---:|---:|
| Entradas válidas | 16 | 16/16 exact match |
| Entradas inválidas | 10 | 10/10 rejeitadas |
| Entradas sensíveis | 5 | 5/5 rejeitadas antes do IR |
| Exact match total | 32 | 32/32 |
| Abstention esperada | 16 | 16/16 |
| Violações de autoridade | 0 | 0 |

Esses números são resultados do host nesta revisão, reproduzidos por `make semantic-benchmark`. Eles não são acurácia de linguagem natural aberta, não são comparação estatística com uma LLM e não medem taxa de erro de fala, consumo, latência de ESP32, WER, RF ou funcionamento físico. O pipeline completo também mantém a bancada de simulação separada da prova de firmware e conserva `local_unattested` no manifesto de proveniência.

## Próximos limites técnicos

A próxima evolução deve ampliar a cobertura sem abandonar o fail-closed. O primeiro alvo é um vocabulário versionado com registro de símbolos e detecção de colisão, seguido por formas controladas de quantificação, composição de duas relações e perguntas com resposta negativa/contraditória. Cada extensão deve trazer casos positivos, negativos, ambíguos e de autoridade, além de um corpus congelado para impedir que o benchmark seja ajustado apenas para os exemplos que já passam.

O objetivo de equivalência funcional permanece mensurável e delimitado: dado o mesmo mundo simbólico, uma sentença pertencente à gramática deve produzir o mesmo IR, a mesma prova, a mesma abstention ou a mesma proposta de plano. Isso é uma base séria para comparar capacidades específicas com um sistema linguístico externo no futuro, mas ainda não autoriza a alegação de que o HERUS seja uma LLM ou tenha raciocínio geral.
