# Resultado multi-host real: MIntRec

## Decisão

O protocolo multi-host foi executado com dados reais da revisão pública `THU-IAR/MIntRec`, usando S04, S05 e S06 como **domínios temporais proxy**. O gate aceitou a integridade estrutural:

```text
READY_FOR_ANALYSIS
```

Isso não autoriza SOTA. O próprio gate mantém `claim_allowed=false` porque um protocolo válido não é evidência de estado da arte.

## Protocolo

- dataset: `THU-IAR/MIntRec`;
- fonte: API pública do Hugging Face Dataset Server;
- hosts proxy: temporadas S04, S05 e S06;
- split por host: 70% ajuste, 15% calibração, 15% holdout;
- seeds: 11, 23 e 47;
- candidato: `calibrated_symbiotic_prototype`;
- baseline forte: máximo entre Naive Bayes e centróide por acurácia seletiva;
- nenhum leakage detectado;
- o holdout não participa da calibração.

## Resultado por domínio

| Domínio temporal | Precisão seletiva do candidato | Melhor baseline | Diferença | Cobertura do candidato |
|---|---:|---:|---:|---:|
| S04 | 0,8000 | 0,3765 | +0,4235 | 0,0588 |
| S05 | 0,7400 | 0,4817 | +0,2583 | 0,2618 |
| S06 | 0,4000 | 0,3103 | +0,0897 | 0,1724 |

Os resultados são idênticos entre as três seeds neste adapter determinístico.

## Interpretação correta

O candidato apresenta **precisão seletiva superior ao melhor baseline clássico nos três domínios**, mas com cobertura baixa. Portanto, a conclusão defensável é:

> O adapter demonstrou um comportamento seletivo promissor em três holdouts temporais reais, abstendo-se da maioria dos exemplos. Isso não prova superioridade de classificação geral, simbiose geral, nem SOTA.

A acurácia não seletiva continua baixa nos domínios. A vantagem observada é de assurance seletiva, não de cobertura ampla.

## Limites decisivos

1. Temporada não é um hospedeiro físico; é apenas um proxy temporal de domínio.
2. Os rótulos MIntRec não são eventos HERUS.
3. O candidato é um adapter calibrado, não o núcleo `SymbioticLearner` puro.
4. A comparação ainda não inclui transformer forte neste mesmo protocolo multi-host.
5. Não há speaker-independent split verificado.
6. O resultado não sustenta a afirmação SOTA.

O ledger completo está em `research/evidence/multi_host_real_mintrec_v1.json`.
