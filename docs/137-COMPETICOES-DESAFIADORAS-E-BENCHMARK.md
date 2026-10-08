# Competições desafiadoras e benchmark do HERUS

**Editor científico:** HERUS  
**Data:** 2026-10-07  
**Escopo:** seleção crítica de cinco competições históricas fornecidas para avaliar a utilidade experimental de um sistema HERUS.  
**Conclusão curta:** a lista abaixo é uma seleção por valor experimental e dificuldade operacional observável, **não** uma afirmação de que sejam objetivamente as cinco competições mais difíceis da história.

## 1. Como a seleção foi feita

As cinco fichas foram ordenadas por uma combinação explícita de seis critérios:

1. **Complexidade da modalidade:** quantidade e heterogeneidade dos tipos de entrada/saída, necessidade de representação especializada e distância entre o contrato original e o objeto atual do HERUS.
2. **Escala:** volume de exemplos, cardinalidade, número de classes/entidades, tamanho dos arquivos e esparsidade.
3. **Risco de leakage:** exposição a informação futura, identidade de usuário/entidade, dados suplementares, tuning no leaderboard ou regras temporais estritas.
4. **Custo computacional:** memória, armazenamento, GPU/CPU, tempo de treinamento, inferência e infraestrutura de submissão.
5. **Disponibilidade/reprodutibilidade:** acesso legal, hashes e versões, labels de teste, scorer, servidor histórico e possibilidade de executar um protocolo offline.
6. **Distância para o HERUS atual:** incompatibilidade com o núcleo atual, que é mais próximo de um **gate de memória episódica auditável com proposta e abstenção** em contexto estruturado do que de um preditor multimodal, regressor, sistema de recomendação ou encoder visual.

A escala da tabela é ordinal, de **1 (baixo/favorável)** a **5 (alto/desfavorável)**, exceto disponibilidade, em que **5 significa melhor disponibilidade/reprodutibilidade relativa**. A pontuação não é uma métrica universal de dificuldade; ela serve apenas para tornar a ordenação auditável.

### 1.1 Matriz comparativa

| Ordem | Desafio | Modalidade e escala | Complexidade modal (1–5) | Escala (1–5) | Risco de leakage (1–5) | Custo computacional (1–5) | Disponibilidade (1–5) | Distância do HERUS (1–5) | Enquadramento recomendado |
|---:|---|---|---:|---:|---:|---:|---:|---:|---|
| 1 | **ILSVRC** | Imagem RGB; 1.281.167 treino, 50.000 validação, 100.000 teste, 1.000 classes na classificação; localização/detecção adicionais | 5 | 5 | 4 | 5 | 2 | 5 | **Desafio de referência**; adaptação visual futura, não alvo direto |
| 2 | **Google Landmark Recognition 2019 / GLDv2** | Imagem RGB; 4.132.914 treino, 761.757 índice, 117.577 teste, mais de 200 mil labels de instância; forte OOD | 5 | 5 | 4 | 5 | 3 | 5 | **Desafio de referência**; adaptação de retrieval/reconhecimento somente com ponte visual |
| 3 | **Kaggle Avito Demand Prediction** | Tabular + texto + imagens + séries de anúncios/tempo; 146,76 GB e cerca de 1,4–1,5 milhão de exemplos | 5 | 5 | 5 | 5 | 2 | 4 | **Adaptação válida condicional**, nunca execução direta do HERUS-core |
| 4 | **Netflix Prize** | Rating esparso user–movie–date; 100.480.507 ratings, 480.189 usuários, 17.770 filmes | 3 | 5 | 4 | 5 | 1 | 4 | **Adaptação válida condicional** para filtro colaborativo; referência de especialização |
| 5 | **Kaggle Two Sigma Financial Modeling** | Tabular temporal anonimizado, API online, missing values e alvo numérico por timestamp/id; dimensão completa não confirmada | 2 | 2 | 5 | 3 | 1 | 2 | **Adaptação válida mais próxima**, mas não alvo direto do contrato atual |

### 1.2 Leitura correta da ordem

A ordem privilegia o quanto cada competição estressa uma hipótese experimental útil: escala, representação, causalidade temporal, risco de vazamento e necessidade de adaptação. Ela não transforma uma pontuação ordinal em um ranking histórico de dificuldade. Participação, número de GPUs, tamanho de dataset, diferença entre vencedores e baselines e quantidade de submissões são **evidências de pressão competitiva**, não uma escala comum que permita afirmar que ILSVRC é “mais difícil” que GLDv2 ou que Netflix.

Dois cuidados são centrais:

- **Nenhuma das cinco é alvo direto do HERUS-core atual.** O núcleo atual não é um modelo de classificação visual, regressão financeira, collaborative filtering ou previsão multimodal de probabilidade. Ele deve ser descrito como `HERUS-core`, adaptador, ablação ou controle, e não como uma competência única.
- “Adaptação válida” significa que há uma ponte experimental genérica e pré-especificada, com ablação do núcleo e do adaptador. Não significa que um score obtido depois de inserir ArcFace, CNN, matrix factorization, lags financeiros ou um ensemble específico seja uma demonstração de simbiose geral.

## 2. Seleção detalhada

## 2.1 ILSVRC — maior estresse de percepção visual supervisionada

