# HERUS — Protocolo mínimo de validação humana do HAP-SEM v0.1

**Estado:** proposta preregistrável; nenhum dado humano ou físico foi coletado.  
**Autoria do projeto:** Gustavo — SummaArs/HERUS.  
**Escopo:** decidir se os padrões HAP-SEM são identificáveis, distinguíveis e seguros o bastante para continuar a engenharia.  
**Não é:** uma alegação de que a linguagem já é universal, nem uma autorização para ensaio sem revisão ética e bancada elétrica.

## 1. Pergunta e hipótese falsificável

A pergunta primária é: **um participante consegue identificar um frame HAP-SEM em um relógio vestível, sob um profile ERM ou LRA calibrado, sem receber pistas visuais ou auditivas, com uma taxa de erro e uma matriz de confusão compatíveis com os limites definidos antes da coleta?**

A hipótese de engenharia é que a codificação posicional `SYNC | scope | class | state | urgency | END` pode ser aprendida e reconhecida melhor do que o acaso, mas pode falhar por confusão entre duração, pausas, amplitude, posição, roupa, movimento ou atuador. A hipótese será rejeitada se os intervalos de confiança dos endpoints não atingirem os gates pré-registrados. O resultado “inconclusivo” também é válido quando a amostra não sustenta uma decisão.

> **Regra:** a matriz host-only de 720 combinações por profile prova apenas o contrato de software. Ela não fornece uma estimativa de percepção humana e não pode ser usada como substituto de participantes.

## 2. Fundamentação técnica

O DRV2605L suporta ERM e LRA, possui reprodução controlada por I²C, diagnóstico e calibração do atuador; para LRA, o dispositivo também oferece controle com rastreamento de ressonância. Isso torna necessário registrar o atuador, a configuração, a calibração e as condições elétricas em cada sessão, em vez de tratar um `effect_id` da biblioteca como significado universal [1].

Estudos de percepção vibrotátil mostram que a forma de apresentação, o intervalo temporal, o comprimento do padrão e a aprendizagem alteram a identificação. Um estudo de padrões no antebraço comparou apresentação sequencial e simultânea, utilizou familiarização, randomização, calibração por acelerômetro e respostas por teclado [2]. Um bracelet háptico com atuadores LRA também tratou identificação e psicofísica como propriedades experimentais, não como garantias do hardware [3]. Essas referências orientam o método, mas seus números não são resultados do HERUS.

## 3. Profiles e estímulos

Cada profile deve ser uma configuração versionada que contenha, no mínimo, atuador (`ERM` ou `LRA`), versão do profile, sequência de efeitos, duração, pausas, amplitude de controle, tensão de alimentação, posição no corpo, fixação e evidência de calibração. O experimento deve usar os mesmos profiles que serão versionados no repositório; mudanças de waveform, montagem ou firmware invalidam a comparação e exigem nova identificação do profile.

O conjunto de estímulos deve ser dividido em três níveis. O primeiro é o **alfabeto físico**, incluindo `SYNC`, `END` e os códigos do codebook usados pelos quatro campos posicionais. O segundo são **pares mínimos**, escolhidos para testar uma diferença por vez: mesmo `scope` com `class` diferente, mesma classe com `state` diferente e mesma semântica com `urgency` diferente. O terceiro é o **frame composto**, com combinações balanceadas dos cinco scopes, seis classes, seis estados e quatro urgências.

A validação confirmatória deve cobrir os 720 frames semânticos por profile ou, se o tempo de sessão impedir isso, declarar antes da coleta o subconjunto balanceado e não chamá-lo de cobertura universal. Em qualquer caso, os pares críticos abaixo são obrigatórios:

| Confusão proibida | Motivo de segurança | Resultado aceitável |
|---|---|---|
| `PRIVACY/DENIED` → `ACK/CONFIRMED` | não transformar recusa em confirmação | nenhum erro observado e limite superior de confiança abaixo do gate pré-registrado |
| `ERROR/UNKNOWN` → qualquer estado confirmado | não transformar incerteza em fato | nenhum erro observado e limite superior abaixo do gate |
| `PLAN/PENDING` → confirmação de execução | não converter proposta em ação | nenhum erro observado e nenhum comando emitido |
| `U3` → `U0` ou `U1` | não perder prioridade crítica | nenhum erro observado e limite superior abaixo do gate |
| profile ERM → interpretação do frame LRA | não intercambiar waveforms sem contrato | rejeição explícita ou classificação de profile incompatível |

A ponte `hs_signal_t` continua obrigatória: a resposta do participante identifica ou rejeita um padrão; ela nunca concede ao frame autoridade para transmitir, persistir, executar ou alterar memória.

## 4. Desenho experimental mínimo

O estudo deve ter aprovação ética aplicável, consentimento informado, critérios de inclusão e exclusão, plano de compensação e um responsável por interromper a sessão. O tamanho da amostra não deve ser escolhido por conveniência: deve ser calculado antes da coleta a partir do endpoint primário, do limite de erro tolerado, do número de profiles e da dependência entre tentativas. Um piloto separado pode estimar variância e confusões, mas não deve ser misturado silenciosamente à análise confirmatória.

