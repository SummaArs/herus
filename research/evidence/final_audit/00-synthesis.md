# Síntese de direção científica e de produto — rodada final de alto impacto

**Commit avaliado:** `fbf7471c4903fd6af285f9e6e6a0e10f7cb2a967`  
**Decisão:** `not_proven`  
**Escopo máximo recomendado para a próxima rodada:** mecanismo host-only de proposta e um piloto local formativo; sem alegação de execução segura, produto lançado ou simbiose útil.

## Veredicto executivo

Os seis relatórios convergem no ponto central e os defeitos mais graves são reais, não ruído. O HERUS contém um **mecanismo interessante de reancoragem de uma Skill finita entre hosts**, mas a camada de evidência concede nomes fortes a propriedades que ainda não implementa. Hoje, `SUPPORTED` pode resultar de strings forjadas; um orçamento zero ainda permite uma probe; o “executor” confirma passos que nunca despachou; o oracle copia o rótulo da fixture; e o protocolo humano pode perder controles normativos sem ser bloqueado.[1] [2] [3] [4] [5] [6]

A suíte verde não contradiz esse diagnóstico. No checkout auditado, `PYTHONPATH=. python3 -m unittest discover -s research -p 'test_*.py' -q` executou **142 testes, com um skip e nenhuma falha**, e `./prove.sh --quiet` terminou com sucesso. Esses resultados demonstram regressão interna. Não demonstram as propriedades ausentes, porque os asserts atuais não as exigem.[1] [6]

A decisão de produto é reduzir, não expandir. A última rodada deve transformar o HERUS em uma demonstração honesta de **continuidade de uma rotina comunicacional preparada entre duas interfaces locais**. O runtime somente propõe. A pessoa revisa e confirma uma saída local. Não há envio real, rádio, voz, memória pessoal, atuador ou autoridade implícita. Esse recorte liga o mecanismo existente a uma tarefa humana compreensível sem mascarar lacunas com hardware ou linguagem de segurança.[4] [5]

> **Resultado máximo desta rodada:** “O HERUS constrói ou recusa, com evidência auditável, uma proposta de remapeamento de uma rotina finita entre duas interfaces locais; em sessões formativas, pessoas conseguem entender, confirmar, cancelar e parar o fluxo.”
>
> Mesmo um resultado positivo não autoriza os termos **execução segura**, **simbiose útil**, **produto indispensável**, **comunicação espontânea**, **acessibilidade clínica**, **AGI** ou **pronto para hardware**.

## Confirmações independentes no checkout atual

Foram repetidas sondas curtas no mesmo commit dos relatórios. Elas confirmam que os achados determinantes continuam presentes:

| Sonda | Resultado observado | Conclusão |
|---|---|---|
| Contratos de Skill e host com digests literais `FORGED` em modo `STRICT` | `PROPOSED / SUPPORTED`; uma probe no host; zero probes no ledger | **Bloqueador real:** presença de texto é promovida a suporte. |
| `probe_max=0`, `reset_max=0`, `cost_max=0` em `COMPATIBILITY` | uma probe ocorreu; ledger continuou com zero probes e custo `None` | **Bloqueador real:** o orçamento não governa a operação. |
| Envelope com três passos, custo máximo um e digests `bogus` | `COMMITTED`, três efeitos sintéticos | **Bloqueador real para qualquer claim de execução:** o componente não valida nem executa um plano real. |
| Recovery de operação inexistente, tentativa 99 e ledger arbitrário | `RECOVERY_RESOLVED` | **Bloqueador real para recovery:** não há vínculo com uma tentativa pendente. |
| Mutação do objeto inserido e do snapshot do ledger | ambas alteraram retroativamente o digest | **Bloqueador real para evidência append-only.** |
| Alteração apenas de `expected_negative` na fixture opaca | oracle mudou de `FAIL_UNSAFE` para `SUPPORTED` | **Bloqueador real:** o oracle é circular. |
| Relatório contendo somente métricas de mecanismo | `mechanism_only` | **Bloqueador real:** o classificador viola o próprio contrato fail-closed. |
| Protocolo social sem controles, thresholds, regra de análise e claims proibidos | nenhuma violação | **Bloqueador real:** “ready” não significa protocolo completo. |
| `false | tee` no padrão usado pelo CI | status zero | **Defeito real de infraestrutura:** existe caminho de falso-verde. |
| Imports normais de três módulos de `research` | `ModuleNotFoundError` | **Dívida real**, mas não é o gargalo científico desta rodada. |

