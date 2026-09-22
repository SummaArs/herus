# Auditoria adversarial de produto e utilidade social

**Escopo:** produto inicial, utilidade humana, protocolo social e limites de alegação, com leitura de `docs/00-HERUS-MASTER.md`, `docs/38-PRODUTO-DESEJAVEL-E-ADOCAO.md`, `docs/47-HERUS-INDISPENSAVEL-E-INTELIGENCIA-PROPRIA.md`, `docs/51–55`, `research/symbiont_v2`, `research/stage4`, `research/symbiont_v2/stage5.py`, `research/holdout*`, `research/social` e evidências de auditorias anteriores.

**Commit auditado:** `fbf7471c4903fd6af285f9e6e6a0e10f7cb2a967`.

**Regra:** nenhum código do HERUS foi editado. Este arquivo é o único artefato produzido nesta frente.

## Veredicto

**REPROVADO para lançamento, alegação de “produto indispensável”, alegação de utilidade social ou promoção para `useful_symbiosis`.** O repositório contém uma pesquisa honesta em vários pontos: declara `not_proven`, não inventa participantes, separa proposta de execução em nível conceitual e rejeita explicitamente AGI, consciência e segurança física geral. Isso é mérito metodológico, não evidência de produto.

Hoje há três produtos concorrentes dentro do mesmo projeto: um comunicador sem infraestrutura e semântico no documento mestre; uma memória pessoal seletiva no HERUS One; e um Watch conectado a Paper-Core, conversa e “segundo cérebro”. O único experimento social preparado não testa nenhum desses produtos de modo convincente. Ele testa uma seleção local de respostas preparadas entre condições CB/NS/H, sem mensagem real, sem resultado humano e sem uma implementação social completa. O runner aceita que controles importantes sejam removidos sem bloquear.

A decisão defensável é: **manter o HERUS como protótipo de pesquisa host-only e reduzir o próximo produto a uma experiência local de continuidade de comunicação preparada entre duas interfaces.** Essa experiência não deve ser vendida como produto final, comunicação espontânea, acessibilidade clínica, autonomia ampla, simbiose geral ou AGI. Ela é o menor teste que toca a tese de transferência e pode ser executado sem rádio, nuvem, LLM, microfone, armazenamento de áudio, atuador ou efeito externo.

Não existe, neste momento, evidência para chamar qualquer produto de indispensável. A recomendação abaixo identifica o **primeiro alvo de prova mais indispensável e testável**, não uma promessa de mercado.

## O produto inicial recomendado

### HERUS Bridge: continuidade de uma rotina comunicacional entre interfaces

O primeiro produto de pesquisa deve ser uma pequena aplicação local, acompanhada de dois hosts de entrada acessíveis. A pessoa define uma rotina de quatro intenções preparadas em uma interface de origem. Depois troca para uma interface-alvo com IDs, ordem e controles diferentes. O HERUS transfere apenas o contrato tipado da rotina. A pessoa vê uma prévia local e confirma ou cancela. O laboratório mostra a frase correspondente em uma tela local; nenhuma mensagem é enviada a outra pessoa.

As quatro intenções devem ser limitadas a respostas de baixa consequência, por exemplo **continuar**, **pausar**, **cancelar** e **nenhuma ação**. A intenção `nenhuma ação` é necessária para medir falso acionamento. A interface precisa funcionar sem voz, sem nuvem e sem dados pessoais. O dispositivo físico, rádio e memória pessoal ficam fora do primeiro gate.

Esse recorte é superior às teses de memória, rádio e “segundo cérebro” como primeiro teste por quatro razões. Primeiro, testa diretamente a única propriedade social específica alegada pela API: uma competência verificada que sobrevive à troca de host. Segundo, elimina dependências ainda não demonstradas, como microfone, ASR, LLM local, bateria, RF, alcance, conforto e armazenamento cifrado. Terceiro, torna a falha reversível: uma abstenção impede a confirmação e não causa ação externa. Quarto, permite comparar o HERUS com configuração manual e com o mesmo host sem transferência.

O nome público do experimento deve ser **continuidade de comunicação preparada**, e não “simbiose”, “memória aumentada”, “assistente” ou “inteligência própria”. O resultado máximo permitido é: “nesta tarefa, nesta população, com estas interfaces, a transferência reduziu ou não reduziu o custo de configuração sem aumentar o erro além da margem pré-registrada”.

