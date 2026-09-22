# HERUS — síntese final do plano restante

**ID:** `HERUS-REM-SYNTH-006`  
**Data da auditoria:** 2026-09-22  
**Escopo:** fechar o mecanismo host-only, tornar o holdout temporal/estrutural reproduzível, matar regressões críticas e preparar uma prova humana limitada.  
**Estado da síntese:** plano de implementação; nenhum arquivo de código foi alterado nesta frente.

## Decisão executiva

A sequência correta é **fechar primeiro autoridade e execução, depois o holdout temporal/estrutural, depois as mutações, e somente então preparar e eventualmente executar a prova humana**. Não há atalho seguro que conecte o runtime diretamente a um executor, hardware ou participante.

O repositório está em um estado de regressão local saudável, mas de prova integrada incompleta: a suíte de pesquisa observada passa com **135 testes e 1 teste ignorado**; o benchmark histórico mantém 15 registros e o holdout atual continua classificado como `not_proven`, com 9 hard failures. O contraexemplo `m6a-opaque-01` ainda permite uma proposta no caminho legado, embora o oráculo independente o classifique como inseguro. O caminho `STRICT` já bloqueia alguns hosts sem contrato, mas ainda delega a síntese a `propose_transfer()`, cuja descoberta chama `host.execute()`. Além disso, a telemetria atual informa `probe_execute_calls == 0` apesar dessa chamada de sondagem. Portanto, **não se deve publicar `mechanism_only`, iniciar coleta humana ou operar um alvo físico agora**. [5] [6] [7] [8]

A linha de base `research/evidence/holdout_benchmark_v1.json` e seu schema devem permanecer imutáveis. Os hashes observados no congelamento inicial são:

```text
holdout_benchmark_v1.json  4b0b324ae98c45e4e06b955594f215e8b9919cd839f977f6f2508c2cd8d0326f
holdout_benchmark_schema.json  6a80083ff7d079d90d1f3e5b2b78925625b61cc4819d2ad5b2a5d0aed367908d
```

Esses hashes devem ser recalculados na execução oficial. Eles são âncoras de proveniência, não autorização nem evidência de segurança.

## Sequência única e otimizada

### Fase 0 — congelar a linha de base antes de editar

Esta fase é um gate curto, não uma nova semântica. Registrar commit, versão do Python, dependências, sistema, comandos, seeds, hashes dos dois artefatos v1, digest dos contratos e estado Git. Salvar uma cópia somente de leitura do resultado histórico, incluindo seus 15 registros, sua classificação `not_proven`, hard failures, limitações e o teste ignorado.

Executar, antes e depois de cada fase que altere código:

```bash
cd /home/ubuntu/herus
PYTHONPATH=research python3 -m unittest research.test_holdout_adversarial -v
PYTHONPATH=research python3 -m unittest research.test_stage5_synthesis -v
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
PYTHONPATH=research python3 -m research.holdout_adversarial
```

Qualquer alteração semântica no v1, no schema histórico ou nos 15 registros interrompe a sequência. A nova campanha deve usar somente artefatos versionados e aditivos.

### Fase 1 — fechar a fronteira de autoridade e execução

Esta é a primeira implementação substantiva porque elimina o risco de que plano, digest, `Skill`, `transfer() == True` ou `SUPPORTED` sejam interpretados como autorização.

#### 1.1 Tornar a proposta estritamente não executável

Corrigir `research/symbiont_v2/core.py` e os tipos compartilhados de `stage5.py` sem quebrar os wrappers legados. O caminho `propose_transfer_checked(..., mode="STRICT")` deve:

- validar skill, contrato, observação, época, digest, qualidade, limite e custo antes de qualquer descoberta;
- não receber executor, chave, callback de execução, nonce ou política de retry;
- não assinar nem emitir `ExecutionEnvelope`;
- manter `proposal_execute_calls`, `authorized_calls`, `committed_calls`, `external_effect_count` e `step_count` em zero durante toda a construção da proposta;
- separar `probe_execute_calls` de chamadas autorizadas;
- cobrar qualquer probe ou reset antes da chamada potencialmente mutante;
- bloquear antes da probe quando o host não demonstrar uma interface pública realmente não mutante;
- manter `UNKNOWN` como custo desconhecido (`null`), nunca como zero, `OPEN` ou `SUPPORTED`;
- manter `runtime_reason` limitado ao que a superfície pública sustenta.

