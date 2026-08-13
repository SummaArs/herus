# Consolidação humana — revisão, expiração, conflito, recuperação e remoção

> **Estado da evidência:** este passo prova em host uma máquina de estados de consolidação e suas recusas adversariais. Não prova que um botão, gesto, relógio, tela, eFuse, NVS, flash ou apagamento físico exista no dispositivo final. A afirmação de confirmação física permanece condicionada a um adaptador de hardware e à sua validação em bancada.

O Passo 5 torna a persistência do cartão de memória um **ato de revisão humana limitado no tempo**. Ele não amplia o cartão, não armazena texto, não cria busca semântica e não adiciona inteligência autônoma. A consolidação aceita uma proposta tipada que já passou pela política seletiva, mostra-se como uma etapa separada da captura e só chama o cofre depois de uma confirmação canônica vinculada à mesma sessão física.

| O que a consolidação faz | O que ela não faz |
|---|---|
| Mantém uma proposta tipada em RAM durante uma revisão curta. | Gravar áudio, texto, transcrição, resumo, embedding, identidade, localização ou timestamp. |
| Emite um recibo local somente após confirmação física canônica. | Derivar confirmação de `memory_candidate_t`, fala, diálogo, rádio ou LLM. |
| Encaminha o cartão ao cofre cifrado ou solicita sua recuperação/remoção. | Criar banco de dados, pesquisa semântica, sincronização, HCP ou transmissão. |
| Recusa conflito e falha de cofre em vez de escolher/repetir sozinha. | Resolver contradições, inventar um prazo de retenção persistido ou alegar apagamento físico. |

## 1. Finalidade e posição na cadeia

O NIST Privacy Framework é um instrumento voluntário para ajudar a identificar e gerir riscos de privacidade enquanto produtos e serviços são desenvolvidos [1]. Para o HERUS, isso se traduz em uma regra de desenho, não em declaração de conformidade: uma memória só pode ser mantida para a finalidade estreita de complemento pessoal autorizado e precisa poder ser descartada quando a revisão não se completa.

| Etapa | Estado do dado | Persistência | Autoridade |
|---|---|---:|---|
| Política seletiva | Disposição tipada | Não | Determinística; não escreve. |
| Captura e extração | Janela e candidato tipado | Não | Sessão física prévia; não escreve. |
| Cofre — Passo 4 | Cartão mínimo cifrado | Sim | Exige autorização externa explícita. |
| **Consolidação — Passo 5** | Proposta transitória e recibo de revisão | Só através do cofre | Confirmação física humana. |
| Recuperação semântica — Passo 6 | Ainda não implementada | Ainda não definida | Não recebe autorização de escrita. |

A consolidação não aceita `memory_candidate_t`. A proposta possui deliberadamente a mesma classe de sinais limitados — tipo, escopo, sensibilidade, confiança, origem e razões — mas é um tipo próprio. Essa separação impede que o produto conecte, por engano, a saída de extração diretamente à persistência.

## 2. Máquina de estados

O módulo mantém no máximo uma proposta em RAM. Os únicos estados são `IDLE`, `REVIEWING`, `CONFLICTED`, `EXPIRED` e `FAILED`. Não existe estado “aprovar automaticamente”, fila não limitada, recuperação automática ou tentativa automática após erro.

| Transição | Pré-condição | Efeito | O que nunca ocorre |
|---|---|---|---|
| `begin` | Proposta `AUTO_ELIGIBLE`, sessão física não nula e estado `IDLE` | Copia a proposta para RAM e abre janela de revisão. | Escrita no cofre ou emissão de recibo. |
| `expire` | Janela decorreu em `REVIEWING` ou `CONFLICTED` | Zeroiza proposta e entra em `EXPIRED`. | Retenção por timeout. |
| `mark_conflict` | Revisão viva e identificador concorrente distinto | Entra em `CONFLICTED`. | Escolha automática de um cartão. |
| `confirm_store` | Mesma sessão física e confirmação exatamente igual a `1` | Emite recibo, chama `memory_vault_seal`, zeroiza RAM. | Confirmação de outra sessão ou de modelo. |
| `cancel` | Revisão, conflito ou expiração | Zeroiza proposta e retorna a `IDLE`. | Mudança no cofre. |
| Falha de cofre | Store, open ou erase falha | Zeroiza proposta quando aplicável e entra em `FAILED`. | Retentativa autônoma ou falsa alegação de sucesso. |

O número de milissegundos é apenas relógio transitório para limitar a sessão em host; ele não é persistido no cartão. Assim, a implementação não finge possuir hora confiável, retenção por calendário ou expiração real de dados em flash. Esses recursos exigem um modelo de tempo, UX, política de retenção e hardware que ainda não existem no HERUS.

## 3. Confirmação e recibo

A confirmação é um objeto pequeno com `physical_session_id` e `physical_confirmed`. O módulo exige identificador não nulo, valor booleano canônico `1` e igualdade com a sessão que iniciou a revisão. Um adaptador futuro deve produzir essa asserção a partir de botão, gesto e apresentação de revisão; a biblioteca portátil não pode provar que uma pessoa estava presente.

| Artefato | Retenção | Finalidade | Limite |
|---|---|---|---|
| Identificador da sessão física | Somente durante revisão | Vincular início e confirmação. | Não identifica pessoa nem é persistido. |
| Proposta pendente | Somente durante revisão | Preparar cartão mínimo. | Zeroizada em cancelar, expirar ou falhar. |
| Recibo de revisão | Persistido dentro do cartão cifrado | Distinguir cartão consolidado de proposta. | É opaco; não é identidade nem log de conteúdo. |
| Métricas | Estrutura local numérica | Contar transições e recusas. | Não contêm `card_id`, sessão, chave ou conteúdo. |