## Achados prioritários

### P1 — O projeto não tem uma tese de produto congelada

**Severidade: crítica.** O documento mestre define o HERUS como comunicador semântico off-grid e chega a dizer que ele não é assistente [1]. O documento de adoção muda o motivo inicial para memória seletiva e recuperação pessoal no HERUS One [2]. O documento seguinte adiciona Watch, Paper-Core, conversa curta, cartões de contexto e uma promessa de presença diária [3]. O protocolo social, por sua vez, limita-se a selecionar e confirmar respostas preparadas localmente [8].

Essas teses exigem usuários, hardware, métricas e critérios de sucesso diferentes. Um rádio de emergência não é validado por tempo de setup de uma interface. Uma memória pessoal não é validada por transferência de uma rotina semântica entre hosts. Um “segundo cérebro” não é validado sem recuperação longitudinal, retenção útil, correção, esquecimento e comparação com ferramentas de notas. A documentação reconhece partes dessas limitações, mas a narrativa acumulada ainda permite que um leitor trate uma prova estreita como progresso de todos os produtos.

**Mudança concreta:** congelar uma única frase de produto para a próxima etapa: **“HERUS Bridge transporta uma rotina comunicacional preparada e confirmada entre duas interfaces locais, sem carregar áudio ou conceder autoridade.”** Mover Watch, Paper-Core, memória pessoal, Anchor, mesh, rádio, voz e LLM para roadmap não elegível para a decisão deste gate. Criar uma matriz que ligue cada claim a uma tarefa, população, dispositivo, métrica, resultado e limite de extrapolação. Qualquer claim sem experimento correspondente deve ser marcado `não testado`.

**Teste de aceitação:** um revisor externo recebe apenas o README da etapa e deve identificar um único usuário, uma única tarefa, três condições e um critério de sucesso. Se identificar duas teses ou puder interpretar o experimento como prova de memória, comunicação espontânea ou inteligência, o gate falha.

### P2 — “Indispensável” é uma conclusão sem observação longitudinal

**Severidade: crítica.** O documento de adoção é correto ao afirmar que “indispensável” só poderia ser usado após uso longitudinal [3, §10], mas o conjunto atual não contém retenção, retorno voluntário, situação em que a pessoa escolheria o HERUS sem ser instruída, custo de abandono, disposição de pagar ou comparação com o celular. O Master ainda apresenta números de rádio, energia e arquitetura como se a utilidade futura já estivesse encaminhada [1]. Isso é um plano de engenharia, não necessidade humana demonstrada.

**Mudança concreta:** remover `indispensável`, “second brain”, “private cellular network”, “only radio” e equivalentes de qualquer conclusão da etapa. Usar a escada de claims abaixo:

| Evidência disponível | Claim permitido | Claim proibido |
|---|---|---|
| Hosts determinísticos e testes unitários | mecanismo host-only limitado | utilidade, segurança ou autonomia |
| Piloto local sem mensagem real | viabilidade da tarefa e custo de configuração | melhora de comunicação real ou acessibilidade clínica |
| Repetição em participantes e interfaces pré-especificados | efeito nesta tarefa e população | generalização para todos os usuários |
| Uso longitudinal com retorno voluntário | retenção e valor percebido no cenário testado | indispensabilidade universal |
| Hardware medido | desempenho do protótipo e condições medidas | segurança física, autonomia ou alcance não medidos |

**Teste de aceitação:** um linter documental ou checklist de release deve reprovar qualquer relatório que use “indispensável”, AGI, consciência, autonomia ampla, segurança física ou equivalência ao smartphone sem o artefato correspondente. O contrato congelado já proíbe parte dessas extrapolações [4]; falta fazer a proibição bloquear a narrativa de produto.

### P3 — O protocolo social atual não mede benefício social convincente

**Severidade: crítica.** `utility_protocol_v1.json` chama a tarefa de seleção e confirmação local de respostas preparadas, mas não especifica participante, número de ensaios, origem/alvo, randomização, máquina de estados, schema de resultado, análise ou procedimento de consentimento [8]. O documento de plano humano descreve esses elementos em prosa [10], porém eles não estão implementados no diretório `research/social`.

