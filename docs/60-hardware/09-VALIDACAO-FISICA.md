# 09 — Validação física e operacional do HERUS

**Avanço 3 de 10 · Revisão 0.1 · protocolo de bancada e decisão**

> Um protótipo não é evidência porque funciona uma vez na mesa. No HERUS, uma capacidade só avança de “plausível” para “demonstrada” quando tem um cenário repetível, uma medição registrada, uma regra de aprovação prévia e uma falha que altera a próxima decisão.

Este documento transforma os contratos de voz do Avanço 1 e do runtime do Avanço 2 em uma trilha de validação. Ele não inventa resultados de hardware. A PR entrega o esquema de log, o analisador de sessão, o rig de adaptadores e os gates que uma bancada futura deverá satisfazer.

## 1. Hipóteses e decisões

| Hipótese | Medida primária | Gate inicial | Se falhar |
|---|---|---:|---|
| Linguagem controlada é utilizável em fala real | Acurácia exata de intenção | ≥ 95% em corpus cego | Reduzir gramática, melhorar microfone/AFE ou mover ASR ao Núcleo |
| Ajuda e cancelamento são seguros | Erros críticos | 0 ativações de SOS e 0 envios automáticos | Bloquear a configuração e revisar UX/gramática |
| Ruído não cria rascunhos perigosos | Taxa de falso rascunho em negativas | 0 em cada bateria negativa | Elevar limiar, retirar frase ou melhorar AFE/VAD |
| Interação é responsiva | Latência botão→rascunho | p95 ≤ 2,5 s | Medir divisão AFE/ASR; escolher Core ou Núcleo por dados |
| Confirmação não duplica envio | Hand-offs autorizados ÷ confirmações | exatamente 1,0 | Falha de runtime: não avançar para hardware |
| Energia é compatível com o papel escolhido | Energia medida por sessão | Comparada ao orçamento medido do papel | Mudar local do ASR, janela, modelo ou bateria; não estimar por tensão |

Os limiares de acurácia e latência são critérios iniciais de produto, não especificações do ESP-SR nem promessas de desempenho. O protocolo registra todos os valores por execução e permite trocar um limite apenas com justificativa versionada. A métrica de ASR de referência permanece a WER, definida pelo NIST como a soma de deleções, inserções e substituições dividida pelo total de palavras de referência. [1] Para o HERUS, que aceita uma linguagem controlada, a **acurácia exata de intenção** e os erros críticos são as métricas de liberação mais importantes; uma transcrição com uma palavra errada pode ainda mapear à intenção correta, e uma WER baixa pode ainda errar “cancelar”.

## 2. Corpus e amostragem

A primeira bateria não deve ser um áudio de demonstração. Ela contém uma lista congelada de frases esperadas e negativas, gravada por pessoas que não participaram da implementação.

| Conjunto | Conteúdo | Mínimo por condição | Regra |
|---|---|---:|---|
| Positivo | Chegada, chegada com minuto, ajuda privada e cancelamento | 10 falantes × 12 frases | Cada frase é mapeada para intenção esperada, nunca texto livre |
| Negativo | Conversa comum, ruído, nomes próprios e comandos fora do léxico | 10 falantes × 6 itens | Nenhum item pode criar rascunho |
| Ruído | Silêncio, sala com conversa, rua e ruído de vento reproduzível | Todos os itens | Condição e nível medido entram no log |
| Segurança | Ajuda, cancelar, confirmar, timeout e queda de fonte | 20 repetições por caso crítico | Não admite aprovação por média |

Cada registro recebe `run_id`, `trial_id`, cenário, intenção esperada e intenção observada. **Áudio bruto, transcrição, embeddings, identificador pessoal, localização e chave não são escritos pelo log.** A equipe pode guardar áudio separado e consentido para uma avaliação de WER; tal arquivo nunca entra no formato de telemetria do produto.

## 3. Instrumentação mínima

| Item | Papel | O que registra |
|---|---|---|
| Core ESP32-S3 | Botão, runtime, rádio e vibração | Timestamps monotônicos de eventos e ações |
| Núcleo ou segundo devkit | AFE/VAD/ASR local quando disponível | Apenas `source=core/nucleus` e resultado de intenção, não áudio |
| Medidor de corrente/PMIC | Energia da janela de interação | Integral medida em µJ por `trial_id` |
| Canal de referência | Botão/trigger e captura de energia sincronizada | Alinhamento temporal, se necessário |
| Script `tools/interactionlog.py` | Valida CSV e decide gates | Resumo reprodutível, sem inferência de conteúdo |

