# Validação cross-domain do detector de shift

## Veredito

**Generalização mista; detector bloqueado para promoção.**

O mesmo detector lexical foi calibrado separadamente em dados limpos e avaliou holdouts reais de MIntRec e MInDS-14. Nenhum rótulo do holdout foi usado.

| Domínio | Ataque | Aceitação limpa | Aceitação atacada | Resultado |
|---|---|---:|---:|---|
| MIntRec | prefixo irrelevante | 67,88% | 72,02% | falha: aceitação aumentou |
| MIntRec | typo | 67,88% | 47,15% | detecta shift |
| MIntRec | deleção | 67,88% | 35,23% | detecta shift |
| MInDS-14 | prefixo irrelevante | 37,36% | 1,10% | detecta shift fortemente |
| MInDS-14 | typo | 37,36% | 20,88% | detecta shift |
| MInDS-14 | deleção | 37,36% | 19,78% | detecta shift |

## Decisão

O detector **não entra no caminho padrão**. A regra de promoção agora exige que nenhum domínio independente apresente aumento de aceitação sob um ataque comparável. MIntRec viola essa condição no prefixo irrelevante.

Além disso, a aceitação limpa é baixa, especialmente em MInDS-14. Isso mostra que um detector mais agressivo pode aumentar abstention sem produzir assurance útil.

## Claim permitido

O HERUS demonstrou uma ablação de detecção de shift com generalização **mista**. Não demonstrou robustez adversarial, segurança OOD ou SOTA.

Próximo avanço: substituir o detector lexical por uma combinação calibrada de margem do classificador, distância de representação e teste de invariância, mantendo a decisão fail-closed.