A tarefa local é útil como teste de usabilidade e segurança de confirmação. Ela não demonstra comunicação social no sentido comum porque não há interlocutor real, entrega de mensagem, consequência comunicacional ou medida de compreensão pelo destinatário. Se o HERUS melhora apenas a configuração de um menu preparado, o resultado é uma redução de setup, não “melhor comunicação”, “autonomia” ou “simbiose”. O plano existente reconhece esse risco, mas o manifesto curto não o codifica.

**Mudança concreta:** renomear o estudo para **piloto de continuidade e configuração de interface**. Declarar como desfecho primário `task_error` na seleção/confirmacão local e como desfecho separado `setup_remap_effort`. Declarar que não existe mensagem real nem efeito externo. Se a equipe quiser alegar comunicação, adicionar uma segunda fase, pré-registrada e separada, em que uma mensagem preparada seja entregue a um simulador local e a correção pelo destinatário seja medida. Não misturar as fases.

**Teste de aceitação:** nenhum relatório pode converter `confirmed_correct`, `proposal_status=PROPOSED`, `transfer_success` ou abstenção em “mensagem comunicada”. O analisador deve exigir um evento local de saída e manter separado o desfecho de setup, o desfecho de seleção e o desfecho de entrega simulada.

### P4 — O runner social é uma porta cosmética, não um gate de protocolo

**Severidade: alta.** `research/social/runner.py` valida somente o status sem dados humanos, o conjunto de três condições e três nomes de métricas [9]. Em uma sonda independente, remover `required_controls`, `progression_thresholds`, `analysis_rule` e `prohibited_claims` de uma cópia do manifesto continuou retornando `validate_protocol(...) == ()`. O runner continuou declarando `READY_FOR_EXTERNAL_REVIEW`.

Também não existem os artefatos que o próprio plano humano lista como necessários: `results.schema.json`, `analyze.py`, consentimento em linguagem simples, checklist de acessibilidade e manual do operador. O status `protocol_ready_no_human_data` é honesto sobre a ausência de coleta, mas não significa que o protocolo esteja validado.

**Mudança concreta:** separar três estados, sem ambiguidade: `MANIFEST_SCHEMA_VALID`, `READY_FOR_ETHICS_EXTERNAL_REVIEW` e `HUMAN_DATA_ELIGIBLE`. O primeiro deve validar tipos, versionamento, conditions exatas, limiares, regra de análise, claims proibidos, consentimento, retenção, acessibilidade, parada, schema de resultados e digest de todos os artefatos. O segundo exige revisão humana/ética externa. O terceiro exige a aprovação dessa revisão, um ambiente de coleta isolado e um teste sintético completo. Remover “ready” quando só o JSON mínimo passou.

**Teste de aceitação:** remover ou alterar cada campo normativo deve produzir `BLOCKED`. Deve falhar uma condição duplicada, uma condição desconhecida, limiar negativo, ausência de regra de exclusão, ausência de política de retenção, ausência de parada independente e divergência entre digest de protocolo e digest de consentimento.

### P5 — CB, NS e H ainda não formam um experimento executável único

**Severidade: alta.** O protocolo social define CB, NS e H [8], enquanto `research/interaction_study_manifest.json` define fontes `core` e `nucleus`, itens de voz, latência e energia, com estado `preregistered-no-data` [11]. São experimentos diferentes. Reutilizar o manifesto Core/Nucleus como prova do estudo CB/NS/H criaria uma troca silenciosa de hipótese.

Além disso, o H depende de `propose_transfer_checked`, mas o caminho atual declara `SUPPORTED` a partir de contratos com strings não vazias, sem autenticidade ou vínculo verificável [13]. O holdout histórico contém um oráculo que copia labels da fixture, e a descoberta usa `host.execute()` como sonda. As auditorias anteriores reproduziram que `m7` e `m8` são rótulos calculados, não falhas parciais e overrun observados [14] [15]. O modo estrito pode aceitar contratos forjados. Isso impede uma sessão humana ética: não se deve convidar pessoas para um experimento cuja condição central pode receber uma alegação de segurança que a própria auditoria falsifica.

**Mudança concreta:** bloquear a coleta humana até o gate mecânico passar. Criar um único manifesto social novo, independente do Core/Nucleus, com hosts serializados em processos separados, token opaco, proposta canônica, orçamento efetivo e ausência de qualquer acesso à tabela privada. Reescrever o oráculo para derivar o veredicto da proposta e da verdade privada independente, não de `expected_negative` ou `oracle_reason`. Separar `probe` de `commit`; se uma sonda puder mutar, tratá-la como execução autorizada e não como observação.

