# Recuperação semântica controlada — matching tipado, limiar e ambiguidade explícita

> **Estado da evidência:** este passo prova em host o matching local de um conjunto pequeno de cartões tipados fornecido pelo chamador. Não prova compreensão de linguagem natural aberta, busca sobre uma vida real, relevância humana, utilidade pessoal, LLM local, precisão de ASR, botão/gesto físico, acesso a NVS ou desempenho/energia no ESP32-S3.

O Passo 6 introduz a primeira forma de recuperação do HERUS. “Semântica”, aqui, tem um significado deliberadamente estreito: comparar **tipos, origem, consentimento explícito e limites de confiança** de cartões mínimos já autorizados. Não há texto, vetor de embedding, índice de áudio, base de conversas ou geração de resposta. O resultado é uma apresentação tipada e explicável — uma sugestão de cartão, ausência de correspondência ou ambiguidade — nunca uma alegação de verdade nem uma ação.

| O módulo faz | O módulo não faz |
|---|---|
| Classifica no máximo oito cartões tipados em RAM. | Abrir cofre, carregar raiz, escrever, apagar, transmitir ou registrar dados. |
| Exige asserção física canônica para consultar. | Provar que a asserção veio de botão/gesto físico real. |
| Aplica filtros explícitos de tipo, origem, lembrança explícita e confiança. | Interpretar pergunta livre, voz, texto, imagem, embedding ou contexto externo. |
| Expõe match, no-match ou ambiguidade com razões e pontuações. | Inventar resposta, resumo, história, certeza factual ou recomendação de ação. |
| Recusa fonte sensível, de terceiro, inválida, duplicada ou grande demais. | Recuperar qualquer conteúdo de pessoa diferente ou informação fora do cartão mínimo. |

## 1. Finalidade e posição arquitetural

O TREC do NIST foi criado para apoiar métodos de recuperação com coleções de teste e técnicas de avaliação, reforçando que um recuperador precisa de método e métrica, não apenas demonstração convincente [1]. O HERUS aplica essa disciplina à menor superfície possível: a suíte usa fixtures tipadas para testar seleção, não seleção e ambiguidade; ela não usa corpus pessoal e não afirma uma taxa de relevância humana.

| Camada | Papel | Pode escrever no cofre? | Pode recuperar? |
|---|---|---:|---:|
| Política, captura e extração | Produzem sinais temporários e incertos. | Não | Não |
| Cofre | Persiste um único cartão mínimo cifrado. | Somente com autorização externa | Abre cartão esperado, não busca. |
| Consolidação humana | Controla revisão, recibo, remoção e abertura por identificador. | Sim, após confirmação | Sim, por identificador. |
| **Recuperação controlada** | Faz ranking local sobre cartões já disponíveis em RAM. | **Não** | **Sim, apenas matching tipado.** |
| Modelo futuro | Poderá sugerir apresentação sob avaliação. | **Nunca diretamente** | Não sem fronteira determinística. |

O módulo `memory_retrieval.[ch]` não recebe `memory_vault_t`, chave, porta de armazenamento, candidato cru ou modelo. Um adaptador futuro que obtiver cartões via consolidação/cofre terá de preservar as confirmações exigidas pelas etapas anteriores; o algoritmo de ranking continua puro, local e sem efeitos persistentes.

## 2. Consulta mínima e privacidade

A consulta é um objeto pequeno com preferência de tipo, preferência de origem, exigência opcional de lembrança explícita e confiança mínima. Pelo menos um critério é obrigatório. Portanto, a pessoa não consegue usar este módulo para enumerar silenciosamente todos os cartões; uma consulta vazia é uma recusa, não uma busca ampla.

| Campo da consulta | Regra | Razão de privacidade |
|---|---|---|
| `preferred_kind` | Um tipo conhecido ou `NONE`. | Pergunta por classe de memória, não por conteúdo. |
| `preferred_origin` | Origem explícita/controlada ou `NONE`. | Preserva incerteza sem revelar frase de origem. |
| `require_explicit` | `0` ou `1` exatamente. | Pode exigir apenas itens explicitamente pedidos pela pessoa. |
| `minimum_confidence_pct` | 0–100. | Evita que baixa confiança entre em match por aproximação. |

Não há campo para texto, áudio, transcrição, resumo, embedding, identidade, localização, timestamp, rede, chave, prompt ou saída de modelo. Essa minimização é consistente com a finalidade do NIST Privacy Framework de identificar e gerir riscos de privacidade no desenvolvimento de produtos [2]. Ela é uma escolha de arquitetura, não uma certificação ou conclusão jurídica.

## 3. Fonte, pontuação e limiar

A entrada tem de conter entre um e oito cartões, cada um com identificador único, recibo de revisão, sessão autorizada, escopo `SELF`, sensibilidade `ORDINARY`, origem válida e reavaliação `AUTO_ELIGIBLE` da política. Se um cartão for sensível, de terceiro, malformado ou duplicado, **todo o conjunto é recusado antes do ranking**. A recusa evita que uma fonte não segura seja mascarada pela qualidade de outro cartão.

| Condição | Efeito no cartão | Efeito no resultado |
|---|---|---|
| Filtro de tipo/origem/explicitamente lembrado/confiança não satisfeito | Não concorre. | Pode levar a `NO_MATCH`. |
| Pontuação líder abaixo de 60 | Não é apresentada. | `NO_MATCH`, sem `card_id`. |
| Líder e segundo colocados diferem por menos de 10 pontos | Nenhum vencedor é apresentado. | `AMBIGUOUS`, sem `card_id`. |
| Líder satisfaz limiar e margem | Apresentação mínima com razões. | `MATCH`, um `card_id` e tipo/origem. |

