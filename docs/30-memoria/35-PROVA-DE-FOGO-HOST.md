# Prova de fogo host: campanha F1 de recuperação e quarentena

**Status:** primeira rodada extrema pré-hardware. A campanha usa snapshots sintéticos e uma seed fixa para atacar a composição entre recuperação de reserva e bootstrap de sessão. É evidência C11 em host; não é reset físico, power loss, backend durável, segurança de plataforma ou teste humano.

## Propósito

A série pré-hardware já tinha casos adversariais dirigidos para o oráculo de reserva (T15) e para a quarentena de boot (T16). F1 amplia a superfície de ataque com geração determinística e composição real: cada snapshot passa pelo oráculo e, em seguida, pelo bootstrap. A propriedade central é simples e forte:

> Todo snapshot hostil termina em **gate `BLOCKED` e limpo**, ou em **gate `IDLE` e limpo**. Nenhum resultado pode restaurar uma sessão, nonce, propósito, prazo, uso ou métrica transitória anterior.

O harness não recebe áudio, transcrição, embedding, identidade, localização, cartão, consulta, chave, evento, pessoa, relógio, modelo, rádio, rede ou callback. Seu único log é agregação de seed, rodadas e contadores sintéticos; não é log de produto.

## Rodada F1

| Campo | Valor |
|---|---|
| Seed de reprodução | `0xF1A0C0DE` |
| Snapshots estruturados | 100.000 |
| Snapshots brutos | 20.000 |
| Mutações de bit | 8 topologias canônicas × todos os bits de `memory_physical_session_recovery_snapshot_t` |
| Composição exercida | oráculo T15 → bootstrap T16 → gate real C11 |
| Diagnóstico adicional | AddressSanitizer + UndefinedBehaviorSanitizer, fora do baseline portátil |
| Saída de sucesso | Ação permitida seguida de gate `IDLE` limpo e sucessor novo, ou estado `BLOCKED` limpo |

O gerador usa valores de fronteira para flags, IDs, propósitos e usos: zero, um, sucessores pequenos, limites máximos, valores não canônicos, enums desconhecidos, campos ausentes, combinações contraditórias e bytes totalmente brutos. A mutação de templates assegura que pequenas alterações de topologias permitidas também atravessam o bootstrap.

## Defeito encontrado e corrigido

