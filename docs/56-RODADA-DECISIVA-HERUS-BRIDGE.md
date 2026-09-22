# Rodada decisiva — HERUS Bridge

## Veredicto

A auditoria profunda reprovou o HERUS para qualquer alegação de execução autorizada, holdout independente, simbiose útil ou produto pronto. O estado defensável continua `not_proven`.

A rodada foi reduzida deliberadamente a três objetivos: impedir falsos positivos no código e na narrativa, construir uma campanha causal isolada para a transferência finita e criar um vertical slice local chamado **HERUS Bridge**.

## P0 — Barreira epistemológica

O runtime checked não promove mais uma proposta local a `SUPPORTED`. Mesmo no modo `STRICT`, o máximo local é `SAFE_BUT_UNPROVEN`; `SUPPORTED` fica reservado para uma futura atestação independente que ainda não existe.

O classificador de simbiose agora exige mecanismo, controles de segurança, privacidade e reprodutibilidade completos antes de usar `mechanism_only`. Qualquer dimensão ausente ou hard limit violado permanece `not_proven`.

O protocolo social agora retorna `DRAFT_PROTOCOL_NO_HUMAN_DATA` quando está completo apenas como protocolo, e `BLOCKED` quando controles, thresholds, política de análise ou claims proibidos estão ausentes.

O workflow CI usa `pipefail`, evitando falso-verde quando `prove.sh` falha antes do `tee`.

## P1 — Campanha causal isolada

`research/bridge_causal.py` executa runtime e oracle por subprocessos, com JSON serializado. O oracle usa a verdade privada do caso e não recebe `expected_negative` como base decisória. A campanha inclui variantes `probe–commit gap` com transcript público idêntico e efeitos ocultos diferentes.

O resultado correto é conservador: a variante com efeito oculto recebe `BLOCKED/COMMIT_EFFECT_NOT_CLOSED`; nenhuma variante recebe `SUPPORTED`. Orçamento zero gera zero probes. O pacote inclui digests de caso e resultado, contagem de probes e custo observados.

Esta é uma campanha causal de pesquisa, não um holdout externo definitivo. Ela reduz o risco de circularidade; não prova generalização ampla.

## P2 — HERUS Bridge

`research/bridge_product.py` define três condições locais: baseline convencional, condição sem simbionte e HERUS checked. A sessão exige definir respostas, trocar de interface, gerar proposta, mostrar prévia e somente então permitir confirmação, cancelamento, abstenção ou parada.

A demonstração não envia mensagens, não transmite áudio e não executa efeitos externos. Seu objetivo inicial é verificar se uma pessoa entende o que é rascunho, confirmação, cancelamento, abstenção e bloqueio.

Nenhuma sessão humana foi realizada neste sandbox. Não há alegação de benefício, acessibilidade populacional ou indispensabilidade.

## Resultado verificado

A suíte da rodada específica passa. A campanha causal é rederivável e os estados do Bridge são fail-closed. O histórico dos holdouts anteriores permanece preservado e continua marcado como evidência negativa/regressão histórica.

O próximo gate legítimo, caso esta base seja aprovada, é revisão independente e cinco ou seis sessões formativas externas. Se as pessoas não compreenderem claramente quando algo será ou não será feito, a linha de produto deve ser redesenhada ou encerrada antes de qualquer hardware, rádio, voz, LLM, memória pessoal ou coleta confirmatória.