A pontuação é uma heurística tipada e transparente. Ela soma critérios que a própria consulta declarou e sinais já existentes de confiança, novidade, valor futuro e consequência. Não é probabilidade calibrada, medida de verdade, precisão de usuário ou justificativa para gravar/alterar qualquer memória. O resultado também informa a pontuação do segundo colocado, mas nunca a identidade ou os atributos do perdedor.

## 4. Apresentação e autoridade zero

Uma apresentação contém somente `status`, identificador do cartão vencedor quando existir, tipo, origem, duas pontuações e uma máscara de razões. Para `NO_MATCH` ou `AMBIGUOUS`, identificador, tipo e origem ficam zerados. Essa estrutura permite que uma UI futura diga “há uma correspondência tipada” ou “há ambiguidade; reveja”, sem produzir uma resposta livre que possa parecer conhecimento ou ordem.

| Estado | Significado estrito | Ação proibida |
|---|---|---|
| `NO_MATCH` | Nenhum cartão atingiu os critérios e o limiar. | Escolher “o mais próximo”, inventar memória ou ampliar a consulta. |
| `AMBIGUOUS` | Duas pontuações elegíveis estão próximas demais. | Desempatar por ordem, recência oculta, modelo ou suposição. |
| `MATCH` | Um cartão venceu os critérios tipados com margem. | Tratar cartão como fato, enviar conteúdo, escrever no cofre ou executar ação. |

O NIST AI 600-1 apresenta um perfil voluntário para incorporar considerações de confiabilidade ao desenho, uso e avaliação de IA generativa [3]. Por isso, qualquer futuro modelo deve ficar fora dessa autoridade: ele poderá no máximo receber uma apresentação que a UI permitir e sugerir linguagem local; não pode alterar score, abrir cofre, selecionar vencedor em ambiguidade, escrever memória, enviar mensagem ou confirmar por uma pessoa.

## 5. Avaliação executável

A avaliação é deliberadamente pequena e falsificável. A suíte `make memory-retrieval`, **12ª de 23 suítes**, não usa dados pessoais nem benchmarks inventados. Ela constrói cartões tipados de teste e verifica as propriedades abaixo.

| Prova host | Resultado exigido |
|---|---|
| Consulta elegível com vencedor claro | Um único `MATCH` com razões explícitas. |
| Acesso físico não canônico | Erro e apresentação zerada. |
| Consulta vazia | Erro; não enumera todos os cartões. |
| Scores próximos | `AMBIGUOUS`, sem vencedor. |
| Nenhum cartão conforme filtros | `NO_MATCH`, sem palpite aproximado. |
| Ordem dos cartões alterada | Mesmo vencedor claro; não há preferência oculta de ordem. |
| Fonte sensível, duplicada ou acima da capacidade | Recusa anterior ao ranking. |
| Métricas | Contadores somente numéricos, sem `card_id` ou conteúdo. |

```bash
cd firmware
make memory-retrieval
cd ..
./prove.sh --quiet
```

A métrica apropriada em uma futura avaliação humana será definida antes de coleta: por exemplo, proporção de consultas com cartão útil, falsos matches, ambiguidade corretamente exposta e recusa segura. Inspirar-se na infraestrutura de avaliação do TREC não autoriza usar seus resultados como se fossem resultados do HERUS [1]. Nenhuma precisão, recall, taxa de utilidade ou desempenho de modelo é alegado neste passo.

## 6. Limites e próximos gates

| Limite atual | Por que não é resolvido aqui | Gate futuro |
|---|---|---|
| Linguagem natural aberta | Consulta não possui texto ou ASR. | Adaptador local avaliado separadamente, sem escrita. |
| Busca em dados reais | O módulo recebe cartões em RAM; não abre nem indexa cofre. | Fluxo de UI e acesso físico medidos no alvo. |
| Relevância pessoal | Pontuação usa apenas sinais tipados de fixture. | Estudo pré-registrado com critérios de utilidade e privacidade. |
| LLM local | Não há modelo na API, avaliação ou cálculo. | Perfil de modelo, recursos, riscos e testes adversariais separados. |
| Controle físico | A asserção é um contrato de software. | Botão/gesto, relógio e UX de confirmação em hardware. |
| Persistência/remoção | Não há escrita nem erase neste módulo. | Cofre e backend ESP-IDF com evidência de hardware. |

## 7. Próximo passo

O Passo 7 deve aprofundar a **interface de recuperação humana**: como apresentar `MATCH`, `NO_MATCH` e `AMBIGUOUS` por voz/háptica/tela sem transformar a saída em fala de autoridade. A regra permanece: a recuperação pode sugerir; só uma revisão física pode decidir retenção, remoção ou qualquer ação externa.

## Referências

[1] National Institute of Standards and Technology, *Text REtrieval Conference (TREC)*. [Página oficial](https://www.nist.gov/programs-projects/text-retrieval-conference-trec).

[2] National Institute of Standards and Technology, *Privacy Framework*. [Página oficial](https://www.nist.gov/privacy-framework).

[3] Autio, C. et al., *Artificial Intelligence Risk Management Framework: Generative Artificial Intelligence Profile*, NIST AI 600-1, 2024. [Página oficial](https://www.nist.gov/publications/artificial-intelligence-risk-management-framework-generative-artificial-intelligence) e [DOI](https://doi.org/10.6028/NIST.AI.600-1).
