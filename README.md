# HERUS

**HERUS é uma pesquisa de arquitetura simbiótica verificável.** O projeto investiga se um núcleo persistente pode observar um hospedeiro, aprender uma rotina finita, remapeá-la para outro hospedeiro e produzir uma proposta revisável sem transformar capacidade em autoridade.

> **Estado atual:** `host-only / mechanism candidate / not_proven`.

O HERUS não é uma AGI, não é um produto lançado, não é um sistema de execução segura e ainda não provou simbiose útil ou benefício humano. O repositório contém uma cadeia de engenharia, pesquisa e documentação para descobrir se essas afirmações poderiam algum dia ser justificadas.

## Comece aqui

| Ordem | Documento | Pergunta respondida |
|---:|---|---|
| 1 | [Leia isto primeiro](docs/00-LEIA-ME-PRIMEIRO.md) | Como navegar pelo projeto? |
| 2 | [Linha do tempo](docs/01-LINHA-DO-TEMPO.md) | O que o HERUS foi, como mudou e por quê? |
| 3 | [Mapa da arquitetura](docs/02-MAPA-DA-ARQUITETURA.md) | Como as partes se relacionam? |
| 4 | [Mapa do código](docs/03-MAPA-DO-CODIGO.md) | Onde cada conceito vive? |
| 5 | [Mapa da evidência](docs/04-MAPA-DA-EVIDENCIA.md) | O que foi demonstrado e o que não foi? |
| 6 | [Glossário](docs/05-GLOSSARIO.md) | O que cada termo significa? |
| 7 | [Roadmap e gates](docs/06-ROADMAP-E-GATES.md) | Qual é a próxima sequência legítima? |
| 8 | [Direção soberana](docs/57-DIRECAO-SOBERANA-E-SIMBIONTE.md) | Qual é o futuro objetivo do HERUS? |

## O núcleo da tese

O **ASA Core** mantém uma identidade persistente, observa um hospedeiro, aprende transições finitas e compõe uma Skill verificável. Ao mudar de hospedeiro, ele precisa reconstruir o contexto local. O objetivo futuro é que o Symbiont habite sistemas autorizados sem retirar os dados privados do domínio do usuário, usando uma autoridade física, revogável e auditável.

O mecanismo mais defensável está no pacote [Symbiont v2](research/symbiont_v2/README.md). O primeiro produto de pesquisa é o [HERUS Bridge](docs/56-RODADA-DECISIVA-HERUS-BRIDGE.md), uma demonstração local de proposta, prévia, confirmação, cancelamento, abstenção e parada sem transmissão nem efeitos externos.

A nova hipótese algorítmica é o [Symbiotic Learning v2](docs/84-ALGORITMO-SYMBIOTIC-LEARNING.md): indução contextual, bounded e reversível de efeitos observáveis para transferir uma Skill entre hospedeiros. O nome descreve uma linha de pesquisa proposta; ainda não é um campo científico estabelecido nem uma alegação de superioridade sobre os paradigmas existentes.

Sobre essa camada, o [Meta-Simbionte](docs/85-META-SIMBIONTE-E-HISTORICO-DE-SOLUCOES.md) mantém um histórico append-only de soluções verificadas. O histórico orienta novas sondagens e personaliza propostas, mas nunca substitui evidência fresca, autoridade ou confirmação humana.

## O que já existe

