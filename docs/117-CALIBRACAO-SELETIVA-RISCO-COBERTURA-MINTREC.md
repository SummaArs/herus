# Calibração seletiva, risco e cobertura

## Objetivo

O HERUS não deve ser obrigado a produzir uma decisão quando a evidência é fraca. A saída pode ser:

- **aceitar**: previsão acima do limiar calibrado;
- **abster-se**: solicitar revisão, mais evidência ou outro host.

## Matemática

Para uma entrada `x`, o classificador produz escores `s_1(x), ..., s_K(x)`.

A margem de confiança usada é:

```text
m(x) = maior_escore(x) - segundo_maior_escore(x)
```

Com limiar `τ`:

```text
aceitar(x) se m(x) >= τ
abster(x)  se m(x) < τ
```

A cobertura é:

```text
coverage(τ) = número de aceitos / número total
```

A precisão seletiva e o risco são:

```text
selective_accuracy(τ) = acertos entre aceitos / número de aceitos
risk(τ) = 1 - selective_accuracy(τ)
```

O limiar é escolhido somente em S05 para maximizar cobertura sob uma precisão mínima. S06 permanece oculto até a medição final.

## Resultado no MIntRec

| Meta de precisão na calibração | Cobertura S06 | Precisão S06 | Risco S06 |
|---:|---:|---:|---:|
| 70% | 45,34% | 84,00% | 16,00% |
| 80% | 39,12% | 86,09% | 13,91% |
| 90% | 28,76% | **90,99%** | 9,01% |
| 95% | 22,80% | **93,18%** | 6,82% |

## Leitura científica

O resultado não prova garantia de 95% fora da amostra. A calibração atingiu 95,38% em S05, mas caiu para 93,18% em S06. Isso mede uma limitação de transferência temporal e impede alegar garantia universal.

O avanço real é comportamental: o HERUS transforma incerteza em **abstenção explícita**, reduzindo risco nas decisões aceitas. Isso é mais compatível com assurance do que maximizar cobertura a qualquer custo.
