# Auditoria de fontes — ciclo 05

## Fluent Speech Commands — consulta oficial

URL consultada: https://fluent.ai/fluent-speech-commands-a-dataset-for-spoken-language-understanding-research/

A página oficial informa 30.043 enunciados de 97 falantes, áudio WAV mono a 16 kHz, uma fala por arquivo, três slots (`action`, `object`, `location`) e 31 intenções formadas pelas combinações dos slots. A fonte declara 248 formulações e CSV por split contendo speaker ID, caminho do arquivo, transcrição e slots. A tabela informa 23.132 enunciados/77 falantes no train, 3.118/10 no valid e 3.793/10 no test. A divisão não compartilha falantes entre splits.

A fonte declara lançamento para pesquisa acadêmica e licença **Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International**. Isto permite considerar o corpus para uso local de pesquisa sob os termos, mas não autoriza automaticamente publicar áudio, transcrições ou dados derivados no repositório HERUS. O pacote e seu checksum ainda precisam ser obtidos e fixados; nenhum arquivo foi baixado nesta etapa.

Limite: o alinhamento áudio–transcrição–slots/intenção é declarado pela fonte, mas ainda não foi validado localmente por `sample_id`. As intenções externas também não são `ARRIVE`, `HELP` ou `CANCEL`; não existe mapeamento operacional HERUS neste registro.

## Acesso histórico publicado pela própria fonte

URL consultada: https://groups.google.com/a/fluent.ai/g/fluent-speech-commands

O grupo oficial contém uma publicação de 2021 que aponta o download para `http://fluent.ai:2052/` e outra publicação com a licença pública. O endereço histórico foi aberto em 2026-08-27, mas redirecionou para `https://fluent.ai/`, sem expor pacote, CSV, WAV, checksum ou formulário de download. Não foi baixado nenhum arquivo e não foi usado espelho Kaggle/Hugging Face.

A página dos autores contém a licença completa em `https://fluent.ai/wp-content/uploads/2021/04/Fluent_Speech_Commands_Public_License.pdf`; a redação observada declara uso estritamente acadêmico e proíbe uso comercial, inclusive treinamento, teste, benchmark ou desenvolvimento de produto. Esta restrição impede tratar o corpus como dado de produto HERUS; no máximo, permite um experimento de pesquisa local quando o pacote for obtido de forma legítima e o protocolo de uso for respeitado.

Estado da Frente B: **bloqueada por acesso reproduzível não disponível**, sem contornar o bloqueio por espelho não confirmado. Pareamento e licença continuam declarados pela fonte, mas não validados localmente.

## Sonos / Snips — alternativa auditada

URL oficial: https://github.com/sonos/spoken-language-understanding-research-datasets

O README oficial declara que os datasets incluem queries textuais com intent e slots e que as gravações são crowdsourced, com uma fala por query. A licença exibida exige uso apenas acadêmico/de pesquisa, proíbe uso comercial e permite republicação somente se os datasets forem não modificados e sob os mesmos termos, além de exigir citação do paper de Snips. O acesso aos dados requer preencher o formulário https://forms.gle/JtmFYM7xK1SaMfZYA e aguardar concessão.

Estado: **bloqueado por acesso concedido**, sem formulário, login ou download automático. Não foi usado como atalho. A existência de README/licença não equivale a posse do pacote nem permite publicar áudio/transcrições. Se o usuário fornecer os arquivos obtidos legalmente, o runner poderá validá-los localmente sem subir mídia.

## Licença Fluent — texto extraído da fonte oficial

URL: https://fluent.ai/wp-content/uploads/2021/04/Fluent_Speech_Commands_Public_License.pdf

A licença concede reprodução do material, inteiro ou parcial, somente para finalidade não comercial e acadêmica, e explicitamente diz que o material não é autorizado para finalidade comercial, incluindo treinamento, teste, benchmarking, pesquisa industrial ou desenvolvimento de produto. A licença exige atribuição e preservação de avisos/URL; não concede direito de compartilhar o material licenciado nem material adaptado. A licença também prevê que modificações técnicas necessárias para exercer os direitos não criam material adaptado, mas isso não autoriza compartilhar o material.

Decisão do HERUS: o dataset pode ser usado apenas como experimento de pesquisa local se obtido de maneira legítima e sob os termos; não pode ser tratado como corpus de produto, não pode ser colocado no GitHub e qualquer derivado substancial compartilhável está bloqueado. O status de acesso permanece `not_downloaded`.

## SLURP — registro Zenodo encontrado

URL: https://zenodo.org/records/11106554
DOI: https://doi.org/10.5281/zenodo.11106554

