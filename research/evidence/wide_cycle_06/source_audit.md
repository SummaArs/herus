# Wide Research 06 — auditoria inicial de fontes

## MInDS-14

Fonte consultada: https://huggingface.co/datasets/PolyAI/minds14. A ficha declara um recurso de detecção de intent com fala, cobrindo 14 intents em 14 variedades linguísticas. A estrutura exibida para `fr-FR` contém no mesmo registro `path`, `audio` com array e sampling rate, `transcription`, `english_transcription`, `intent_class` e `lang_id`. A ficha declara licença Creative Commons CC-BY e informa 471 MB para a configuração mostrada.

Estado: **candidato, não baixado**. A ficha Hugging Face é uma fonte de distribuição; ainda falta confirmar o repositório/autoria primária, o commit/snapshot, os termos exatos do pacote efetivamente baixado e a regra de identidade entre áudio, transcrição e classe. Nenhum registro do MInDS-14 foi convertido para ARRIVE/HELP/CANCEL.

Limite: mesmo que o pacote seja confirmado, MInDS-14 pode provar apenas áudio–transcrição–intent externo. Não prova os três eventos HERUS sem anotação independente.

## Proveniência PolyAI adicional

O repositório `PolyAI-LDN/task-specific-datasets` foi aberto em `master`, commit exibido `57ec275`. Ele declara que os datasets compartilhados no repositório usam a licença do arquivo LICENSE, identificado como CC-BY-4.0, mas a listagem visível contém Banking, Span Extraction, NLU++ e EVI; não há prova nesse repositório de que ele hospede o pacote de áudio MInDS-14. Portanto, essa fonte não foi promovida como origem primária do MInDS-14. A ficha `PolyAI/minds14` no Hugging Face continua sendo a fonte de distribuição candidata; autoria primária e snapshot do conjunto real ainda precisam ser fixados.

## MInDS-14 — snapshot de distribuição

A árvore pública `PolyAI/minds14` mostra 18 commits; a conversão para Parquet aparece no commit `40ce77cb32a384e4d50a568e1ec39ac804019d33`. Há configurações por variedade (`en-US`, `fr-FR`, `pt-PT` e outras), tag `cc-by-4.0`, e a ficha informa 1,13 GB gerados no conjunto completo. O exemplo de registro inclui `path`, `audio`, `transcription`, `english_transcription`, `intent_class` e `lang_id`.

Esse commit é um snapshot da distribuição no Hub, não uma prova independente da origem da gravação. Para qualquer download, o ciclo deverá registrar a revisão exata, configuração, arquivos efetivamente obtidos e hashes. A licença pública observada é CC-BY; ela será tratada como licença da distribuição apenas depois da confirmação do pacote/configuração escolhidos. Nenhum áudio foi baixado nesta etapa.

## MInDS-14 — API e amostra estrutural

A API do Hub retorna `sha=40ce77cb32a384e4d50a568e1ec39ac804019d33`, `private=false`, `gated=false`, `disabled=false` e tag `license:cc-by-4.0`. A API Parquet para `config=pt-PT`, `split=train` aponta para `pt-PT/train/0000.parquet`, tamanho declarado de 51.738.310 bytes, com `partial=false`. Os campos declarados são `path`, `audio` com sampling rate 8.000, `transcription`, `english_transcription`, `intent_class` com 14 nomes e `lang_id` com 14 variedades.

A API `first-rows` também retorna URLs assinadas de assets `audio/wav` para a revisão `40ce77...` e registros que incluem áudio, transcrição e classe no mesmo objeto. As URLs assinadas e as sentenças observadas não serão gravadas nos artefatos públicos. Isto habilita uma possível amostra mínima, mas não substitui a confirmação de termos do pacote e o registro de hashes dos bytes efetivamente baixados.

## Primeira amostra de áudio real MInDS-14

A rota oficial do Datasets Server forneceu uma única amostra `pt-PT/train/0` na revisão `40ce77cb32a384e4d50a568e1ec39ac804019d33`. Para evitar publicar caminho, sentence ou label, o artefato registra somente `path_hash=013dd9eb09bc928078cffedc52fd67781fa60bd1fd1408dd6360968a633075dd`, `audio_sha256=fc084982ad50c6ea6cf066f08374b9b3aaa628d9a9accb167be5ae9376dbd275` e métricas técnicas.

O download local foi de 72.422 bytes. O contêiner é RIFF/WAVE com `audio_format=7` (G.711 μ-law), mono, 8.000 Hz, 72.363 frames e 9.045 ms. Transcrição, tradução inglesa, `intent_class` e `lang_id` estão presentes no registro retornado. O áudio não foi decodificado; somente headers, tamanho e estrutura foram validados. A primeira execução falhou porque o módulo Python `wave` não aceita format 7; o probe foi corrigido para validar PCM/A-law/μ-law estruturalmente.

