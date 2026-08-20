# HERUS — Protocolo GAN adversarial e modelo de ameaça

**Estado:** contrato de validação adversarial do passo de resiliência.  
**Objetivo:** fazer cada capacidade do HERUS enfrentar um adversário que conhece sua implementação e tenta obter uma falsa conclusão, autoridade indevida, vazamento ou perda de soberania.

## 1. O que “GAN” significa neste projeto

A analogia com uma GAN é operacional, não uma alegação de que o firmware contém uma rede generativa adversarial. A **frente construtora** propõe código, contratos e testes para uma capacidade. A **frente sabotadora** recebe o contrato, conhece os caminhos públicos e tenta quebrá-los com mutações, entradas, sequências e combinações hostis.

Uma capacidade só é considerada resistente quando a frente sabotadora consegue atingir os caminhos críticos e cada ataque termina em uma das saídas permitidas: rejeição, abstenção, silêncio, quarentena, expiração ou erro bounded. “Não travou no teste” não é suficiente.

> O adversário não precisa produzir uma resposta bonita. Ele vence se conseguir fazer o HERUS acreditar, lembrar, agir, transmitir, aceitar ou afirmar algo sem autoridade suficiente.

## 2. Frente construtora

A frente construtora deve especificar a capacidade em C11 estrito, declarar limites, definir estados de sucesso e falha, manter provenance mínima, produzir fixtures legíveis e incluir uma suíte positiva. Nenhuma capacidade pode esconder uma dependência de rede, relógio externo, texto bruto, embedding, identidade ou LLM hospedada.

A frente construtora também precisa registrar o que ainda não foi provado. Um contrato host-side não pode ser apresentado como prova de energia, latência, alcance, WER, persistência de flash, segurança do secure element ou comportamento humano.

## 3. Frente sabotadora

A frente sabotadora deve atacar sem editar os testes para torná-los mais fáceis. Cada sabotagem recebe um ID, um alvo, uma hipótese de violação, uma mutação ou sequência de entrada, a saída esperada e o motivo pelo qual a saída protege a soberania.

Os ataques devem ser reproduzíveis, bounded e seguros. O harness não deve abrir rádio real, enviar mensagens, apagar dados do usuário ou executar artefatos não confiáveis. No host, ataques físicos são representados por modelos explícitos: escrita interrompida, leitura adulterada, reboot, ausência do Core, perda de sessão, corrupção de RAM e falha de sensor.

## 4. Classes de ataque

| Classe | Exemplos de sabotagem | Vitória do HERUS |
|---|---|---|
| **Formato** | enum inválido, ponteiro nulo, campo não canônico, tamanho máximo, símbolo zero | rejeitar sem leitura insegura ou mutação parcial |
| **Semântica** | ambiguidade, contradição, regra cíclica, premissa ausente, handle incompatível | abster-se ou retornar lacuna explícita |
| **Autoridade** | confirmação falsa, confirmação reutilizada, escopo ampliado, Core como decisor | bloquear, revogar ou exigir confirmação exata |
| **Temporalidade** | rollback, replay, expiração, geração futura, reboot no meio da transação | rejeitar ou restaurar somente estado autenticado |
| **Integridade** | digest/HMAC adulterado, slot corrompido, provenance falsa, bit flip | falha fechada antes de promover estado |
| **Exaustão** | memória cheia, orçamento zero, saturação, flood, repetição de proposta | limite bounded sem falso sucesso |
| **Privacidade** | contexto sensível, terceiro, localização, áudio, texto bruto, identidade | bloquear e não registrar conteúdo privado |
| **Transporte** | Core ausente, enlace expirado, rádio indisponível, resposta atrasada | continuar localmente ou retornar `CORE_UNAVAILABLE` |
| **Combinada** | reboot + rollback + conflito; consentimento revogado + proposta pendente | entrar em estado seguro sem recuperar autoridade antiga |

## 5. Critérios de vitória e derrota

A frente construtora vence um ataque somente se a resposta observada estiver no conjunto permitido para aquele cenário e se não houver efeitos laterais: nenhuma inserção parcial, nenhum contador usado como autoridade, nenhum avanço de turno, nenhum envio, nenhum atuador, nenhum log proibido e nenhuma promoção implícita.

A frente sabotadora vence se obtiver qualquer um dos seguintes resultados: fato pessoal sem confirmação, proposta sensível apresentada proativamente, escolha automática entre alternativas conflitantes, replay aceito, rollback promovido, escopo de confirmação ampliado, Core tratado como cérebro, memória corrompida aceita, saída stale retornada como atual ou sucesso declarado após saturação parcial.

Crash, timeout ou consumo ilimitado também são derrotas do construtor quando o contrato prometia bounded. O harness deve distinguir falha do programa, rejeição esperada e abstenção válida.

