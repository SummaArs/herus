# Guardian de Runtime e Interface Humana — Auditoria da contribuição

## Decisão arquitetural

A contribuição é útil como **segunda camada de assurance operacional**. Ela conecta a verificação do sistema a uma pessoa por fatos tipados, alertas e registro de evidência. A divisão correta é:

> O HERUS detecta e bloqueia localmente; a interface informa e recebe uma decisão; a autoridade de runtime continua no contrato original do HERUS.

A interface física pode ser tela, voz ou pulso. Essas modalidades são canais de apresentação, não fontes de autorização. O pulso é relevante porque oferece um canal de atenção de baixa largura de banda para incidentes críticos, mas não deve ser tratado como uma pulseira obrigatória nem como prova de identidade por si só.

## Componentes aproveitados

| Componente da contribuição | Decisão |
|---|---|
| Eventos tipados | Incorporado no modelo host-only |
| Severidade determinística | Incorporado no classificador de risco |
| Bloqueio automático para eventos críticos | Incorporado como latch de bloqueio, sem liberação autônoma |
| ACK humano | Incorporado somente como reconhecimento auditável |
| Snapshot de evidência | Incorporado com serialização determinística |
| Tela/voz/pulso | Mantido como contrato futuro de transporte/apresentação |
| HAP-SEM completo | Adiado até haver framing, autenticação, anti-replay e testes de codec |
| Autoridade de liberação no Guardian | Rejeitada |

## Defeitos encontrados na implementação original

A contribuição original contém um erro de cooldown: a condição usa `!is_cooled_down`, fazendo o primeiro alerta crítico não ser emitido quando `last_alert_ms == 0`. O comportamento correto é emitir o primeiro alerta e suprimir apenas repetições dentro da janela.

A função de decisão humana também declara uma semântica de ACK que sugere liberação, mas não possui estado suficiente para provar qual alerta foi respondido, se a decisão é autêntica ou se a política permite liberar a ação. Em um sistema crítico, um ACK recebido por si só não pode limpar um bloqueio. O modelo implementado mantém o bloqueio após ACK.

Além disso, o código original aceita configurações com gatilhos duplicados, não valida a severidade da observação, usa `correlation_id` diretamente como ação bloqueada sem contrato de namespace e serializa snapshots com representação nativa de memória. Esses pontos impedem tratá-lo como firmware pronto.

## Implementação atual

`research/critical_runtime_guardian.py` é um modelo de pesquisa independente do firmware. Ele possui limites finitos, rejeita entradas inválidas, registra observações tipadas, impõe capacidade máxima de bloqueios e retorna erro quando a capacidade é excedida. O classificador de risco usa tabela determinística, com escalonamento explícito para baselines de segurança e disponibilidade.

O modelo usa quatro decisões numéricas apenas para teste de contrato: bloqueio, ACK, escalonamento e decisão reservada. Nenhuma delas libera uma operação crítica. A liberação continua pertencendo aos fluxos existentes, como confirmação física, `assurance_decide`, revogação e demais contratos do HERUS.

## Pipeline correto

| Etapa | Responsabilidade | Pode autorizar ação crítica? |
|---|---|---:|
| Observação | Registrar fato tipado e correlacionado | Não |
| Classificação | Determinar severidade e categoria | Não |
| Guardian | Latch de bloqueio e escalonamento | Não; pode bloquear |
| Interface humana | Exibir alerta e transportar resposta | Não |
| Runtime HERUS | Aplicar seus contratos de confirmação e autoridade | Somente conforme contrato existente |
| HCAE | Verificar cobertura, evidência e mutação | Não |

A frase “ajuda a tomar uma ação preventiva” deve ser interpretada como **apresentar informação estruturada e manter uma ação preventiva segura**, não como executar uma ação física autônoma. Qualquer atuação automática fora do bloqueio exigiria um contrato separado, prova específica por ativo e autorização de segurança independente.

## Resultado e próximo limite

A camada é promissora e foi incorporada como modelo determinístico testável. Ela não deve ser enviada ainda ao ESP32-S3 como subsistema autônomo. Antes disso, são necessários um protocolo binário versionado, autenticação e anti-replay de decisões humanas, máquina de estados de sessão de alerta, política explícita de expiração e um adaptador para os sinais reais de `assurance_snapshot_t`.

O próximo passo de maior valor não é adicionar voz ou hardware. É provar a fronteira de autoridade: um alerta, ACK, timeout, replay ou decisão malformada nunca pode produzir uma liberação que o runtime HERUS não produziria por seus próprios contratos.