O registro é um dataset publicado em 2024-05-02, versão v1, com um arquivo `slurp.zip` de 9,6 GB e MD5 exibido `62f96f9b1f605117e26e2639349869d5`. O criador exibido é Afsara Benazir; o registro declara **Creative Commons Attribution 4.0 International**. O preview mostra diretórios como `audio`, `dataset`, `slurp_real` e `slurp_synth`, mas o conteúdo e a relação exata com o SLURP original ainda precisam ser auditados; o registro não é o repositório original dos autores do paper.

Decisão: não baixar o pacote de 9,6 GB para “superar” o bloqueio por volume. Primeiro verificar a procedência, se o arquivo contém somente material licenciado sob CC BY 4.0, se os IDs/transcrições/intent estão alinhados e se o pacote não inclui arquivos de origem com termos incompatíveis. Se a auditoria permitir, baixar apenas um arquivo/parte mínima reproduzível, conferindo checksum; caso contrário, registrar como espelho não utilizável.

## API Zenodo — metadados confirmados

URL consultada: https://zenodo.org/api/records/11106554

A API confirma registro publicado `11106554`, versão v1, acesso aberto, licença `cc-by-4.0`, criador Afsara Benazir/University of Virginia, e um único arquivo `slurp.zip` com 9.644.648.983 bytes (aproximadamente 9,6 GB), checksum `md5:62f96f9b1f605117e26e2639349869d`. O endpoint de conteúdo é `https://zenodo.org/api/records/11106554/files/slurp.zip/content`.

A API não confirma que cada arquivo interno herdou corretamente CC BY 4.0 nem que o pacote corresponde sem alterações ao SLURP original. O tamanho torna o download integral desnecessário nesta etapa. O pacote continua candidato condicionado a auditoria de conteúdo e proveniência; não foi baixado.

## Probe HTTP Range do ZIP Zenodo

Foi feita uma requisição limitada ao endpoint oficial com `Range` de 4 MiB e depois 16 MiB, sem extração nem execução. O servidor respondeu `206 PARTIAL_CONTENT` e confirmou o tamanho total `9.644.648.983` bytes. O arquivo é ZIP64. O diretório central começa no offset `9.624.755.610`, tem `19.893.275` bytes e termina no offset `9.644.648.885`; portanto, a cauda de 16 MiB não contém o diretório central inteiro (`central_directory_not_fully_in_range`).

O probe confirma somente o transporte parcial, o tipo ZIP64 e os metadados estruturais do diretório central; não confirma os nomes de todos os membros nem a proveniência/licença de cada conteúdo. Não foi baixado o pacote integral. O artefato `zenodo_zip_range_probe.json` registra o resultado `blocked`, `raw_data_downloaded=false`, `extracted=false` e `executed=false`.

## Paper original Fluent Speech Commands

URL: https://arxiv.org/html/1904.03670

O paper original confirma áudio WAV mono a 16 kHz, um comando inglês por arquivo, slots `action`, `object`, `location`, 31 intenções e 248 formulações. Reporta 23.132/3.118/3.793 enunciados em train/valid/test, 77/10/10 falantes sem compartilhamento entre splits e 19,0 horas no total. O texto afirma que os participantes consentiram com a liberação dos dados e que a coleta foi crowdsourced. O paper aponta o caminho histórico `fluent.ai/research/fluent-speech-commands/` para código e dados, mas o endpoint histórico de download observado pelo grupo redirecionou para a página principal.

O paper descreve resultados de modelos neurais treinados no dataset; esses números são contexto do benchmark, não resultados HERUS e não serão usados como validação do pipeline finito.

## Caminho histórico citado pelo paper

URL consultada: https://fluent.ai/research/fluent-speech-commands/

A URL histórica redirecionou para a página atual do artigo da Fluent.ai. Não foi disponibilizado pacote, CSV, WAV ou checksum por essa rota. Resultado: o bloqueio de aquisição do Fluent permanece confirmado por duas rotas oficiais observadas: o grupo aponta para `fluent.ai:2052`, que redireciona para a home, e o caminho do paper também redireciona para a página descritiva.

## Probe ZIP com diretório central completo

Com uma faixa de 20.000.000 bytes, o diretório central completo foi lido sem extração. O ZIP64 contém 141.759 membros. Um filtro por `metadata.json`/`LICENSE` encontrou `slurp/LICENSE.txt`, `slurp/dataset/slurp/metadata.json` e o sidecar macOS `._metadata.json`. A entrada principal tem compressão deflate, 1.472.796 bytes comprimidos, 18.473.896 bytes descomprimidos, CRC32 `1112906406` e offset local `2845081669`.