A correção não deve simplesmente renomear `host.execute()` para probe. O host atual `PublicHoldoutHost` conserva `_fixture`, `_private_state` e executa mutações durante `discover()`. Um contrato que declara `NON_MUTATING` precisa ser verificável por uma API de leitura, por um clone descartável ou por um proxy/processo separado. Se isso não existir, o modo estrito deve retornar `UNSUPPORTED_BY_CONTRACT` sem sondar.

A síntese deve permanecer aditiva: `transfer()` e `propose_transfer()` continuam disponíveis para compatibilidade, mas nunca são autoridade. Consumidores que tratem uma proposta ou `True` como comando devem ser identificados, receber telemetria de depreciação e ser impedidos de alcançar um efeito sem o adaptador externo.

#### 1.2 Implementar o protocolo externo em processo separado

Criar, como conjunto mínimo, os seguintes módulos novos:

```text
research/stage4/execution_contracts.py
research/stage4/ledger.py
research/stage4/executor.py
research/stage4/recovery.py
research/stage4/runner.py
research/stage4/test_executor.py
research/stage4/test_recovery.py
research/stage4/test_protocol.py
```

`ExecutionEnvelope` deve ser versionado e canonicalizado. Deve vincular `proposal_digest`, `action_sequence_digest`, `host_context_digest`, host, época, skill, operação, tentativa, nonce, janela temporal, limites, política `NO_AUTOMATIC_RETRY` e snapshot de revogação. A assinatura deve ser verificável contra uma raiz de autoridade que não pertence ao runtime.

O ledger deve ser append-only, encadeado por digest, monotônico e durável para a reserva de nonce. A ordem obrigatória é: canonicalizar, verificar autoridade e escopo, verificar relógio monotônico, verificar revogação, verificar tentativa e limites, reservar nonce atomicamente, registrar aceitação e somente então despachar o primeiro passo. Replay rejeita sem segunda chamada.

O executor deve verificar TTL e revogação antes de cada passo. Deve distinguir:

- `FAILED_PARTIAL + RECOVERY_REQUIRED` quando um prefixo foi comprovadamente comprometido e um passo posterior falhou;
- `UNKNOWN_OUTCOME + RECOVERY_REQUIRED` quando há timeout, crash, ACK ausente ou recibos conflitantes;
- `COMMITTED` somente quando todos os passos têm recibos terminais válidos;
- `RETRY_BLOCKED` quando não há atestação de recuperação válida.

`reset()`, reboot, `bind()` e nova sessão não podem limpar nonce consumido, recovery pendente, contadores ou cadeia do ledger. A recuperação deve usar uma fonte independente e pode concluir somente `RESOLVED_COMMITTED`, `RESOLVED_NOT_COMMITTED`, `RESOLVED_PARTIAL` ou `RECOVERY_CONFLICT`. Mesmo uma conclusão positiva de recovery não autoriza retry: exige novo envelope externo, nonce, TTL, tentativa, digest-pai e escopo residual.

Nesta fase é possível implementar e testar um **executor de laboratório determinístico** com autoridade fictícia controlada pelos testes. Isso não equivale a uma autoridade de produção nem concede permissão para agir em hardware.

### Fase 2 — tornar o holdout temporal e estrutural reproduzível

Somente depois da barreira proposta–execução estar fechada, criar uma campanha nova e aditiva. O v1 não deve ser reescrito nem usado para retroajuste.

Criar:

```text
research/stage4/extended_contracts.py
research/stage4/extended_fixtures.py
research/stage4/public_host.py
research/stage4/independent_oracle.py
research/stage4/extended_runner.py
research/stage4/extended_results.schema.json
research/test_extended_holdout.py
research/test_extended_oracle.py
research/test_extended_mutations.py
research/evidence/extended_holdout_v1/manifest_public.json
research/evidence/extended_holdout_v1/manifest_private.json
research/evidence/extended_holdout_v1/results.jsonl
```

O manifesto deve separar três planos:

| Plano | Conteúdo permitido | Conteúdo proibido |
|---|---|---|
| Público | IDs opacos, observações, ações, contratos, limites, recibos públicos e, quando aplicável, token de época | classe, família, motivo esperado, agenda de deriva, estado latente, gabarito e tabela privada |
| Oráculo | estado verdadeiro, transições por época, pré-condições, ordem, recursos, custos e efeitos proibidos | decisão do runtime antes do fechamento e módulos de transição do runtime |
| Avaliação | seeds, digests, split, rótulos privados, critérios e resultados | acesso do runtime durante a rodada |

A `PublicObservationV2` deve separar `runtime_bind_epoch` de `host_epoch` e vincular a evidência a `binding_digest`, época, episódio e sequência. Evidência stale, replayada, regressiva, de outro vínculo ou de reset não atestado deve ser descartada. O reset precisa declarar explicitamente uma das semânticas `RESET_SAME_EPOCH_FULL`, `RESET_NEW_EPOCH` ou `RESET_PUBLIC_ONLY` e emitir recibo público compatível.

O contrato estrutural deve representar pré-condições, recursos, consumo, piso, ordem, custo, número máximo de passos, orçamento, escopo de observação e fechamento de efeitos. O runtime não pode usar `expected_final` como prova da trajetória. Alias estrutural ou observacional, pré-condição não provada, ordem não comutativa, recurso ausente, custo desconhecido, deriva latente sem fechamento e efeito proibido devem resultar em `ABSTAIN` ou `UNSUPPORTED_BY_CONTRACT`, conforme a causa pública.

Fixar os três splits e seus digests antes da campanha. Usar as seeds planejadas `4403` para desenvolvimento, `4503` para validação e `4603` para holdout. IDs, ações, campos, unidades e ordem de enumeração devem ser permutados. Cada caso deve rodar três vezes em processos limpos, com relógio monotônico injetável. Status, motivos públicos, digests, contadores e traços devem ser iguais byte a byte quando o trace público for igual.

O oráculo deve ser declarativo e isolado. Um teste de import deve falhar se ele tentar usar `core.py`, `stage5.py`, `Effect.from_states()`, `_apply()`, `WorldModel` ou `Goal.satisfied()`. Duas fixtures com a mesma projeção pública e verdades privadas distintas devem produzir a mesma decisão pública; a diferença pode surgir somente no resultado pós-decisão do oráculo.

### Fase 3 — matar as mutações críticas

Implementar um harness em cópia temporária, inspirado em `tools/test_proof_fire_mutations.py`, sem alterar o repositório principal ou os artefatos v1. Cada mutação deve ter ID, substituição única, teste matador, variante renomeada e evidência de morte. Mutante que não compila não conta como mutante morto; deve ser reportado separadamente.

As quatro classes obrigatórias são:

1. **Remoção de abstention:** escolher a primeira ação em alias, aceitar conflito, reutilizar stale/replay, sondar antes do preflight ou converter contrato ausente em proposta.
2. **Custo desconhecido como zero/infinito:** zerar `None`, aceitar limite ausente como infinito, cobrar depois da chamada, não cobrar reset/recovery ou ignorar overrun.
3. **Execução durante a proposta:** chamar executor, despachar ação, emitir envelope, assinar, reservar nonce ou esconder a execução em contador de probe.
4. **Digest sem atestação:** aceitar `Effect.from_states()`, digest próprio, `contract_digest`, `observation_digest`, `TransferProposal`, `True` ou `SUPPORTED` como prova de autoridade, completude ou fechamento.

Adicionar as variantes de autoridade, época, recovery e oráculo: host/ação/proposta adulterados; nonce reservado depois do efeito; replay após reboot; TTL ou revogação verificados somente no início; timeout convertido em sucesso; retry sem recovery; `oracle_reason` copiado para `runtime_reason`; reconhecimento dos IDs históricos; e seleção por nome ou posição.

A aceitação deve observar o efeito e o traço, não apenas a string `reason`. Um plano devolvido, uma chamada extra ou um efeito que depois seja revertido continuam sendo falhas.

### Fase 4 — fechar a prova mecânica e publicar uma classificação honesta

Rodar, em ordem, a regressão v1, a suíte nova, o oráculo isolado, a campanha estendida, o harness de mutação e três repetições em processos limpos. Comparar resultados normalizados e verificar todos os hard limits.

A etapa 6 só pode ser classificada como `mechanism_only` se todos estes itens forem verdadeiros:

