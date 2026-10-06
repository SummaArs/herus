# Validação externa em MInDS-14

## Protocolo

Foi executado o mesmo núcleo de comparação em uma segunda fonte real, independente do MIntRec:

- dataset: `PolyAI/minds14`;
- configuração: `pt-PT`;
- fonte: API pública do Hugging Face;
- 604 transcrições reais;
- 14 classes de intenção;
- 417 exemplos para ajuste, 89 para calibração e 98 para holdout;
- partição determinística dentro de cada classe por ordem do caminho de origem.

A calibração seleciona o limiar; o holdout não participa dessa escolha.

## Resultado

| Método | Cobertura | Acurácia / precisão seletiva |
|---|---:|---:|
| Naive Bayes | 100% | 91,84% de acurácia |
| Protótipo por cosseno | 100% | 87,76% de acurácia |
| Consenso seletivo NB + protótipo | 90,82% | 94,38% de precisão seletiva |

O consenso não vence o Naive Bayes em acurácia global: abstém em 9,18% dos casos. Porém, entre as propostas aceitas, apresenta precisão maior neste holdout.

## Interpretação

O resultado é uma validação externa positiva da política de consenso, não uma prova de simbiose geral. O padrão observado no MIntRec — ganho seletivo acompanhado de perda de cobertura — não desaparece, mas é muito melhor em MInDS-14. Isso indica dependência forte do domínio e impede uma porcentagem universal baseada em um único corpus.

## Limites

A fonte expõe um único split `train`; a partição foi criada localmente. Não foi verificada independência por locutor. O áudio não foi decodificado: somente as transcrições foram usadas. Os rótulos MInDS-14 não são eventos HERUS e não houve mapeamento automático para comandos. É necessária uma validação por locutor e uma tarefa com rótulos próprios do HERUS.

A evidência bruta está em `research/evidence/minds14_real_benchmark_v1.json`.
