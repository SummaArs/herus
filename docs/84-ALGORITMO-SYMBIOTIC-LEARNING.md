# Symbiotic Learning — proposta de campo algorítmico

**Status:** hipótese de pesquisa v1; não é uma alegação de que um novo campo já foi aceito pela comunidade.

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

## Algoritmo v1 — Bounded Effect Induction

1. Capturar episódios públicos `(before, action, after, cost, risk)`.
2. Calcular o delta observável `Δ = after − before`.
3. Agrupar episódios pelo mesmo `Δ`.
4. Formar um protótipo somente quando o efeito for observável e a ação não for ambígua.
5. Em outro hospedeiro, sondar candidatos dentro de orçamento.
6. Propor a única ação que produz o mesmo `Δ`.
7. Retornar `ABSTAIN` quando houver alias, risco acima do limite, orçamento esgotado ou efeito não observado.
8. Manter a atualização reversível por snapshot e rollback.

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

Ainda não há evidência de superioridade sobre baselines. O próximo experimento deve comparar regras fixas, nearest-prototype e o algoritmo simbiótico em hospedeiros black-box desconhecidos, com os mesmos holdouts e métricas de falso aceite, abstention, custo e latência.

## Como importar e executar

```bash
PYTHONPATH=research python3 -c \
  'from symbiotic_learning import SymbioticLearner; print(SymbioticLearner().export())'

PYTHONPATH=research python3 -m unittest research.test_symbiotic_learning -v
```

## Veredicto atual

O HERUS agora tem uma **proposta de algoritmo próprio** que pode ser estudada, importada e comparada. Ainda não tem um novo campo científico estabelecido, nem prova de generalidade, benefício humano ou equivalência física. O nome só merece ser promovido depois de baselines independentes, holdouts adversariais, revisão externa e uma tarefa útil demonstrada.
