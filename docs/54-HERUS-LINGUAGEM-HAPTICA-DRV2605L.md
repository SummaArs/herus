# HERUS — Linguagem háptica universal sobre DRV2605L

**Status:** especificação v0.1; sem alegação de universalidade perceptiva antes de validação humana e ensaio com LRA/ERM.

## Princípio

O HERUS não deve transmitir frases naturais por vibração. Deve transmitir **eventos semânticos tipados** cuja interpretação não dependa do português, inglês ou de outro idioma. A camada háptica é uma apresentação e um canal de confirmação; ela não concede autoridade, não executa plano, não inicia transmissão e não transforma uma vibração em consentimento.

A TI documenta o DRV2605L como controlador para ERM/LRA com biblioteca de waveforms, sequenciador, reprodução em tempo real por I2C e rastreamento de ressonância [1]. Um guia independente confirma que efeitos podem ser carregados na memória em uma sequência de oito slots e disparados em ordem, e observa que seu uso prático foi validado com um motor ERM específico [2]. Portanto, a semântica deve ser estável, mas a síntese perceptiva deve ser parametrizada por perfil de atuador.

> **Universalidade proposta:** o significado e o formato lógico são independentes do idioma; a sensação física ainda precisa ser calibrada e validada por população, posição corporal e atuador.

## Arquitetura em camadas

| Camada | Responsabilidade | Pode variar? |
|---|---|---|
| `HAP-SEM` | Evento, classe, estado, urgência e escopo | Não, salvo versão explícita |
| `HAP-FRAME` | Delimitadores, tokens, validade e fragmentação | Não dentro da versão |
| `HAP-PROFILE` | Mapeia tokens para efeitos DRV2605L e tempos | Sim, por LRA/ERM, montagem e calibração |
| `HAP-UI` | Apresenta, repete, silencia e confirma compreensão | Sim, por preferência e acessibilidade |

O significado nunca deve ser inferido de um ID bruto de waveform. O ID pertence ao perfil físico. A camada semântica usa tokens estáveis; o perfil traduz cada token para uma entrada válida da biblioteca, RTP ou sequência compatível.

## Frame semântico

A forma canônica é:

```text
HMSG(v, scope, class, state, urgency, data, fragment, integrity)
```

`v` é a versão da linguagem. `scope` informa a origem sem conceder confiança. `class` identifica a família do evento. `state` informa o resultado ou situação. `urgency` controla prioridade de apresentação. `data` é opcional e limitado a códigos pré-declarados. `fragment` permite dividir mensagens maiores que uma emissão. `integrity` permite detectar corrupção entre nós digitais, mas não é uma garantia de compreensão humana.

A primeira versão aceita no máximo **oito slots de sequência por emissão**, conforme o modelo prático documentado para o Waveform Sequencer [2]. O frame lógico pode ser fragmentado em várias emissões, sempre com delimitadores de continuação e expiração. Um frame incompleto não deve ser interpretado como comando.

## Vocabulário semântico v0.1

### Escopo

| Código | Significado |
|---|---|
| `SYS` | Estado interno do dispositivo |
| `COM` | Comunicação recebida ou aguardando envio |
| `MEM` | Memória pessoal ou registro local |
| `PLAN` | Proposta de planejamento |
| `SFTY` | Segurança, falha ou proteção |

O escopo é apenas contexto; não é identidade de remetente nem autorização.

### Classe

| Código | Significado |
|---|---|
| `NOTICE` | Informação não urgente |
| `ALERT` | Atenção necessária |
| `QUERY` | Há uma pergunta ou escolha pendente |
| `ACK` | Recebimento/estado confirmado pelo sistema |
| `ERROR` | Operação recusada, falha ou inconsistência |
| `PRIVACY` | Evento que exige proteção, descarte ou revisão |

### Estado

| Código | Significado |
|---|---|
| `NEW` | Evento novo, ainda não revisado |
| `PENDING` | Aguarda confirmação ou recurso |
| `CONFIRMED` | Confirmação física ou lógica já registrada |
| `DENIED` | Negado explicitamente |
| `UNKNOWN` | Evidência insuficiente; abstention |
| `EXPIRED` | Não deve mais ser apresentado como atual |

### Urgência

| Código | Semântica |
|---|---|
| `U0` | Silencioso/baixa prioridade; apresentar apenas em contexto permitido |
| `U1` | Normal |
| `U2` | Alta; pode repetir uma vez dentro da validade |
| `U3` | Crítica; requer política explícita e nunca substitui confirmação física |

Urgência não deve ser codificada apenas por amplitude, porque amplitude percebida depende de pele, posição, pressão, roupa e atuador. A primeira versão usa estrutura temporal e política de repetição; amplitude é apenas parâmetro de perfil.

## Alfabeto temporal

O encoder não envia os nomes acima. Ele os transforma em uma sequência de tokens físicos de baixa cardinalidade. Na versão v0.1, os quatro campos são códigos posicionais de até quatro bits; o profile converte cada token para um efeito do DRV2605L:

| Token | Papel semântico | Forma abstrata |
|---|---|---|
| `SYNC` | Início inequívoco | dois pulsos curtos separados |
| `MARK` | Separação de campos | pulso curto isolado |
| `UNIT0` | símbolo de baixo valor | pulso curto |
| `UNIT1` | símbolo alternativo | pulso duplo |
| `UNIT2` | símbolo de alta atenção | pulso longo |
| `UNIT3` | símbolo de transição/continuação | rampa ou par assimétrico |
| `END` | fim do frame | pulso longo seguido de silêncio |
| `ABSTAIN` | informação não confiável | padrão reservado, nunca reutilizado como sucesso |

A tabela não afirma que todas as formas serão perceptualmente distinguíveis. Ela define candidatos de um codebook que serão testados. Se `UNIT1` e `UNIT2` forem confundidos acima do limite definido, o perfil deve reduzir o alfabeto ou aumentar a separação temporal; não deve reinterpretar silenciosamente o evento. O encoder C11 atual já prova a estrutura e o checksum, mas não a percepção.

## Codificação temporal

O frame físico v0.1 usa posições fixas:

```text
SYNC | scope | class | state | urgency | END
```

São seis slots: quatro códigos semânticos posicionais, mais delimitadores de início e fim. Isso evita ambiguidade e cabe no orçamento de oito slots. `MARK` fica reservado para a futura extensão de fragmentação; não é inserido artificialmente nesta versão. Dados e fragmentos que não couberem são rejeitados como `HL_E_FRAGMENT`, nunca truncados silenciosamente. A primeira versão ainda não codifica `data` no frame compacto.

O perfil define uma tabela `token -> waveform_id, duration_class, pause_class, amplitude_class`. `waveform_id` é específico do DRV2605L/biblioteca escolhida. A tabela deve registrar atuador, versão da biblioteca, calibração e condições de alimentação.

## Semântica de confirmação

A linguagem separa apresentação de ação:

| Situação | Háptico permitido | Ação automática |
|---|---|---|
| Fato novo | `MEM/NOTICE/NEW` | nenhuma persistência sem confirmação |
| Regra ou plano proposto | `PLAN/QUERY/PENDING` | nenhuma execução |
| Confirmação física recebida | `ACK/CONFIRMED` | apenas o efeito previamente autorizado |
| Contradição ou baixa confiança | `ERROR/UNKNOWN` ou `ABSTAIN` | nenhuma escolha automática |
| Dados sensíveis detectados | `PRIVACY/DENIED` | descartar/abster conforme política |

Um padrão `ACK` significa apenas que a camada correspondente registrou uma confirmação válida. Ele não deve ser reproduzido quando o dispositivo apenas recebeu uma proposta ou quando a ação física falhou.

## Invariantes da linguagem v0.1

| ID | Invariante | Falha correta |
|---|---|---|
| `HAP-01` | Todo frame possui versão e `SYNC` | rejeitar |
| `HAP-02` | Todo frame termina em `END` ou expira como incompleto | `UNKNOWN` |
| `HAP-03` | Uma mensagem não excede oito slots por emissão | fragmentar ou rejeitar |
| `HAP-04` | Perfil de atuador incompatível não é aceito como equivalente | `PROFILE_MISMATCH` |
| `HAP-05` | Token desconhecido não vira sucesso por fallback | `UNKNOWN` |
| `HAP-06` | Urgência não concede autoridade | nenhuma mutação |
| `HAP-07` | `ACK` só representa confirmação comprovada | rejeitar falso ACK |
| `HAP-08` | Corrupção ou frame incompleto não seleciona uma alternativa | `ABSTAIN` |
| `HAP-09` | Repetição tem orçamento, validade e deduplicação | silenciar/expirar |
| `HAP-10` | O encoder não coloca texto, áudio, identidade ou localização no frame | rejeitar |
| `HAP-11` | O mesmo frame semântico pode ser traduzido por perfis físicos diferentes | exigir mesma semântica, não mesma waveform |
| `HAP-12` | Nenhum evento háptico sozinho inicia transmissão ou execução | confirmação física separada |

## O que ainda não foi provado

Não há alegação de que oito tokens sejam distinguíveis em qualquer pessoa, roupa, posição ou atuador. Também não há WER háptico, taxa de confusão, limiar de intensidade, latência, consumo ou segurança física medidos. A próxima fase deve implementar um encoder/simulador determinístico e gerar a matriz de todos os frames permitidos, incluindo fragmentação, corrupção, perfil incompatível e expiração.

## Referências

[1]: https://www.ti.com/document-viewer/DRV2605L/datasheet "Texas Instruments — DRV2605L datasheet SLOS854"

[2]: https://learn.adafruit.com/adafruit-drv2605-haptic-controller-breakout?view=all "Adafruit — DRV2605L Haptic Controller Breakout"