O README também afirma **286 testes Python**, enquanto a descoberta documentada e reproduzida encontra 142; aponta seis documentos inexistentes; e declara `pre_hardware_frozen` apesar de as novas fronteiras de evidência permanecerem abertas.[6] [7] O documento mestre publica números de alcance que divergem da saída corrente de `tools/budget.py`.[1] [5] Isso não invalida o mecanismo host-only, mas torna a narrativa de maturidade maior que a evidência.

## Triagem: realidade, dívida e ruído

“Ruído” aqui não significa que o tema seja falso. Significa que atacá-lo nesta rodada consumiria tempo sem reduzir a principal incerteza científica ou aproximar a tarefa humana escolhida.

| Família de achados | Julgamento | Decisão |
|---|---|---|
| `SUPPORTED` por contrato não autenticado | **Real e crítico agora** | Tornar o estado inalcançável no produto até existir verificador externo verdadeiro. |
| Probe mutante e orçamento decorativo | **Real e crítico agora** | Separar probe de commit e debitar orçamento antes de cada operação. |
| Oracle circular, host no mesmo processo, holdout público e m7/m8 rotulados | **Real e crítico agora** | Substituir por uma campanha causal isolada; rebaixar a campanha atual a regressão histórica. |
| Executor sem dispatcher, custo, TTL por passo, retry e recovery vinculados | **Real e crítico se o claim for mantido** | Não construir um executor de produção agora. Remover o componente do caminho de produto e rotulá-lo como harness sintético. |
| Ledger mutável, receipts sem atestação e recovery forjável | **Real** | Corrigir o necessário para integridade do harness; não alegar durabilidade ou autorização externa. |
| Classificador e runner social permissivos | **Real e crítico agora** | Fazer ambos falharem fechados antes de qualquer sessão com pessoas. |
| CI sem `pipefail`, contagem de testes, links quebrados e linguagem inflada | **Real e barato** | Corrigir no primeiro commit da rodada. |
| Imports inconsistentes e falta de `pyproject.toml` | **Real, mas secundário** | Corrigir apenas o caminho usado pela nova campanha; adiar empacotamento geral. |
| Proveniência de release, SBOM, tags e builder assinado | **Real para release, prematuro agora** | Não chamar o repositório de release; adiar a cadeia completa. |
| Números RF divergentes e ausência de medição física | **Real, fora do experimento escolhido** | Quarentenar os claims físicos; não recalibrar o produto em torno deles. |
| Implementar agora PKI, nonce durável, revogação distribuída e executor transacional | **Ruído estratégico desta rodada** | O custo é alto e não responde se a transferência finita ajuda alguém. Só volta ao roadmap se um caso de uso exigir efeito externo. |
| Aumentar o número de fixtures ou de testes sem mutações matadoras | **Ruído** | Não otimizar contagem. Otimizar a capacidade de detectar fraude e regressão causal. |
| Congelar já 12 participantes, 5 pontos percentuais e 20% como “benefício” | **Ruído prematuro** | Usar esses números, se mantidos, apenas como gates exploratórios; primeiro validar compreensão e mensuração. |
| Retomar wearable, rádio, voz, LLM, memória pessoal, mesh ou “segundo cérebro” | **Distração de produto** | Manter fora da rodada. |

## As três mudanças que devem ser feitas

### P0 — Construir uma barreira epistemológica no código e na comunicação

**Objetivo:** impedir que estados, nomes e dashboards façam o projeto parecer mais provado do que está.

Esta mudança deve ser o primeiro commit e não depende de redesenhar a arquitetura. `SafetyClaim.SUPPORTED` permanece no schema apenas como valor reservado, mas nenhum caminho atual pode emiti-lo. Contrato presente e contrato atestado tornam-se estados distintos; o máximo do runtime atual é `SAFE_BUT_UNPROVEN`. O módulo `ExternalExecutorAdapter` deve sair do caminho de produto e ser renomeado ou encapsulado como `SyntheticExecutionHarness`; seus commits tornam-se explicitamente simulados. A campanha v1 permanece versionada, mas passa a ser chamada **regressão pública histórica**, não holdout independente.[2] [3] [8] [9]

