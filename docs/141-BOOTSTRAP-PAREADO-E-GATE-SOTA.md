# Bootstrap pareado e gate final de SOTA

## Resultado estatístico

Foram comparados os ledgers por exemplo do HERUS adapter e do transformer pequeno, usando seed pareada 11 e 4.000 reamostragens bootstrap.

| Domínio | Delta acurácia bruta HERUS − transformer | IC95% | Delta precisão seletiva HERUS − transformer | IC95% |
|---|---:|---:|---:|---:|
| S04 | -0,1413 | [-0,2353; -0,0471] | +0,7939 | [+0,3333; +1,0000] |
| S05 | -0,0103 | [-0,0681; +0,0471] | +0,1166 | [-0,3621; +0,8250] |
| S06 | -0,0525 | [-0,1379; +0,0345] | +0,3974 | [+0,1000; +0,7273] |

## Leitura correta

O HERUS perde em acurácia bruta nos três domínios. Isso é esperado em parte porque ele se abstém de muitos exemplos e a métrica bruta conta abstenções como erro.

Na precisão seletiva, o HERUS apresenta vantagem pontual nos três domínios. Contudo, a vantagem não é estatisticamente robusta no S05: o intervalo de confiança inclui zero.

A afirmação defensável é:

> O HERUS apresenta um mecanismo de assurance seletiva promissor, com vantagem estatística em dois dos três domínios temporais avaliados contra o transformer pequeno, ao custo de cobertura reduzida.

Isso não é SOTA geral.

## Gate final

O novo gate retorna `BLOCKED` porque ainda faltam:

- dataset independente;
- comparação com baseline atual de estado da arte;
- isolamento do núcleo HERUS sem adapter;
- curva risco–cobertura em cobertura equivalente;
- bootstrap pareado multi-dataset.

Mesmo quando todos esses itens forem preenchidos, o gate primeiro retornará `ELIGIBLE_FOR_INDEPENDENT_REVIEW`, não aceitação automática de SOTA.

O claim SOTA deve depender de uma revisão independente e de um protocolo pré-registrado.