Veredito: **uma amostra áudio–texto–intent estruturalmente íntegra, sem convergência semântica HERUS**. Não há mapeamento para `ARRIVE`, `HELP` ou `CANCEL`; não há autoridade operacional.

## Parser C na mesma amostra MInDS-14

A amostra pt-PT foi enviada ao parser C oficial usando sua transcrição real, sem imprimir a sentença. O parser recebeu uma linha; `DRAFT=0`, `CANCEL_LOCAL=0`, `UNKNOWN=0`, `REJECTED=1`, `parser_non_ascii=1`, `parser_overlong=0`, `parser_empty=0`, mapeamento automático `0` e autoridade de comando `0`.

A rejeição decorre da política fail-closed do parser para bytes não ASCII, não de uma decisão semântica sobre a intenção bancária externa. O resultado é uma verificação de integridade e fronteira operacional da amostra, não convergência áudio–transcrição nem convergência HERUS.

## Resultado após suporte WAV e regressões

A correção do probe para WAV G.711 μ-law foi coberta por cinco testes: μ-law válido, PCM válido, A-law válido, formato desconhecido rejeitado, chunk truncado rejeitado e ausência de data rejeitada. A suíte de pesquisa passou com **64 testes**.

A amostra real baixada por rota oficial continua local em `/tmp` e não está no repositório. Seu hash técnico é `fc084982ad50c6ea6cf066f08374b9b3aaa628d9a9accb167be5ae9376dbd275`; o artefato público registra apenas hash e metadados técnicos. O parser C recebeu a transcrição do mesmo registro e rejeitou por não ASCII; nenhum comando HERUS foi produzido.

## MInDS-14 — endpoint de amostragem corrigido

O endpoint `first-rows` ignorou o offset solicitado e retornou a primeira página. O teste em `https://datasets-server.huggingface.co/rows?dataset=PolyAI%2Fminds14&config=pt-PT&split=train&offset=100&length=1` retornou `row_idx=100`, `num_rows_total=604`, `num_rows_per_page=100` e `partial=false`. A amostragem determinística do lote será corrigida para usar `/rows`, preservando offsets explícitos e sem publicar os registros retornados.

## Lote determinístico MInDS-14

O batch foi corrigido após dois erros reais do endpoint: `first-rows` não respeitava offsets e `/rows` não repetia os campos dataset/config/split no JSON. Com `/rows`, os offsets `0,100,500,600` retornaram `row_idx` exatos no split `pt-PT/train`.

Resultado agregado: 4 registros, 4 áudios presentes, 4 WAVs estruturalmente válidos, 4 transcrições presentes, 4 intents presentes, 4 hashes de caminho distintos, 4 hashes de áudio distintos, 0 duplicatas no batch. Foram 4 WAVs temporários, todos apagados após a auditoria. O parser C executou 4 transcrições: `DRAFT=0`, `CANCEL_LOCAL=0`, `UNKNOWN=0`, `REJECTED=4`, `parser_non_ascii=4`, `automatic_label_mapping=0` e `herus_command_authority=0`.

O batch comprova integridade estrutural de uma pequena amostra áudio–texto–intent distribuída pelo Hub. Não comprova semântica HERUS: a licença é externa, a intent é bancária, o parser rejeita a transcrição por não ASCII e não existe anotação independente para os três eventos operacionais.

## Comparador endurecido

O comparador passou a recusar `identity_ambiguous`, `identity_conflict`, `label_only_pairing`, `order_only_pairing`, `basename_only_pairing` e `timestamp_mismatch`. `multimodal_rate` agora recusa `duplicate_sample_id` entre pares válidos, em vez de contar duas observações do mesmo registro.

A primeira execução direta pela raiz falhou por import relativo (`ModuleNotFoundError: convergence`); o teste foi corrigido para inserir o diretório `research` no path, sem mudar o comportamento do comparador. Após a correção, a suíte passou com **70 testes**.

## Gates Makefile do batch

O alvo `make -C research minds14-batch-audit` sem `MINDS14_ALLOW_AUDIO=1` falhou fechado com `missing MINDS14_ALLOW_AUDIO=1; explicit local research audio gate`. Com a flag explícita, o alvo normal passou. `make -C research minds14-batch-sanitizers MINDS14_ALLOW_AUDIO=1` também passou com ASan/UBSan no parser C e com os quatro WAVs temporários apagados após o processamento.

## Auditoria de identidade SLURP real

O auditor corrigido processou os três JSONL reais e o metadata local: 16.521 linhas, 72.395 referências de gravação únicas e 72.395 nomes únicos no metadata. Não houve referência ausente nem duplicação de `slurp_id` entre os splits. O metadata contém **1 filename duplicada**, com **0 conflitos de assinatura**; o texto contém **1 referência repetida**, com uma ocorrência excedente.