O mesmo patch deve corrigir os gates de linguagem. `classify()` retorna `not_proven` diante de qualquer dimensão ou controle obrigatório ausente. `mechanism_only` só é possível quando mecanismo, segurança, privacidade e reprodutibilidade estiverem completos e apenas o valor humano não tiver sido demonstrado. O runner social deve retornar `DRAFT_PROTOCOL_NO_HUMAN_DATA` ou `BLOCKED`, nunca uma prontidão ambígua. O CI deve usar `pipefail`, validar que o ledger final foi produzido e executar os validadores de evidência existentes.[5] [6] [10] [11]

Por fim, README e documentos ativos devem refletir o estado real: 142 testes na suíte de pesquisa observada; ausência de release; links válidos; campanha host-only pública; claims físicos suspensos enquanto `tools/budget.py` e o documento mestre divergem. Não se deve apagar o histórico nem regenerar resultados para fazê-los parecer positivos.

**Por que esta prioridade atende aos três critérios:** elimina autoengano imediatamente; torna qualquer demonstração interpretável; e evita levar uma falsa garantia para a experiência humana.

**Definição de pronto:** não há emissão de `SUPPORTED`, `COMMITTED` não aparece como execução real, um pacote humano incompleto fica em `not_proven`, uma remoção de controle social bloqueia o runner, e uma falha deliberada em `prove.sh` deixa o CI vermelho.

### P1 — Substituir o holdout teatral por um único experimento causal isolado

**Objetivo:** demonstrar o mecanismo central com uma fronteira que não dependa da boa vontade do próprio runtime.

Criar uma nova campanha pequena, específica para o HERUS Bridge. Runtime, host e oracle devem rodar em processos separados e trocar somente JSON canônico. O runtime recebe token opaco, ações públicas, observações e orçamento. Não recebe a fixture, labels, estado privado, motivo esperado ou referência Python para o host. O oracle recebe a proposta final e uma tabela privada de transições; deriva o veredicto das ações, ordem, pré-condições, custo e efeitos. `expected_negative` pode existir apenas como assert posterior.[3] [4]

A API do host precisa separar `probe()` de `commit()`. `probe()` é não mutante por construção e o runner independente compara o digest privado antes e depois. Se o host não oferecer essa garantia, `STRICT` bloqueia antes da probe. O orçamento é reservado antes de cada probe e reset; os contadores e o custo do resultado são derivados do trace. A campanha de proposta não deve conter m7/m8 como se fossem propriedades do mecanismo. Esses casos só podem permanecer em uma suíte separada, claramente sintética, quando realmente produzirem falha parcial, ACK ausente e custo excedido no harness.[1] [3] [6]

O caso central é uma família gerada de `probe–commit gap`: duas variantes produzem transcripts públicos idênticos em todas as probes, mas uma acrescenta efeito oculto somente no primeiro commit. Sem fechamento autenticado, ambas devem receber a mesma decisão conservadora; nenhuma pode virar `SUPPORTED`. IDs, nomes, ordem e estado inicial devem ser permutáveis por seed para impedir regras especiais por prefixo.[3] [4]

O artefato da campanha deve conter trace bruto ou seu manifesto verificável, commit, digest do runtime, digest do oracle, digest do conjunto de casos, seed, comando, ambiente, schema e contadores rederiváveis. Um validador independente deve rejeitar qualquer alteração material. O objetivo não é segredo permanente dentro do Git; é **separação causal durante a execução** e geração após o freeze do runtime. Um holdout realmente externo continua necessário para uma publicação forte.

**Por que esta prioridade atende aos três critérios:** mata os contraexemplos mais fortes com poucos casos; gera uma demo reproduzível que um auditor entende; e fornece a base segura para mostrar a transferência a uma pessoa.

**Definição de pronto:** mutações de label não mudam o oracle; probes não mudam o estado privado; orçamento zero causa zero chamadas; o caso `probe–commit gap` nunca recebe suporte; e qualquer alteração de trace, contador ou digest invalida o pacote.

### P2 — Entregar o HERUS Bridge como vertical slice local e medir compreensão antes de benefício

**Objetivo:** converter a tese abstrata em uma tarefa humana curta, reversível e observável.

A experiência deve permitir que uma pessoa defina três respostas preparadas e a opção **nenhuma ação** em uma interface de origem, troque para uma interface-alvo com controles e ordem diferentes, revise a proposta e escolha confirmar ou cancelar. A confirmação mostra apenas a frase correspondente em uma tela local. Nenhuma mensagem é enviada e nenhum efeito externo é autorizado. O produto transporta um contrato tipado de rotina, não áudio, texto livre, memória pessoal ou autoridade.[5]

