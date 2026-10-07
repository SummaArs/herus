# Incerteza estatística e próximo gate

A rodada adiciona intervalos de Wilson de 95% às métricas de acurácia, cobertura e precisão seletiva já publicadas.

## Resultado metodológico

Os intervalos são calculados a partir das contagens publicadas. Isso é melhor que apresentar percentuais pontuais sem incerteza, mas não equivale a um bootstrap pareado completo.

O HERUS ainda não arquiva os vetores de previsão por exemplo para todos os modelos. Portanto, o relatório **não inventa** bootstrap, teste de McNemar ou significância pareada.

## Exemplo principal

- HERUS seletivo: 88 decisões aceitas em 386 casos; 82 corretas.
- A precisão observada é 93,18% entre aceitos.
- A cobertura observada é 22,80%.
- Os intervalos de Wilson estão no artefato JSON reproduzível.

## Próximo gate de 100%

Para uma comparação estatística forte, cada benchmark deve salvar:

1. ID do exemplo;
2. rótulo verdadeiro;
3. previsão de cada modelo;
4. decisão de abstenção;
5. seed e versão do código.

Só então podemos executar bootstrap pareado, McNemar, intervalos para Macro-F1 e testes entre HERUS, transformers e baselines clássicos.

A porcentagem de conclusão aumenta pela qualidade da medição, não por declarar vitória antes da evidência.
