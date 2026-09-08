# 10 — Investigação pré-registrada: Core versus Núcleo

**Avanço 4 de 10 · HERUS-A4-001 · Revisão 1.0 · sem dados observados**

> **Pergunta.** Em uma linguagem de interação HERUS deliberadamente controlada, qual fonte local de ASR — Core ou Núcleo — satisfaz os gates de segurança e oferece a melhor combinação observável de acurácia de intenção, latência e energia por sessão?

Este é um plano de investigação, não um relatório de resultados. Ele é congelado antes de coletar áudio, transcrições ou telemetria de hardware. A escolha de pré-registro cria um plano específico antes do estudo e permite distinguir análises confirmatórias das exploratórias. [1] Alterações posteriores não invalidam o trabalho, mas devem ser registradas em `research/changes/` com motivo, data, impacto e classificação como confirmatória ou exploratória.

## 1. Escopo e unidade experimental

A unidade experimental é uma tentativa de interação iniciada por um botão físico, com uma frase de uma lista controlada ou uma condição negativa. Uma tentativa começa em `button_ms` e termina em `sent`, `cancelled`, `rejected`, `timed_out` ou `source_lost`. A voz e a transcrição não entram no log de produto; quando a gravação consentida for necessária para WER, ela fica fora do repositório e recebe controle de acesso separado.

| Elemento | Definição prévia |
|---|---|
| Fontes comparadas | `core` e `nucleus`, cada uma executando processamento **local** |
| Participantes | 12 adultos consentidos, identificados apenas por `p01`–`p12` no CSV |
| Desenho | Intra-participante; mesma bateria para ambas as fontes, ordem de fonte balanceada por participante |
| Conjunto de desenvolvimento | Pode calibrar AFE, microfone, gramática e limiares; **nunca** entra no resultado confirmatório |
| Conjunto confirmatório | Itens congelados antes da coleta, sem retuning de modelo, gramática ou limiar após o primeiro item |
| Ambientes | Silêncio, conversa de fundo controlada, rua/ruído reproduzível e vento reproduzível; condição e instrumento anotados |
| Firmware | Hash do commit, modelo de devkit, microfone, atuador, medidor de energia e configuração de ASR registrados por execução |

O uso de conjuntos separados segue uma distinção básica de avaliação: dados usados para desenvolvimento não devem ser reutilizados para testar a hipótese. O plano OpenSAT do NIST distingue conjuntos de desenvolvimento e teste e separa as tarefas de detecção de fala, busca e ASR. [2]

## 2. Hipóteses confirmatórias

| ID | Hipótese que pode falhar | Estatística e regra de decisão |
|---|---|---|
| H1 | Cada fonte reconhece a intenção HERUS com confiabilidade suficiente | Acurácia exata de intenção no conjunto confirmatório; limite inferior de Wilson unilateral 95% ≥ 0,90 para cada fonte |
| H2 | Nenhuma negativa cria rascunho ou intenção | 0 falsos rascunhos em 300 negativas por fonte; limite superior de Wilson unilateral 95% < 0,01 |
| H3 | O pipeline mantém tempo de resposta de produto | p95 de `draft_ms − button_ms` ≤ 2.500 ms por fonte, com todas as amostras válidas reportadas |
| H4 | A confirmação preserva o contrato de envio | `sent / confirm_ms > 0` é exatamente 1,0 e existem 0 envios sem confirmação por fonte |
| H5 | O Núcleo só se torna preferido por benefício mensurável | H1–H4 devem passar; então Núcleo precisa ter **p95 de latência pelo menos 250 ms menor** *e* mediana de energia por tentativa ≤ 80% da do Core. Caso contrário, Core permanece padrão |

H5 é uma regra de produto pré-comprometida, não uma alegação de significância estatística de diferença entre médias. Ela evita promover o Núcleo a dependência quando a melhoria é pequena ou ambígua. Análises de diferença, subgrupos de ruído ou frases individuais são exploratórias, a menos que esta versão seja atualizada **antes** de acessar o conjunto confirmatório.

## 3. Tamanho e composição da amostra

Cada fonte terá 192 tentativas positivas e 300 negativas confirmatórias, totalizando 984 tentativas para as duas fontes.