Implementar no mesmo runner as três condições: **CB**, remapeamento manual no alvo; **NS**, uso do alvo sem runtime ou identidade HERUS; e **H**, proposta de transferência pelo HERUS. Todas usam a mesma tarefa, número de opções, prévia e modalidade de entrada. O schema de eventos deve registrar setup, entradas, troca de host, proposta, abstenção, confirmação, cancelamento, timeout, fallback, intervenção, parada e incidente de acessibilidade. Em NS, uma sentinela deve provar zero import ou chamada ao HERUS. Em H, uma sentinela deve provar zero commit do runtime.[4] [5]

Depois de um ensaio sintético completo, realizar somente **cinco ou seis sessões formativas** para verificar se pessoas entendem `rascunho`, `confirmar`, `cancelar`, `abster` e `bloqueado`, e se conseguem parar sem ajuda. Não calcular “utilidade social” nessa fase. Um piloto pareado CB/NS/H só deve ser preparado depois que P1 passar e o protocolo, consentimento, acessibilidade, retenção e análise tiverem validação independente.

A decisão ao fim da rodada deve ser binária. Prosseguir se o mecanismo passa a campanha e as pessoas entendem o fluxo sem autoridade ambígua. Encerrar ou redesenhar a linha se a troca continua exigindo remapeamento, a abstenção impede uso frequente, ou a pessoa não sabe quando algo será feito.

**Por que esta prioridade atende aos três critérios:** obriga o projeto a declarar uma utilidade concreta; produz uma demonstração compreensível em 15–20 minutos; e permite matar a tese de produto cedo, antes de hardware ou coleta ampla.

**Definição de pronto:** os três braços são causalmente separados; eventos inválidos são rejeitados; uma abstenção nunca conta como sucesso; confirmar sem prévia é impossível; e todas as modalidades oferecidas conseguem iniciar, cancelar e parar sem intervenção do runtime.

## Ordem de execução e gates de parada

| Ordem | Entrega | Gate para avançar |
|---|---|---|
| 1 | Patch P0 de estados, classificadores, CI e documentação | Todas as sondas de falsa promoção falham fechadas. |
| 2 | Runner isolado P1 e família `probe–commit gap` | Zero falso suporte, zero mutação em probe e pacote rederivável. |
| 3 | HERUS Bridge sintético com CB/NS/H | Separação causal e máquina de estados passam por mutação. |
| 4 | Cinco ou seis sessões formativas | Usuários entendem ação, não ação, cancelamento e parada. |
| 5 | Decisão explícita: avançar, redesenhar ou parar | Nenhuma nova frente começa sem esta decisão. |

Se P0 falhar, não há demonstração pública. Se P1 falhar, não há sessão humana. Se P2 exigir rádio, LLM, voz, memória pessoal ou executor externo para parecer útil, a hipótese de produto escolhida falhou e não deve ser salva por expansão de escopo.

## Testes obrigatórios da rodada

| ID | Teste | Resultado exigido |
|---|---|---|
| T1 | Contrato ausente, textual, forjado, stale, de outro host ou alterado após digest | Nunca emitir `SUPPORTED`; zero probe quando a garantia é necessária. |
| T2 | Orçamento de zero probes/resets/custo | Zero chamada ao host; ledger e trace permanecem coerentes. |
| T3 | Mutação de `expected_negative`, `oracle_verdict` e `oracle_reason` | Veredicto causal permanece igual e a justificativa é derivada. |
| T4 | Tentativa de acesso a `_fixture`, `_private_state`, `_inner` ou `__dict__` | O processo do runtime não possui a referência nem o caminho privado. |
| T5 | Digest privado antes/depois de cada probe | Igualdade obrigatória; qualquer mutação bloqueia a proposta. |
| T6 | Variantes segura e insegura de `probe–commit gap` com transcript idêntico | Mesma decisão conservadora; nenhum `SUPPORTED` sem fechamento externo. |
| T7 | IDs, ações, ordem, estado e seed permutados | O resultado depende da causalidade, não do nome. |
| T8 | m7/m8, se mantidos | Traces reais e distintos de parcial/ACK e overrun; caso contrário, remover os claims. |
| T9 | Mutação de status, contador, custo, trace, fixture, seed ou digest | Validador rejeita o pacote. |
| T10 | Falha deliberada de `prove.sh` sob o comando do workflow | Job vermelho, mesmo com `tee`. |
| T11 | Remoção isolada de dimensão ou controle do relatório humano | `not_proven` ou `BLOCKED`; nunca `mechanism_only` por completude parcial. |
| T12 | CB, NS e H com sentinelas | CB não chama runtime; NS não importa HERUS; H não comita efeito externo. |
| T13 | Confirmar sem prévia, duplicar trial, reordenar timestamps, converter timeout ou abstenção em acerto | Schema/validador rejeita. |
| T14 | Iniciar, cancelar, parar e sair em cada modalidade oferecida | Sucesso sem ajuda do runtime; intervenção vira desvio publicado. |