- os 15 registros v1 e o schema histórico continuam reproduzíveis;
- alias, conflito, stale, replay, digest inválido, campo ausente e ausência de atestação produzem decisão tipada e proposta nula quando exigido;
- nenhum custo desconhecido vira zero, `OPEN` ou `SUPPORTED`;
- probe, reset, passo, retry e recovery são cobrados antes da chamada e permanecem monotônicos;
- construção de proposta tem zero execução autorizada e zero efeito externo;
- envelopes sem autoridade, fora de escopo, expirados, revogados ou com nonce repetido são rejeitados antes do efeito;
- `FAILED_PARTIAL` e `UNKNOWN_OUTCOME` ativam recovery sem retry automático;
- o recovery é independente e não fabrica rollback;
- o oráculo não compartilha a lógica de transição do runtime;
- três repetições limpas são determinísticas;
- cada mutação crítica é morta, inclusive em variante renomeada.

Uma única violação dura mantém a classificação em **`not_proven`**. Passar nos negativos por abstenção universal também não basta: é necessário um controle positivo representável, com contrato completo, custo finito verificável, ação unívoca, probe realmente não mutante e atestação externa. Mesmo nesse caso, o resultado é um plano suportado no escopo de teste, não autorização nem segurança física.

### Fase 5 — preparar a prova humana sem misturá-la ao mecanismo

A implementação do protocolo social e seus testes sintéticos pode ocorrer agora, em uma árvore nova. A coleta humana só começa depois de a Fase 4 passar e depois de obter as aprovações necessárias.

Criar:

```text
research/social/utility_protocol_v1.json
research/social/results.schema.json
research/social/runner.py
research/social/analyze.py
research/social/consent_pt-BR.md
research/social/accessibility_checklist.md
research/social/operator_manual.md
research/social/test_utility_protocol.py
research/evidence/stage7_social_v1/protocol.json
research/evidence/stage7_social_v1/privacy_accessibility.md
```

O protocolo deve congelar antes de qualquer resultado: tarefa, três condições `CB`, `NS` e `H`, seed, contrabalanço, prática, número de ensaios, limites de tempo, métricas, limiares, retenção, exclusão, consentimento e análise. A tarefa deve ser local, reversível e não médica: escolher uma resposta preparada, visualizar uma prévia e confirmar ou cancelar. Nenhuma mensagem real ou atuador deve ser acionado.

A condição `H` deve usar somente uma proposta; abstenção não pode ser repetida automaticamente nem contada como sucesso. `NS` deve rodar em processo limpo, sem runtime, skill ou chamada HERUS. `CB` deve reproduzir a configuração manual e ter assistência, layout, modalidade e limites comparáveis. A máquina de estados deve exigir `prompt_shown → input_started → intent_selected → preview_shown → confirm_or_cancel → local_outcome`.

Registrar por participante e ensaio erro primário, tipos de erro, tempos separados, entradas, correções, remapeamentos, fallback, prompts de operador, configuração de acessibilidade, incidentes e retirada. Não registrar áudio, vídeo, transcript, embedding, nome, e-mail, localização, chave, endereço de rádio ou texto livre. Os testes `HUM-01` a `HUM-15` devem operar em dados sintéticos e nunca ser apresentados como evidência humana.

A regra de progressão já congelada é apenas deste piloto: 12 adultos, H contra NS como contraste principal, H contra CB como não deterioração; erro de H no máximo 5 pontos percentuais acima de CB; redução de pelo menos 20% no esforço de configuração/remapeamento contra CB; e penalidade de troca de host de H menor que a de NS ou explicitamente sem benefício. Esses limiares não sustentam generalização populacional.

## O que deve ser implementado agora

| Prioridade | Entrega local | Condição de aceite antes de seguir |
|---|---|---|
| 0 | pacote de baseline, hashes, comandos e proteção do v1 | regressão inicial registrada e v1 intacto |
| 1 | preflight estrito, telemetria real, bloqueio de probe mutante e separação proposta–execução | zero chamada autorizada/efeito durante proposta; custo desconhecido preservado |
| 2 | contratos do envelope, ledger durável, executor de laboratório, recovery e runner | nonce, TTL, revogação, partial, unknown e retry bloqueado passam por traço |
| 3 | contratos v2, fixtures, proxy serializado, oráculo independente, runner e manifestos | três repetições limpas determinísticas sem vazamento de rótulo |
| 4 | harness de mutação e testes focados/renomeados | todas as mutações críticas equivalentes são mortas |
| 5 | campanha mecânica completa e relatório de classificação | qualquer hard failure mantém `not_proven`; nenhum resultado positivo compensa autoridade insegura |
| 6 | protocolo social, schema, runner local, análise e testes sintéticos | protocolo selado, sem dados humanos, sem execução externa |