**Posição:** 1.  
**Tarefa original:** classificação de imagem, localização ou detecção, dependendo da edição; a edição ILSVRC2012 também teve classificação fina de cães.  
**Métrica:** classificação por **top-5 error**; localização por top-5 com classe correta e IoU superior a 0,5; detecção por AP/mAP com matching de caixas e IoU.  
**Enquadramento HERUS:** **desafio de referência**. Uma avaliação futura só seria uma adaptação válida após criar um encoder visual e uma saída compatível; não é execução direta.

### Por que está no topo

ILSVRC combina grande escala, mil classes, objetos pequenos, clutter, localização e teste oculto. Para ILSVRC2012–2014, o paper reporta 1.281.167 imagens de treino, 50.000 de validação e 100.000 de teste em 1.000 classes. Na localização de 2012–2014, 523.966 imagens de treino tinham caixas, totalizando 593.173 caixas; a detecção de 2014 usava 200 classes, 456.567 imagens de treino e 478.807 caixas de treino.

A dificuldade observável não é apenas o tamanho: há objetos minúsculos — por exemplo, cerca de 1,3% da área média para óculos, 1,5% para bolas de pingue-pongue e 2,0% para bolas de basquete — e muito clutter. A análise histórica reporta que a chance de uma caixa aleatória atingir IoU ≥ 0,5 era inferior a 0,2% em algumas categorias; ping-pong ball, basketball e puck exigiam muitas janelas de objectness. O baseline AlexNet citado no paper tinha aproximadamente 60 milhões de parâmetros, foi treinado por cinco a seis dias em duas GPUs GTX 580 de 3 GB e alcançou 15,3% de erro top-5 em 2012, contra 26,2% do segundo colocado.

Esses números justificam **custo, escala e especialização**, não uma afirmação de que ILSVRC seja o benchmark mais difícil do mundo. O próprio histórico registra ambiguidades de anotação: classificação rotula uma classe por imagem, e parte das imagens de localização foi descartada por ambiguidade de múltiplas instâncias.

### O que seria uma adaptação honesta

Uma ponte mínima teria de definir:

- um encoder visual fixo e versionado, treinado somente nos dados permitidos;
- uma interface `imagem -> representação -> decisão/abstenção`;
- se a saída é classe, classe+caixa ou detecção com confiança;
- um gate HERUS que possa abster-se, sem receber a label verdadeira nem informação futura;
- baselines visuais e uma ablação sem o gate/memória;
- custo de treino, memória, latência e política para dados externos.

O resultado deveria responder se a camada adaptativa melhora risco–cobertura, calibração, robustez ou custo sob o mesmo protocolo. Não deveria ser apresentado como “HERUS venceu ILSVRC” sem replicação do split, métrica, release e servidor.

### Disponibilidade e limitações

Classificação e localização têm especificações públicas e validação local possível, mas o resultado oficial de teste depende de labels ocultos e do servidor. A página atual também informa atualização do conjunto de teste em 2019. Detecção exige fixar edição, 200 classes e subconjunto de treino anotado. Imagens e subconjuntos do ImageNet dependem de login/termos de uso; copyright e responsabilidade de cada imagem devem ser verificados.

**Executável agora:** somente a preparação de manifesto, auditoria, baselines em um snapshot autorizado e um protótipo visual separado.  
**Não executável agora:** benchmark direto do HERUS-core ou score oficial de teste sem encoder visual, dados autorizados e release congelado.

**Fontes principais:**