F1 encontrou a issue [#31](https://github.com/SummaArs/herus/issues/31): um registro `COMMITTED` autenticado com `reservation_id == UINT32_MAX` era aceito. O bootstrap importava esse valor como piso. Como uma sessão nova exige ID estritamente maior que o piso, não existia nenhum `uint32_t` sucessor possível. Não houve bypass de confirmação ou revivificação de autoridade, mas a capacidade de iniciar sessões futuras ficava esgotada.

| Antes do patch | Correção |
|---|---|
| `UINT32_MAX` podia produzir `USE_COMMITTED` ou promoção preparada, seguido por gate `IDLE` terminal. | IDs `COMMITTED` e `PREPARED` iguais a `UINT32_MAX` são não canônicos e retornam `E_INVALID`/`BLOCKED`. |
| Não havia contraprova dirigida para esgotamento de espaço de ID. | T15 testa `COMMITTED` e `PREPARED` terminais; F1 exige que todo sucesso tenha sucessor representável. |
| Bootstrap permanecia estruturalmente limpo, mas indisponível para sessões novas. | Bootstrap recebe apenas uma recuperação não terminal; sucessor pode iniciar com nova afirmação canônica. |

A correção é deliberadamente fail-closed. Ela não cria wraparound, não redefine o piso, não reutiliza IDs e não inventa um mecanismo de migração de contador. Um adaptador futuro deve tratar aproximação de capacidade como condição operacional explícita e escolher, evidenciar e testar uma estratégia de rotação compatível com o alvo antes de qualquer alegação física.

## Invariantes F1

| Invariante | Como F1 a falsifica |
|---|---|
| Pureza do oráculo | Compara byte a byte o snapshot antes/depois de cada chamada. |
| Canonicalidade | Introduz flags não canônicas, enums desconhecidos, campos ausentes, usos inválidos e bytes brutos. |
| Falha fechada | Exige ação `BLOCKED`, bootstrap `E_RECOVERY` e gate limpo/bloqueado para recuperação inválida. |
| Quarentena | Exige `IDLE`, piso coerente e campos ativos/métricas zerados para toda recuperação aceita. |
| Sem reuso | Tenta iniciar sessão com o próprio piso e exige recusa. |
| Sucessor obrigatório | Tenta iniciar sessão com `piso + 1`; portanto piso terminal é contraprova. |
| Sem UB/memória corrompida | Repete F1 com sanitizers como diagnóstico suplementar. |

## Reprodução

```sh
cd firmware
make memory-physical-session-recovery
make memory-physical-session-recovery-stress
make memory-physical-session-recovery-stress \
  CFLAGS='-O1 -g -Wall -Wextra -std=c11 -fsanitize=address,undefined -fno-omit-frame-pointer'
cd ..
./prove.sh --quiet
```

Após F1, o ledger previsto passa a **36 suítes**, **81 invariantes de prova** e mantém **74 invariantes de sistema simulado**. Esses números contam cenários de host e simulação existentes. Eles não medem energia, RF, durabilidade de mídia, reboot físico, qualidade de fala, LLM, ergonomia, interação humana ou probabilidade de ataque.

## Limites e próxima rodada

F1 não prova que `UINT32_MAX` jamais chegará a um backend real, que o contador do adaptador é atômico, que há proteção contra rollback físico, que uma interrupção ocorreu, que a RAM foi apagada, que o evento é novo, que o relógio é confiável ou que o hardware executa a quarentena. A próxima rodada host deve atacar a recuperação de coleção, o índice e os interleavings de callbacks com o mesmo formato de seed e redução de caso. Somente depois de a campanha host esgotar falhas reproduzíveis de alto impacto a matriz deve avançar para placa, instrumento e backend declarados.

## Rodadas F2, F3 e F4

A campanha foi ampliada sem misturar seus domínios. F2 ataca somente o oráculo puro de recuperação de coleção; F3 ataca somente o classificador de ameaças; F4 não cria uma versão alternativa do produto, mas remove temporariamente controles selecionados em cópias privadas e exige que provas existentes detectem a remoção. Essa separação acompanha a ideia de engenharia de sistemas confiáveis: evidência, arquitetura, teste e fronteira de responsabilidade precisam ser distinguíveis, em vez de uma única execução ampla esconder a causa de uma falha [1].

| Rodada | Superfície | Ataque determinístico | Resultado host | Limite que permanece |
|---|---|---|---|---|
| F2 | Recuperação de coleção | 100.000 snapshots estruturados, 20.000 brutos e mutação de todos os bits de sete topologias. | Apenas ação de recuperação compatível com a topologia ou `BLOCKED`; nenhum snapshot é mutado. | Não prova mídia, callback, corte de energia, autenticidade material, raiz, desgaste ou backend. |
| F3 | Modelo de ameaças | 50.000 snapshots canônicos, 10.000 brutos, escopos inválidos e mutação bit a bit de evidência completa. | Formato ou escopo inválido não escalam para mitigação host; modelo, telemetria e plataforma continuam restritos ao contrato declarado. | Não é monitor de produção, pentest, modelo real, privacidade de operação nem segurança física. |
| F4 | Detectabilidade de controles | Compilação temporária com remoção deliberada de quatro controles: piso terminal, evidência de scrub, autenticação de coleção e barreira de envio do modelo. | T12/T15/F1/T9 detectam cada remoção; a mutação não é aceita como mudança de produto. | Não é cobertura exaustiva de mutação, prova formal, análise de binário alvo ou validação de ferramenta/compilador. |

F4 serve como teste de qualidade dos próprios testes. Uma mutação que não falhasse seria lacuna de prova, não resultado positivo de segurança. Essa prática é compatível com a finalidade do SSDF de reduzir vulnerabilidades e prevenir recorrência pela identificação de causas e melhoria contínua, mas o HERUS não declara aderência ou certificação ao framework [2].

## Consolidação da campanha host

| Medida | Estado após F1–F4 |
|---|---|
| Suítes no ledger | 39 |
| Invariantes de prova | 87 |
| Invariantes de sistema simulado | 74 |
| Defeitos P0/P1 encontrados nesta rodada | Nenhum reproduzível |
| Defeitos P2 encontrados nesta rodada | 1: piso de sessão terminal `UINT32_MAX`, issue [#31](https://github.com/SummaArs/herus/issues/31), corrigida e convertida em T15/F1/F4 |
| Dados de produto em harnesses | Nenhum: somente seeds, contadores e diagnósticos sintéticos |

As quatro rodadas reforçam a evidência host e não alteram os gates físicos. A orientação de resiliência de firmware distingue proteção, detecção e recuperação de plataformas e dados [3]; aqui, F1–F4 exercitam apenas decisões portáveis de detecção e recuperação em C11. Boot, armazenamento, interrupção, medição, rádio, energia e interação continuam pendentes no adaptador e na bancada declarados.

## Próxima transição deliberada

A campanha host deve parar nesta rodada porque os próximos cenários de maior valor — `PREPARED`/piso/`COMMITTED` interrompidos em mídia real, reset observado, RAM pós-reset, fonte de evento, RNG, relógio, replay, energia, RF e interação — são classificados pelo manifesto como `BLOCKED_BY_HARDWARE`. Acrescentar aleatoriedade host sem uma nova hipótese falsificável não equivaleria a evidência adicional. A próxima campanha deve começar apenas quando houver adaptador, instrumento, procedimento de interrupção e schema privado de coleta declarados.

## Referências

[1] National Institute of Standards and Technology, *SP 800-160 Vol. 1 Rev. 1: Engineering Trustworthy Secure Systems*. [Publicação oficial](https://csrc.nist.gov/pubs/sp/800/160/v1/r1/final).

[2] National Institute of Standards and Technology, *SP 800-218: Secure Software Development Framework (SSDF) Version 1.1*. [Publicação oficial](https://csrc.nist.gov/pubs/sp/800/218/final).

[3] National Institute of Standards and Technology, *SP 800-193: Platform Firmware Resiliency Guidelines*. [Publicação oficial](https://csrc.nist.gov/pubs/sp/800/193/final).