- Firmware C com contratos de semântica, memória, interação, confiança, recuperação e ameaças.
- Simulador C com mundo controlado, distância e adversários.
- Protótipos Python para Semantic IR, raciocínio finito, Symbiont e avaliação de utilidade.
- Holdouts históricos e uma campanha causal isolada para efeitos ocultos.
- Benchmark real MIntRec com holdout por temporada e comparação contra Naive Bayes, 1-NN, centróides e maioria.
- Comparação entre paradigmas supervisionado, não supervisionado, auto-supervisionado e reforço proxy no mesmo holdout real; nenhum resultado atual supera o supervisionado.
- Primeiro baseline transformer real: BERT pequeno, S04→S05→S06, 17,62% no holdout; comparação direta concluída sem alegação de vitória.
- Probe de atalhos em 386 utterances reais: prefixos superficiais mudaram até 55,18% das previsões dos baselines, revelando fragilidade que acurácia sozinha não mostra.
- Normalização canônica limitada recuperou 99,22%–100% das previsões após os fillers testados, sem apagar palavras internas; a correção permanece restrita ao vocabulário declarado.
- Habilidade de programação importável inspirada no Jev: decompõe tarefas tipadas, diagnostica falhas observadas e gera propostas de código/testes sem executar, escrever arquivos ou conceder autoridade; um avaliador separado mede candidatos em subprocesso controlado e uma síntese enumerativa bounded rejeita hipóteses que falham no holdout ou excedem seu orçamento explícito.
- Ledger de programação inspirado no REA: propostas só fecham quando casos positivos, negativos e malformados têm evidência autenticada e autoridade comparável.
- Auditoria crítica de erros e contrato executável que bloqueia alegações de vitória contra transformers ou de simbiose geral sem evidência equivalente.
- Revisão acadêmica sobre simbolismo puro: núcleo constitucional 100% simbólico; aprendizado limitado nas camadas de percepção e proposta.
- Manifestos de hardware e proveniência local.
- Documentação histórica desde o comunicador semântico original até a auditoria ASA.

## Foco atual: algoritmo

O objetivo imediato é concluir e falsificar o algoritmo Symbiotic Learning e seu meta-aprendizado. A conformidade C11 e o protocolo externo são tratados como implementações de biblioteca e testes de interoperabilidade; não são uma solicitação para iniciar validação física.

A superfície importável está em [`herus_symbiotic`](docs/91-BIBLIOTECA-PUBLICA-SYMBIOTIC-LEARNING.md), versão `0.1.0`. A API é proposal-only: importar e inferir não executa ações, acessa rede ou concede autoridade.

## O que ainda não existe

Ainda não existe prova de autonomia, alcance de rádio, consumo, temperatura, ergonomia, segurança física, benefício social, acessibilidade populacional, adaptação a qualquer dispositivo, linguagem aberta, execução externa ou simbiose geral.

## Verificação

```bash
./prove.sh --quiet
make -C research test
PYTHONPATH=research python3 -m research.bridge_causal
PYTHONPATH=research python3 -m research.bridge_product
python3 tools/provenance_audit.py --strict research/software_provenance_manifest.json
```

