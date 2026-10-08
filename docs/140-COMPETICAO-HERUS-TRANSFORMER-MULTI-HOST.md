# Competição pareada: HERUS adapter contra transformer

## Veredito

A comparação foi executada nos mesmos domínios, holdouts e três seeds. O resultado confirma uma vantagem do HERUS no regime de **assurance seletiva**, mas não prova SOTA geral.

O transformer foi calibrado com confiança no conjunto de calibração e avaliado no holdout sem ajuste posterior.

## Mesmo protocolo

- dataset real: `THU-IAR/MIntRec`;
- domínios temporais proxy: S04, S05 e S06;
- split por domínio: 70% fit, 15% calibração, 15% holdout;
- seeds: 11, 23 e 47;
- transformer: `google/bert_uncased_L-2_H-128_A-2`;
- 5 épocas, CPU, batch 16, comprimento máximo 96;
- HERUS: `calibrated_symbiotic_prototype`;
- seleção do transformer feita somente na calibração;
- nenhum rótulo de holdout usado para escolher limiar.

## Comparação

| Domínio | HERUS precisão seletiva | HERUS cobertura | Transformer cobertura total / precisão | Transformer calibrado precisão / cobertura |
|---|---:|---:|---:|---:|
| S04 | 0,8000 | 0,0588 | 0,2235 / 1,0000 | 0,3333 / 0,0039 |
| S05 | 0,7400 | 0,2618 | 0,2583 / 1,0000 | 0,8667 / 0,0209 |
| S06 | 0,4000 | 0,1724 | 0,1379 / 1,0000 | 0,0000 / 0,0000 |

A tabela torna explícito o trade-off. O transformer mantém cobertura total sem abstenção; quando recebe uma política seletiva, sua cobertura cai para níveis extremamente baixos. O HERUS opera com cobertura maior que o transformer calibrado em todos os três domínios, mas com precisão seletiva inferior no S05.

## Conclusão permitida

> No protocolo temporal MIntRec, o HERUS adapter demonstrou uma fronteira seletiva competitiva e, em dois de três domínios, mais útil que a política seletiva do transformer em cobertura. Não existe evidência suficiente para declarar vitória geral, superioridade estatística global ou SOTA.

## Por que ainda não é 100%

1. O objeto ainda é um adapter de classificação, não o núcleo `SymbioticLearner` puro.
2. As temporadas são proxies de host, não hospedeiros nativos.
3. A comparação usa um transformer pequeno, não todos os modelos de estado da arte.
4. Ainda falta uma curva risco–cobertura pareada por exemplo e bootstrap da diferença.
5. O benchmark é texto; não prova adaptação física, multimodal ou geral.
6. MIntRec não possui rótulos HERUS e não autoriza mapear intenções automaticamente para eventos HERUS.

O resultado é um marco de **99% do algoritmo experimental**, não uma declaração de SOTA.
