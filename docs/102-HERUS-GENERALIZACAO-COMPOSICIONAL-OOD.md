# HERUS — Generalização Composicional Fora da Distribuição

**Estado:** prova local concluída em host; hardware e fala humana continuam pendentes.

**Objetivo:** medir se o núcleo generativo recompõe elementos conhecidos em estruturas inéditas sem transformar ausência, conflito ou orçamento excedido em fluência enganosa.


## Motivação acadêmica

A avaliação in-distribution é insuficiente para um sistema que pretende funcionar como inteligência pessoal. O COGS foi proposto justamente para separar teste comum de gaps sistemáticos, como combinações novas de estruturas sintáticas conhecidas e combinações novas de palavras com estruturas familiares. No resumo do trabalho, modelos Transformer e LSTM alcançaram 96–99% in-distribution, mas somente 16–35% nos gaps de generalização, com sensibilidade de aproximadamente 6–8 pontos a seed [1]. Portanto, um resultado alto em casos conhecidos não pode ser apresentado como raciocínio composicional.

O gSCAN estende essa preocupação a linguagem grounded em estados de um mundo situado. Seu protocolo testa se regras linguísticas podem ser aplicadas a situações novas, como interpretar adjetivos relativamente ao estado atual e combinar advérbios com verbos inéditos; os baselines descritos falharam quando a generalização dependia de regras composicionais sistemáticas [2]. No HERUS, o “estado situado” é o contexto local tipado: intenção, geração, memória corrente, autoridade, sessão física e canal háptico.

O CFQ propõe maximizar a divergência entre compostos de treino e teste mantendo pequena a divergência entre átomos. O trabalho relata uma correlação negativa forte entre compound divergence e acurácia [3]. O benchmark do HERUS adota a mesma ideia de forma controlada: os símbolos e operadores permanecem conhecidos, enquanto entidades, profundidade, encadeamentos e combinações são reservados para casos held-out.

## Contrato experimental

O contrato machine-readable está em [`research/compositional_ood_contract_v1.json`][4], e os casos determinísticos estão em [`research/benchmarks/compositional_ood/cases.jsonl`][5]. Não há geração aleatória nem amostragem dependente de seed. Cada caso possui uma resposta estrutural esperada: `DIRECT`, `DERIVED`, `NO_EVIDENCE`, `CONTRADICTION`, `AMBIGUOUS` ou `LIMIT`.

| Regime | Pergunta científica | Casos |
|---|---|---:|
| In-distribution | O reasoner preserva as capacidades básicas já conhecidas? | 2 |
| Held-out | Átomos e regras conhecidos são recompostos em estruturas inéditas? | 7 |
| Safety | Ausência, conflito, ambiguidade e limite continuam fail-closed? | 3 |
| Setup C11 | O fechamento do reasoner e as interfaces do gerador compilam e executam? | 1 |

O benchmark usa quatro fatos de parentesco conhecidos, uma regra de duas premissas para `grand`, uma regra unary para `ancestor` e uma regra de extensão que recompõe `ancestor` com `parent`. Com isso, a cadeia Alice→Bob→Cara→Davi→Erin permite testar profundidades diferentes sem introduzir conhecimento textual externo.

## Resultado reproduzível

A suíte C11 em [`firmware/core/test_compositional_ood.c`][6] passou integralmente. O wrapper em [`tools/compositional_ood_benchmark.py`][7] separa o resultado por regime e mantém a contagem de 12 casos classificados distinta do caso de setup.

```text
COMPOSITIONAL OOD BENCHMARK: PASS
  in_distribution: 2/2
  held_out: 7/7
  safety: 3/3
  structured_exact: 12/12
  c11_suite: 13/13
```

O gerador textual local também foi atravessado em dois casos: uma composição ancestral held-out foi linearizada sem lexema ausente e uma composição ausente produziu abstention explícita. Isso demonstra uma superfície textual bounded apoiada em resultado tipado; não demonstra geração aberta de linguagem natural.

## Frente sabotadora

A campanha em [`tools/test_compositional_ood_redteam.py`][8] compilou mutantes reais do reasoner e verificou se a suíte os derrubava. Foram removidos, individualmente, o bloqueio de contradição, a abstention por ausência, a marcação de ambiguidade, o limite de derivação, a profundidade registrada e as raízes de provenance.

```text
COMPOSITIONAL OOD REDTEAM: 6/6 critical mutants killed
```

O valor do redteam é negativo e diagnóstico: ele não prova que o reasoner cobre todas as falhas possíveis. Prova somente que os seis controles declarados são necessários para os casos escolhidos e que a suíte não passa silenciosamente quando cada um é removido.

## Interpretação para o HERUS

| O que foi demonstrado | O que ainda não foi demonstrado |
|---|---|
| Recombinação de fatos e regras conhecidos em profundidade maior | Compreensão de frases livres em português |
| Preservação de `DIRECT` versus `DERIVED` | Conhecimento aberto de mundo |
| Abstention para ausência, conflito, ambiguidade e budget | Fluência comparável à de uma LLM |
| Proof roots bounded e profundidade de derivação | Planejamento geral de longo horizonte |
| Linearização de resultado tipado conhecido | Fala, escuta, WER ou interação humana |
| Execução determinística no host | Latência, energia e RAM reais no ESP32-S3 |

A melhoria é relevante para a arquitetura do HERUS porque desloca a pergunta de “o sistema respondeu uma frase?” para “ele recompôs uma estrutura nova preservando prova, limites e autoridade?”. Ainda assim, o resultado não autoriza a alegação de equivalência com uma LLM. O reasoner atual é uma base composicional verificável, não um modelo geral de linguagem.

## Próximo limite técnico

O próximo avanço deve ampliar esse protocolo para uma camada de linguagem estruturada: paráfrases controladas, ordem variável de argumentos, negação, elipse limitada, referências anafóricas bounded e composição de consultas multi-turno. Cada extensão precisa manter um alvo estrutural canônico, uma classe de abstenção e uma campanha adversarial. Só depois será possível medir se o HERUS está deixando de ser apenas um sistema simbólico e começando a oferecer uma interface generativa local mais aberta.

## Referências

[1]: https://aclanthology.org/2020.emnlp-main.731/ "Kim & Linzen, COGS, EMNLP 2020"

[2]: https://proceedings.neurips.cc/paper_files/paper/2020/hash/e5a90182cc81e12ab5e72d66e0b46fe3-Abstract.html "Ruis et al., gSCAN, NeurIPS 2020"

[3]: https://arxiv.org/abs/1912.09713 "Keysers et al., Measuring Compositional Generalization, ICLR 2020"

[4]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/research/compositional_ood_contract_v1.json "Contrato OOD do HERUS"

[5]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/research/benchmarks/compositional_ood/cases.jsonl "Corpus determinístico OOD do HERUS"

[6]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/firmware/core/test_compositional_ood.c "Suíte C11 de composição OOD"

[7]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/tools/compositional_ood_benchmark.py "Wrapper do benchmark OOD"

[8]: https://github.com/SummaArs/herus/blob/feat/herus-semantic-compiler/tools/test_compositional_ood_redteam.py "Redteam do benchmark OOD"
