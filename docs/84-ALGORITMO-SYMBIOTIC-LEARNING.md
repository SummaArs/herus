# Symbiotic Learning — proposta de campo algorítmico

**Status:** hipótese de pesquisa v2; não é uma alegação de que um novo campo já foi aceito pela comunidade.

## Tese

Os paradigmas comuns de aprendizado alteram um modelo a partir de exemplos, rótulos, objetivos ou recompensas. **Symbiotic Learning** propõe uma pergunta adicional: como uma competência aprendida pode permanecer útil quando o corpo computacional, o vocabulário de ações e as restrições do hospedeiro mudam?

A unidade aprendida não é o nome de uma ação. É uma relação observável:

```text
estado antes + ação + estado depois → efeito abstrato
```

A transferência procura uma ação no hospedeiro novo que produza o mesmo efeito sob as mesmas pré-condições observáveis.

## Definição operacional

> **Symbiotic Learning é aprendizado incremental, bounded e reversível de competências abstratas, ancoradas em efeitos observáveis, para rebind entre hospedeiros heterogêneos sob contratos explícitos de custo, risco, evidência e autoridade.**

A definição exige cinco propriedades:

1. **corpo variável:** o hospedeiro pode mudar;
2. **efeito invariável:** a competência é descrita por consequência observável;
3. **descoberta black-box:** o mapa interno do hospedeiro não é usado;
4. **abstenção:** ambiguidade, efeito ausente, risco ou orçamento bloqueiam;
5. **autoridade externa:** aprender e propor não concede permissão para executar.

## Algoritmo v2 — Contextual Bounded Effect Induction

1. Capturar episódios públicos `(context, before, action, after, cost, risk, step)`.
2. Calcular o delta observável `Δ = after − before`.
3. Agrupar por `(contexto, Δ)`, não apenas por `Δ`.
4. Manter evidência negativa fora do repertório promovível.
5. Formar um protótipo somente quando o efeito for observável e a ação não for ambígua.
6. Detectar deriva quando episódios equivalentes ficam separados além da janela temporal.
7. Em outro hospedeiro, sondar candidatos dentro de orçamento.
8. Propor a única ação que produz o mesmo `Δ` sob o contexto compatível.
9. Retornar `ABSTAIN` em alias, deriva, risco alto, orçamento esgotado ou efeito ausente.
10. Manter a atualização reversível por snapshot e rollback.

O algoritmo não executa ações, acessa rede, recebe credenciais ou promove autoridade.

## Relação com os paradigmas existentes

| Paradigma | Sinal de aprendizado | O que o Symbiotic Learning adiciona |
|---|---|---|
| Supervisionado | rótulo | efeito transferível entre hospedeiros |
| Não supervisionado | estrutura nos dados | estrutura condicionada a estados e consequências |
| Auto-supervisionado | sinal gerado pelos próprios dados | mudança de corpo e de vocabulário como parte do problema |
| Reforço | recompensa | restrições de autoridade, risco e custo antes da ação |
| **Symbiotic Learning** | episódio observado e contrato de efeito | rebind de competência com abstention e autoridade separada |

Essa tabela é uma hipótese taxonômica, não uma prova de superioridade.

## O que já foi implementado

`research/symbiotic_learning.py` é um pacote Python importável com:

- episódios tipados;
- indução incremental de protótipos de efeito;
- transferência por efeito, não por nome;
- orçamento de observações e custo;
- limite de risco;
- contexto e pré-condições observáveis;
- evidência negativa não promovível;
- detecção de deriva temporal;
- abstention determinística;
- snapshot e rollback;
- exportação serializável;
- nenhuma função de execução.

## Falsificação

A tese falha se, em holdout congelado, o algoritmo:

- aceitar alias de efeitos;
- aceitar efeito não observado;
- ignorar risco ou orçamento;
- transferir para uma ação errada;
- perder o estado após rollback;
- precisar do mapa interno do hospedeiro;
- transformar confiança em autoridade.

O benchmark v2 já compara regras por nome, correspondência simples por efeito e o algoritmo contextual nos mesmos cinco casos locais. Isso é apenas um harness de regressão: ainda não é evidência de superioridade. A próxima avaliação deve usar hospedeiros black-box desconhecidos, holdouts não vistos e métricas de falso aceite, abstention, custo, latência e deriva.

## Como importar e executar

```bash
PYTHONPATH=research python3 -c \
  'from symbiotic_learning import SymbioticLearner; print(SymbioticLearner().export())'

PYTHONPATH=research python3 -m unittest research.test_symbiotic_learning research.test_symbiotic_learning_benchmark -v
PYTHONPATH=research python3 -m research.symbiotic_learning_benchmark
```

## Veredicto atual

O HERUS agora tem uma **proposta de algoritmo próprio v2** que pode ser estudada, importada e comparada. Ainda não tem um novo campo científico estabelecido, nem prova de generalidade, benefício humano ou equivalência física. O nome só merece ser promovido depois de baselines independentes fortes, holdouts adversariais, revisão externa e uma tarefa útil demonstrada.
