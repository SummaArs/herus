# Transformer bloqueado pelo contrato universal

## Experimento

O roteador HERUS recebeu três famílias de candidatos no MInDS-14:

- Naive Bayes supervisionado;
- centróide não supervisionado;
- transformer fine-tuned `google/bert_uncased_L-2_H-128_A-2`, seed 11.

A divisão foi mantida: fit, calibração separada e holdout de 98 exemplos.

## Resultado

| Método | Acurácia no holdout | Status no roteador |
|---|---:|---|
| Naive Bayes | 91,84% | admitido |
| Centróide | 87,76% | admitido |
| Transformer | 18,37% | bloqueado |
| Roteador universal | **92,86%** | aceito |

O transformer não atingiu a precisão mínima de calibração do contrato (`min_precision=0.80`). Por isso, não recebeu limiar seguro e não participou da decisão do holdout.

## Veredito

O HERUS não fingiu que o transformer era um candidato válido. Ele detectou que o modelo não estava confiável no regime avaliado e o excluiu de forma explícita.

Isso demonstra uma propriedade importante do meta-algoritmo:

> **O HERUS não precisa escolher sempre um paradigma; ele pode rejeitar um candidato inteiro quando sua evidência não satisfaz o contrato do hospedeiro.**

A acurácia do roteador permaneceu 92,86%, igual ao experimento de dois candidatos, porque o transformer não foi admitido.

## Limites

- apenas um transformer pequeno e uma seed foram avaliados nesta integração;
- não é prova contra transformers em geral;
- o roteador ainda não integrou uma referência LLM com calibração própria;
- o resultado não é claim SOTA geral.

O bloqueio é um resultado de assurance, não uma vitória de acurácia contra todos os transformers.
