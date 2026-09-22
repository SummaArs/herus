# HERUS — protocolo de utilidade humana v1

**ID:** `HERUS-REM-HUM-004`  
**Frente:** protocolo de utilidade humana  
**Status:** desenho implementável; **nenhuma alteração de código foi feita nesta frente**.  
**Escopo:** primeira tarefa social pequena, reversível e não médica para medir valor humano separadamente do mecanismo host-only.

## 1. Decisão executiva

A primeira tarefa deve medir se uma rotina curta de comunicação preparada pode ser preservada quando a pessoa troca de interface. A tarefa será deliberadamente de baixa consequência: a pessoa escolherá uma resposta pronta em uma interface e confirmará ou cancelará uma visualização local. Nenhuma mensagem será enviada a uma pessoa real, nenhum atuador será acionado e nenhum resultado será usado para diagnóstico, tratamento ou decisão de saúde.

A avaliação terá três condições congeladas e comparáveis:

1. **CB — baseline convencional:** a pessoa usa a interface-alvo convencional e configura manualmente, uma vez, a mesma preferência de comandos que será usada na condição HERUS.
2. **NS — sem simbionte:** a pessoa usa a mesma interface-alvo e a mesma modalidade de entrada, mas sem runtime HERUS, sem Skill transferida e com o mapeamento nativo padrão do host.
3. **H — HERUS:** a pessoa usa uma interface de origem para estabelecer a rotina e aprova uma proposta HERUS que a reancora na interface-alvo. A proposta é somente um plano; não há execução externa.

A comparação principal será **H contra NS** para testar se a transferência acrescenta continuidade em relação ao host sem simbionte. **H contra CB** testará se a mesma continuidade pode reduzir o custo de configuração manual sem piorar o erro. CB não deve ser escolhido depois dos resultados: seu procedimento, layout, número de opções, tempo de configuração e assistência permitida devem estar no manifesto congelado.

O protocolo só deve ser executado como avaliação confirmatória depois de o gate mecânico Stage 6 passar. O estado atual ainda preserva lacunas host-only descritas no holdout adversarial e nos planos Stage 5. Se a tarefa humana for executada antes desse gate, será uma prova de viabilidade exploratória, classificada como **não provada**, e não como demonstração de utilidade do HERUS.

## 2. Tarefa social e cenário

### 2.1 Tarefa

O participante recebe, em tela acessível, um cartão de conversa de baixa consequência. O cartão informa qual resposta preparada deve ser escolhida. As respostas são neutras e não sensíveis:

- `continuar`: “Sim, podemos continuar.”
- `pausar`: “Prefiro pausar por agora.”
- `cancelar`: “Cancelar esta ação.”
- `nenhuma`: nenhum cartão deve ser confirmado.

O participante escolhe uma resposta na interface, visualiza uma prévia local e confirma ou cancela. O operador simula o interlocutor com cartões previamente escritos; não há conversa gravada, transcrição, reconhecimento de fala ou envio de mensagem. A opção `nenhuma` é necessária para medir falsos acionamentos. A opção `cancelar` encerra o ensaio localmente e não produz efeito externo.

A tarefa mede uma capacidade social estreita: **expressar uma preferência comunicacional preparada ao mudar de interface**. Não mede compreensão aberta de linguagem, reconhecimento de fala, autonomia geral, eficácia clínica, segurança física ou qualidade de relações sociais.

### 2.2 Participantes e unidade experimental

A primeira campanha deve ser um piloto de viabilidade com **12 adultos**, identificado como `u01`–`u12` no protocolo novo. O número não deve ser usado para alegar potência para a população. A inclusão pode contemplar pessoas que já usam modalidades de entrada alternativas, desde que o estudo não recolha diagnóstico, prontuário ou informação clínica. Também é aceitável incluir participantes sem uso prévio de tecnologia assistiva para testar a operação do protocolo; a análise deve estratificar essa característica apenas se ela for coletada com consentimento e pré-especificada.