**Teste de aceitação:** o caso de efeito oculto deve ser bloqueado antes de qualquer probe em modo estrito. O contrato forjado nunca pode retornar `SUPPORTED`. O oráculo deve dar o mesmo veredicto quando labels textuais forem mutados. `m7` deve produzir recibo real de `FAILED_PARTIAL` ou `UNKNOWN_OUTCOME`; `m8` deve exceder um `max_cost` declarado e verificável. O cenário de proposta deve registrar zero commits, e o host deve provar externamente que probes não mutaram o mundo.

### P6 — A validação de utilidade permite classificar evidência incompleta como `mechanism_only`

**Severidade: alta.** `research/symbiosis_utility.py` retorna `mechanism_only` quando as métricas de mecanismo estão presentes, mesmo se faltarem dimensões obrigatórias de baseline, segurança, privacidade ou reprodutibilidade. Uma sonda independente forneceu apenas o bloco `mechanism` completo: `classify()` retornou `mechanism_only`, enquanto `validate_report()` listou todas essas dimensões como ausentes. Isso contradiz o contrato congelado, que reserva `mechanism_only` ao caso em que o mecanismo passa e somente o valor humano ainda falta [4] [17].

**Mudança concreta:** implementar a regra fail-closed em três níveis. Primeiro, qualquer dimensão, campo ou hard limit ausente produz `not_proven`. Segundo, `mechanism_only` exige mecanismo completo, segurança, privacidade e reprodutibilidade completas, com apenas `human_value` ausente ou não provado. Terceiro, `useful_symbiosis` exige dados humanos brutos, baseline congelado, comparação NS/CB/H e todos os gates, nunca apenas um dicionário que contenha as chaves certas.

**Teste de aceitação:** testar a remoção isolada de cada dimensão e controle. Todos devem retornar `not_proven`. Um relatório com mecanismo completo e sem valor humano deve retornar `mechanism_only`. Um relatório completo, mas sem dados observados ou com valores auto-declarados, deve ser rejeitado como evidência, mesmo que o schema seja preenchido.

### P7 — A acessibilidade está no texto, não na experiência

**Severidade: alta.** O plano descreve tamanho de texto, contraste, teclado, switch, tempo ajustável, pausas, parada independente e intervenção do operador [10]. O diretório executável social não contém checklist, manual ou máquina de estados que imponha essas condições. A experiência proposta também não exige voz, o que é uma boa redução de risco, mas ainda não prova que pessoas que usam modalidades alternativas conseguem completar a tarefa sem ajuda.

O protocolo deve evitar dois erros opostos: tratar qualquer participante com necessidade de acessibilidade como proxy de uma população clínica, ou remover essas pessoas para facilitar a análise. O primeiro gera alegação clínica indevida; o segundo elimina justamente a hipótese de utilidade.

**Mudança concreta:** testar primeiro com adultos, sem diagnóstico ou prontuário, recrutados pela modalidade de entrada que escolherem. Registrar modalidade, configuração e assistência como fatores de contexto, não como atributos identitários desnecessários. Oferecer teclado, touch e switch quando o ambiente suportar; não exigir microfone. A parada deve ser um controle fora do HERUS. Uma falha de acessibilidade invalida o ensaio e gera incidente; não vira “erro do participante”. Ensaios com dica do operador ficam fora da análise formal, nunca são apagados.

**Teste de aceitação:** antes de qualquer participante, um testador usando cada modalidade prevista deve conseguir iniciar, cancelar, parar e sair sem ajuda do runtime. O harness deve rejeitar confirmação sem prévia, timeout contado como sucesso, intervenção não codificada e configuração de acessibilidade ausente.

### P8 — O protocolo é pequeno, mas seus limiares ainda são escolhas sem justificativa de necessidade

**Severidade: média/alta.** O manifesto usa penalidade máxima de erro de 5 pontos percentuais e redução mínima de 20% no esforço de setup [8]. O plano humano acrescenta 12 participantes e contrastes pareados [10]. Esses números podem ser regras úteis de progressão, mas não são automaticamente diferenças importantes para a pessoa. Não há, no manifesto executável, estimando primário, denominador, tratamento de retirada, falha de equipamento, carryover, familiaridade ou análise de dados incompletos.

