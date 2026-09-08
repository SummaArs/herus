# Interface humana de recuperação — status simbólico, incerteza e controle one-shot

> **Estado da evidência:** este passo prova em host que um resultado tipado de recuperação pode tornar-se uma apresentação simbólica, limitada e descartável. Não prova inteligibilidade de voz, percepção de vibração, legibilidade de tela, acessibilidade, compreensão da pessoa, funcionamento de botão/gesto, latência, energia ou integração de periféricos no ESP32-S3.

O Passo 7 não faz o HERUS “responder” uma memória. Ele estabelece uma fronteira mais modesta e mais segura: converter os três resultados do recuperador — `MATCH`, `NO_MATCH` e `AMBIGUOUS` — em **códigos locais de status** que uma UI futura poderá apresentar por voz controlada, háptica ou tela. A camada preserva a incerteza, não revela cartão ou conteúdo em estados sem vencedor e não recebe autoridade para alterar memória, enviar mensagem ou executar ação.

| A interface faz | A interface não faz |
|---|---|
| Valida e apresenta um resultado tipado por ciclo. | Consultar o cofre, carregar chave, buscar cartões ou calcular score. |
| Exige asserção física canônica e associa o status à sessão local. | Provar que o gesto/botão ocorreu no mundo físico. |
| Emite código de frase controlada e plano háptico abstrato. | Sintetizar áudio, acionar motor, desenhar tela ou afirmar acessibilidade. |
| Diferencia match, ausência e ambiguidade. | Escolher empate, inventar resposta, resumir ou declarar fato. |
| Descarta o status sob a mesma sessão e zeroiza RAM transitória. | Escrever, apagar, transmitir, criar HCP ou confirmar decisão humana. |

## 1. Posição na cadeia de memória

O recuperador do Passo 6 já faz matching local tipado sobre cartões autorizados em RAM. Seu resultado é uma sugestão com limiar e margem, não uma verdade factual. O apresentador do Passo 7 é exclusivamente consumidor desse resultado: ele não chama `memory_retrieval_query`, não recebe `memory_vault_t` e não possui caminho para política, captura, cofre, consolidação, rádio ou modelo.

| Camada | Recebe | Produz | Pode persistir ou agir? |
|---|---|---|---:|
| Recuperação controlada | Consulta e cartões tipados já autorizados | `MATCH`, `NO_MATCH` ou `AMBIGUOUS` | Não |
| **Apresentação humana** | Resultado de recuperação e asserção física | Código de frase, padrão háptico e tipos permitidos | **Não** |
| Adaptador futuro de UI | Códigos simbólicos | Voz, vibração ou pixels físicos | Não sem contrato próprio |
| Consolidação e cofre | Fluxo humano separado | Escrita/remover cartão sob confirmação | Somente nesses limites |

Essa divisão implementa uma consequência concreta de papéis e responsabilidades explícitos para supervisão humano-IA, como proposto no AI RMF do NIST [1]: o recuperador classifica, a apresentação informa e a pessoa decide. Nenhuma camada posterior recebe poder implícito de mudar a decisão anterior.

## 2. Resultado canônico e apresentação mínima

O apresentador rejeita qualquer resultado de recuperação que não respeite o contrato do Passo 6. A validação ocorre antes de gerar efeito local. Assim, um `NO_MATCH` com identificador escondido, uma ambiguidade com tipo/origem de vencedor ou um match sem razões não chegam ao adaptador de voz, háptica ou tela.

| Resultado recebido | Apresentação permitida | Detalhes permitidos | Detalhes proibidos |
|---|---|---|---|
| `MATCH` inequívoco | `MATCH_AVAILABLE` | Tipo, origem e máscara de razões já autorizadas. | `card_id`, conteúdo, texto, áudio, score, segundo colocado, chave ou blob. |
| `NO_MATCH` | `NO_MATCH` | Nenhum. | Qualquer cartão próximo, tipo/origem, razão ou tentativa automática de ampliar a consulta. |
| `AMBIGUOUS` | `AMBIGUOUS_REVIEW` | Nenhum. | Identidade, tipo/origem ou razões de qualquer concorrente; desempate por ordem, modelo ou suposição. |
| Formato inválido | Nenhuma apresentação; `BLOCKED`. | Nenhum. | Repetição, fallback permissivo ou recuperação automática. |