Cada participante executa os três blocos em desenho intraindivíduo. A ordem `CB → NS → H`, `CB → H → NS`, `NS → CB → H`, `NS → H → CB`, `H → CB → NS` e `H → NS → CB` deve ser distribuída por uma sequência de contrabalanço congelada antes da coleta. O participante não deve ser exposto ao rótulo “simbionte” como uma promessa de desempenho; pode ser informado, de modo neutro, que há três formas de configurar a interface.

### 2.3 Blocos e ensaios

Cada condição tem quatro ensaios de origem para estabelecer ou revisar a rotina e oito ensaios na interface-alvo. Os oito ensaios-alvo contêm seis respostas positivas, duas para cada intenção, e dois ensaios `nenhuma`. Os três blocos, portanto, têm o mesmo número de ensaios, a mesma distribuição semântica e o mesmo limite de tempo. A ordem dos cartões é sorteada por participante e por bloco, com seed registrada.

Antes dos ensaios formais, o participante faz uma prática não analisada de seis cartões. A prática termina quando a pessoa entende a confirmação e o cancelamento ou quando atinge o limite pré-registrado; não se aumenta a prática apenas para melhorar uma condição. O operador pode explicar a operação da interface durante a prática, mas não pode corrigir respostas durante os ensaios formais.

Na condição CB, a configuração manual ocorre antes dos ensaios-alvo, usando um procedimento escrito e a mesma preferência semântica usada na condição H. Na condição NS, não há configuração manual nem chamada ao runtime; o participante usa o mapeamento padrão da interface-alvo. Na condição H, a transferência é construída pelo caminho checked e apresentada para confirmação explícita do participante. Se o runtime abster-se, a decisão é registrada como `ABSTAIN` ou `UNSUPPORTED_BY_CONTRACT`; não se repete automaticamente a proposta e não se conta a sessão como sucesso. Um fallback manual pode ser oferecido somente para recuperar a pessoa, e seus ensaios são marcados separadamente, nunca incorporados silenciosamente ao resultado H.

### 2.4 Máquina de estados do ensaio

O estado observável do ensaio deve seguir esta ordem:

```text
prompt_shown
  → input_started
  → intent_selected
  → preview_shown
  → confirm_or_cancel
  → local_outcome
```

`local_outcome` pode ser `confirmed_correct`, `confirmed_wrong`, `cancelled`, `timeout`, `participant_aborted` ou `system_error`. Um `confirmed_correct` só é válido quando a intenção selecionada coincide com o rótulo congelado no plano e a confirmação ocorreu depois da prévia. Um timeout não é convertido em acerto. Uma tentativa de confirmar sem prévia é erro de protocolo, ainda que a intenção final coincida por acaso.

O local de ensaio deve ter um botão ou gesto de parada independente do HERUS. A parada deve funcionar antes da confirmação e durante qualquer tela de transferência. Depois de uma falha, o operador oferece uma instrução manual curta e uma rota de saída. `reset()` da interface não é apresentado como rollback de um efeito externo; nesta tarefa não existe efeito externo autorizado.

## 3. Baselines e isolamento causal

### 3.1 Baseline convencional congelado

CB representa a solução convencional que uma pessoa poderia usar hoje para transportar uma preferência entre duas interfaces: configurar manualmente o mapeamento no host-alvo e depois usar a interface direta. O procedimento de configuração, os controles disponíveis, a ordem das telas, o limite de tempo e a assistência do operador são fixados no manifesto.

A configuração manual deve produzir o mesmo mapeamento semântico que H produz quando a proposta é aceita. Assim, a comparação H–CB mede o custo de obter a configuração, em vez de comparar duas tarefas diferentes. O tempo e os eventos de entrada da configuração são medidos separadamente do tempo de seleção dos cartões.

### 3.2 Condição sem simbionte

NS é o controle causal de ausência do mecanismo. O processo HERUS não é iniciado nessa condição, não há `herus_id` no registro de condição, não há chamada de `propose_transfer_checked()` e não há Skill ou evidência transferida. A interface-alvo mantém seu mapeamento nativo, com o mesmo número de opções, tamanho visual, modalidade de entrada e tela de confirmação utilizados em H. O teste de isolamento deve verificar a ausência de chamadas HERUS, não apenas confiar no rótulo da linha de dados.