A separação segue a noção de controles de privacidade e segurança que são configuráveis e devem fornecer tanto uma função quanto evidência de sua capacidade [2]. No HERUS, uma mensagem de modelo que pareça autorização não satisfaz a pré-condição: modelos, candidatos, rádio e diálogo não são parâmetros da chamada de consolidação.

## 4. Conflito sem autonomia

Um conflito é informado explicitamente com um identificador concorrente; a biblioteca não lê conteúdo rival, não calcula similaridade e não decide qual observação é verdadeira. Enquanto o estado é `CONFLICTED`, até uma confirmação física válida é recusada. A pessoa precisa cancelar a proposta e iniciar nova revisão com contexto de produto apropriado.

> **Princípio:** uma memória complementar pode apontar uma tensão para revisão; ela não deve resolver a biografia de alguém sozinha.

Essa decisão evita uma falsa promessa de “consistência semântica” antes de haver conteúdo, recuperação, representação de versão e UX avaliados. O Passo 6 pode estudar recuperação e apresentação; ele não deve converter isso em autoridade de escrita.

## 5. Recuperação e remoção

A recuperação do Passo 5 é uma abertura do cofre por `card_id` esperado. Ela exige confirmação física canônica, não faz consulta por palavras, não usa LLM e não apresenta conteúdo bruto inexistente. Caso o cofre recuse tag, geração, raiz, contexto ou backend, a saída é zerada e o consolidator passa a `FAILED`.

| Operação | Requisito | Resultado em sucesso | Resultado em falha |
|---|---|---|---|
| `recall` | Acesso físico canônico e `card_id` não nulo | Devolve cartão tipado mínimo. | Sem cartão de saída e estado `FAILED`. |
| `erase` | Acesso físico canônico | Chama erase do cofre; preserva piso anti-rollback. | Estado `FAILED`; não declara exclusão. |
| `cancel` | Fluxo de revisão não concluído | Remove somente proposta em RAM. | Não toca o cofre. |

O NIST descreve sanitização de mídia como tornar acesso ao dado alvo inviável para dado nível de esforço [3]. A revisão usada aqui foi sucedida por uma nova revisão, e esse passo não pretende certificar nenhum método de sanitização. Portanto, `erase` é uma **remoção lógica/operacional do registro do cofre**, condicionada ao backend retornar sucesso. Não é alegação de crypto erase, purga de flash, destruição de mídia ou resistência a recuperação física.

## 6. Ameaças cobertas e limites

| Evento | Comportamento provado em host | O que permanece pendente |
|---|---|---|
| Proposta sensível, de terceiro ou inelegível | Recusada antes de entrar em revisão. | Classificação de fala real e cobertura semântica. |
| Confirmação de outra sessão | Recusada sem escrita. | Garantia de que a sessão veio de hardware físico real. |
| Expiração | Proposta zeroizada, sem recibo e sem escrita. | Relógio persistente, RTC e retenção por calendário. |
| Conflito | Bloqueia confirmação; só cancelamento permite novo fluxo. | Detecção de conflito e apresentação de alternativas no produto. |
| Store ou erase falho | Estado `FAILED`; nenhuma alegação de sucesso ou retry autônomo. | Atomicidade, power-loss, NVS e comportamento físico do ESP32-S3. |
| Recuperação | Leitura por identificador com confirmação física. | Busca semântica, ranking, LLM local e avaliação de utilidade. |

## 7. Prova executável

A suíte `make memory-consolidation` é a **11ª de 22 suítes** no pipeline. Ela prova proposta insegura recusada, revisão sem escrita, sessão divergente, expiração com zeroização, conflito bloqueante, cancelamento, emissão de recibo somente após confirmação, recuperação por identificador, remoção autorizada, falha de erase e falha de store.

```bash
cd firmware
make memory-consolidation
cd ..
./prove.sh --quiet
```

Um resultado positivo prova as transições injetadas em host; não prova comportamento de usuário, qualidade de UX, precisão de reconhecimento de fala, confiabilidade de botão, retenção em calendário, sanitização física ou integração ESP-IDF.

## 8. Continuidade: recuperação tipada implementada em host

O Passo 6 agora implementa a [recuperação semântica controlada](22-RECUPERACAO-SEMANTICA.md): matching local de cartões mínimos em RAM, consulta tipada limitada, limiar, margem de ambiguidade e apresentação de autoridade zero. Não há busca livre, cofre, escrita, rede, modelo ou resposta factual neste módulo.

O próximo avanço deve estudar a interface humana de recuperação: como apresentar `MATCH`, `NO_MATCH` e `AMBIGUOUS` por voz, háptica ou tela sem ampliar conteúdo, sem apagar incerteza e sem conceder poder de retenção ou ação à camada de apresentação.

## Referências

[1] National Institute of Standards and Technology, *Privacy Framework*. [Página oficial](https://www.nist.gov/privacy-framework).

[2] Joint Task Force, *Security and Privacy Controls for Information Systems and Organizations*, NIST SP 800-53 Rev. 5. [Página oficial](https://csrc.nist.gov/pubs/sp/800/53/r5/upd1/final) e [DOI](https://doi.org/10.6028/NIST.SP.800-53r5).

[3] National Institute of Standards and Technology, *Guidelines for Media Sanitization*, NIST SP 800-88 Rev. 1, retirada e sucedida por revisão posterior. [Página de publicação](https://csrc.nist.gov/pubs/sp/800/88/r1/final) e [DOI](https://doi.org/10.6028/NIST.SP.800-88r1).