O ESP32-S3 dispõe de temporizador de sistema, interfaces I2S e modos de energia distintos, de modo que um adaptador pode fornecer timestamps monotônicos e integrar microfone; esses recursos não substituem medição de placa real. [2] Para energia, o protocolo aceita somente a integral de um instrumento ou PMIC. O Power Profiler Kit II é um exemplo de ferramenta voltada a medir e otimizar consumo de sistemas embarcados. [3]

## 4. Formato de log

Uma linha CSV representa uma tentativa de interação. O cabeçalho é obrigatório e a ordem dos campos é normativa.

```text
run_id,trial_id,scenario,source,expected,observed,button_ms,draft_ms,confirm_ms,send_ms,energy_uj,outcome
lab-a,001,arrival-10,core,arrive,arrive,1000,1520,1710,1720,18400,sent
lab-a,002,negative-noise,nucleus,none,none,2000,0,0,0,11200,rejected
```

| Campo | Regra |
|---|---|
| `run_id`, `trial_id` | Identificam execução e tentativa; não podem conter identidade humana |
| `scenario` | Nome de bateria, nunca transcrição |
| `source` | `core` ou `nucleus` |
| `expected`, `observed` | `arrive`, `help`, `cancel` ou `none` |
| `button_ms` | Timestamp monotônico inicial; `0` para uma negativa sem botão, se aplicável |
| `draft_ms`, `confirm_ms`, `send_ms` | Timestamps monotônicos ou `0` quando a transição não ocorreu |
| `energy_uj` | Integral medida da sessão; inteiro positivo ou `0` se instrumento indisponível |
| `outcome` | `sent`, `cancelled`, `rejected`, `timed_out` ou `source_lost` |

O script rejeita timestamp fora de ordem, envio sem confirmação, `outcome=sent` quando intenção é `none`, vocabulário desconhecido e energia negativa. Esses são erros de log e de sistema, não observações a serem “corrigidas” manualmente.

## 5. Rig de adaptadores e cenários

Antes da bancada, `firmware/core/interaction_rig.[ch]` executa o runtime contra adaptadores determinísticos. O rig faz quatro coisas: injeta botão, transcrição, confirmação e disponibilidade de fonte; registra ações haptics/captura/apresentação; mede latência por timestamps fornecidos; e só registra `sent` quando `interaction_take_send()` retorna sucesso.

A suíte de rig cobre o caminho nominal, recusa, timeout de fala, timeout de confirmação, queda da fonte durante captura e queda após o rascunho. Ela é uma prova de integração entre os Avanços 1 e 2, mas não substitui AFE, ASR ou driver elétrico.

## 6. Procedimento de bancada

1. Grave a versão de firmware, hash de commit, modelo de devkit, microfone, atuador, fonte e instrumento de energia.
2. Execute `./prove.sh` no commit que será testado. Se falhar, **não** inicie coleta.
3. Zere o medidor de energia e execute uma bateria de silêncio para verificar falso rascunho.
4. Execute positivas e negativas em ordem aleatória por condição, com o operador que anota resultados diferente de quem implementou a gramática.
5. Exporte somente o CSV normativo; execute `python3 tools/interactionlog.py --csv <arquivo> --strict`.
6. Anexe o resumo gerado à issue/PR de hardware. Não publique áudio sem consentimento explícito e separado.
7. Se qualquer gate crítico falhar, abra uma correção com o CSV e o cenário; não ajuste a contagem ou exclua a linha.

## 7. Decisão Core versus Núcleo

O primeiro experimento mede as mesmas frases com `source=core` e `source=nucleus`. O Núcleo só se torna o local preferido de ASR se mantiver todos os gates críticos e demonstrar benefício mensurável em latência p95, energia por sessão ou robustez de acurácia. Se não houver benefício, o Core mantém ASR local e o Núcleo continua opcional. Essa regra evita transformar o puck em dependência por estética.

## Referências

[1] [NIST OpenSAT20 Evaluation Plan](https://www.nist.gov/document/2020opensat20evaluationplanv16)
[2] [Espressif ESP32-S3 Series Datasheet](https://documentation.espressif.com/esp32-s3_datasheet_en.pdf)
[3] [Nordic Power Profiler Kit II](https://www.nordicsemi.com/Products/Development-hardware/Power-Profiler-Kit-2)