NS não deve ser descrita como “grupo sem ajuda” nem como condição inferior. Ela responde somente ao que acontece quando o mesmo host é usado sem a camada de transferência. Diferenças de familiaridade, acessibilidade ou aprendizado devem ser registradas e discutidas, não escondidas em uma média.

### 3.3 Condição HERUS

H começa com a rotina na interface de origem e usa a interface-alvo com IDs e ordem de ações diferentes. O runtime recebe somente o contrato público autorizado. O oráculo e a tabela privada do holdout não podem ser entregues ao participante, ao operador ou ao analisador durante o ensaio. A construção da proposta deve manter `proposal_execute_calls == 0`, `authorized_calls == 0` e `external_effect_count == 0`.

H não pode usar o resultado esperado do ensaio para escolher a ação. O cartão do ensaio pode indicar a intenção que a pessoa deve expressar, mas essa informação pertence à tarefa e ao avaliador; ela não pode ser injetada como uma previsão do runtime nem escrita automaticamente no campo `observed`.

## 4. Métricas e análise congeladas

Os resultados devem ser guardados por participante, condição, host, ensaio e estado da máquina. Médias agregadas não substituem a matriz bruta. A análise primária deve ser pareada por participante; não se deve tratar os ensaios da mesma pessoa como observações independentes.

### 4.1 Erro da tarefa

O erro primário no host-alvo é:

```text
task_error_rate =
  (confirmed_wrong + timeout + participant_aborted) / target_trials
```

Devem ser publicados separadamente `wrong_intent_rate`, `false_confirmation_rate`, `timeout_rate`, `cancel_rate` e `system_error_rate`. `nenhuma` confirmada é erro se o cartão exige silêncio; uma intenção válida cancelada pelo participante não é confundida com erro do sistema. `system_error` inclui sequência de eventos impossível, confirmação sem prévia, perda de uma entrada já reconhecida ou apresentação de um rótulo diferente do plano. Qualquer erro do sistema deve ser investigado por traço, não compensado por acertos humanos.

O contraste primário é H–NS. O contraste H–CB é uma verificação de não deterioração. Para uma regra de progressão pré-registrada, H deve ter erro no máximo **5 pontos percentuais acima de CB** e não pode ter erro de sistema ou confirmação indevida. A margem é um critério de decisão deste piloto, não um padrão universal de acessibilidade.

### 4.2 Tempo e esforço

O tempo deve ser dividido em `setup_ms`, `prompt_to_selection_ms`, `selection_to_confirmation_ms` e `prompt_to_outcome_ms`. O esforço observável deve incluir `input_event_count`, `correction_count`, `manual_remap_count`, `fallback_count` e `operator_prompt_count`. Depois de cada bloco, o participante pode informar esforço percebido em escala de sete pontos, com opção “não consigo avaliar”; essa medida é secundária e não substitui o traço observável.

O resultado principal de esforço é reportado em dimensões separadas. Não se deve criar um índice composto depois de observar qual condição parece melhor. O critério de progressão proposto é uma redução de pelo menos **20% no esforço de configuração e remapeamento de H contra CB**, sem aumento de erro primário além da margem acima. Tempo de ensaio e contagem de entradas permanecem resultados separados, com mediana e intervalo interquartil por participante.

### 4.3 Dependência de dispositivo ou interface

A dependência será medida pela penalidade de troca de host e pela necessidade de intervenção específica do host:

```text
host_switch_penalty(metric) =
  median(target_metric) - median(source_metric)

interface_dependence =
  manual_remap_count + fallback_count + host_specific_prompt_count
```

As intenções e a distribuição de cartões de origem e alvo devem ser pareadas. Para erro, uma diferença positiva representa piora no alvo; para acertos, a análise deve usar a diferença de taxa em sentido contrário. O relatório deve publicar a penalidade por condição, não somente um índice agregado.

