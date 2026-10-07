# Síntese wide research — plano para levar o HERUS de 94% a evidência defensável

## 1. Escopo e veredito executivo

Este relatório sintetiza cinco auditorias independentes sobre eficácia, ledger pareado, custo, transferência cross-domain, cobertura conformal e reprodutibilidade. O objetivo não é encontrar uma formulação favorável aos resultados, mas definir o menor conjunto de experimentos que transforme a evidência atual em uma base auditável para claims estreitos e pré-especificados.

### Veredito

**Status atual: BLOQUEADO para qualquer claim amplo sobre superioridade, eficiência, transferência cross-domain, segurança/calibração geral, causalidade, universalidade ou SOTA do HERUS.**

A evidência atual permite somente um diagnóstico limitado:

1. Existe um holdout temporal MIntRec S04/S05 → S06 com 386 linhas e números internamente coerentes no material arquivado.
2. O melhor desempenho de acurácia total publicado no S06 é do Naive Bayes (0,492228; macro-F1 0,374427), seguido do transformer (0,466321; macro-F1 0,338785). O resultado publicado para `HERUS context memory` é muito inferior (0,062176; macro-F1 0,118770).
3. O valor 1,0 associado ao HERUS é **acurácia seletiva em apenas 24/386 casos cobertos**, não acurácia geral.
4. O comportamento conformal é uma observação empírica de um controle lexical/Naive Bayes em um único holdout; não valida representação aprendida, pipeline HERUS completo ou garantia futura.
5. O resultado de MInDS-14 é de baselines de classificação de intenções em texto, não de HERUS, e não demonstra transferência de eventos HERUS, áudio ou multimodalidade.
6. Eficiência, replicabilidade independente e significância pareada completa ainda não foram demonstradas.

Portanto, **94% não deve ser tratado como uma lacuna de desempenho a ser preenchida por ajuste incremental**, mas como um sinal de que faltam contratos experimentais, identidade por exemplo, definição de endpoint, execução do pipeline completo e replicação. Não há base para prometer SOTA geral; a meta racional é primeiro tornar um claim estreito falsificável e reproduzível.

---

## 2. O que converge entre as auditorias

### 2.1 O resultado agregado atual não sustenta superioridade do HERUS

As auditorias convergem nos mesmos números do holdout S06:

| Sistema/resultado | Accuracy | Macro-F1 | Interpretação correta |
|---|---:|---:|---|
| Naive Bayes multinomial | 0,492228 | 0,374427 | Melhor baseline publicado em acurácia total |
| Transformer | 0,466321 | 0,338785 | Ledger internamente consistente, mas abaixo do Naive Bayes |
| HERUS `context memory` | 0,062176 | 0,118770 | Resultado negativo da execução fornecida; não é o pipeline HERUS completo |
| HERUS seletivo, alvo 95 | cobertura 0,227979 | accuracy seletiva 0,931818 | Ponto seletivo de baixa cobertura; não é acurácia geral |
| `finite_context_memory` seletivo | cobertura 0,062176 | accuracy seletiva 1,0 | 24 casos cobertos; não é desempenho global |

As diferenças reportadas envolvendo HERUS são separadas de zero no artefato, mas isso não resolve os problemas de identidade, completude e especificação: o código pareia previsões por posição, e não por `example_id` verificável; o ledger dos comparadores não está arquivado de forma equivalente; e a execução auditada é explicitamente apenas `context memory`.

### 2.2 Há uma base documental real, mas não uma cadeia de prova completa

É positivo que os arquivos declarem S04 para ajuste, S05 para calibração e S06 para holdout, além de 386 linhas, bootstrap pareado com 4.000 reamostragens e seed 17. A auditoria do ledger disponível encontrou 386 entradas, índices contíguos 0–385, consistência de `correct` com `label == prediction`, 180 acertos e macro-F1 recalculado de 0,338785.

Isso prova **consistência interna de parte do artefato**, não sua proveniência completa. Faltam IDs estáveis, hashes por entrada ou chaves verificáveis; snapshot/checksum imutável do dataset; ledger linha a linha de todos os sistemas; seeds e checkpoints completos; ambiente fechado; e reprodução independente.

### 2.3 A evidência de MInDS-14 está sendo superinterpretada

As auditorias convergem que MInDS-14, configuração pt-PT, contém 604 linhas (417 ajuste, 89 calibração, 98 holdout) e fornece um diagnóstico real de classificação textual. Porém:

- `accuracy 0,918367` e `macro-F1 0,892947` são do **Naive Bayes**;
- `accuracy 0,877551` e `macro-F1 0,876193` são do **centróide**;
- `coverage 0,908163` e `selective accuracy 0,943820` são de `consensus_selective`;
- não há avaliação equivalente do núcleo Symbiotic Learning v2/HERUS;
- o áudio não foi decodificado;
- a divisão por ordem de path dentro de cada rótulo não demonstrou independência por falante/grupo;
- não existe contrato pré-registrado ligando os rótulos de intenção a eventos HERUS.

Assim, MInDS-14 é, no máximo, um candidato a estudo externo futuro — não uma validação cross-domain já obtida.

### 2.4 Custo não foi medido em comparação válida

A matriz MInDS-14 contém tempos de quatro modelos TF-IDF, não do HERUS. No scorecard MIntRec S06, 12 de 14 linhas têm tempos nulos; somente `tensor_train_rank_32` e `rank_64` têm tempos. Não há ambiente/hardware/versões/threads/batch/dtype, fronteira de medição, memória de pico, repetição ou variância comparável para HERUS e os baselines relevantes.

Logo, nenhum número de acurácia seletiva, cobertura ou tempo isolado autoriza claim de eficiência. Eficiência só poderá ser liberada com utilidade comparável, mesmo ambiente e ledger pareado.

### 2.5 Conformal é evidência parcial, não assurance geral

No S06, as coberturas marginais observadas para alvos nominais de 80%, 90%, 95% e 99% foram 81,3472%, 90,1554%, 94,8187% e 99,4819%, com tamanhos médios de conjunto 8,564767; 12,836788; 15,976684; e 19,111399. Isso é uma observação útil, mas tem três limites materiais:

1. os conjuntos são grandes e a taxa de singleton foi apenas 1,0363% no alvo de 80% e 0% nos alvos de 90%, 95% e 99%;
2. a análise class-conditional ficou em 77,7202%, 89,6373% e 93,7824% para alvos nominais de 80%, 90% e 95%;
3. a implementação usa `ceil(q*n_cal)-1` sem explicitar a correção finita padrão baseada em `n_cal+1`.

Sem correção/verificação do quantil, intervalos de incerteza, suporte por classe e janelas temporais adicionais, não cabe chamar isso de garantia formal, cobertura futura, cobertura condicional ou segurança em produção.

---

## 3. Conflitos aparentes e resolução correta

| Aparente conflito | Resolução honesta |
|---|---|
| “HERUS tem 100%” versus `accuracy = 0,062176` | 1,0 é accuracy seletiva em 24 casos cobertos; a acurácia total registrada é 0,062176. Reportar sempre cobertura junto da accuracy seletiva. |
| “HERUS supera os modelos” versus Naive Bayes em 0,492228 e transformer em 0,466321 | A execução auditada de `context memory` não supera nenhum baseline; além disso, não representa necessariamente o pipeline HERUS completo. |
| “Transformer supera Naive Bayes” | A diferença transformer − Naive Bayes é −0,025907, com IC [−0,077720, 0,025907]; inclui zero e não demonstra superioridade. |
| “MInDS valida HERUS” versus resultados altos no MInDS | Os resultados são de Naive Bayes, centróide e consenso seletivo; não de HERUS. São texto de intenção, não eventos HERUS nem áudio. |
| “95% garantido” versus 94,8187% marginal e 93,7824% class-conditional | Uma observação de um holdout não é garantia futura; a implementação do quantil ainda requer revisão finita e devem ser publicados intervalos, suporte e desempenho por classe. |
| “Ledger prova o pipeline completo” versus auditoria do ledger | O ledger disponível é parcial, sem IDs estáveis, sem hashes por exemplo e sem ledger comparável dos sistemas. Prova consistência interna, não pareamento auditável nem completude. |
| “Tempos do MInDS mostram eficiência do HERUS” | Medem quatro modelos TF-IDF no MInDS, não HERUS no MIntRec. O claim é inválido por falta de medição do objeto alegado. |
| “Nova representação melhorou” versus uso de `b.vector` e probabilidades NB | Não há comparação pré-registrada entre representação lexical de controle e representação aprendida; a evidência atual não identifica efeito de representação. |
| “Ablação demonstra causalidade” | Sem intervenção isolada, registro completo de variantes, split fixo e comparação pareada por ID, há associação operacional, não causalidade. |

---

## 4. Princípio de desenho: trocar uma narrativa ampla por um endpoint estreito

Antes de novos experimentos, o projeto deve escolher **um claim primário** que possa ser falsificado. A recomendação é não começar por “HERUS é superior” ou “HERUS é geral”, mas por algo do tipo:

> **Endpoint candidato:** em MIntRec S06, sob o mesmo snapshot, mesmo protocolo S04/S05 → S06 e mesmo orçamento de execução, a implementação identificada do pipeline HERUS obtém melhora pré-especificada em uma métrica primária (por exemplo, macro-F1 ou utilidade seletiva em cobertura mínima fixada) contra um baseline nomeado, com replicação em pelo menos três seeds e limite inferior do intervalo pareado acima da margem prática definida.

O endpoint deve especificar, antes de olhar S06:

- qual implementação conta como **HERUS completo** e quais componentes são apenas controles (`context memory`, consensus, representação lexical etc.);
- métrica primária e métricas secundárias;
- cobertura mínima para qualquer claim seletivo;
- margem prática de superioridade, não somente significância estatística;
- unidade de pareamento (exemplo ou diálogo, caso haja dependência intra-diálogo);
- seeds, checkpoints, regra de seleção e proibição de tocar no holdout;
- tratamento de abstentions, classes ausentes e falhas de execução;
- critério de parada e correção para múltiplas comparações, se houver mais de um endpoint.

Sem esse contrato, cada novo número pode apenas aumentar a flexibilidade analítica e não a força da evidência.

---

## 5. Ordem ótima dos gates e experimentos

A ordem abaixo minimiza trabalho desperdiçado: primeiro torna os dados e o objeto testado identificáveis; depois testa eficácia; só então mede seletividade, custo e transferência; por fim exige replicação independente. Gates podem ser executados em paralelo apenas quando sua pré-condição estiver satisfeita.

### Gate 0 — Contrato de claims e pré-registro

**Objetivo:** congelar o que será testado e impedir que acurácia seletiva, um baseline externo ou um componente parcial seja promovido a claim do HERUS.

**Entregáveis:** protocolo com endpoint primário, comparadores, margem prática, cobertura mínima, seeds, splits, métricas, política de abstenção, regras de exclusão e plano de análise.

**Passa se:** dois revisores conseguem mapear cada número futuro a uma hipótese, unidade de análise e fonte de dados sem ambiguidade.

**Falha se:** o protocolo mantém termos como “melhor”, “seguro”, “geral” ou “eficiente” sem comparador, orçamento e limiar operacional.

### Gate 1 — Proveniência, identidade e reconstruibilidade do MIntRec

**Objetivo:** substituir o pareamento por posição por pareamento por identidade verificável.

**Experimento/execução:** congelar snapshot/revisão/checksum do MIntRec; publicar manifesto dos 386 `example_id` de S06; validar IDs e rótulos sem perdas, duplicatas ou colisões; registrar commit, dependências, hardware, versões, seeds e checkpoints. Criar um ledger único por exemplo e por seed com `example_id`, `y_true`, previsões de HERUS/transformer/Naive Bayes, scores, confidências, `accepted/abstained`, `correct_selective`, versão e links para logs.

**Passa se:** um auditor independente reconstrói o conjunto, verifica 386 IDs estáveis, obtém o mesmo conjunto de rótulos, encontra zero perda/duplicata e reproduz as métricas agregadas a partir do ledger sem usar posição como chave.

**Falha se:** houver qualquer discrepância não explicada, ledger incompleto de um comparador, checksum ausente ou dependência oculta de ordem.

### Gate 2 — Rerun de eficácia do pipeline completo no holdout S06

**Objetivo:** testar o objeto alegado, não `context memory` isoladamente.

**Execução:** executar o commit identificado do Symbiotic Learning v2/HERUS completo e os comparadores do protocolo no mesmo S04/S05 → S06; ajustar e selecionar somente em S04, calibrar somente em S05 e manter S06 intocado; repetir no mínimo três seeds pré-declaradas; publicar accuracy, macro-F1, matriz de erros e resultados por seed, além de diferenças pareadas por ID ou diálogo.

**Passa se:**

- o pipeline executado corresponde à definição congelada de HERUS;
- o ledger de todos os sistemas passa o Gate 1;
- a métrica primária e a margem prática foram pré-especificadas;
- o limite inferior do IC pareado da diferença contra o comparador primário excede a margem definida, ou, se o endpoint for não-inferioridade, não cai abaixo da margem de não-inferioridade;
- o resultado permanece qualitativamente consistente entre as três seeds.

**Falha se:** somente um componente parcial for executado, a vantagem existir apenas em uma seed, ou o intervalo incluir a margem nula/prática. Nesse caso, o claim deve ser rebaixado a “resultado exploratório no S06”.

### Gate 3 — Seletividade e conformal com cobertura explicitamente contratada

**Objetivo:** separar utilidade seletiva de performance global e corrigir a base estatística do claim de cobertura.

