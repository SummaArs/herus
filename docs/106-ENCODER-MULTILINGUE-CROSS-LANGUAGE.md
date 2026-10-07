# Encoder multilíngue congelado

## Hipótese

A falha anterior poderia estar na representação lexical, não necessariamente na política de consenso. Para testá-la, foi usado `distilbert/distilbert-base-multilingual-cased` como encoder congelado.

Não houve fine-tuning, atualização de pesos ou uso de rótulos portugueses.

## Protocolo

- ajuste: MInDS-14 `en-US`, 416 exemplos;
- calibração: `en-US`, 147 exemplos;
- holdout: MInDS-14 `pt-PT`, 604 exemplos;
- 14 classes compartilhadas pelo dataset;
- representação: média mascarada dos embeddings do encoder;
- decisão: protótipo por classe e limiar calibrado em inglês.

## Resultado

| Sistema | Acurácia no português | Cobertura |
|---|---:|---:|
| Naive Bayes lexical cross-language | 8,11% | 100% |
| Consenso lexical cross-language | 2,92% de precisão seletiva | 11,92% |
| Encoder multilíngue congelado | **48,68%** | **99,01%** |
| Naive Bayes treinado em português | 91,84% | 100% |

O encoder multilíngue elevou a transferência de 8,11% para 48,68% sem rótulos no idioma-alvo. Isso é um avanço real, mas não é paridade com o modelo supervisionado local.

## Interpretação

A hipótese foi parcialmente confirmada: a representação multilíngue resolve parte da barreira lexical. Ainda permanecem erros de alinhamento de intenção, domínio e decisão. O encoder não é o algoritmo HERUS completo; é um componente de representação que pode ser usado como proposta para o núcleo simbiótico.

O próximo teste deve comparar adaptação não supervisionada no português contra o encoder congelado, mantendo o holdout rotulado intocado.

## Limites

Um único par de idiomas foi usado. Não houve áudio, independência por locutor ou fine-tuning. O benchmark não prova compreensão geral nem simbiose geral.

A evidência está em `research/evidence/multilingual_encoder_minds14_v1.json`.