O critério de progressão é H apresentar penalidade de troca menor que NS e não maior que CB, além de manter a semântica correta nos IDs e na ordem de ações permutados. Uma melhoria de H em um único host, sem teste de troca, não é evidência de menor dependência de interface.

### 4.4 Tratamento estatístico e dados incompletos

A análise deve publicar mediana, intervalo interquartil, diferença pareada por participante e intervalo de confiança obtido por reamostragem pareada ou teste de permutação previamente especificado. Com 12 participantes, os resultados são de viabilidade; não se deve apresentar um p-valor como prova de generalização populacional. A análise não deve parar quando um resultado favorável surgir.

Nenhuma ausência de dado primário será imputada como acerto, erro zero ou esforço zero. Retirada, interrupção, falha de equipamento e exclusão por violação do protocolo devem ter códigos distintos e ser publicados. Uma análise por protocolo pode ser acompanhada de uma análise de todos os ensaios observados, mas não pode esconder a attrition.

### 4.5 Gate de qualidade do estudo

Antes de falar em benefício, os seguintes gates devem passar:

1. O protocolo, os contrastes, os limiares, a seed, a ordem de contrabalanço e o schema estão selados antes do primeiro resultado humano.
2. Todos os participantes recebem as três condições com contagem e distribuição iguais, ou a falta é reportada sem imputação.
3. A condição NS não chama HERUS; a condição CB não recebe um resultado H; H não recebe o gabarito do ensaio para decidir.
4. A proposta H não produz execução externa, e uma abstenção é registrada como abstenção, não como transferência bem-sucedida.
5. H não supera CB em erro além de 5 pontos percentuais e reduz em pelo menos 20% o custo de configuração/remapeamento na regra de progressão congelada.
6. A penalidade de troca de host de H é menor que a de NS ou, quando não houver diferença, a análise registra explicitamente ausência de benefício de dependência.
7. Não há liberação de áudio, transcript, embedding, identidade direta, localização, chave, endereço de rádio ou outro dado sensível sem consentimento específico. O limite de liberação sensível não consentida é zero.
8. Todos os participantes conseguem parar, cancelar um ensaio e recuperar-se de uma abstenção sem depender de uma decisão escondida do runtime.

Esses gates são critérios de qualidade e progressão. Mesmo que passem, o estudo só autoriza a classificação de evidência humana dentro desta tarefa e desta população. Ele não converte um mecanismo host-only em segurança geral nem em simbiose ampla automaticamente.

## 5. Consentimento, acessibilidade e privacidade

### 5.1 Consentimento

Antes da prática, o participante recebe uma folha em linguagem simples explicando que a tarefa é simulada, que nenhuma mensagem real será enviada, que não há diagnóstico ou tratamento, que o sistema pode abster-se e que a pessoa pode sair sem justificar-se e sem perder compensação. O consentimento deve incluir coleta de tempos, contagens de entradas, decisões observadas, falhas, respostas de esforço e registros de acessibilidade.

A compreensão deve ser verificada com pelo menos quatro perguntas: se há mensagem real, se a pessoa pode interromper, se a condição HERUS tem autorização para atuar fora da tela e quem pode solicitar exclusão dos dados. Resposta errada exige nova explicação antes da inclusão; não se deve registrar o conteúdo da resposta de compreensão junto ao pseudônimo se isso não for necessário.

O consentimento deve ser armazenado separadamente do resultado, com identificador pseudônimo e carimbo de versão. Uma pessoa de apoio pode estar presente se o participante solicitar, mas não pode escolher a intenção nem operar o dispositivo. A retirada deve interromper imediatamente a coleta e marcar as linhas já produzidas para exclusão conforme a política aprovada.

### 5.2 Acessibilidade

A interface deve oferecer, antes da randomização, tamanho de texto ajustável, alto contraste, foco de teclado, navegação por switch ou outro dispositivo de entrada já escolhido pelo participante, tempo de permanência configurável e confirmação explícita. A mudança de tamanho, contraste ou modalidade deve ser registrada como configuração, não como desempenho.