- [Hub oficial ILSVRC](https://www.image-net.org/challenges/LSVRC/)
- [ILSVRC 2012](https://www.image-net.org/challenges/LSVRC/2012/)
- [Resultados ILSVRC 2012](https://www.image-net.org/challenges/LSVRC/2012/results.html)
- [Download e termos](https://www.image-net.org/download.php)
- [Evaluation server](https://www.image-net.org/challenges/LSVRC/eval_server.php)
- [Paper do desafio, versão HTML](https://arxiv.org/html/1409.0575v3)
- [AlexNet, NeurIPS](https://proceedings.neurips.cc/paper/4824-imagenet-classification-with-deep-convolutional-neural-networks.pdf)

## 2.2 Google Landmark Recognition 2019 / GLDv2 — escala, long tail e rejeição OOD

**Posição:** 2.  
**Tarefa original:** reconhecer, a partir de uma imagem, no máximo um `landmark_id` com confiança; é possível devolver previsão vazia.  
**Métrica:** **Global Average Precision at k=1 (GAP@1/µAP)**, ordenando globalmente as previsões por confiança; consultas OOD sem label esperada penalizam falsos positivos.  
**Enquadramento HERUS:** **desafio de referência** neste momento; pode tornar-se adaptação válida somente com ponte visual/retrieval explicitamente congelada.

### Por que é experimentalmente valioso

O desafio usado contém 4.132.914 imagens de treino, 761.757 de índice e 117.577 de teste, com mais de 200 mil labels de instância. Os arquivos são distribuídos em 500 TARs de treino, 100 de índice e 20 de teste, com CSVs, manifests, MD5 e `recognition_solution_v2.1.csv`. O paper arredonda para cerca de 4,1 milhões de imagens e 203 mil landmarks de treino.

A distribuição long-tail é severa: aproximadamente 57% das classes têm no máximo dez imagens e 38% no máximo cinco. Há forte variação intraclasse — vistas, detalhes, ambientes internos e externos — e ruído das fontes públicas. O componente mais crítico para um gate com abstenção é a composição open-set: cerca de 1,1% das consultas seriam landmarks do treino visíveis e 98,9% seriam out-of-domain sem resultado esperado. A métrica penaliza positivamente prever onde se deveria rejeitar.

O paper fornece pontos de referência: ResNet101+ArcFace em `GLDv2-train-clean` reporta 27,34% µAP no split de teste e 26,40% no split de validação; DELG com verificação espacial reporta 56,35% e 55,01%, respectivamente. São baselines/resultados de benchmark, não SOTA atual nem ranking universal de dificuldade.

### Relação com o HERUS

GLDv2 é interessante para testar **abstenção, calibração e OOD**, mas não a memória episódica em seu contrato atual. Uma adaptação honesta precisaria:

- receber RGB e produzir uma única hipótese ou `ABSTAIN`;
- separar representação visual, recuperação e gate;
- limitar a memória ao índice permitido e não olhar o ground truth do teste;
- medir GAP@1 junto com cobertura, falsos positivos OOD e qualidade da rejeição;
- comparar sem memória, memória/retrieval trivial, encoder especializado e HERUS-gate;
- manter uma política única de adaptação que também possa ser testada em outro domínio.

Codificar priors de landmarks, ajustar calibração especificamente ao long tail ou treinar ArcFace/DELG especializado pode produzir uma boa solução para GLDv2, mas isso não demonstra generalidade simbiótica.

### Disponibilidade e limitações

O CVDF fornece shards, metadados, checksums e ground truth v2.1, o que permite um benchmark offline versionado. Contudo, o estágio 1 histórico dependia de URLs Kaggle que podem ter desaparecido ou mudado; o estágio 2 usava conjunto separado; e o leaderboard privado não é uma validação local independente. As regras impõem restrições de uso, e a licença de cada imagem deve ser auditada. O desafio não mede generalização para classes não vistas: treino e índice compartilham amplamente o espaço de labels. O protocolo de reconhecimento aceita no máximo uma label mesmo quando uma consulta pode conter mais de um landmark.

**Executável agora:** somente um protocolo offline sobre snapshot autorizado, com hashes, e um adaptador visual novo.  
**Não executável agora:** HERUS-core diretamente, sem encoder RGB/retrieval e sem snapshot autorizado.

**Fontes principais:**

- [Overview Kaggle](https://www.kaggle.com/competitions/landmark-recognition-2019/overview)
- [Dados Kaggle](https://www.kaggle.com/competitions/landmark-recognition-2019/data)
- [Regras Kaggle](https://www.kaggle.com/competitions/landmark-recognition-2019/rules)
- [Repositório CVDF](https://github.com/cvdfoundation/google-landmark)
- [Anúncio Google Landmarks v2](https://research.google/blog/announcing-google-landmarks-v2-an-improved-dataset-for-landmark-recognition-retrieval/)
- [Paper CVPR 2020](https://openaccess.thecvf.com/content_CVPR_2020/papers/Weyand_Google_Landmarks_Dataset_v2_-_A_Large-Scale_Benchmark_for_Instance-Level_CVPR_2020_paper.pdf)
- [Dataset e métrica DELF](https://github.com/tensorflow/models/tree/master/research/delf/delf/python/google_landmarks_dataset)

## 2.3 Kaggle Avito Demand Prediction — heterogeneidade e risco máximo de vazamento

**Posição:** 3.  
**Tarefa original:** prever `deal_probability` entre 0 e 1 para cada anúncio do teste.  
**Métrica:** RMSE, menor é melhor; o leaderboard privado usou aproximadamente 69% do teste.  
**Enquadramento HERUS:** **adaptação válida condicional**, não alvo direto.

### Por que é um teste difícil

A página de dados lista 14 arquivos e 146,76 GB, com 75 colunas no Data Explorer. A tarefa junta texto, imagens, categorias de alta cardinalidade, preço, região/cidade, datas, `user_id`, `item_id`, anúncios ativos no mesmo período (`train_active`/`test_active`) e intervalos de exibição/renovação (`periods_train`/`periods_test`). O alvo é uma probabilidade operacional imperfeita: a descrição informa que `deal_probability` pode ser qualquer float entre 0 e 1 porque nem toda transação pode ser verificada.

Há pressão concreta de infraestrutura: discussão oficial de participante relata cerca de 1,5 milhão de imagens e aproximadamente 20 GB para armazenar imagens 64×64×3 em `uint8`; esse número é evidência operacional, não especificação formal. Um artigo IEEE relata cerca de 1,4 milhão de exemplos. A competição recebeu 8.293 entrantes, 2.313 participantes, 1.868 equipes e 43.509 submissões, o que demonstra competição real, mas não um ranking absoluto de dificuldade.

O maior risco científico é leakage. `user_id`, anúncios semelhantes, anúncios ativos, períodos de renovação e agregações temporais podem transportar informação do futuro ou do mesmo anúncio para a validação. Um score aleatório ou uma agregação sem corte temporal não é comparável ao objetivo real.

### Adaptação HERUS recomendada

O protocolo deve separar três objetos:

1. **HERUS-core/gate:** memória episódica, proposta e abstenção, sem fingir que prevê o alvo se o rótulo já entrou na entrada.
2. **Adaptadores de modalidade:** tabular, texto e imagem, cada um com versão e ablação.
3. **Modelo preditivo:** regressão/ensemble que recebe representações sem leakage.

Rodar pelo menos três variantes: tabular-only; tabular+texto; multimodal com imagem. Comparar contra média/global, regressão regularizada, árvore/ensemble e uma fusão neural simples. A solução especializada histórica incluiria embeddings de texto, CNN/embeddings de imagem, agregados leakage-safe e ensembles calibrados; isso deve ser rotulado como `Avito-specialized`, não como HERUS genérico.

### Disponibilidade e limitações

O acesso exige login/registro, aceitação das regras e autorização para usar os arquivos; os dados são “Subject to Competition Rules”. Dados externos só eram permitidos com direito de uso e publicação da fonte no fórum. O teste rotulado e o split privado não são públicos. Não há baseline oficial normativo claramente publicado na ficha; scores de participantes não substituem um baseline controlado.

**Executável agora:** auditoria do contrato, desenho dos splits, detector de leakage e adaptador tabular/textual somente após obter os dados por canal legítimo.  
**Não executável agora:** execução multimodal reproduzível sem os 146,76 GB, licenças, cache de imagens e pipeline distribuído.

**Fontes principais:**

- [Página oficial da competição](https://www.kaggle.com/competitions/avito-demand-prediction)
- [Dados](https://www.kaggle.com/competitions/avito-demand-prediction/data)
- [Regras](https://www.kaggle.com/competitions/avito-demand-prediction/rules)
- [Leaderboard](https://www.kaggle.com/competitions/avito-demand-prediction/leaderboard)
- [Discussão oficial sobre imagens/infraestrutura](https://www.kaggle.com/c/avito-demand-prediction/discussion/59917)
- [Artigo IEEE](https://doi.org/10.1109/ICCCNT45670.2019.8944783)
- [Registro IEEE](https://ieeexplore.ieee.org/abstract/document/8944783/)

## 2.4 Netflix Prize — escala esparsa e ganho marginal de engenharia especializada

**Posição:** 4.  
**Tarefa original:** prever ratings explícitos 1–5 para pares cliente–filme e data no qualifying set.  
**Métrica:** RMSE; Cinematch tinha 0,9525 no test, e o limiar do Grand Prize era 0,8572, isto é, 90% de 0,9525. O quiz era público; o test permanecia oculto.  
**Enquadramento HERUS:** **adaptação válida condicional** para um adaptador de collaborative filtering; também é uma forte referência de engenharia especializada, não um alvo direto.

### Escala e dificuldade observável

O treino contém exatamente 100.480.507 ratings de 480.189 usuários e 17.770 filmes; cerca de 99% da matriz potencial está ausente. Há datas, IDs anônimos, ratings de 1 a 5, título e ano. O probe tem 1.408.395 ratings rotulados; o relatório da UCSD descreve 2.817.131 pares no quiz/qualifying, enquanto as regras usam “mais de 2,8 milhões”. O holdout privilegiava ratings mais recentes, e usuários leves eram particularmente difíceis.

A competição foi apertada: mais de 20.000 equipes de 152 países foram registradas; em um momento havia mais de 2.000 equipes com submissões e mais de 13.000 submissões. O blog da Netflix fala em mais de 2.000 horas de trabalho e uma combinação de 107 algoritmos para o primeiro Progress Prize. Após quase 33 meses, a solução combinada cruzou 10%; o paper reporta 0,856704 para o vencedor e 0,856714 para The Ensemble, diferença de 0,000010, com desempate por 20 minutos.

A solução vencedora combinava baseline, matrix factorization, dinâmica temporal em modelos de vizinhança, RBM e blending/stacking por GBDT com até 454 preditores. Isso mostra o custo de extrair ganhos marginais em collaborative filtering; não prova que tal ensemble seja representativo do HERUS nem que o benchmark seja universalmente o mais difícil.

### Adaptação HERUS recomendada

Uma adaptação válida teria de transformar registros esparsos em uma interface explícita `user_id, movie_id, timestamp, histórico -> rating`, sem colocar a resposta futura na entrada. O protocolo mínimo incluiria:

- baseline global, média por usuário/filme e bias temporal;
- matrix factorization como baseline especializado;
- HERUS fixo versus HERUS com mecanismo adaptativo, mantendo o mesmo orçamento;
- split temporal, validação no probe e holdout próprio intocado;
- RMSE, cobertura/abstenção se a saída puder ser incompleta, e custo de memória/tempo;
- análise separada para usuários leves, filmes raros e deriva temporal.

Não se deve comparar diretamente “HERUS” com BellKor/Pragmatic Chaos como se fossem a mesma classe de método. Um resultado alto pode significar uma boa solução de collaborative filtering, não capacidade simbiótica geral.

### Disponibilidade e limitações

A reprodução local é parcial. É necessário obter legalmente uma cópia exata dos arquivos históricos, registrar hash, versão e autorização. O test rotulado nunca foi divulgado, o scorer/leaderboard histórico está indisponível e o ranking final não pode ser verificado de forma independente. Cópias em mirrors não oficiais exigem auditoria jurídica e de integridade. A matriz de aproximadamente 8,5 bilhões de pares potenciais e 100 milhões de ratings exige processamento e armazenamento substanciais. Os dados foram anonimizados e perturbados; resultado histórico no proxy não é resultado em produção.

Há também um mismatch de objetivo: RMSE de rating explícito não mede ranking top-N, CTR, retenção, cold start, causalidade ou utilidade de negócio.

**Executável agora:** protocolo, parser, hashes e um experimento em split controlado se os arquivos forem obtidos legalmente; não o ranking oficial.  
**Não executável agora:** avaliação direta do test histórico e reprodução da tabela final sem labels/scorer.

**Fontes principais:**

- [Regras Netflix Prize](https://www.netflixprize.com/rules.html)
- [Descrição técnica UCIC/KDD](https://www.cs.uic.edu/~liub/KDD-cup-2007/NetflixPrize-description.pdf)
- [Blog técnico da Netflix](http://techblog.netflix.com/2012/04/netflix-recommendations-beyond-5-stars.html)
- [Paper BellKor](https://www2.seas.gwu.edu/~simhaweb/champalg/cf/papers/KorenBellKor2009.pdf)
- [Paper dos organizadores](https://chrisvolinsky.com/files/publications/all-together-now-netflix-prize.pdf)
- [Workshop UCSD](https://cseweb.ucsd.edu/~elkan/KddNetflixWorkshop.pdf)
- [Relato contemporâneo CNN](https://www.cnn.com/2009/TECH/09/02/netflix.prize/)

## 2.5 Kaggle Two Sigma Financial Modeling — adaptação online e causalidade temporal

**Posição:** 5.  
**Tarefa original:** prever, em ordem temporal, `y` por combinação de `timestamp` e `id` usando apenas features fornecidas; o código era executado em uma API que revelava os dados progressivamente.  
**Métrica:** `R = 1 - sum((y_pred-y)^2) / sum((y-mean(y))^2)`, com valores negativos truncados em -1 no score exibido; a API calcula reward por timestamp/dia e o ranking usa conjunto privado.  
**Enquadramento HERUS:** **adaptação válida mais próxima**, mas não alvo direto do contrato atual.

### Por que é a ponte mais próxima

Entre os cinco, Two Sigma se aproxima mais de uma hipótese de adaptação: há estado por timestamp/id, dados temporais, missing values, inferência causal e feedback/reward já liberado. Porém, o objetivo original é regressão financeira sob uma API, não proposta de ação com abstenção nem aprendizagem de episódios. A feature engineering dos vencedores — lags, agregados de mercado, seleção por regime/volatilidade e ensembles Ridge/Extra Trees — é precisamente o tipo de especialização que precisa ser separado da contribuição do HERUS.

A competição foi a primeira Code Competition do Kaggle e impunha execução em container: até duas submissões ou cinco com erro por dia, 20 minutos/8 GB no modo Run e 60 minutos/16 GB no modo Submit. O leaderboard privado usava aproximadamente 63% do teste. O post oficial relata instabilidade de reinforcement learning e mudança forte do leaderboard, evidência de não estacionariedade e risco de overfitting temporal, não prova de dificuldade absoluta.

### Adaptação HERUS recomendada

A comparação mais informativa seria:

- **HERUS estático/offline:** estado e modelo congelados após treino;
- **HERUS adaptativo/online:** atualização ou seleção de especialistas somente com observações e rewards passados;
- zero/constante, média histórica, regressão regularizada e árvore/ensemble como controles;
- split walk-forward com três ou mais cortes temporais;
- `R` por timestamp, média temporal, distribuição, cumulativo e estabilidade por regime;
- auditoria de latência, memória, número de atualizações e falhas;
- nenhuma fonte forward-looking, informação material não pública, rotulagem humana do teste ou feature derivada do futuro.

Esse desenho testa coadaptação temporal em uma tarefa tabular, mas não deve ser chamado de prova de causalidade ou de simbiose geral.

### Disponibilidade e limitações

O treino era baixável, mas o teste era transmitido pela API; a execução histórica Kaggle Gym/Kernels, o container e os limites originais podem não ser restauráveis. A página atual pode exigir autenticação e não fornece uma contagem confiável de linhas/features. Não há baseline numérico oficial completo claramente publicado. Features anonimizadas, regime shift e leaderboard privado tornam um único score frágil.

**Executável agora:** desenhar e testar um adaptador temporal em dados autorizados ou um simulador causal que nunca revela o futuro; a infraestrutura de ledger, seeds e walk-forward pode ser implementada imediatamente.  
**Não executável agora:** alegar reprodução do leaderboard histórico ou avaliar o score privado sem dataset/API original.

**Fontes principais:**

- [Overview Kaggle](https://www.kaggle.com/c/two-sigma-financial-modeling/overview)
- [Dados](https://www.kaggle.com/c/two-sigma-financial-modeling/data)
- [Regras](https://www.kaggle.com/c/two-sigma-financial-modeling/rules)
- [Instruções de submissão](https://www.kaggle.com/c/two-sigma-financial-modeling/overview/submission-instructions)
- [Leaderboard](https://www.kaggle.com/competitions/two-sigma-financial-modeling/leaderboard)
- [Discussão oficial](https://www.kaggle.com/c/two-sigma-financial-modeling/discussion/26128)
- [Entrevista da equipe top-5](https://medium.com/kaggle-blog/two-sigma-financial-modeling-code-competition-5th-place-winners-interview-team-best-fitting-279a493c76bd)
- [Anúncio Two Sigma/Kaggle](https://www.twosigma.com/articles/two-sigma-partners-with-kaggle/)

## 3. O que pode ser executado agora e o que exige reimplementação

### 3.1 Executável imediatamente no repositório HERUS

Sem baixar dados restritos, já é possível executar os componentes científicos comuns a todas as cinco competições:

1. **Object Lock:** fixar tarefa, entrada, target, unidade independente, split, métrica, orçamento, versão e claim permitida.
2. **Manifesto de proveniência:** URL, autorização, hash SHA-256/MD5, release, parser, pré-processamento, seed, hardware e commit.
3. **Auditoria adversarial de leakage:** impedir `y_true`, dados futuros, labels ocultas, agregações que atravessem o corte e tuning repetido no leaderboard.
4. **Ledger por exemplo/consulta/entidade:** registrar previsão, abstenção, versão do modelo, timestamp, custo, erro, provenance e motivo.
5. **Baselines triviais e controles:** maioria/média, nearest case ou lookup, regressão simples, modelo especializado mínimo, conforme a modalidade.
6. **Curvas risco–cobertura:** se o HERUS puder abster-se, reportar desempenho condicionado à aceitação e desempenho global, sem chamar seletividade de acurácia geral.
7. **Harness sintético:** testar casos positivos/negativos, conflito, contexto vazio, estado incompatível, evidência antiga, `step=0`, ordem embaralhada e target derivado do rótulo. O último caso deve falhar fechado.
8. **Relatório de custo:** tempo, RAM/VRAM, armazenamento, latência, número de atualizações e energia quando mensurável.

Essas etapas validam o **objeto e a disciplina experimental**, não geram score oficial de nenhuma competição.

### 3.2 O que exige reimplementação de modalidade

| Desafio | Adaptador mínimo necessário | Métrica e validação que precisam ser preservadas | Status honesto |
|---|---|---|---|
| ILSVRC | Encoder RGB; cabeça de classificação/localização/detecção; confiança e, opcionalmente, abstenção | Top-5 error; IoU > 0,5; AP/mAP conforme edição e split | Não direto; referência visual |
| GLDv2 | Encoder/retriever RGB, índice versionado, política de rejeição OOD e uma label máxima | GAP@1/µAP, ranking global e falsos positivos OOD | Não direto; referência visual/retrieval |
| Avito | Parser tabular, tokenizer/encoder textual, cache de imagens e agregações temporais leakage-safe | RMSE com split temporal; comparar modalidades e custo | Adaptação válida condicional |
| Netflix | Parser esparso, bias temporal, matrix factorization e interface user–movie–date | RMSE no probe/split controlado; test oficial não verificável | Adaptação válida condicional |
| Two Sigma | Runner causal por timestamp, estado persistente, missing values e atualização online | R por timestamp/média temporal, walk-forward e simulador API | Adaptação válida mais próxima |

Nenhuma adaptação pode ocultar o trabalho de representação. Se o encoder visual, o modelo de linguagem, a fatoração ou os lags forem necessários, o relatório deve separar `HERUS-core`, `adapter`, `specialized baseline` e `full system`.

## 4. Protocolo de benchmark em fases

O protocolo abaixo é comum, mas cada competição deve ter um Object Lock próprio. A ordem evita gastar meses em datasets que não podem ser legalmente avaliados ou cujo alvo não é compatível.

### Fase 0 — triagem de acesso e congelamento

**Gate de entrada:** nenhum treino antes de satisfazer estes itens.

- Confirmar autorização institucional e termos de uso.
- Fixar a edição, release, arquivos, URLs e hashes.
- Registrar se o teste é rotulado, se há scorer e se o leaderboard privado ainda funciona.
- Definir a unidade independente: imagem, consulta, anúncio, rating ou timestamp/id.
- Definir o target sem usar o rótulo do holdout na entrada.
- Registrar política de dados externos, inclusive imagens, texto, índices e modelos pré-treinados.
- Fixar hardware, limite de tempo/memória, seeds e versão de software.
- Declarar antes do experimento se o resultado é **oficial**, **offline controlado** ou **proxy temporal**.

**Saída:** `object-lock.yaml`, `dataset-manifest.json`, `claims.md` e decisão `go/no-go`.

### Fase 1 — teste de validade e leakage

Executar um harness barato antes de usar modelos pesados:

- embaralhar IDs quando isso for permitido e verificar se o score não depende de identidade indevida;
- bloquear qualquer coluna derivada de futuro;
- executar teste de duplicatas e quase-duplicatas entre treino/validação/teste;
- verificar usuários, filmes, anúncios, landmarks e instrumentos que aparecem em mais de uma partição;
- testar contexto vazio, missing values, labels múltiplas e consulta OOD;
- garantir que o target não seja passado em `context`, `target_effect`, cache, nome de arquivo ou metadados;
- confirmar que o scorer local corresponde à definição publicada;
- testar a mesma configuração após reordenar entradas.

**Gate:** qualquer vazamento, versão ambígua ou unidade independente indefinida interrompe a campanha. O resultado deve ser reportado como diagnóstico, não como benchmark.

### Fase 2 — baseline e HERUS mínimo

Para cada desafio, executar controles em ordem de complexidade:

1. **Controle nulo:** zero, média, maioria ou previsão global apropriada.
2. **Controle estruturado:** média por entidade, regressão regularizada, lookup/nearest case ou modelo tabular simples.
3. **Baseline especializado mínimo:** matrix factorization para Netflix; encoder visual simples para ILSVRC/GLDv2; GBDT/texto para Avito; Ridge/Extra Trees e lags causais para Two Sigma.
4. **HERUS-core/adaptador:** manter memória, gate, proposta e abstenção separados do encoder/regressor.
5. **Full system especializado:** somente como teto operacional, com a etiqueta de solução de competição.

Três seeds podem ser usadas como replicações da execução, não como três datasets independentes. A unidade de inferência e a dependência — episódio, usuário, anúncio, classe ou timestamp — precisam constar no intervalo de incerteza.

### Fase 3 — ablações da hipótese de adaptação

O núcleo da comparação não é “HERUS versus campeão histórico”; é uma ablação pareada:

- sem memória/gate;
- memória sem adaptação;
- adaptação sem memória;
- HERUS completo;
- especializado sem HERUS;
- full system com todos os componentes.

Além disso, variar apenas uma modalidade por vez. No Avito, por exemplo, comparar tabular-only, tabular+texto e multimodal; no visual, encoder fixo versus encoder ajustado; no Netflix, sem dinâmica temporal versus dinâmica; no Two Sigma, estático versus online. O orçamento de dados, tuning, memória, latência e seeds deve ser o mesmo entre comparações equivalentes.

### Fase 4 — avaliação de generalização e custo

A avaliação mínima inclui:

- split temporal intocado quando a tarefa tem tempo;
- holdout por entidade/usuário/landmark quando fizer sentido;
- subgrupos de cauda longa, usuários leves, OOD, anúncios sem imagem, missingness e regimes voláteis;
- curva de risco–cobertura e taxa de abstenção quando houver rejeição;
- métrica oficial e métricas auxiliares, sem substituir uma pela outra;
- latência p50/p95, RAM/VRAM, armazenamento, tempo de treino e número de chamadas ao servidor;
- sensibilidade a seed e intervalo pareado por unidade independente;
- custo de adaptação: quantidade de observações, atualizações e feedback necessários para mudar a decisão.

Para competições com test oculto, o score local deve ser denominado `offline/proxy`; nunca “resultado oficial” ou “posição histórica” sem acesso ao scorer correspondente.

### Fase 5 — auditoria e reprodução

Um segundo executor deve poder reconstruir o resultado a partir de:

- snapshot ou instrução legal para obter o dado;
- hashes e scripts de parse;
- configuração, seed, commit e ambiente;
- pesos/checkpoints ou instrução para reproduzi-los;
- ledger por exemplo;
- tabela de ablação e log de falhas;
- política de dados externos e licenças;
- declaração explícita do que não foi reproduzido.

A campanha termina com uma de três conclusões: **evidência de ganho adaptativo limitado no domínio**, **efeito de engenharia/modalidade**, ou **ausência de efeito**. Nenhuma das três autoriza SOTA geral.

## 5. Métricas e critérios por desafio

| Desafio | Métrica primária | Métricas HERUS adicionais | O que não pode ser inferido |
|---|---|---|---|
| ILSVRC | Top-5 error; localização/detecção conforme edição | cobertura, risco seletivo, calibração, IoU por tamanho de objeto, custo | não mede memória episódica, cooperação ou generalização simbólica |
| GLDv2 | GAP@1/µAP | cobertura de rejeição, falsos positivos OOD, long tail, estabilidade do ranking | não mede classes não vistas nem transferência geral |
| Avito | RMSE | RMSE por modalidade/subgrupo, leakage tests, memória/latência, custo de imagens | não prova multimodalidade genérica nem causalidade de venda |
| Netflix | RMSE em probe/split controlado | RMSE por usuário leve/filme, cobertura, tempo/memória e ranking auxiliar separado | não mede top-N, CTR, retenção, cold start ou utilidade de negócio |
| Two Sigma | R por timestamp e média temporal | distribuição temporal, walk-forward, estabilidade por regime, latência e atualizações | não prova causalidade, trading real ou RL geral |

## 6. Riscos transversais e decisões de bloqueio

### 6.1 Dados não disponíveis não são um detalhe administrativo

ILSVRC, GLDv2, Avito, Netflix e Two Sigma têm, em graus diferentes, login, termos, imagens com licenças variadas, arquivos históricos incompletos, test oculto ou API aposentada. Um mirror informal ou um arquivo sem hash não deve ser incorporado ao scorecard principal. Se não houver autorização e versão verificável, o experimento pode ser um **protocolo de reimplementação**, não um resultado.

### 6.2 Leaderboard privado não é ground truth reprodutível

No Netflix, labels do test nunca foram divulgadas; ILSVRC e GLDv2 ocultam labels de teste; Avito e Two Sigma dependem de frações privadas do teste. Uma validação local é útil para comparação controlada, mas não permite recuperar o ranking histórico. Relatar um número local como se fosse oficial é erro de nomenclatura e de validade externa.

### 6.3 Leakage pode transformar uma adaptação em uma ilusão

A maior ameaça à hipótese HERUS é confundir acesso ao target, identidade de entidade ou feedback futuro com adaptação. Toda atualização deve registrar o timestamp e a fonte do feedback. A origem de qualquer `target_effect`, rótulo, reward ou agregação deve ser auditável. O sistema deve falhar fechado quando o alvo de teste participa da decisão.

### 6.4 Especialização é válida, mas precisa ser nomeada

Não há problema em usar um encoder visual, factorization, CNN, lags, embeddings de texto ou ensemble. O problema é atribuir o ganho inteiro ao HERUS. A tabela de resultados deve distinguir:

- `HERUS-core`;
- `adapter` de modalidade;
- baseline especializado;
- `HERUS + adapter`;
- solução completa de competição.

A conclusão deve dizer qual camada foi responsável pelo ganho e se a mesma política de adaptação foi aplicável em outro domínio.

## 7. O que significa “vencer”

“Vencer” tem dois sentidos incompatíveis se não forem separados.

### 7.1 Vencer a competição histórica

Significa reproduzir as regras originais, usar dados e recursos permitidos, produzir o formato de submissão correto, obter o score no scorer/leaderboard oficial e respeitar o prazo e a política de submissões. Para estas cinco fichas, isso está parcialmente ou totalmente impedido por test oculto, scorer aposentado, API indisponível, mudanças de release, termos de uso ou ausência de arquivos. Portanto, **não é permitido declarar vitória histórica do HERUS** com uma validação offline.

### 7.2 Vencer cientificamente como teste do HERUS

Um resultado favorável seria mais estreito:

1. não há leakage e a auditoria de identidade passa;
2. o adaptador é declarado antes do teste e separado do núcleo;
3. HERUS supera controles comparáveis na métrica primária ou na curva risco–cobertura, dentro do mesmo orçamento;
4. o ganho persiste em cortes temporais/por entidade e não depende de um caso raro;
5. ablações mostram que o ganho não vem apenas do encoder, da engenharia de features ou do acesso a informação futura;
6. o sistema registra abstenção, custo, latência e falhas, em vez de ocultá-los;
7. outro executor reproduz a tabela a partir dos artefatos versionados;
8. a claim final é específica: “evidência de um gate adaptativo no domínio X sob o protocolo Y”.

Mesmo que todos os itens passem, isso não significa que HERUS seja SOTA geral, AGI, sistema causal, agente cooperativo ou vencedor universal. Se apenas o modelo especializado superar o baseline, a conclusão correta é que o desafio mede especialização; se HERUS só melhora quando recebe o encoder adequado, o resultado é sobre adaptação de modalidade; se a abstenção aumenta precisão mas destrói cobertura, o resultado é seletividade e deve ser reportado como tal.

## 8. Recomendação executiva

1. **Não iniciar pelos dois benchmarks visuais** como campanha principal. ILSVRC e GLDv2 exigem reimplementação de percepção e têm distância máxima do HERUS-core; use-os como referências ou extensão posterior.
2. **Usar Two Sigma como primeiro adaptador experimental**, mas em um runner temporal local, com walk-forward e sem alegação de reprodução do leaderboard. Ele testa melhor a hipótese de atualização causal sob feedback passado.
3. **Usar Netflix como segundo estudo**, se e somente se houver cópia legal e exata dos arquivos. É um teste claro de adaptação a registros esparsos e mostra o quanto um resultado depende de collaborative filtering especializado.
4. **Deixar Avito para uma fase de multimodalidade**, depois de haver pipeline de leakage, armazenamento e licença. É rico, mas pode consumir todo o orçamento em engenharia de dados antes de testar a hipótese HERUS.
5. **Manter ILSVRC/GLDv2 em um registro de referência**, com possível estudo de abstenção/OOD no futuro, não como promessa de benchmark direto.
6. **Congelar primeiro o Object Lock e o ledger**. O valor científico desta seleção é maior quando ela falsifica ou delimita claims do HERUS do que quando adiciona mais uma tabela de scores.

## 9. Síntese final

A seleção cobre cinco formas diferentes de dificuldade: percepção visual em larga escala, reconhecimento de instâncias e rejeição OOD, multimodalidade com risco de leakage, fatoração esparsa em escala industrial e previsão temporal online. Ela é experimentalmente complementar, mas não homogênea. Comparar RMSE de Netflix, GAP de GLDv2, top-5 error de ILSVRC, RMSE de Avito e R financeiro em uma única classificação seria metodologicamente inválido.

O resultado mais honesto hoje é: **nenhuma competição é alvo direto do HERUS-core atual; Two Sigma é a adaptação válida mais próxima, Netflix e Avito são adaptações condicionais que exigem reimplementação de modalidade, e ILSVRC/GLDv2 são desafios de referência até existir uma ponte visual versionada**. A vitória relevante não é obter um número alto em um leaderboard histórico inacessível. É construir um experimento no qual a contribuição do HERUS possa ser isolada, reproduzida, derrotada por controles simples se estiver errada e descrita sem transformar especialização, abstenção ou adaptação de entrada em uma alegação de simbiose geral.