A escolha reflete o princípio de mensagens de status: informar resultado ou estado relevante sem mudança de contexto desnecessária [2]. A recomendação do W3C também alerta que feedback excessivo pode tornar a experiência intrusiva; por isso, o contrato é **one-shot**, sem loop de anúncio, e a interface de hardware futura deve ser avaliada com usuários antes de qualquer alegação de qualidade.

## 3. Códigos simbólicos e háptica abstrata

A API contém identificadores de frase, não frases livres. Eles são contratuais: uma futura renderização em português deve respeitar seu significado limitado e não expandi-los para instrução, ação ou explicação inventada. Do mesmo modo, o plano háptico é uma sequência abstrata de pulsos validada pelos limites portáveis já existentes (`HAPTIC_MAX_PULSE`, duração ligada e duração total); não é comando de GPIO nem medida de sensação humana.

| Código de status | Significado de produto estrito | Evento háptico simbólico | Não significa |
|---|---|---|---|
| `MATCH_AVAILABLE` | Há uma correspondência tipada inequívoca sob os critérios já aplicados. | Padrão curto de status local. | Que a memória é verdadeira, completa, atual ou que alguma ação deve ocorrer. |
| `NO_MATCH` | Nenhum cartão alcançou os critérios e limiar. | Padrão de ausência/cancelamento local. | Que a pessoa não sabe, que não existe memória ou que a consulta deve ser ampliada. |
| `AMBIGUOUS_REVIEW` | Há proximidade entre resultados; revisão humana é necessária. | Padrão curto de incerteza. | Que o sistema escolheu um cartão ou que a pessoa deve aceitar um deles. |
| `REJECTED` | Reservado a adaptador futuro para erro local; o módulo atual falha sem efeito. | Não gerado como sucesso. | Que erro pode ser ignorado ou reexecutado automaticamente. |

A noção de que uma apresentação deve ser autodescritiva, controlável e tolerante a erros segue princípios de desenho centrado na pessoa citados pelo NIST [3]. Neste passo, esses princípios são transformados em contratos de software verificáveis; não equivalem a teste de usabilidade, conformidade de acessibilidade ou certificação de ergonomia.

## 4. Sessão física, one-shot e descarte

`memory_retrieval_present_show` exige `physical_session_id != 0` e `physical_confirmed == 1`. Quando bem-sucedido, guarda somente o identificador de sessão necessário para permitir descarte e entra em `SHOWN`. Um novo `show` é recusado até que a pessoa descarte o estado com a mesma sessão física; em seguida, os dados transitórios são zerados e o módulo retorna a `IDLE`.

| Evento | Pré-condição | Efeito | Resultado em falha |
|---|---|---|---|
| `show` | Estado `IDLE`, acesso canônico e resultado canônico | Um status simbólico, sessão ativa e estado `SHOWN`. | Saída zerada; nenhum efeito local. |
| Segundo `show` | Estado `SHOWN` ou `BLOCKED` | Não aplicável. | `E_STATE`; não repete anúncio. |
| `dismiss` | Estado `SHOWN` e mesma sessão física | Zeroiza payload e volta a `IDLE`. | `E_ACCESS` ou `E_STATE`; status não é removido por outra sessão. |
| Resultado malformado | Qualquer acesso canônico | Zeroiza payload e entra em `BLOCKED`. | `E_RESULT`; requer reinicialização explícita. |

O contrato de sessão é uma asserção de software, não prova de presença humana. A integração real deve ligar esse valor a botão/gesto e testar perdas de evento, acessibilidade, tempo de atenção, reinicialização e interação acidental em dispositivo físico.