O participante pode pedir pausas, repetição de instrução e um ensaio de familiarização. A assistência deve ser padronizada por um manual. O operador não pode selecionar a resposta, tocar no dispositivo ou sugerir o cartão durante um ensaio formal. Uma falha de acessibilidade deve suspender o bloco e gerar um registro de incidente; não deve ser reduzida a “erro do participante”.

O protocolo deve testar a saída independente do HERUS, inclusive o botão de parada e a rota de cancelamento. Se a pessoa não consegue cancelar ou sair sem a ajuda do runtime, a condição é inválida para alegação de autonomia e o ensaio deve terminar.

### 5.3 Minimização e retenção

O registro deve conter somente `study_id`, pseudônimo, condição, host, bloco, trial, intenção esperada, intenção observada, estado terminal, timestamps relativos, contagens de entrada, configuração de acessibilidade, versão de firmware/software e códigos de incidente. Não deve conter áudio, vídeo, transcrição, embedding, nome, e-mail, localização, chave, endereço de rádio ou texto livre da conversa.

A política recomendada é manter o vínculo de consentimento separado dos dados de desempenho, conservar os dados brutos pseudonimizados pelo período aprovado pelo responsável ético, por exemplo 90 dias para auditoria, e depois apagar os eventos brutos, mantendo somente agregados não identificáveis e o digest do relatório. Qualquer prazo diferente deve ser congelado no formulário e no manifesto antes da coleta.

## 6. Arquivos prováveis e responsabilidades

A implementação deve ser aditiva e não deve rebatizar o manifesto existente `research/interaction_study_manifest.json`. Esse manifesto mede condições `core`/`nucleus`, não possui os três braços CB/NS/H e não pode ser usado sozinho como prova de utilidade humana.

| Arquivo provável | Responsabilidade | Restrição |
|---|---|---|
| `research/social/utility_protocol_v1.json` | Manifesto congelado com tarefa, condições, ensaios, seeds, ordem, limiares, consentimento, retenção e regras de exclusão | Não recebe resultados; qualquer mudança gera `v2` |
| `research/social/results.schema.json` | Schema de eventos e resultados normalizados | Rejeita áudio, transcript, embedding, identidade direta e linhas duplicadas |
| `research/social/runner.py` | Gera ordem contrabalanceada, executa a máquina de estados local e registra eventos | Não chama executor externo; H só chama o caminho de proposta permitido |
| `research/social/analyze.py` | Valida linhas, calcula erro, tempo/esforço e dependência por participante | Não usa `expected_final`, `oracle_reason` ou resultado do runtime como gabarito |
| `research/social/consent_pt-BR.md` | Informação ao participante, consentimento, retirada e contatos | Versão e digest selados no manifesto |
| `research/social/accessibility_checklist.md` | Checklist de configuração, parada, cancelamento, contraste e modalidade de entrada | Falha de checklist bloqueia a sessão |
| `research/social/operator_manual.md` | Roteiro de instrução e política de assistência | Proíbe dicas durante ensaio formal |
| `research/evidence/stage7_social_v1/protocol.json` | Cópia arquivada do protocolo e seus digests | Congelado antes da coleta |
| `research/evidence/stage7_social_v1/raw_results.jsonl` | Eventos pseudonimizados, um por linha | Não é reescrito após análise |
| `research/evidence/stage7_social_v1/baseline_results.jsonl` | Resultado separado de CB e NS, incluindo falhas | Não recebe linhas de H |
| `research/evidence/stage7_social_v1/analysis.md` | Relatório cego de resultados e desvios | Publica denominadores, perdas e resultados negativos |
| `research/evidence/stage7_social_v1/privacy_accessibility.md` | Auditoria de consentimento, retenção, acessibilidade e recuperação | Não contém dados pessoais diretos |
| `research/evidence/stage7_social_v1/deviations.jsonl` | Desvios, interrupções e incidentes, com códigos | Não converte desvio em exclusão silenciosa |