Foi baixado somente esse membro por Range; o fetch verificou nome, cabeçalho local, deflate, tamanho, CRC32 e SHA-256 `be7943d576c0b79b34ae32796e5c9474bd3d4d123bb35e38f73497f4a429fb83`. O arquivo temporário foi `/tmp/slurp_metadata.json`, não foi executado nem publicado. O restante do archive e todos os áudios permaneceram fora do ambiente local.

## Repositório original SLURP — licença por modalidade

URL: https://github.com/pswietojanski/slurp

O README do repositório dos autores do paper diz que as anotações textuais estão no repositório em `dataset/slurp/`, enquanto o áudio correspondente é obtido por `scripts/download_audio.sh` a partir do Zenodo original e armazenado em FLAC, exigindo cerca de 6 GB. O próprio README separa as licenças: dados textuais distribuídos pelo repositório sob **CC BY 4.0**; dados de áudio armazenados no Zenodo sob **CC BY-NC 4.0**, com possibilidade de licença menos restritiva mediante contato com `info@emotech.co`.

Esta informação impede usar o registro Zenodo `11106554` como se todos os membros fossem CC BY 4.0: o registro de terceiro declara CC BY 4.0, mas o repositório original declara licença mais restritiva para áudio. O archive do terceiro também contém `.git`, arquivos sintéticos e outros membros; portanto, o audio não será baixado. O metadata.json baixado por Range será tratado como cópia de metadados para auditoria local, não como autorização de redistribuição nem como corpus de produto.

O repositório original confirma que o pacote textual é o caminho adequado para recuperar `intent`/`slots`; o membro Zenodo auditado contém apenas metadados de gravação, sentenças normalizadas/originais e IDs, sem campo de intent/slot visível no resumo agregado.

## Auditoria textual SLURP — primeira execução e falha de join detectada

Repositório local temporário: `pswietojanski/slurp` no commit `8eb16545762be97ace75334109d73824217311f1`. Os arquivos JSONL reais têm SHA-256: `train.jsonl` `4a05a3de6c7f028aff298c4a90196e6a3fecd1938d96fb1c125d6fd10d5e301e` (11.514 linhas), `devel.jsonl` `00d346b183ecc2d160853cd3dc20e8ccd207d9e1809cf6e2c05182076bd0ecc9` (2.033 linhas) e `test.jsonl` `fe3449af69b42fda7163345482556066c35a80f7e552a6d67e212a0e2f0783cc` (2.974 linhas). Total: 16.521 linhas, 16.521 `slurp_id` únicos, 93 intents e 18 cenários.

O metadata.json real extraído por Range tem 18.473.896 bytes e SHA-256 `be7943d576c0b79b34ae32796e5c9474bd3d4d123bb35e38f73497f4a429fb83`, com 16.521 entradas e 72.396 gravações `.flac`. A primeira versão do auditor tentou juntar `slurp_id` textual com `nlub_id` do metadata e encontrou somente 4 coincidências; isso não deve ser tratado como alinhamento. A amostra inicial mostra `slurp_id=9024` no JSONL e top-level metadata indexado por `0`, com `nlub_id=0`, indicando que esses campos não são a chave de join.

A correção obrigatória é juntar por `recordings[].file` do JSONL com as chaves `recordings` do metadata, que são os nomes de arquivos de áudio associados à mesma anotação. O relatório provisório com `metadata_joined_rows=4` é marcado como **falha de metodologia**, será sobrescrito após o join correto e não será publicado como resultado final.

## Auditoria textual SLURP — resultado corrigido

O join correto por `recordings[].file` encontrou 16.521/16.521 linhas com metadados correspondentes e 72.396/72.396 referências presentes. Porém, há 1 filename duplicado no metadata e 1 referência de gravação repetida no conjunto textual. O conflito de metadados para o filename duplicado é zero (as assinaturas são idênticas), mas a repetição ainda impede chamar o join de unívoco sem um protocolo de deduplicação independente.

Veredito: **integridade textual e presença de referências confirmadas; alinhamento unívoco bloqueado por duplicatas; conteúdo áudio–transcrição não validado porque nenhum FLAC foi carregado; convergência HERUS não provada**. O relatório agregado final não publica o filename nem o `slurp_id` afetado.

## LICENSE.txt interno do archive SLURP