**Execução:** comparar controle lexical/Naive Bayes e uma única representação candidata; ajustar em S04, calibrar em S05 e avaliar em S06; revisar e congelar o quantil finito conformal, explicitando a convenção baseada em `n_cal+1`; medir cobertura marginal com intervalo binomial, accuracy seletiva, macro-F1 nos aceitos, tamanho médio/mediano e percentis dos conjuntos, cobertura e suporte por classe, taxa de singleton corretamente nomeada e curva coverage–accuracy.

**Passa se:** a cobertura observada satisfizer o endpoint e sua incerteza, a cobertura mínima pré-definida for respeitada, não houver classe sem suporte relevante, e a redução de tamanho/ganho de utilidade sobre o controle superar a margem prática sem tocar S06 durante seleção. Para claims temporais, repetir em pelo menos três janelas rolling-origin independentes.

**Falha se:** a cobertura só ocorrer com conjuntos grandes, desaparecer por classe, depender de um limiar escolhido após ver o holdout ou não se distinguir do controle. Nesse caso, reportar comportamento empírico, nunca “garantia”.

### Gate 4 — Benchmark de custo e utilidade equivalente

**Objetivo:** avaliar eficiência no mesmo problema e não importar tempos de outro dataset.

**Execução:** no MIntRec S06, medir HERUS e os dois comparadores do protocolo no mesmo hardware, ambiente, commit, threads, batch, dtype e fronteira de medição; realizar warm-up; repetir pelo menos cinco seeds pré-declaradas; arquivar tempos brutos de treino, calibração e inferência por entrada e por lote (p50/p95), RSS RAM, memória GPU de pico, falhas e custos dos abstidos. Amarrar cada medição ao ledger por `example_id`/seed e separar cobertura total do ponto seletivo.

**Passa se:** houver vantagem ou dominância definida previamente, em mediana e incerteza, para uma utilidade equivalente — por exemplo, não reduzir cobertura/accuracy abaixo da margem permitida enquanto reduz tempo/memória/custo. O claim deve especificar se é treino, inferência ou ambos.

**Falha se:** qualquer sistema não tiver medição comparável, o ganho depender de cobertura menor, ou os intervalos se sobrepuserem de modo incompatível com a vantagem alegada. Isso não impede uma descrição de custo local, mas bloqueia “mais eficiente” em geral.

### Gate 5 — Transferência cross-domain com contrato semântico independente

**Objetivo:** testar transferência, não somente executar baselines em MInDS-14.

**Pré-condições:** Gates 0–2 aprovados ou, no mínimo, o commit do HERUS claramente executável; snapshot/checksum externo; divisão independente por falante, grupo ou domínio; mapeamento de cada rótulo externo para evento/efeito HERUS revisado e congelado antes do holdout; política explícita para rótulos ambíguos ou sem mapeamento.

**Execução:** usar um segundo dataset com domínio não visto, executar HERUS completo e os mesmos baselines no mesmo holdout, calibrar somente em ajuste/calibração, arquivar previsões, scores, abstenções, erros, custos e versões. Se o claim for sobre áudio ou multimodalidade, decodificar e testar a modalidade correspondente; texto de MInDS-14 não é substituto.

**Passa se:** o split independente for verificável, o contrato de rótulos tiver cobertura e taxa de exclusão pré-definidas, e o endpoint primário atingir a margem definida com incerteza por unidade independente.

**Falha se:** o mapeamento for pós-hoc, houver dependência por falante não controlada, apenas baselines forem executados ou o estudo continuar restrito a transcrições enquanto o claim disser áudio/multimodalidade.

### Gate 6 — Reprodução independente e robustez temporal

**Objetivo:** converter execução interna em evidência replicável e limitar extrapolações.

**Execução:** uma segunda execução limpa, por ambiente ou equipe independente, reconstrói o scorecard a partir do ledger imutável; repetir seeds fixadas, o dataset externo e, para conformal, janelas rolling-origin. Registrar diferenças entre ambientes, falhas e qualquer decisão manual.

**Passa se:** o resultado primário e os limites de incerteza forem reproduzidos dentro de tolerância pré-definida, sem alteração silenciosa do código, dados ou regra de seleção.

**Falha se:** a reprodução depender de intervenção manual não documentada ou o efeito desaparecer fora da execução original. Nesse caso, a conclusão deve ser “não replicado”, não “quase SOTA”.

---

## 6. Matriz de claims que podem ou não ser liberados

### Claims liberáveis agora, com redação estreita