Na última verificação local, a pesquisa passou com **239 testes e um skip**, e o proof integral confirmou as invariantes host-only e de simulação. Em dados reais MIntRec, após corrigir um vazamento metodológico, a política risco–cobertura atingiu 60,50% de precisão seletiva com 30,83% de cobertura no ponto calibrado a 0,50; para metas de 70% ou mais, a cobertura foi zero. Um consenso entre Naive Bayes e protótipos elevou marginalmente a cobertura para 34,20% e a precisão seletiva para 60,61%. Na validação externa MInDS-14 em português, o consenso alcançou 94,38% de precisão seletiva com 90,82% de cobertura, mas o Naive Bayes manteve maior acurácia global (91,84%). A transferência lexical inglês→português falhou, mas um encoder multilíngue congelado elevou a acurácia cross-language para 48,68% com 99,01% de cobertura, sem rótulos portugueses; ainda ficou abaixo do Naive Bayes treinado localmente. A primeira adaptação não supervisionada por alinhamento de média foi rejeitada: produziu exatamente 48,68%, sem ganho sobre o encoder congelado. A matriz clássica no MInDS-14 mostrou Linear SVM e Random Forest em 95,92%, acima do baseline HERUS de 91,84%. Na comparação seletiva com alvo de 95%, Random Forest alcançou 98,95% de precisão seletiva com 96,94% de cobertura, superando o consenso HERUS de 94,38% e 90,82%. A rede tensorial Tensor-Train com 1.024 características e rank 64 atingiu 95,92%, empatando a SVM em acurácia, superando sua macro-F1 (95,66% contra 95,53%) e reduzindo a inferência medida de 6,70 ms para 1,04 ms. O híbrido encoder multilíngue congelado + cabeça tensorial alcançou 87,76%, pior que o Tensor-Train hash puro; a combinação foi rejeitada nesta configuração. No segundo dataset real, MIntRec com holdout temporal S06, o Tensor-Train rank 64 alcançou 42,23%, abaixo do Naive Bayes de 49,22%; a paridade no MInDS-14 não generalizou. A seleção adaptativa de representação, calibrada em S05 e refeita em S04+S05, elevou o resultado a 56,48% no S06, superando Naive Bayes em 7,26 pontos percentuais; a escolha ainda não coincide sempre com o melhor oracle. A seleção estável em duas validações temporais confirmou o mesmo teto e mostrou que estabilidade isolada não identifica sempre a melhor representação futura. O ensemble por votação foi testado sem vazamento: a votação dos três chegou a 57,25%, abaixo da regressão logística de 57,77%, portanto não foi contado como ganho. A combinação calibrada de margens manteve 56,48% de acurácia, mas elevou o macro-F1 de 47,13% para 48,02%, uma melhoria parcial de equilíbrio entre classes. A calibração seletiva formalizou margem, cobertura e risco: no S06, aceitou 22,80% dos casos com 93,18% de precisão e 6,82% de risco, abstendo-se do restante. A biblioteca agora também expõe DataScienceSkill, que audita datasets, detecta bloqueios, propõe baselines e define protocolos de validação sem executar pipelines ou conceder autoridade. A entrada recomendada agora é uma única fachada: `from herus_symbiotic import Herus`, que reúne dados, programação, observação, propostas e inspeção em uma interface didática. A matriz final agrega 18 modelos ou variantes em dois datasets reais e bloqueia explicitamente a alegação de vencedor universal: hoje o HERUS vence apenas no eixo seletivo de risco–cobertura, não em acurácia de cobertura total. O DistilBERT multilíngue forte atingiu 46,63% no MIntRec, melhor que o BERT tiny, mas ainda abaixo do Naive Bayes com 49,22%. Portanto o HERUS ainda não venceu universalmente os baselines, mas obteve seu primeiro empate forte de custo–desempenho. Esses resultados são dependentes do domínio e não demonstram superioridade geral. A campanha ampla, o holdout local, o harness black-box, a API pública, a auto-supervisão experimental, o baseline transformer, o probe de atalhos, a normalização limitada, a habilidade de programação proposal-only, o ciclo de reparo guiado por falha, o avaliador executável controlado, a síntese enumerativa bounded com orçamento explícito, o ledger de obrigações e o protocolo entre processos também separam acerto seguro de falso aceite. Isso é uma regressão interna e uma análise real, não uma prova de produto ou de campo.

## Documentos de decisão