**Mudança concreta:** declarar que o piloto é de viabilidade e não de potência populacional. Fixar antes da coleta: unidade experimental, estimando primário, denominador, regra para timeout/cancelamento/abstenção, perdas, desvios, efeito de ordem, análise por participante e intervalo de incerteza. Justificar os limiares como **gates de progressão do protótipo**, não como benefício clinicamente ou socialmente significativo. Se não houver justificativa externa para 5 pp e 20%, reportar os resultados sem transformar o limiar em evidência.

**Teste de aceitação:** gerar dados sintéticos em que H reduz setup mas piora erro; H melhora erro mas aumenta esforço; H abstém; há retirada no meio; e há falha de acessibilidade. O analisador deve publicar cada resultado separadamente e nunca produzir um índice composto pós-hoc que escolha a condição vencedora.

### P9 — O protocolo correto já existe em prosa, mas não foi reduzido a uma experiência convincente

**Severidade: média.** O plano humano é longo e tecnicamente cuidadoso [10], mas a experiência continua parecendo um benchmark: quatro respostas artificiais, operador simulando interlocutor, nenhuma mensagem real e muitos gates invisíveis ao usuário. Isso é apropriado para um primeiro teste de segurança, mas não é uma demonstração que uma pessoa reconheça como necessidade cotidiana. Adicionar Watch, Dock, rádio, voz ou “memória” agora só esconderia essa fraqueza sob cosmética.

**Mudança concreta:** construir uma sessão de 15–20 minutos com um único fluxo narrativo: “você tem uma rotina de respostas que precisa levar para outra interface”. A pessoa escolhe uma modalidade de entrada, define três respostas e uma opção de não agir, troca de interface, revisa a proposta, confirma ou cancela e observa apenas a saída local. O facilitador pergunta depois se a pessoa escolheria a transferência novamente e por quê, mas essa resposta é secundária, não prova de benefício. O fluxo deve ter a mesma tarefa e o mesmo número de opções em CB, NS e H.

**Teste de aceitação:** cinco sessões formativas, sem alegação confirmatória, devem identificar se a pessoa entende o estado `rascunho`, `confirmar`, `cancelar`, `abster` e `bloqueado` sem tutorial longo. Se o usuário não souber quando uma ação será feita, o design falha mesmo que o benchmark passe.

### P10 — Os números físicos publicados ainda não são base de decisão de produto

**Severidade: média/alta.** O Master publica, para a configuração Band/SF9, 2.305 m em campo aberto, 1.155 m suburbano, 650 m urbano denso e cerca de 1,9 km em três saltos [1]. A execução atual de `tools/budget.py` imprime valores diferentes, incluindo 3.653 m, 1.831 m, 1.030 m e cerca de 3,1 km para a revisão corrente [16]. O próprio orçamento contém eras de frame e potência misturadas, segundo a auditoria anterior [14]. Não há medição física do rádio no protocolo social.

**Mudança concreta:** não usar alcance, “rede celular privada”, autonomia solar, conforto, IPX7 ou segurança de wearable para vender o primeiro experimento. Escolher uma revisão de potência/frame, remover a seção legada ou marcá-la como histórica, regenerar a documentação e criar um CI que compare números publicados com o ledger escolhido. Hardware só entra depois que a experiência local provar uma necessidade humana independente do rádio.

**Teste de aceitação:** `tools/budget.py` e o snapshot versionado devem concordar para Tier 0, Tier 1, Tier 0.5, voz, SOS e três hops. Qualquer divergência bloqueia a publicação de claim físico.

## Protocolo social reduzido e executável

### Fase 0 — segurança mecânica e ensaio sintético

Antes de recrutar, corrigir o holdout, o oráculo, o contrato `STRICT`, o orçamento e a classificação. Rodar CB, NS e H em fixtures sintéticas com nomes e ordem permutados. O H deve construir apenas uma proposta local. O NS não pode importar ou iniciar HERUS. O CB deve realizar apenas o mapeamento manual congelado. Nenhuma condição pode enviar mensagem real.

### Fase 1 — teste formativo de acessibilidade

Realizar cinco ou seis sessões curtas, explicitamente **formativas e sem alegação de benefício**. Usar adultos que escolham teclado, touch ou switch. Não recolher áudio, transcrição, vídeo, diagnóstico, nome, localização ou conteúdo livre. Medir somente compreensão do estado, capacidade de parar/cancelar e erros de operação. Corrigir a interface apenas entre sessões e registrar mudanças; não misturar esta fase com confirmação estatística.