| Estrato | Por participante/fonte | Participantes | Por fonte | Função |
|---|---:|---:|---:|---|
| Positivas | 16 | 12 | 192 | Medir H1, H3, H4 e H5 |
| Negativas | 25 | 12 | 300 | Medir H2 e segurança contra falso rascunho |
| Total | 41 | 12 | 492 | Bateria confirmatória por fonte |

As 16 positivas devem conter chegada sem tempo, chegada com tempo, ajuda privada e cancelar, em proporções definidas em `research/interaction_study_manifest.json`. “Cancelar” é uma intenção local correta, mas não é uma mensagem enviável; a análise de H4 considera somente confirmações que realmente chegaram a `READY_SEND`.

O intervalo de Wilson é usado porque o NIST/SEMATECH o descreve para proporções e alerta que limites normais simétricos podem ser inadequados para amostras pequenas ou poucas falhas. [3] Com 300 negativas e nenhuma falha, o limite superior unilateral de Wilson a 95% fica abaixo de 1%; o requisito foi escolhido para tornar a ausência observada de falha uma afirmação quantitativa, não um slogan.

## 4. Dados, randomização e cegamento prático

A sequência de tentativas por participante é pré-gerada por `tools/studyplan.py` a partir do manifesto e uma semente declarada no arquivo de execução. A ordem de `core`/`nucleus` é balanceada por participante. O operador que registra o resultado recebe apenas `item_id`, cenário e a fonte a executar; ele não altera a intenção de referência durante a sessão. A pessoa que implementa uma alteração de gramática não pode avaliar o conjunto confirmatório após essa alteração.

| Campo | Permitido no CSV confirmatório | Proibido no CSV confirmatório |
|---|---|---|
| Pseudônimo | `p01`–`p12` | Nome, e-mail, voz identificável ou outro identificador pessoal |
| Referência e resposta | `expected`/`observed` do vocabulário HERUS | Texto transcrito, embeddings, tokens livres ou áudio |
| Tempo e energia | Timestamps monotônicos, energia medida em µJ | Localização, rota, chave, endereço de rádio |
| Contexto técnico | `source`, `scenario`, `device_id`, hash de firmware | Segredos, seed de domínio ou material de sessão |

A WER permanece uma métrica complementar para um conjunto de áudio consentido, anotado e separado. O NIST define WER como deleções + inserções + substituições dividido pelo total de palavras de referência. [2] Ela não substitui a métrica de intenção: “cancelar” transcrito de modo próximo, porém mapeado a chegada, é um fracasso de produto mesmo com baixa WER.

## 5. Dados inválidos, perdas e desvio de protocolo

O analisador reprova uma linha se há envio sem confirmação, timestamps fora de ordem, intenção fora do vocabulário, rascunho em negativa, energia ausente em tentativa enviada ou hash de firmware incompatível com o manifesto. Essas linhas não são removidas silenciosamente: entram em uma tabela de desvios e contam como falha de H4 quando representam evento de envio inseguro. Uma tentativa `source_lost` ou `timed_out` é resultado válido de operação; não é apagada nem convertida em “sem dados”.

A coleta confirmatória é interrompida se houver qualquer envio sem confirmação, qualquer promoção de ajuda para SOS ou qualquer alteração não registrada de firmware/gramática. A investigação volta ao estado exploratório até que uma nova versão do plano seja aprovada. A transparência de mudanças é parte do método de pré-registro, não uma exceção. [1]

## 6. Relatório e decisão

`tools/interactionstudy.py` gera um relatório de dados confirmatórios contendo contagens, proporções, intervalos de Wilson, latência p50/p95, energia mediana e as decisões H1–H5. Ele não pode declarar “Núcleo preferido” se a amostra estiver incompleta, se os gates de segurança falharem ou se a diferença pré-definida não for atingida. O relatório sempre inclui uma seção de desvios e uma seção separada para análises exploratórias.

> **Resultado nulo é informativo.** Se Core e Núcleo passam os gates mas o Núcleo não ganha pelos limiares de H5, o produto mantém Core como padrão e o Núcleo continua opcional. A decisão é sucesso do método: evita custo, consumo e dependência sem benefício demonstrado.

## 7. Referências

[1] [Center for Open Science — Preregistration](https://www.cos.io/initiatives/prereg)
[2] [NIST OpenSAT20 Evaluation Plan](https://www.nist.gov/document/2020opensat20evaluationplanv16)
[3] [NIST/SEMATECH — Confidence intervals for proportions](https://www.itl.nist.gov/div898/handbook/prc/section2/prc241.htm)