Cada participante deve passar por familiarização, calibração e blocos de teste. A ordem de ERM/LRA, os frames, os pares mínimos e as condições de movimento devem ser randomizados ou contrabalanceados. Deve existir uma condição estacionária e, se a segurança e a fixação permitirem, uma condição de movimento leve; a condição de movimento não pode ser usada para esconder falhas da condição estacionária.

Os participantes devem responder por teclado ou tela sem feedback corretivo durante o bloco medido. A sessão deve conter pausas e um mecanismo de resposta “não sei/não senti”. Uma resposta forçada não deve ser contada como acerto. Para reduzir pistas externas, o procedimento deve mascarar sons audíveis do atuador e impedir visão do profile; o operador que registra o ensaio não deve ver a classe correta durante a resposta.

A calibração deve medir, para cada profile e sessão, pelo menos amplitude/acceleration no ponto de contato, duração real, intervalo, corrente ou energia por estímulo quando disponível, tensão, temperatura do conjunto e falhas I²C. O driver deve ser configurado explicitamente para ERM ou LRA e a sessão deve ser invalidada se a calibração não for reproduzível dentro da tolerância previamente definida. A documentação do DRV2605L recomenda considerar a variação dos atuadores reais e o uso de medição apropriada ao observar a saída; por isso, o número de biblioteca sozinho não constitui evidência de equivalência [1].

## 5. Endpoints e análise

O endpoint primário é a **identificação exata do frame**, com intervalo de confiança de 95% por profile e condição. Endpoints secundários são identificação por campo, matriz de confusão, tempo de resposta, taxa “não senti”, aprendizagem entre blocos, diferença ERM/LRA, efeito de movimento e estabilidade após repetição. Os dados devem ser analisados por participante e por frame; a média global sozinha pode esconder um profile que falha para uma parte dos usuários.

Os gates numéricos devem ser congelados no preregistro. Como proposta para revisão, não como resultado do HERUS, o gate confirmatório pode exigir limite inferior de 95% para identificação exata acima do limiar de uso definido pelo produto, limite superior de 95% para cada confusão crítica abaixo do risco permitido e ausência de qualquer caminho em que uma classificação háptica seja interpretada como autorização. Se a decisão depender de um limiar, esse limiar deve ser justificado antes de olhar os dados.

A matriz de confusão deve preservar a direção do erro. “`ALERT` foi percebido como `NOTICE`” não é equivalente a “`NOTICE` foi percebido como `ALERT`”. Para cada par, devem ser reportados contagem, denominador, intervalo de confiança e condição. Correções pós-hoc de waveform só podem gerar um novo profile versionado e uma nova rodada; não se pode reinterpretar o erro como outro significado depois da coleta.

## 6. Privacidade, segurança e parada

O log do produto e o dataset do estudo não devem conter áudio, transcrição, embedding, identidade, localização, texto livre, chave ou conteúdo pessoal. O registro mínimo é um identificador aleatório temporário da sessão, profile versionado, código do estímulo, resposta categórica, condição, latência quantizada, calibração numérica e estado de segurança. A tabela que liga uma pessoa ao código da sessão deve permanecer fora do repositório e ser destruída conforme o plano ético.

A sessão deve parar imediatamente em dor, dormência, irritação de pele, desconforto persistente, aquecimento anormal, falha elétrica, vibração não comandada ou perda de controle de intensidade. O dispositivo deve possuir um corte físico de energia e o ensaio não deve prosseguir em presença de falha de I²C, profile não validado, sensor de calibração ausente ou discrepância de atuador. Nenhuma resposta do participante pode disparar rádio, transmissão, persistência de memória ou ação externa.

## 7. Decisão ao final

Há quatro decisões possíveis. **Avançar** significa que o profile e a condição atingiram todos os gates pré-registrados, sem confusões críticas, e podem entrar em uma bancada mais ampla. **Reduzir o alfabeto** significa que alguns tokens são confundidos e devem ser removidos ou separados temporalmente. **Recalibrar/reprojetar** significa que o problema está em montagem, atuador, amplitude, duração ou profile, não necessariamente na semântica. **Abster-se** significa que a evidência não permite alegar distinguibilidade ou universalidade.

Mesmo no cenário “avançar”, o resultado não autoriza alegar universalidade em qualquer pele, roupa, posição, população ou contexto. A linguagem só poderá receber essa descrição depois de replicação independente, múltiplos profiles, condições relevantes e uma análise de segurança que não converta reconhecimento estatístico em autoridade operacional.

## 8. Referências

[1]: https://www.ti.com/lit/ds/symlink/drv2605l.pdf "Texas Instruments — DRV2605L 2- to 5.2-V Haptic Driver for LRA and ERM with Effect Library and Smart-Loop Architecture"

[2]: https://www.mdpi.com/2076-3417/14/1/43 "Yeganeh, Makarov, Kristjánsson e Unnthorsson — Discrimination Accuracy of Sequential Versus Simultaneous Vibrotactile Stimulation on the Forearm, Applied Sciences, 2024"

[3]: https://doi.org/10.1109/TRO.2022.3164840 "Pezent et al. — Design, Control, and Psychophysics of Tasbi: A Force-Controlled Multimodal Haptic Bracelet, IEEE Transactions on Robotics, 2022"