A implementação deve ser aditiva quando possível. Não editar `holdout_benchmark_v1.json`, seu schema, `research/interaction_study_manifest.json` ou os wrappers históricos para introduzir a nova semântica. Os artefatos novos devem ter versão, digest e caminho próprios.

## O que exige pessoas, hardware ou autorização externa

### Pessoas e revisão ética

A coleta da Fase 5 exige responsável pelo estudo, operador treinado, revisão ética ou institucional aplicável, consentimento em linguagem simples, política de retirada, proteção de dados e suporte de acessibilidade. O piloto planejado requer 12 adultos. Esses participantes não podem ser simulados e seus resultados não podem ser substituídos por testes sintéticos, relatos de preferência ou uma demonstração do operador.

### Hardware e acesso ao ambiente-alvo

Os testes host-only podem usar fixtures e processos locais. A prova humana exige duas interfaces comparáveis e a modalidade de entrada escolhida pelo participante. Qualquer alegação sobre ESP32, rádio, atuador, dispositivo físico ou segurança operacional exige o hardware correspondente, bancada isolada, caminho de parada independente, instrumentação de efeitos e avaliação de segurança própria. O protocolo humano proposto não deve acionar hardware externo.

### Autoridade e infraestrutura de execução

Uma execução real requer uma autoridade independente com identidade, raiz de confiança, chave, época e política de revogação fora do runtime; um executor autorizado separado; armazenamento durável do ledger e do nonce; relógio monotônico verificável; e uma fonte independente para recovery. Também requer autorização explícita do responsável pelo host e pelo efeito. Nenhum digest criado pelo HERUS, `SUPPORTED`, skill verificada ou envelope emitido localmente substitui essa autorização.

Enquanto esses elementos externos não existirem, o que pode ser entregue é o protocolo, o executor de laboratório, os testes e a evidência `not_proven` ou `mechanism_only` dentro de um escopo simulado. Não se deve apresentar a simulação como permissão para agir no mundo.

## Testes de aceitação consolidados

1. **Regressão:** preservar os 15 registros v1, o schema, os hashes, a classificação histórica e a suíte atual de 135 testes com 1 ignorado.
2. **Separação:** instalar sentinela no executor e verificar que toda proposta mantém `proposal_execute_calls == 0`, `authorized_calls == 0`, `committed_calls == 0`, `external_effect_count == 0` e `step_count == 0`.
3. **Autoridade:** ausência, assinatura inválida, escopo adulterado, host/época divergente, TTL inválido, revogação stale e autoridade circular devem rejeitar antes do efeito.
4. **Ledger:** reserva de nonce ocorre antes do primeiro efeito; replay, reboot, reset, bind e recovery não liberam nonce nem reduzem contadores.
5. **Resultado desconhecido:** timeout, crash, ACK ausente e recibo conflitante produzem `UNKNOWN_OUTCOME + RECOVERY_REQUIRED`, nunca zero, sucesso ou retry automático.
6. **Temporalidade:** replay, epoch rollback, rebind, reset sem recibo e mistura de épocas invalidam a evidência no modo estrito.
7. **Estrutura:** pré-condição falsa, ordem invertida, recurso ausente, piso violado, custo desconhecido, alias e efeito proibido não geram proposta positiva.
8. **Independência:** imports proibidos, payloads, razões e tabela privada não atravessam a fronteira do oráculo antes do fechamento do runtime.
9. **Mutação:** executar todas as classes `ABS`, `COST`, `SEP`, `ATT`, `AUTH`, `NONCE`, `TTL`, `REVOKE`, `UNKNOWN`, `REC`, `RETRY`, `OBS`, `ORACLE`, `VAR` e suas variantes renomeadas.
10. **Determinismo:** três processos limpos com manifesto, commit, seed e relógio iguais produzem resultados normalizados iguais.
11. **Social sintético:** validar três condições, ausência real de HERUS em `NS`, ausência de execução em `H`, máquina de estados, cancelamento, abstenção, retirada, privacidade, acessibilidade e análise sem `oracle_reason`.
12. **Classificação:** qualquer hard failure, lacuna de autoridade, mutante sobrevivente relevante ou não determinismo mantém `not_proven`; mecanismo sem humanos permanece `mechanism_only`; somente todas as dimensões do contrato social podem sustentar uma conclusão humana limitada à tarefa e população estudadas.