| Documento | Papel |
|---|---|
| [Documento mestre](docs/00-HERUS-MASTER.md) | Tese histórica do comunicador semântico e da arquitetura física. |
| [Arquitetura finita](docs/48-ARQUITETURA-FINITA-E-LINGUAGEM.md) | Semântica controlada e linguagem subordinada. |
| [Semantic IR](docs/50-INTENT-COMPILER-E-SEMANTIC-IR.md) | Compilação de intenção e representação intermediária. |
| [API Symbiont v2](docs/51-API-SIMBIONTE-V2.md) | Fronteira host-independent e transferência finita. |
| [Simbiose útil](docs/52-DEFINICAO-SIMBIOSE-UTIL.md) | Contrato congelado de benefício e segurança. |
| [Holdout adversarial](docs/53-BENCHMARK-HOLDOUT-ADVERSARIAL.md) | Contraexemplos e falsificação do mecanismo. |
| [Observabilidade checked](docs/54-CORRECAO-CONTRATO-OBSERVABILIDADE.md) | Ledger e decisão fail-closed. |
| [Etapas finais](docs/55-ETAPAS-FINAIS-EXECUTOR-HUMANO.md) | Executor sintético, holdout estendido e protocolo social. |
| [Rodada decisiva](docs/56-RODADA-DECISIVA-HERUS-BRIDGE.md) | P0, campanha causal e HERUS Bridge. |
| [Direção soberana](docs/57-DIRECAO-SOBERANA-E-SIMBIONTE.md) | Soberania de dados, Symbiont transferível e futuro objetivo. |
| [Symbiotic Learning](docs/84-ALGORITMO-SYMBIOTIC-LEARNING.md) | Algoritmo bounded de indução de efeitos e transferência de Skills. |
| [Meta-Simbionte](docs/85-META-SIMBIONTE-E-HISTORICO-DE-SOLUCOES.md) | Histórico verificável e adaptação personalizada sem reutilização cega. |
| [Campanha ampla](docs/86-CAMPANHA-WIDE-SYMBIOTIC-LEARNING.md) | 100 casos, baselines e falso aceite separado do acerto. |
| [Holdout independente](docs/87-HOLDOUT-INDEPENDENTE-SYMBIOTIC.md) | 60 fixtures separados e oracle não fornecido ao learner. |
| [Host black-box](docs/88-HOST-BLACK-BOX-E-ROTACAO.md) | Estado parcial, ações renomeadas e rotação de interface. |
| [Protocolo externo](docs/89-PROTOCOLO-EXTERNO-E-ISOLAMENTO.md) | Host em processo separado, JSONL, corrupção e timeout fail-closed. |
| [Host JSONL v1](docs/90-PROTOCOLO-HERUS-HOST-JSONL-V1.md) | Contrato congelado de interoperabilidade entre learner e hospedeiro. |
| [Biblioteca pública](docs/91-BIBLIOTECA-PUBLICA-SYMBIOTIC-LEARNING.md) | API importável v0.1.0, proposal-only e sem efeitos colaterais. |
| [Benchmark real](docs/92-BENCHMARK-DADOS-REAIS-E-BASELINES.md) | MIntRec S04/S05→S06 contra baselines clássicos; revela cobertura baixa da memória simbiótica. |
| [Paradigmas de ML](docs/94-COMPARACAO-PARADIGMAS-ML-REAIS.md) | Comparação real com supervisionado, K-means e bandit proxy, sem alegar equivalência indevida. |
| [Transformer real](docs/96-TRANSFORMER-REAL-MINTREC.md) | BERT pequeno executado no mesmo holdout temporal, com custo e limites registrados. |
| [Simbolismo puro](docs/97-SIMBOLISMO-PURO-E-SYMBIOTIC-LEARNING.md) | Decisão baseada em papers sobre o que deve ser simbólico e onde o aprendizado é necessário. |
| [Probe de atalhos](docs/98-PROBE-ATALHOS-MINTREC.md) | Teste de estabilidade contra fillers em holdout real; revela dependência de superfície. |
| [Habilidade de programação](docs/99-HABILIDADE-PROGRAMACAO-JEV-INSPIRED.md) | Decomposição tipada e propostas de código/testes sem execução ou autoridade. |
| [Auditoria REA](docs/100-AUDITORIA-REA-E-LEDGER-DE-PROGRAMACAO.md) | Evidência autenticada, obrigações abertas e autoridade comparável para propostas de código. |
| [Avaliador real de programação](docs/101-AVALIADOR-REAL-DE-PROGRAMACAO.md) | Mede candidato correto, incorreto e não terminante em subprocesso controlado. |
| [Síntese enumerativa bounded](docs/102-SINTESE-ENUMERATIVA-BOUNDED.md) | Gera hipótese por gramática finita e rejeita ajuste que falha no holdout. |
| [Risco–cobertura MIntRec](docs/103-RISCO-COBERTURA-MINTREC.md) | Avalia abstenção e precisão seletiva em holdout temporal real, sem vazamento. |
| [Validação externa MInDS-14](docs/104-VALIDACAO-EXTERNA-MINDS14.md) | Repete a política em outro corpus real de intenções em português. |
| [Transferência cross-language](docs/105-TRANSFERENCIA-CROSS-LANGUAGE-MINDS14.md) | Testa inglês→português e registra a falha de generalização lexical. |
| [Encoder multilíngue](docs/106-ENCODER-MULTILINGUE-CROSS-LANGUAGE.md) | Mede representação congelada e transferência sem rótulos no idioma-alvo. |
| [Ablação de alinhamento](docs/107-ABLACAO-ALINHAMENTO-NAO-SUPERVISIONADO.md) | Registra a hipótese rejeitada de correção global sem rótulos. |
| [Matriz de baselines ML](docs/108-MATRIZ-BASELINES-ML-MINDS14.md) | Compara HERUS com Logistic Regression, SVM, k-NN e Random Forest reais. |
| [Matriz risco–cobertura](docs/109-MATRIZ-RISCO-COBERTURA-BASELINES.md) | Compara seletividade do HERUS contra baselines calibrados para 95% de precisão. |
| [Redes tensoriais](docs/110-REDES-TENSORIAIS-MINDS14.md) | Mede Tensor-Train compacto e a fronteira rank–custo–acurácia em dados reais. |
| [Híbrido encoder–tensorial](docs/111-ENCODER-TENSORIAL-HIBRIDO.md) | Testa a composição de encoder congelado e cabeça Tensor-Train. |
| [Tensor-Train no MIntRec](docs/112-TENSOR-TRAIN-MINTREC-TEMPORAL.md) | Testa transferência para 20 classes com holdout temporal S06. |
| [Seleção adaptativa no MIntRec](docs/113-SELECAO-ADAPTATIVA-MINTREC.md) | Seleciona a representação em S05 e refaz o treino antes do holdout S06. |
| [Seleção estável no MIntRec](docs/114-SELECAO-ESTAVEL-MINTREC.md) | Testa seleção por média e pior caso em validações temporais independentes. |
| [Ensemble no MIntRec](docs/115-ENSEMBLE-REPRESENTACOES-MINTREC.md) | Avalia votação de representações diversas sem vazamento temporal. |
| [Margens calibradas no MIntRec](docs/116-ENSEMBLE-MARGENS-CALIBRADAS-MINTREC.md) | Combina margens de palavra e caractere com pesos escolhidos antes do holdout. |
| [Calibração seletiva](docs/117-CALIBRACAO-SELETIVA-RISCO-COBERTURA-MINTREC.md) | Formaliza confiança, abstenção, risco e cobertura em dados temporais reais. |
| [Ciência de dados e ML](docs/118-HABILIDADE-CIENCIA-DE-DADOS-E-ML.md) | Audita datasets e propõe pipelines e baselines de ML com separação de autoridade. |
| [Fachada única Herus](docs/119-FACHADA-UNICA-HERUS.md) | Uma importação didática reúne análise, programação, observação, propostas e inspeção. |
| [Competição final de ML](docs/120-COMPETICAO-FINAL-ML-SCORECARD.md) | Scorecard real com 18 variantes, dois datasets e Pareto precisão–cobertura. |
| [Transformer forte e Manus](docs/121-TRANSFORMER-FORTE-E-COMPARACAO-MANUS.md) | DistilBERT multilíngue comparado no mesmo holdout; comparação direta com Manus permanece não executada. |
| [Incerteza estatística](docs/122-INCERTEZA-ESTATISTICA-E-PROXIMO-GATE.md) | Intervalos de Wilson e próximo gate para bootstrap pareado sem inventar significância. |
| [Auditoria crítica](docs/95-AUDITORIA-CRITICA-ERROS-E-CORRECOES.md) | Erros metodológicos registrados e regras permanentes contra overclaiming. |

## Código por papel

- [Firmware](firmware/README.md)
- [Pesquisa](research/README.md)
- [Simulador](sim/README.md)
- [Ferramentas](tools/README.md)
- [Índice completo de documentos](docs/README.md)

## Licença

Proprietary. Copyright © 2026 Gustavo Gonçalves. Todos os direitos reservados — veja [LICENSE](LICENSE).