## O que não deve ser tocado nesta rodada

1. **Não reabrir firmware, rádio, mesh, energia, alcance, LRA ou placa física.** Esses componentes não respondem se a transferência entre interfaces é real ou útil. Os claims físicos divergentes devem ser suspensos, não “corrigidos” por mais simulação.
2. **Não adicionar LLM, voz, diálogo aberto, memória pessoal, Watch, Paper-Core ou “segundo cérebro”.** Cada item introduz novas hipóteses, riscos e métricas sem fortalecer o teste central.
3. **Não construir agora um executor externo de produção.** Assinatura, PKI, relógio confiável, revogação, nonce durável, dispatcher, armazenamento transacional e recovery distribuído formam outro programa. Nesta rodada, remova o claim e preserve apenas um harness sintético explicitamente rotulado.
4. **Não reescrever o ASA Core, a álgebra, VSA/HDC, identidade persistente ou as suítes de firmware que já passam.** O gargalo está na fronteira de evidência e no produto, não no volume de arquitetura existente.
5. **Não ampliar a campanha com dezenas de casos semelhantes.** Implemente poucas famílias causais e mutation tests que matem atalhos. Contagem de testes não é métrica de validade.
6. **Não coletar um piloto confirmatório antes dos gates P0 e P1.** Nenhuma pessoa deve ser exposta a um fluxo cuja condição H ainda promove contrato forjado ou cuja probe pode mutar o host.
7. **Não escolher vencedor por um índice composto pós-hoc.** Erro, esforço, penalidade de troca, abstenção, intervenção e acessibilidade devem permanecer resultados separados.
8. **Não apagar nem “embelezar” os artefatos v1.** Preserve-os como evidência histórica de `not_proven`; publique um v2 somente com novo schema e nova campanha.
9. **Não gastar a rodada em SemVer, SBOM, assinatura de release ou empacotamento completo.** Corrija o mínimo necessário para executar P1 em ambiente limpo. A cadeia de release só se justifica depois de uma decisão positiva de produto.

## Critério de encerramento da rodada

A rodada termina com sucesso somente se houver um repositório que **não consegue declarar mais do que demonstrou**, uma campanha independente que falsifica atalhos conhecidos e uma experiência local que uma pessoa entende sem confundir proposta com ação. O HERUS pode então ser descrito como **mecanismo host-only demonstrado para uma tarefa finita e candidato a piloto humano**.

Se qualquer elo falhar, a classificação permanece `not_proven`. Isso não é fracasso administrativo; é o resultado científico correto. O pior desfecho seria continuar acumulando módulos, fixtures e claims sobre uma fronteira que os próprios testes não conseguem vigiar.

## Referências

[1]: 01-soundness.md "Auditoria adversarial de soundness código–documentação"
[2]: 02-executor_security.md "Auditoria adversarial de segurança do executor"
[3]: 03-blackbox_holdout.md "Auditoria adversarial do holdout, do oráculo e da fronteira black-box"
[4]: 04-symbiosis_theory.md "Auditoria adversarial da hipótese ASA e da simbiose"
[5]: 05-human_product.md "Auditoria adversarial de produto e utilidade social"
[6]: 06-engineering_quality.md "Auditoria adversarial de engenharia e integração"
[7]: ../../../README.md "README do repositório HERUS"
[8]: ../../symbiont_v2/core.py "Runtime Symbiont v2 e decisão checked"
[9]: ../../stage4/executor.py "Adaptador experimental de executor externo"
[10]: ../../symbiosis_utility.py "Validador e classificador de simbiose útil"
[11]: ../../../.github/workflows/prove.yml "Workflow de prova do repositório"