## 6. Mutação mínima obrigatória

A campanha deve remover ou inverter controles críticos individualmente: autenticação, comparação constante, verificação de digest, anti-rollback, validação de contagem, zeroing de saída, bloqueio de contexto sensível, consentimento proativo, verificação de escopo, consumo único e barreira de autoridade do Core.

Cada mutante crítico precisa ser morto por pelo menos um teste específico. Uma campanha que passa porque um mutante não alcançou o caminho crítico não é evidência suficiente; o relatório deve marcar mutantes não alcançados como lacunas.

## 7. Protocolo de combinação de falhas

Depois dos ataques individuais, o harness deve compor pelo menos duas falhas de classes diferentes. A combinação deve preservar a ordem de gates: formato antes de semântica, integridade antes de promoção, autoridade antes de execução, expiração antes de uso e quarentena antes de recuperação.

Combinações prioritárias:

| Combinação | Estado seguro esperado |
|---|---|
| Core ausente + memória local conflitante | consulta local contradita ou abstinência; nunca pedir ao Core para escolher |
| Reboot + slot novo adulterado | recuperar somente o slot antigo autenticado ou permanecer sem cursor |
| Consentimento revogado + proposta pendente | proposta silenciosamente invalidada, sem apresentação |
| Confirmação correta + escopo alterado | confirmação rejeitada como pertencente a outra proposta |
| Memória cheia + nova evidência conflitante | rejeição sem sobrescrever evidência autorizada |
| Geração expirada + replay autenticado antigo | rejeição temporal antes da proposta |
| Haptic indisponível + consulta válida | resposta lógica permanece válida; apresentação física falha sem alterar verdade |

## 8. Resultado mínimo para publicação

Uma PR adversarial só pode ser publicada se o ledger global passar, os mutantes críticos forem mortos, os ataques combinados tiverem resultados permitidos, a proveniência local for reconciliada e os limites host-only permanecerem documentados.

O resultado deve informar separadamente: testes positivos, ataques rejeitados, mutantes mortos, mutantes não alcançados, invariantes simuladas, falhas deliberadas e aquilo que ainda não foi testado em hardware.

## 9. Invariantes REDTEAM-01 a REDTEAM-10

| ID | Invariante |
|---|---|
| `REDTEAM-01` | Todo controle crítico possui pelo menos um sabotador que o mata quando removido. |
| `REDTEAM-02` | Ataque bem-sucedido nunca pode conceder autoridade; deve falhar fechado. |
| `REDTEAM-03` | Falha combinada não pode reativar estado expirado, revogado ou em quarentena. |
| `REDTEAM-04` | O harness é determinístico, bounded e não transmite no mundo real. |
| `REDTEAM-05` | Rejeição, abstenção, silêncio, quarentena e limite são resultados observáveis distintos. |
| `REDTEAM-06` | Mutante não alcançado é lacuna de cobertura, não mutante morto. |
| `REDTEAM-07` | Corrupção de saída não pode sobreviver ao zeroing fail-closed. |
| `REDTEAM-08` | A ausência do Core não pode transformar dúvida local em autoridade remota. |
| `REDTEAM-09` | Nenhuma combinação pode transformar confirmação de uma proposta em confirmação de outra. |
| `REDTEAM-10` | Os resultados físicos não podem ser inferidos de simulação host-side. |

## 10. Primeiro resultado executável

A primeira campanha foi aplicada à política de autonomia A0–A4. Foram construídos seis mutantes reais: barreira de contexto sensível, consentimento proativo, confirmação única, escopo silencioso, bits canônicos e vínculo confirmação–proposta. A campanha matou **6/6 mutantes**.

A primeira execução global foi deliberadamente considerada uma falha: a suíte nova passou 16 casos, enquanto o ledger ainda esperava 15. O sistema parou com `SOMETHING REGRESSED` e não houve publicação dessa execução. Depois da correção explícita da expectativa e da reconciliação de proveniência, a segunda execução passou com **63 suítes**, **111 invariantes de sistema simulado**, mutação histórica **7/7** e red-team de autonomia **6/6**.

Isso demonstra que o harness também encontra regressões de integração e não somente sabotagens C. Não demonstra ainda ataques combinados de reboot, flash, Core e transporte; eles permanecem no próximo estágio.

## 11. Limite ético e operacional

A campanha adversarial deve buscar as piores falhas de integridade sem tentar explorar terceiros, transmitir no rádio, acessar contas ou apagar dados reais. A agressividade é dirigida ao contrato e ao harness local. O objetivo é aumentar a confiabilidade do HERUS, não criar uma ferramenta de ataque externo.