- “No artefato arquivado, o ledger do transformer em MIntRec S06 é internamente consistente: 386 entradas, 180 acertos, accuracy 0,466321 e macro-F1 recalculado 0,338785.”
- “Na execução fornecida, `HERUS context memory` teve accuracy total 0,062176; seu valor 1,0 foi accuracy seletiva em 24 casos cobertos.”
- “No holdout publicado, o Naive Bayes apresentou accuracy 0,492228 e macro-F1 0,374427, acima do transformer em accuracy agregada; a diferença transformer–Naive Bayes não demonstra superioridade estatística no IC fornecido.”
- “Foi observado comportamento empírico de split conformal em um controle lexical/Naive Bayes sobre um único holdout temporal MIntRec, sujeito a revisão do quantil e sem garantia futura.”
- “MInDS-14 contém resultados de baselines textuais de intenção; esses resultados não constituem avaliação do HERUS.”

### Claims ainda bloqueados

- HERUS supera transformer, Naive Bayes ou “os métodos atuais” em geral.
- HERUS tem accuracy geral de 100%, ou qualquer accuracy geral próxima de 100% baseada em accuracy seletiva.
- O ledger atual prova o pipeline HERUS completo, pareamento auditável ou causalidade.
- HERUS demonstra reconhecimento de eventos HERUS, generaliza para áudio, vídeo, produção, outros domínios ou domain shift robusto.
- A política de abstenção é segura, calibrada, confiável ou garantida em geral.
- HERUS é mais eficiente: tempo de treino/inferência, memória, energia, FLOPs, custo monetário ou fronteira de Pareto.
- HERUS domina tensor trains ou qualquer baseline sem medição no mesmo ambiente e utilidade equivalente.
- A cobertura nominal de 95% é garantia finita, futura, condicional, por classe ou de produção.
- Existe superioridade pareada completa ou significância pareada de eficácia/eficiência.
- A representação aprendida foi validada ou causou redução de tamanho/ganho de desempenho.
- MInDS-14 comprova transferência cross-domain, Symbiotic Learning v2, multimodalidade ou independência por falante.
- Há estabilidade entre seeds, reprodutibilidade independente ou generalização universal.
- Há superioridade geral em reinforcement learning, “simbiose geral” ou AGI; um proxy bandit não substitui um ambiente sequencial real.
- Qualquer claim de SOTA geral ou de universalidade do HERUS.

---

## 7. Ação imediata recomendada

**Congelar imediatamente o protocolo do Gate 0 e executar os Gates 1–2 como uma única campanha de reconstrução do MIntRec S06.** O artefato central deve ser um ledger imutável, único e indexado por `example_id`, contendo HERUS completo, transformer e Naive Bayes, com snapshot/checksum, seeds, checkpoints, versões e logs. Nenhum resultado seletivo, custo ou cross-domain deve ser promovido antes de essa cadeia de identidade passar.

A primeira pergunta experimental não deve ser “como fazer o HERUS chegar a 100%”, mas:

> **Quando o pipeline HERUS completo é executado sob identidade, split, seeds e endpoint pré-especificados, ele supera ou ao menos não é inferior ao baseline primário em uma métrica e cobertura operacionalmente relevantes?**

Se a resposta for não, o projeto deve pivotar para uma contribuição estreita — por exemplo, análise de seletividade, compressão, representação ou conformal — e reescrever a narrativa sem atribuir ao HERUS resultados de baselines. Se for sim, os Gates 3–6 determinarão se o efeito é seletivo, eficiente, transferível e replicável.

---

## 8. Impacto esperado sobre a conclusão do projeto

A conclusão imediata ficará menos ampla, mas mais forte: **há sinais diagnósticos e um protocolo recuperável, não ainda uma demonstração de superioridade do HERUS**. O trabalho adicional não deve ser usado para “inflar” a taxa de sucesso; ele deve reduzir incerteza sobre identidade, endpoint, cobertura e replicação.

O melhor cenário plausível após os gates é um claim delimitado — por exemplo, uma vantagem em uma métrica específica, sob uma cobertura e domínio especificados, com custo e incerteza conhecidos. Mesmo nesse cenário, isso não autoriza SOTA geral, universalidade, segurança em produção, áudio/vídeo não testados ou AGI. O cenário negativo também é informativo: se o HERUS não superar o controle após auditoria correta, o resultado delimita uma falha real do pipeline e evita atribuir ao método ganhos que pertencem ao Naive Bayes, ao transformer ou ao procedimento seletivo.

**Conclusão final:** o próximo marco não é uma porcentagem maior; é uma prova experimental que sobreviva a identidade por exemplo, replicação, comparação justa e definição explícita do que conta como HERUS.
