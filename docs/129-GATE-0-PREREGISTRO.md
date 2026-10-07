# Gate 0 — pré-registro do experimento HERUS

**Status:** protocolo congelado; execução bloqueada até o Gate 1.  
**Progresso algorítmico:** 94,0%.

## Objetivo

Substituir a meta vaga de “vencer os algoritmos atuais” por um endpoint falsificável no holdout temporal MIntRec S06.

## Contrato congelado

- Dataset: `THU-IAR/MIntRec`.
- Ajuste: S04 e S05.
- Holdout: S06, esperado com 386 exemplos.
- Chave de união: `example_id`; união posicional é proibida.
- Comparadores: Naive Bayes multinomial e DistilBERT multilíngue.
- Seeds: 17, 29 e 43.
- Endpoint primário: macro-F1 em cobertura total.
- Abstention conta como não-acerto no endpoint primário.
- Margem prática: 0,02.
- Critério: limite inferior do intervalo pareado de 95% acima da margem contra cada comparador.
- Bootstrap: 10.000 reamostragens por exemplo, ou por diálogo se houver chave de agrupamento.
- Seleção: somente em S05; S06 permanece intocado.

O endpoint seletivo é secundário e exige cobertura mínima de 20%, numerador, denominador e limite inferior de Wilson explicitamente reportados.

## O que ainda bloqueia a execução

O identificador da implementação HERUS ainda não foi congelado. O experimento não pode ser executado como se `context_memory`, `consensus_selective` e o pipeline Symbiotic Learning v2 fossem o mesmo objeto.

O Gate 1 precisa primeiro produzir:

1. snapshot e checksum do dataset;
2. manifesto dos 386 `example_id` do S06;
3. ledger por ID e por seed;
4. definição exata do adaptador textual HERUS completo;
5. versões, checkpoints, ambiente e custos.

O validador retorna `PASS_FOR_PROTOCOL_ONLY`, mas `execution_authorized: false`. Isso é intencional: um protocolo bom não substitui um objeto experimental bem definido.

## Claims bloqueados

Continuam bloqueados: SOTA geral, superioridade universal, vitória sobre transformers, eficiência universal, simbiose geral, assurance de produção e AGI.