### Fase 2 — piloto humano pré-registrado

Usar três condições intraindividuais:

- **CB:** remapeamento manual da rotina no host-alvo;
- **NS:** host-alvo com seu mapeamento nativo, sem runtime, Skill ou `herus_id`;
- **H:** rotina criada no host de origem e proposta de transferência checked no host-alvo.

Cada condição deve ter a mesma quantidade de ensaios, a mesma distribuição de três intenções e `nenhuma ação`, a mesma modalidade de entrada e a mesma tela de prévia. A ordem deve ser contrabalanceada por seed congelada. O participante pode cancelar, parar ou retirar-se a qualquer momento. A condição H deve apresentar abstenção como indisponibilidade, nunca como sucesso.

Os resultados primários devem ser publicados separadamente por participante e condição:

1. erro de tarefa no host-alvo;
2. tempo e contagem de entradas, separados em setup e ensaio;
3. penalidade de troca de host;
4. correção, timeout, cancelamento, abstenção, fallback e intervenção;
5. incidentes de acessibilidade e privacidade.

A regra de progressão pode manter provisoriamente “H não mais que 5 pontos percentuais pior que CB e pelo menos 20% menos esforço de setup que CB”, mas isso deve ser descrito como **gate de protótipo congelado**, não como efeito social universal. H também precisa ter penalidade de troca menor que NS; se não tiver, a tese de continuidade falha mesmo que o setup melhore.

### Fase 3 — decisão

Há somente três decisões legítimas:

- **Avançar para teste de repetição:** todos os gates de segurança, acessibilidade e privacidade passam, e há sinal consistente na tarefa pré-especificada.
- **Revisar a interface:** a tarefa é segura, mas a pessoa não entende estados, cancela com dificuldade ou o ganho é apenas cosmético.
- **Parar a linha de produto:** não há redução de esforço sem deterioração, a troca de host continua dependente de remapeamento, ou a transferência absten-se tanto que não entrega a tarefa.

Nenhuma dessas decisões autoriza rádio, memória pessoal, conversa aberta, uso clínico, segurança física, AGI ou lançamento comercial.

## Checklist mínimo antes de coleta humana

| Gate | Evidência exigida | Estado atual |
|---|---|---|
| Produto único | frase, usuário, tarefa e claim permitido congelados | **Falha: teses concorrentes** |
| Holdout | oracle independente, processo separado, probes não mutantes | **Falha: labels e mutação documentados** |
| `STRICT` | atestação autenticada e vinculada a host/skill/proposta | **Falha: strings não vazias bastam** |
| Orçamento | ledger consumido antes de cada operação e persistente | **Falha: contadores decorativos** |
| Protocolo social | schema, analyzer, consentimento, acessibilidade e manual executáveis | **Falha: só runner mínimo existe** |
| Condições | CB/NS/H no mesmo manifesto, sem reutilizar Core/Nucleus | **Falha: manifests diferentes** |
| Dados | eventos brutos pseudonimizados, negativos e desvios | **Não coletado** |
| Acessibilidade | parada, cancelamento e modalidades testados sem runtime | **Não demonstrado** |
| Hardware | orçamento físico único e medições reais | **Stale/sem medição física suficiente** |

## Testes de reabertura obrigatórios