O formato pode aproveitar as invariantes de `tools/interactionlog.py` e `tools/interactionstudy.py`, como ordenação de timestamps, confirmação antes do envio local, rejeição de duplicatas e proibição de campos de privacidade. Deve ser criado um schema de estudo social novo, porque os analisadores existentes aceitam somente `core` e `nucleus` e não distinguem CB, NS e H.

## 7. Testes e mutações obrigatórios

Os testes devem ser executados em dados sintéticos e não devem ser apresentados como evidência humana. A suíte proposta é `research/social/test_utility_protocol.py`, complementada por testes de análise e por um harness de mutações em cópia temporária.

| ID | Mutação ou falha simulada | Teste matador | Aceitação |
|---|---|---|---|
| `HUM-01` | Remover uma condição ou alterar sua ordem | Manifesto incompleto/permutação não balanceada | Falha antes da coleta; não preencher condição ausente |
| `HUM-02` | Executar NS com runtime HERUS ou Skill persistente | Sentinela de chamadas e processo limpo | Zero chamadas HERUS e zero proposta em NS |
| `HUM-03` | Copiar uma linha H para CB ou NS | Digest de condição, host e sessão | A análise rejeita mistura e duplicata |
| `HUM-04` | Usar `expected_final` ou `oracle_reason` como gabarito | Fixture com previsão do runtime divergente da intenção congelada | O resultado usa apenas plano do ensaio e traço público |
| `HUM-05` | Confirmar antes de mostrar prévia | Máquina de estados com evento fora de ordem | Linha inválida; não virar acerto |
| `HUM-06` | Recodificar timeout, cancelamento ou `nenhuma` como sucesso | Dados negativos e ensaios de não ação | Taxas separadas; nenhuma imputação favorável |
| `HUM-07` | Reordenar timestamps ou duplicar trial | Validador de monotonicidade e chave única | Falha estrita e registro de desvio |
| `HUM-08` | Calcular dependência apenas no host-alvo | Remover ensaios de origem | Análise falha por ausência de comparação de troca |
| `HUM-09` | Permitir que H repita automaticamente após `ABSTAIN` | Host que retorna abstenção na primeira proposta | Uma abstenção, sem retry automático; fallback marcado separadamente |
| `HUM-10` | Incluir áudio, transcript, embedding, nome ou localização | Campos proibidos no schema | Rejeição e zero liberação não consentida |
| `HUM-11` | Ignorar retirada ou parada do participante | Evento `withdrawn` no meio do bloco | Coleta para imediatamente; linhas posteriores são rejeitadas |
| `HUM-12` | Deixar o operador sugerir a intenção | Log de prompt/assistência durante ensaio | Sessão marcada como desviada; não contar como observação formal |
| `HUM-13` | Dar mais prática ou tempo a uma condição | Manifesto com limites assimétricos | Validador rejeita a campanha ou publica o desvio |
| `HUM-14` | Escolher ordem depois de ver uma resposta | Seed ou ordem gerada a partir do resultado | Falha de reprodutibilidade e bloqueio da análise |
| `HUM-15` | Aceitar digest sem consentimento compatível | Consentimento em versão diferente do protocolo | Sessão inelegível até novo consentimento válido |

A mutação decisiva de circularidade é `HUM-04`: o gabarito da tarefa deve vir do plano de ensaio selado e da intenção que o cartão especifica, enquanto a validade mecânica do HERUS deve continuar no oráculo independente. Nenhum desses gabaritos pode ser substituído por `expected_final`, `transfer_success`, uma decisão `SUPPORTED` ou uma narrativa do operador.

## 8. Riscos, circularidade e mitigação

**Aprendizado e ordem de condições.** A pessoa pode melhorar simplesmente por repetir os cartões. O contrabalanço, a mesma distribuição por bloco, a prática congelada e a análise por participante reduzem o risco. A diferença residual deve ser reportada como possível carryover; não se corrige a ordem depois dos resultados.

**CB e H não são verdadeiramente comparáveis.** Configuração manual e transferência podem deixar layouts finais iguais, mas ter assistência ou tempo diferente. O manual do operador, os limites e a contagem de eventos devem ser idênticos. A configuração é um desfecho separado, e não pode ser omitida para fazer H parecer melhor.