## Riscos que permanecem mesmo após a implementação local

- **Autoridade circular:** runtime, host e autoridade podem compartilhar chave ou emissor. Mitigação: raiz, processo, identidade e política independentes, com teste de proveniência.
- **Probe mutante disfarçada:** separar contadores não torna `host.execute()` não mutante. Mitigação: API de leitura, clone descartável ou proxy de processo; caso contrário, bloquear.
- **Nonce volátil:** perder o ledger em reboot permite replay. Mitigação: durabilidade comprovada ou bloqueio fechado.
- **Commit confundido com ACK:** resposta, `expected_final` ou contagem de chamadas podem ser incompletos. Mitigação: recibo terminal por passo e recovery independente.
- **Circularidade do oráculo:** processos separados podem compartilhar tabela ou função defeituosa. Mitigação: tabela declarativa distinta, imports proibidos, digest de proveniência e variantes estruturais.
- **Sobreajuste ao holdout:** reconhecer IDs ou nomes mascara falhas. Mitigação: IDs opacos, permutações, seeds separadas, novos topologias e mutações.
- **Compatibilidade permissiva:** consumidores antigos podem transformar `True` ou proposta em comando. Mitigação: auditoria de consumidores, depreciação explícita e adaptador externo obrigatório.
- **Abstenção universal:** bloquear tudo não demonstra competência. Mitigação: controle positivo representável e independente, sem compensar hard failures negativos.
- **Confusão entre mecanismo e utilidade:** um holdout aprovado não prova benefício humano. Mitigação: pacote social separado, baseline convencional, `NS`, métricas por participante e análise de falhas.
- **Risco humano e de privacidade:** assistência, ordem, familiaridade, retirada ou logs podem fabricar benefício ou expor dados. Mitigação: protocolo selado, operador cego, saída independente, minimização, retenção e incidentes publicados.
- **Generalização indevida:** 12 adultos, uma tarefa e duas interfaces não representam outras populações, tarefas ou contextos. Mitigação: limitar literalmente a conclusão ao protocolo observado.

## Conclusão permitida

Até a conclusão da sequência, a frase correta é: **“O HERUS possui um plano implementável para fechar a fronteira proposta–execução e testar um mecanismo host-only, mas a prova integrada permanece `not_proven`.”**

Se a Fase 4 passar integralmente, a frase máxima é: **“No escopo dos hosts, contratos, épocas, budgets, fixtures, autoridade de teste e falhas congelados, o mecanismo evitou os falsos consensos e as execuções não autorizadas especificados.”**

Somente após a coleta humana aprovada, com os três braços e todos os hard limits, pode-se dizer que houve uma diferença medida nesta tarefa, população, interfaces, período e protocolo. Nada nesta sequência autoriza alegações de segurança física geral, privacidade de produto, eficácia clínica, autonomia ampla, consciência ou inteligência geral.

## Referências

[1]: ./01-external_executor.md "Plano restante — executor externo e recuperação"
[2]: ./02-extended_holdout.md "Plano restante — holdout temporal e estrutural estendido"
[3]: ./03-mutation_gates.md "Plano restante — mutações e gates adversariais"
[4]: ./04-human_utility_protocol.md "Plano restante — protocolo de utilidade humana"
[5]: ./05-system_architecture.md "Plano restante — arquitetura restante para integração final"
[6]: ../../symbiont_v2/core.py "HERUS Symbiont v2 research runtime"
[7]: ../../symbiont_v2/stage5.py "Tipos Stage 5 de decisão e ledger de orçamento"
[8]: ../../holdout_adversarial.py "Campanha Stage 4 minimum holdout"
[9]: ../holdout_benchmark_v1.json "Resultado histórico do holdout v1"
[10]: ../../symbiosis_utility_contract.json "Contrato congelado de utilidade da simbiose HERUS"

<!-- Síntese produzida a partir dos cinco relatórios e da inspeção do estado atual do código. -->