1. **Mutação do manifesto:** remover cada controle, limiar, claim proibido, regra de análise, política de retenção ou condição deve resultar em `BLOCKED`.
2. **Isolamento causal:** em NS, zero import, chamada, Skill persistida ou `herus_id`; em H, zero commit e zero executor externo; em CB, zero runtime.
3. **Máquina de estados:** confirmar sem prévia, reordenar timestamps, duplicar trial, converter timeout em acerto e suprimir cancelamento devem falhar.
4. **Abstenção:** uma abstenção não pode ser contada como transferência bem-sucedida; fallback deve ser linha separada.
5. **Privacidade:** áudio, transcript, embedding, nome, localização, chave e endereço de rádio devem ser rejeitados pelo schema.
6. **Acessibilidade:** cada modalidade prevista deve iniciar, confirmar, cancelar e parar sem toque do operador; falha deve gerar incidente, não erro do participante.
7. **Análise:** remover ensaios de origem, misturar condições, alterar ordem pós-resultado ou usar `expected_final` como gabarito deve bloquear.
8. **Oráculo:** mutar `expected_negative`, `oracle_verdict` e `oracle_reason` não pode mudar o veredicto causal; proposta com ação removida ou ordem trocada deve ser rejeitada.
9. **Efeito oculto:** a família equivalente a `m10a-probe-commit-gap-01` deve retornar `UNSUPPORTED_BY_CONTRACT` ou `SAFE_BUT_UNPROVEN`, nunca `SUPPORTED` por inferência pública.
10. **Classificação:** mecanismo incompleto, controle ausente ou dado humano ausente deve permanecer `not_proven`; somente mecanismo completo sem valor humano pode ser `mechanism_only`.
11. **Proveniência:** digest de protocolo, código, fixture, seed, comando, ambiente, resultados brutos e negativos deve ser recomputável.
12. **Hardware:** qualquer divergência entre `tools/budget.py` e claims publicados deve bloquear release.

## Limite de linguagem

A documentação deve repetir, na página de produto e no relatório social, a seguinte formulação: **“O HERUS não demonstrou AGI, consciência, entendimento aberto, eficácia clínica, segurança física geral, comunicação espontânea, utilidade social ampla ou indispensabilidade. O próximo experimento testa somente a continuidade de uma rotina comunicacional preparada entre duas interfaces locais.”**

Uma experiência convincente não precisa parecer inteligente. Ela precisa deixar claro para a pessoa o que foi transferido, o que ainda não foi feito, quando a confirmação ocorre, como cancelar e o que acontece quando o sistema não sabe. Se o projeto não consegue entregar essa clareza em um fluxo de quinze minutos, adicionar um rádio, uma LLM ou um wearable será cosmética, não produto.

## Referências

[1]: ../../../docs/00-HERUS-MASTER.md "HERUS Master Design Document"
[2]: ../../../docs/38-PRODUTO-DESEJAVEL-E-ADOCAO.md "HERUS — Produto desejável e estratégia de adoção"
[3]: ../../../docs/47-HERUS-INDISPENSAVEL-E-INTELIGENCIA-PROPRIA.md "HERUS indispensável: relógio, memória e inteligência própria"
[4]: ../../../docs/52-DEFINICAO-SIMBIOSE-UTIL.md "Definição congelada de simbiose útil"
[5]: ../../../docs/53-BENCHMARK-HOLDOUT-ADVERSARIAL.md "Benchmark holdout adversarial"
[6]: ../../../docs/54-CORRECAO-CONTRATO-OBSERVABILIDADE.md "Contrato de observabilidade e decisão checked"
[7]: ../../../docs/55-ETAPAS-FINAIS-EXECUTOR-HUMANO.md "Etapas finais — execução externa, holdout estendido e prova humana"
[8]: ../../../research/social/utility_protocol_v1.json "Protocolo social de utilidade humana v1"
[9]: ../../../research/social/runner.py "Runner mínimo do protocolo social"
[10]: ../remaining_plan/04-human_utility_protocol.md "Plano detalhado de protocolo de utilidade humana"
[11]: ../../../research/interaction_study_manifest.json "Manifesto Core versus Núcleo"
[12]: ../../../research/symbiont_v2/core.py "Runtime do simbionte v2"
[13]: ../../../research/symbiont_v2/stage5.py "Tipos de observabilidade, decisão e ledger"
[14]: 01-soundness.md "Auditoria adversarial de soundness código–documentação"
[15]: 03-blackbox_holdout.md "Auditoria adversarial do holdout, do oráculo e da fronteira black-box"
[16]: ../../../tools/budget.py "Ledger reproduzível de orçamento físico"
[17]: ../../../research/symbiosis_utility.py "Verificador do contrato de simbiose útil"
[18]: ../../../research/test_remaining_stages.py "Testes das etapas restantes"

---

**Conclusão operacional:** não construir ainda o Watch, Paper-Core, Anchor ou mesh para provar uma história de produto. Primeiro construir uma experiência local de continuidade de comunicação preparada, corrigir os gates mecânicos e medir se uma pessoa realmente transfere uma rotina com menos setup, sem mais erro e sem perder controle. Se isso não produzir valor observável, o restante do portfólio é ornamentação sobre uma hipótese não demonstrada.
