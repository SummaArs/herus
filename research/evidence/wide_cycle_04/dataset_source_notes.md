# Fontes de dados reais — notas brutas de consulta

## MIntRec — fonte oficial

URL: https://github.com/thuiar/MIntRec

A página oficial descreve o MIntRec como um dataset de reconhecimento de intenção multimodal em cenários conversacionais reais. A fonte bruta é a série de TV Superstore. O conjunto possui texto, áudio e vídeo por segmento, com 2.224 segmentos, 43 vídeos, 20 classes finas de intenção, 15.658 palavras e 2.562 palavras únicas. Cinco anotadores combinam texto, vídeo e áudio e salvam amostras com pelo menos três votos. Os segmentos têm duração média de 2,38 s e máxima de 9,59 s.

A distribuição indicada pelo README inclui `train.tsv`, `dev.tsv` e `test.tsv` com índices de segmento, texto limpo e anotações multimodais; `raw_data` com segmentos `.mp4`; `audio_data/audio_feats.pkl`; `video_data/video_feats.pkl`; e anotações de speaker. O repositório informa links de download por Google Drive e BaiduYun. O código do repositório está sob MIT, mas os termos específicos de redistribuição dos vídeos/áudios precisam ser verificados antes de colocar os arquivos no HERUS.

Limite para o HERUS: MIntRec oferece modalidades alinhadas e intenção anotada, mas suas 20 classes não são o vocabulário operacional finito de `ARRIVE`, `HELP` e `CANCEL`. Portanto, pode testar alinhamento multimodal, consistência de representação e rejeição, mas não deve ser remapeado automaticamente para comandos HERUS. Um mapeamento teria de ser tratado como tarefa nova, com anotação independente e risco de falso consenso.

O README afirma que os recursos textuais foram extraídos com BERT, visuais com Faster R-CNN/ResNet-50 e áudio com wav2vec 2.0. Para o HERUS, esses recursos pré-computados são evidência de entrada/benchmark, não prova de que uma camada neural entendeu a Semantic IR.

## OpenReview — MIntRec2.0

URL solicitada: https://openreview.net/forum?id=nY9nITZQjc

A página redirecionou para uma verificação de navegador e não permitiu leitura sem interação. Nenhuma alegação sobre MIntRec2.0 deve ser usada como fato verificado apenas pelo snippet do buscador. Se for necessário usar essa fonte, buscar o paper ou repositório oficial alternativo e registrar a confirmação.

## Decisão provisória

MIntRec é candidato real para a frente de alinhamento multimodal texto–áudio–vídeo, mas não deve ser chamado de banco do domínio HERUS. A primeira execução deve baixar apenas os metadados/TSV e comparar o alinhamento de IDs e splits. Não versionar dados crus antes de verificar licença e hashes. Dados de sensores ainda exigem uma fonte separada; MIntRec não cobre sensores vestíveis.

## Common Voice — fontes oficiais

URLs consultadas:

- https://commonvoice.mozilla.org/en/datasets
- https://www.mozillafoundation.org/en/blog/common-voice-18-dataset-release/
- https://github.com/common-voice/cv-dataset

A página oficial de metadados `cv-dataset` lista, no estado consultado, Scripted Speech v26.0 com 294 idiomas e Spontaneous Speech v4.0 com 78 idiomas. A página de datasets lista `Common Voice Scripted Speech 26.0 - Portuguese`, locale `pt`, tarefa ASR, formato MP3, tamanho indicado de 4,83 GB. A página de release da Mozilla informa que o Common Voice 18 foi disponibilizado com clips de voz e texto associado sob CC0; isso deve ser conferido para a versão efetivamente baixada e para o pacote específico antes de redistribuir artefatos.

O repositório oficial informa acesso atual pelo Mozilla Data Collective, por navegador, API ou SDK Python. A base de fala é adequada para medir cobertura e robustez intramodal do parser/ASR: transcrição de referência versus transformação canônica, ruído, acento, erro de transcrição e rejeição fora do vocabulário. Ela não é uma base de intenção HERUS e não contém, por si só, botão ou sensor sincronizado. Portanto, Common Voice não prova convergência multimodal; prova apenas que o frontend vocal pode ser avaliado em dados de fala reais, condicionado a escolher ou anotar um subconjunto de frases compatível com o domínio.

Nenhum áudio foi baixado nesta etapa. A seleção de uma versão e o download devem ocorrer somente após fixar locale, snapshot, licença aplicável, tamanho esperado e checksum. Não versionar o pacote bruto no GitHub.

## WESAD — fontes UCI e autores

URLs consultadas:

- https://archive.ics.uci.edu/dataset/465/wesad+wearable+stress+and+affect+detection
- https://ubi29.informatik.uni-siegen.de/usi/data_wesad.html
- DOI: https://doi.org/10.24432/C57K5T

O registro UCI descreve 15 sujeitos em estudo de laboratório, com dados fisiológicos e de movimento de dispositivos no pulso e no peito. As modalidades incluem BVP, ECG, EDA, EMG, respiração, temperatura corporal e aceleração triaxial. O UCI informa 63.000.000 de instâncias e série temporal multivariada. Os sensores do RespiBAN são amostrados a 700 Hz; o Empatica E4 fornece BVP a 64 Hz, EDA e temperatura a 4 Hz e aceleração a 32 Hz. O dataset cobre estados neutro, estresse e amusement, além de autorrelatos.