O status calculado foi `AMBIGUOUS`, `identity_unambiguous=false` e `identity_gate_passed=false`. Joins por ID numérico, label, ordem ou basename foram explicitamente desabilitados. O resultado permanece `identity_blocked; no pair promotion`, com mapeamento HERUS `0` e convergência HERUS `false`.

## Suíte após integração

Depois de integrar MInDS-14, o batch `/rows`, o probe WAV e o auditor de identidade, `make -C research test` passou com **80 testes**. A execução sem `MINDS14_ALLOW_AUDIO=1` continua bloqueada antes da rede; os alvos normal e ASan/UBSan só executam com flag local explícita.

## Brecha real no parser encontrada no corpus

A primeira execução do split completo encontrou **1 falso `DRAFT`** em 604 transcrições. O diagnóstico local identificou uma sentença contextual de domínio externo contendo uma palavra HELP. A regra anterior disparava HELP por ocorrência de qualquer palavra-chave dentro da sentença. A frase individual, seu índice e seu hash não fazem parte do pacote publicado.

Correção no parser C: HELP agora exige uma frase controlada (`socorro`, `ajuda`, `ajude`, `preciso de socorro`, `preciso de ajuda` ou `preciso de ajude`); CANCEL recebeu a mesma exigência de forma exata. Palavras de controle embutidas em sentenças não reconhecidas são rejeitadas. A regressão foi executada com `make -C firmware voice` e o split completo foi repetido.

Após a correção: 604 linhas, `DRAFT=0`, `CANCEL_LOCAL=0`, `UNKNOWN=53`, `REJECTED=551`, `parser_non_ascii=545`, `automatic_label_mapping=0` e `herus_command_authority=0`. A correção reduz falso positivo, mas a rejeição por não ASCII de transcrições pt-PT continua sendo comportamento fail-closed do parser ASCII controlado.

## Regressão firmware do falso positivo

Foram adicionadas regressões para `preciso de ajuda` válido e para palavras HELP/CANCEL embutidas em sentenças não controladas. `make -C firmware voice` passou; a suíte de pesquisa passou com **81 testes**. O único `DRAFT` do audit anterior foi eliminado no split completo MInDS-14, sem introduzir autoridade operacional.

## Endurecimento dos wrappers

Os testes unitários offline cobrem JSON raiz inválido, newline embutido na sentença antes de chamar o runner C, host de áudio diferente de `datasets-server.huggingface.co`, WAV truncado, formato desconhecido, chunks ausentes e asset acima de 2 MiB sem escrita do arquivo. Foram **11 testes** específicos dos wrappers, todos aprovados; nenhum fixture acessa a rede ou contém mídia real.

## Protocolo de anotação independente

Foi versionado `herus-independent-annotation-v1` com seis estados finitos: `ARRIVE`, `HELP`, `CANCEL`, `OTHER`, `AMBIGUOUS` e `CONFLICT`. O protocolo exige julgamento independente, não permite mapeamento de `intent_class` externo e não expõe caminho para a bridge, frame, confirmação ou autoridade operacional. Ausência de evidência resulta em `AMBIGUOUS`; julgamentos incompatíveis resultam em `CONFLICT`; fala fora do vocabulário HERUS resulta em `OTHER`. Ainda não há alegação de anotação real deste ciclo.

Os testes garantem que `OTHER`, `AMBIGUOUS` e `CONFLICT` não atravessam a fronteira operacional e que até `ARRIVE`/`HELP`/`CANCEL` anotado independentemente não equivale a confirmação física. O schema JSON publicado em `herus_annotation_protocol_v1.json` corresponde à implementação; seus **8 testes** específicos passaram. A suíte completa de pesquisa, já incluindo esse protocolo e as regressões dos wrappers, passou com **93 testes**, conforme `research_tests_final.txt`.

## Higiene do pacote

Os quatro arquivos `*_raw.txt` que eram byte-a-byte idênticos aos JSONs agregados foram removidos. Permanecem somente JSONs redigidos, o relatório textual e logs de execução sem frases, identificadores individuais, caminhos de mídia ou URLs assinadas.

## Gates finais do ciclo

Após o rerun das auditorias reais, `make -C research test` passou com 93 testes; MIntRec real permaneceu em 1.779 linhas com `DRAFT=0`, `CANCEL=0`, `UNKNOWN=1.636`, `REJECTED=143` e mapeamento automático zero. Os gates textuais SLURP disponíveis localmente passaram mantendo identidade bloqueada e áudio não carregado. `make -C firmware all`, `make -C firmware analyzer`, `make -C firmware sanitizers`, `make -C sim sanitizers` e `./prove.sh --quiet` passaram. A validação JSON, `provenance_audit.py --strict` e `git diff --check` também passaram.

O scan final não encontrou tokens de assinatura, credenciais, a sentença individual do contraexemplo ou arquivos de mídia crua no diretório. O manifesto de proveniência continua deliberadamente não atestado externamente; o resultado verde confirma somente os inputs locais declarados.