## 5. Privacidade, autoridade e ameaças

A superfície pública não contém texto, áudio, transcrição, embedding, identidade, localização, timestamp, `card_id`, cofre, chave, blob, candidato, rádio, rede, HCP, callback de ação ou modelo. As métricas são somente contadores de match/no-match/ambiguidade exibidos, descartes e recusas; não registram conteúdo ou sessão.

| Evento adversarial | Comportamento provado em host | Limite da prova |
|---|---|---|
| Acesso físico ausente/não canônico | Saída zerada, sem apresentação. | Não prova botão ou biometria física. |
| Tentativa de reapresentar | Estado recusa a repetição. | Não mede se a pessoa percebeu o primeiro sinal. |
| `NO_MATCH` com vencedor escondido | Rejeita e bloqueia. | Não protege adaptador que ignore o contrato. |
| Ambiguidade com proveniência de concorrente | Rejeita e bloqueia. | Não mede compreensão humana da ambiguidade. |
| Match sem razões/campos inválidos | Rejeita antes de gerar status. | Não verifica recuperador em dados pessoais reais. |
| Falha/reinicialização explícita | Estado não faz retry autônomo. | Não prova power-loss, reset do MCU ou retenção física de RAM. |

O AI RMF também enfatiza que saídas, limites de conhecimento e supervisão humana devem ser documentados, e que testes/medidas precisam ocorrer antes da implantação [1]. O HERUS mantém essa separação: o contrato host prova que a camada não obtém agência; percepção, utilidade e impacto da interface permanecem hipóteses para avaliação posterior.

## 6. Prova executável

A suíte `make memory-retrieval-present` é a **13ª de 24 suítes**. Ela cobre match inequívoco, `NO_MATCH`, ambiguidade, acesso não canônico, sessão divergente, apresentação repetida, descarte, match malformado, no-match com vencedor oculto, ambiguidade com proveniência e métricas numéricas.

```bash
cd firmware
make memory-retrieval-present
cd ..
./prove.sh --quiet
```

O pipeline acrescenta duas invariantes: a apresentação física/one-shot com limpeza de estado e a exposição de incerteza sem selecionar contender. Um resultado positivo prova somente esses cenários injetados em host. Não mede inteligibilidade, WER, taxa de compreensão, aceitação, fadiga háptica, acessibilidade, autonomia de bateria, latência de periférico ou desempenho de LLM.

## 7. Continuidade: Grand Finale de memória implementado em host

O Passo 8 agora implementa o [Grand Finale de memória](24-GRAND-FINALE-MEMORIA.md): uma fixture composta que percorre captura, extração, política, revisão, cofre, recuperação e apresentação, mais um auditor sem efeitos colaterais que bloqueia evidência ausente, conflito, política insegura, apresentação inválida ou presença de modelo no caminho. Ele não transforma os códigos simbólicos em voz, vibração ou tela reais.

A próxima etapa é **evidência humana e de hardware**, antes de ampliar a superfície com linguagem natural ou modelo. A investigação deverá pré-registrar tarefas, critérios de parada, medidas de entendimento de `MATCH`/`NO_MATCH`/`AMBIGUOUS`, falsos entendimentos, eventos de cancelamento, energia e privacidade de logs. Não deve alegar que um sinal simbólico foi entendido apenas porque o código produziu um plano háptico.

## Referências

[1] National Institute of Standards and Technology, *AI RMF Core*, AI RMF 1.0. [Recurso oficial](https://airc.nist.gov/airmf-resources/airmf/5-sec-core/).

[2] W3C Web Accessibility Initiative, *Understanding Success Criterion 4.1.3: Status Messages*, WCAG 2.2. [Documento oficial](https://www.w3.org/WAI/WCAG22/Understanding/status-messages.html).

[3] National Institute of Standards and Technology, *Human Centered Design*. [Página oficial](https://www.nist.gov/itl/iad/human-centered-technologies/human-factors-human-centered-design).