A página original dos autores informa download de um arquivo de 2,5 GB com gravações fisiológicas originais do peito e pulso e autorrelatos. O disclaimer permite uso científico não comercial com atribuição aos proprietários. O registro UCI não mostra uma licença ampla própria; portanto, não publicar dados brutos no GitHub e tratar WESAD como fonte de preparação local para experimento científico não comercial.

Limite para o HERUS: WESAD é multimodal entre sensores fisiológicos/movimento e contém sincronização interna entre dispositivos, mas não tem texto, fala ou botão como entrada humana do HERUS. Pode testar estabilidade temporal, qualidade, janela, incerteza e cartão de contexto derivado de sensores. Não pode provar convergência voz–sensor ou texto–sensor. Qualquer ligação com `ARRIVE`, `HELP` ou `CANCEL` seria uma tarefa de rotulagem nova e não deve ser inventada.

A página dos autores declara resultados de benchmark de até 80% em três classes e até 93% em stress versus non-stress. Esses números não são resultados do HERUS e não devem ser reutilizados como validação de Semantic IR.

## MIntRec — armazenamento oficial observado

URL: https://drive.google.com/drive/folders/18iLqmUYDDOwIiiRbgwLpzw76BD62PK0p

A pasta pública exibiu os diretórios `audio_data`, `raw_data`, `speaker_annotations_data` e `video_data`, além de `train.tsv` (~74 KB), `dev.tsv` (~25 KB), `test.tsv` (~26 KB) e um arquivo `MIA-datasets.tar.gz` (~828,2 MB). Os TSVs são candidatos para download imediato sem baixar vídeos/áudios. A interface mostrou os arquivos como compartilhados e com ação Download; a disponibilidade real deve ser confirmada por checksum após download.

O material multimodal pesado não deve ser baixado apenas para inflar o corpus. A primeira preparação deve usar os TSVs para verificar IDs, textos, classes e splits; só depois selecionar uma amostra de segmentos se os termos e a licença permitirem. O repositório original informa que os segmentos correspondem a texto, áudio e vídeo, mas o HERUS ainda não possui mapeamento operacional das 20 classes MIntRec para seus três eventos finitos.

## MIntRec — confirmação visual da pasta pública

A pasta pública foi aberta no navegador e exibiu oito entradas: `audio_data`, `raw_data`, `speaker_annotations_data`, `video_data`, `dev.tsv` (~25 KB), `MIA-datasets.tar.gz` (~828,2 MB), `test.tsv` (~26 KB) e `train.tsv` (~74 KB). A interface mostra ação de download para os arquivos. O HTML foi salvo localmente durante a consulta em `/home/ubuntu/browser_html/drive_google_com_18iLqmUYDDOwIiiRbgwLpzw76BD62PK0p_1787825511159.html`.

A interface não expôs no texto os IDs individuais dos arquivos. A preparação deve usar a ação de download da interface ou extrair os IDs do HTML com parser, sem adivinhar URLs. O pacote de 828,2 MB não será baixado antes de avaliar se os TSVs bastam para a primeira medição.

## MIntRec — seleção de arquivo

A pasta permaneceu pública e exibiu `train.tsv`, `dev.tsv` e `test.tsv` com tamanhos aproximados de 74 KB, 25 KB e 26 KB. A tentativa de selecionar a linha pela lista genérica não revelou um link individual no texto extraído; não foi iniciado download do pacote pesado. O resultado suficiente para o planejamento é: os TSVs são pequenos e acessíveis pela interface pública, enquanto os dados de áudio/vídeo permanecem separados e devem ser tratados segundo licença e checksum.


## Fluent Speech Commands — fonte adicional auditada em 2026-08-27

Fonte dos autores: https://fluent.ai/fluent-speech-commands-a-dataset-for-spoken-language-understanding-research/

A página oficial informa 30.043 enunciados de 97 falantes, arquivos WAV mono de 16 kHz, uma transcrição por arquivo, três slots (`action`, `object`, `location`) e 31 intenções derivadas das combinações dos slots. O CSV de cada split lista speaker ID, caminho do áudio, transcrição e slots; os splits são train/valid/test e não compartilham falantes. A divisão indicada é 23.132/3.118/3.793 enunciados.

Licença declarada pela fonte: **Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International**; a página diz que o lançamento é somente para pesquisa acadêmica. Portanto, o dataset pode ser candidato a um teste local de alinhamento áudio–texto–intenção, mas arquivos de áudio/transcrições não serão republicados neste repositório sem revisão dos termos. A fonte não fornece mapeamento anotado para `ARRIVE`, `HELP` ou `CANCEL`; qualquer teste HERUS deve manter a intenção externa como benchmark separado e não convertê-la automaticamente em comando.

Resultado de escopo: `aligned_modalities=declared_by_source`; `herus_command_mapping=0`; `herus_convergence_proven=false` até que uma amostra seja obtida de forma permitida, o mesmo ID seja verificado em áudio e CSV, e um protocolo de mapeamento explícito seja aprovado.