**NS pode medir novidade, não ausência de simbionte.** O host padrão pode ser menos familiar. A prática comum, a modalidade igual, a coleta de familiaridade e o contraste H–CB ajudam a separar continuidade de mera novidade. Se não for possível equalizar familiaridade, o resultado deve ser “efeito de configuração/transferência neste protocolo”, não efeito geral do simbionte.

**O participante pode adivinhar a hipótese.** Os nomes das condições não devem aparecer na interface. A explicação deve ser neutra, os prompts devem ser balanceados e o operador que aplica os cartões não deve ver a análise agregada. A expectativa do participante pode ser perguntada ao final somente como resultado secundário e consentido.

**O operador pode virar um segundo sistema de assistência.** Toda intervenção deve ter código. Uma intervenção que sugere intenção, reorganiza o layout ou corrige a seleção invalida o ensaio formal, sem apagar o evento.

**O efeito pode ser apenas de menor configuração, não de melhor comunicação.** Por isso a análise separa erro da tarefa, esforço de configuração, esforço de ensaio e dependência de host. Uma redução de setup não pode ser narrada como compreensão ou aumento de autonomia sem evidência de continuidade e escolha independente.

**Circularidade de benchmark.** O mesmo esquema de intenções, IDs ou regras usado para criar a Skill não pode determinar o resultado humano de H. A habilidade pode transferir uma rotina, mas o avaliador compara a intenção congelada do cartão com a saída observada. O holdout mecânico deve permanecer separado da planilha humana.

**Abstenção interpretada como utilidade.** Uma abstenção segura protege contra falso consenso, mas interrompe a tarefa. Ela deve contar como contenção mecânica e como falha/indisponibilidade na métrica de utilidade, salvo se o participante escolher o cancelamento previsto. Não se atribui benefício a uma sessão que não completou a tarefa.

**Fadiga ou frustração.** A sessão deve ter pausas, limite máximo de duração e parada independente. Incidentes e desistências são resultados, não linhas removíveis. O protocolo não deve pedir esforço físico além da modalidade escolhida pelo participante.

**Privacidade e reidentificação.** Um pseudônimo não basta se combina timestamps raros, dispositivo e configuração. Deve-se separar o vínculo de consentimento, reduzir precisão temporal quando não necessária e não publicar células pequenas que possam reidentificar uma pessoa. A retenção deve ser aplicada por política, não por promessa informal.

**Generalização indevida.** Doze adultos em uma tarefa simulada não representam todos os usuários de comunicação, todos os dispositivos ou todos os contextos sociais. A análise deve publicar o cenário, o host, a modalidade de entrada, a população recrutada e as falhas. Nenhum resultado positivo autoriza extrapolação para situações de emergência ou saúde.

## 9. O que não pode ser alegado

Mesmo com todos os gates deste protocolo aprovados, não se pode alegar:

- que o HERUS possui inteligência geral, consciência, entendimento aberto de linguagem ou intenção humana;
- que a tarefa demonstra eficácia clínica, reabilitação, diagnóstico, tratamento ou segurança para pessoas com uma condição médica;
- que uma melhora em mensagens preparadas prova melhora na comunicação espontânea, relações sociais, autonomia ampla ou qualidade de vida;
- que o sistema é fisicamente seguro fora do ambiente local, ou que qualquer ação externa é autorizada pelo runtime;
- que a proposta `SUPPORTED`, `transfer_success`, uma Skill verificada ou uma abstenção segura equivale a execução, benefício ou autorização;
- que a redução de setup ou dependência em duas interfaces generaliza para todos os dispositivos, ambientes, idiomas ou populações;
- que a ausência de áudio ou transcript no arquivo prova privacidade de produto em produção;
- que `SHA-256`, um digest de observação ou uma decisão do runtime prova completude, autenticidade do sensor, ausência de efeitos ocultos ou independência da autoridade;
- que a condição NS é um placebo perfeito, nem que qualquer diferença entre NS e H é causada exclusivamente por “simbiose” se familiaridade, configuração ou ordem não forem controladas;
- que um piloto de viabilidade é uma demonstração confirmatória ou um impacto social amplo.

