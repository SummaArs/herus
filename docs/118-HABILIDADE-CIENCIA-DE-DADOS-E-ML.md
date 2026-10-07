# Habilidade de ciência de dados e ML do HERUS

O HERUS agora possui uma camada importável para atuar como **analista de dados e engenheiro de ML proposal-only**.

Ela não executa pipelines, não acessa rede, não publica modelos e não concede autoridade. Recebe registros fornecidos pelo chamador e produz um plano auditável.

## O que analisa

- schema e colunas;
- valores ausentes;
- duplicatas;
- ausência ou degeneração do rótulo;
- desbalanceamento de classes;
- risco de split aleatório inadequado;
- necessidade de split temporal ou por entidade;
- baselines supervisionados simples;
- métricas de classe, cobertura-risco, latência e memória;
- testes adversariais e holdout fora de distribuição.

## Contrato

```python
from herus_symbiotic import DataScienceSkill

plan = DataScienceSkill().analyze(
    ({"text": "...", "label": "intent_a"},),
    label_column="label",
    objective="classify intent",
)
```

O resultado é `PROPOSE` quando o dataset tem estrutura mínima ou `ABSTAIN` quando há um bloqueio, como rótulo ausente ou apenas uma classe.

## Método científico incorporado

1. congelar schema e proveniência;
2. auditar duplicatas e valores ausentes;
3. escolher split por tempo ou entidade quando aplicável;
4. ajustar pré-processamento somente no treino;
5. comparar maioria, regressão logística, SVM e baseline de abstenção;
6. medir accuracy, macro-F1, precisão por classe, risco-cobertura, latência e memória;
7. reservar holdout real e intocado;
8. registrar falhas e não transformar uma métrica em alegação universal.

A habilidade é o primeiro passo para o HERUS atuar como um **engenheiro de ML verificável**, usando o mesmo princípio central do projeto: proposta separada de autoridade.