O membro `slurp/LICENSE.txt` foi extraído por Range a partir do offset `4388`, validado com 216 bytes comprimidos, 583 bytes descomprimidos, CRC32 `19559804` e SHA-256 `97547dd6236684935697b26629fb6e70b3be4ed535c2d85fdc053a6389192849`. O conteúdo confirma: parte textual sob CC BY 4.0; parte de áudio sob CC BY-NC 4.0, com links para os textos legais da Creative Commons.

Isto confirma que o archive contém aviso interno incompatível com tratar o áudio como CC BY 4.0. Nenhum áudio foi extraído. O pacote continua utilizável apenas para auditoria local de metadados/texto sob os termos correspondentes, não para publicação do archive nem para produto HERUS.

## Parser C real sobre texto SLURP

O parser C oficial foi compilado com `-O2 -Wall -Wextra -Werror -std=c11` junto ao runner host-only de JSONL e executado sobre train/devel/test reais. Foram vistas 16.521 linhas; 16.521 foram enviadas ao C; falhas de extração JSONL: 0; linhas longas: 0; texto vazio: 0; bytes não ASCII: 0; `DRAFT=0`; `CANCEL_LOCAL=0`; `UNKNOWN=16.426`; `REJECTED=95`; autoridade de comando HERUS: 0; mapeamento automático: 0.

O resultado é cobertura/rejeição do parser português sobre sentenças inglesas de domínio externo. Não é acurácia de SLU, não é convergência áudio–texto e não é validação de comando HERUS. O artefato agregado é `slurp_voice_parser_audit.json`; nenhuma sentença ou ID individual foi escrito no log.

## Alvos Makefile SLURP

`make -C research slurp-text-audit SLURP_ROOT=/tmp/slurp-original SLURP_METADATA=/tmp/slurp_metadata.json` passou. O alvo recompila o parser C real, executa as três partes JSONL, atualiza `slurp_voice_parser_audit.json` e executa `analyze_slurp_text.py`.

`make -C research slurp-text-sanitizers SLURP_ROOT=/tmp/slurp-original` também passou. O binário C foi compilado com ASan/UBSan e processou as 16.521 sentenças sem erro; a chamada adicional com stdin vazio confirmou o caminho de encerramento. Os resultados permanecem `DRAFT=0`, `CANCEL_LOCAL=0`, `UNKNOWN=16.426`, `REJECTED=95`, sem autoridade de comando.

## Manifesto e testes após inclusão do SLURP

O manifesto agora registra SLURP como `PAIRED_DECLARED_BY_SOURCE`, com texto CC BY 4.0, áudio CC BY-NC 4.0, `source_commit=8eb16545762be97ace75334109d73824217311f1`, metadata baixado por Range e áudio não baixado. O terceiro registro Zenodo permanece separado como candidato bloqueado por discrepância de proveniência/licença e volume.

A suíte `make -C research test` passou com **58 testes**. O alvo exige `mapping_count=0` para todas as fontes e mantém a política de não pareamento sintético, não publicação de raw data e não uso de espelho sem lineage.

## Ataque de limite do runner C

O runner foi compilado com `-Wall -Wextra -Werror`. Uma linha sem newline de 4095 bytes produziu `parser_input_rows=1`, `parser_overlong=0` e não causou falha de memória; uma linha de 4096 bytes produziu `parser_overlong=1`, `parser_rejected=1` e foi descartada. O teste confirma a distinção entre EOF legítimo e truncamento real no limite do buffer.

## Sanitizadores após correção do Makefile

O alvo `make -C research slurp-text-sanitizers SLURP_ROOT=/tmp/slurp-original` passou com ASan/UBSan ativo no subprocesso C que recebeu todas as 16.521 sentenças. A saída foi idêntica à execução não sanitizada: `DRAFT=0`, `CANCEL_LOCAL=0`, `UNKNOWN=16.426`, `REJECTED=95`, `parser_overlong=0`, `parser_empty=0`. Não houve diagnóstico de ASan/UBSan.

## Falha de proveniência autorreferente

A primeira bateria global falhou em `prove.sh` porque o digest da árvore `research/evidence/wide_cycle_05` ficou desatualizado após novos logs. Ao tentar registrar o próprio `provenance_final.txt` nessa árvore protegida, o digest passou a ser autorreferente e o gate falhou novamente. A correção foi remover a árvore mutável de evidências do conjunto `protected_inputs`, mantendo no manifesto somente arquivos estáveis e explicitamente hasheados. Os logs continuam versionados como evidência, mas não são usados como input cujo digest valida a própria execução.

Após a correção: manifesto estrito **válido** (`1 active, 3 pending`); `prove.sh --quiet` **passou** com `ALL INVARIANTS HOLD — host contracts pass; controlled bench flash may begin, physical gates remain pending.`