A classificação máxima permitida por este desenho, isoladamente, é **valor humano medido nesta tarefa e neste protocolo**. A classificação congelada de `useful_symbiosis` exige também que todos os gates mecânicos, de segurança, privacidade, acessibilidade, reprodutibilidade e baseline passem. Se faltar uma dimensão ou um hard limit for violado, o resultado permanece **`not_proven`**; se apenas o mecanismo estiver comprovado sem a evidência humana completa, permanece **`mechanism_only`** [1] [2].

## 10. Ordem de implementação

1. Congelar `utility_protocol_v1.json`, consentimento, manual do operador, critérios e seed; calcular digests antes de recrutar.
2. Verificar o gate Stage 6 e manter os artefatos holdout v1 imutáveis. Não iniciar uma alegação humana enquanto a campanha mecânica estiver `not_proven`.
3. Implementar a máquina de estados e o schema social em módulo novo, sem alterar `interaction_study_manifest.json`, `tools/interactionstudy.py` ou `tools/interactionlog.py`.
4. Implementar três ambientes isolados: CB, NS e H. Validar em dados sintéticos que NS não inicia HERUS e H não executa efeitos externos.
5. Rodar os testes `HUM-01` a `HUM-15`, incluindo mutações em cópia temporária, antes de qualquer sessão humana.
6. Selar a ordem contrabalanceada e o lote de cartões. Fazer uma sessão de ensaio com dados sintéticos e uma revisão de acessibilidade; não usar resultado humano para alterar o protocolo.
7. Obter aprovação ética ou equivalente institucional, consentimento e registro de retirada antes da coleta.
8. Executar o piloto, publicar resultados brutos e negativos, desvios, perdas, falhas de acessibilidade e métricas por condição.
9. Aplicar os gates sem alterar margens, denominadores ou definição de erro após observar os resultados. Qualquer nova pergunta exige a versão `v2`.

## Referências

[1]: https://github.com/SummaArs/herus/blob/09d20fb96daee5534fb928365997e7b504bb2f77/research/symbiosis_utility_contract.json "Contrato machine-readable de simbiose útil"

[2]: https://github.com/SummaArs/herus/blob/09d20fb96daee5534fb928365997e7b504bb2f77/docs/52-DEFINICAO-SIMBIOSE-UTIL.md "Definição congelada de simbiose útil — Etapa 3"

[3]: https://github.com/SummaArs/herus/blob/09d20fb96daee5534fb928365997e7b504bb2f77/docs/53-BENCHMARK-HOLDOUT-ADVERSARIAL.md "Benchmark holdout adversarial — Etapa 4"

[4]: https://github.com/SummaArs/herus/blob/09d20fb96daee5534fb928365997e7b504bb2f77/docs/54-CORRECAO-CONTRATO-OBSERVABILIDADE.md "Correção do contrato de observabilidade — Etapa 5"

[5]: https://github.com/SummaArs/herus/blob/09d20fb96daee5534fb928365997e7b504bb2f77/research/evidence/stage5_plan/00-synthesis.md "Síntese arquitetural mínima da Etapa 5"

[6]: https://github.com/SummaArs/herus/blob/09d20fb96daee5534fb928365997e7b504bb2f77/research/evidence/remaining_plan/05-system_architecture.md "Arquitetura restante para integração final e caminho social"

[7]: https://github.com/SummaArs/herus/blob/09d20fb96daee5534fb928365997e7b504bb2f77/tools/interactionlog.py "Validador existente de logs de interação"

[8]: https://github.com/SummaArs/herus/blob/09d20fb96daee5534fb928365997e7b504bb2f77/research/interaction_study_manifest.json "Manifesto existente de estudo de interação"

<!-- Nenhum arquivo de código, manifesto existente ou resultado histórico foi alterado nesta frente. -->
