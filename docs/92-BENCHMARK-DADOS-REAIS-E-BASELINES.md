# Benchmark real: MIntRec contra baselines

## Pergunta

A memória contextual finita do Symbiotic Learning melhora uma tarefa real de classificação em relação a métodos simples usados em aprendizado de máquina?

## Dados

Fonte acessada:

- Dataset: `THU-IAR/MIntRec`
- API: `https://datasets-server.huggingface.co/rows`
- Registros reais: 2.224
- Rótulos: 20 intenções
- Treino: temporadas S04 e S05, 1.838 registros
- Holdout: temporada S06, 386 registros

O mirror público expõe apenas um split `train`; portanto, o holdout S06 foi criado por temporada, não por sorteio. Isso evita vazamento de segmentos entre treino e teste, mas não é o split oficial usado no paper do MIntRec.

Nenhum rótulo MIntRec foi convertido em evento HERUS. O experimento mede classificação textual no domínio original e não pode ser apresentado como prova de simbiose geral.

## Métodos

- maioria: baseline de frequência;
- Naive Bayes multinomial: classificador probabilístico clássico;
- 1-NN por similaridade cosseno: memória de vizinho mais próximo;
- centróide por similaridade cosseno: protótipo de classe;
- memória contextual finita simbiótica: referência exata de contexto, com abstenção quando não há correspondência.

As implementações são determinísticas e usam somente a biblioteca padrão do Python. O objetivo é tornar as comparações auditáveis, não simular uma implementação de uma biblioteca externa.

## Resultado

| Método | Acurácia | Macro-F1 | Cobertura | Acurácia seletiva |
|---|---:|---:|---:|---:|
| Naive Bayes multinomial | 49,22% | 37,44% | 100% | 49,22% |
| 1-NN cosseno | 39,90% | 32,20% | 100% | 39,90% |
| Centróide cosseno | 37,31% | 34,02% | 100% | 37,31% |
| Maioria | 11,92% | 1,06% | 100% | 11,92% |
| Memória simbiótica finita | 6,22% | 11,88% | 6,22% | 100% |

## Interpretação crítica

A memória simbiótica não compete atualmente com Naive Bayes ou 1-NN como classificador textual geral. Ela encontra poucas correspondências exatas, mas quando propõe dentro de sua cobertura não erra nesse holdout.

Isso demonstra uma propriedade de segurança:

> Cobertura baixa, precisão seletiva alta.

Não demonstra capacidade geral. Para se tornar um algoritmo competitivo, o HERUS precisa aprender representações transferíveis de contexto e efeito sem transformar a abstenção em uma forma de simplesmente não resolver a tarefa.

## Correção de direção

O resultado invalida qualquer narrativa de que a versão atual já supera classificadores convencionais. O próximo trabalho deve medir a fronteira entre:

1. cobertura;
2. precisão seletiva;
3. custo de sondagem;
4. detecção de deriva;
5. falsos aceites;
6. generalização para contexto novo.

A hipótese forte do HERUS só será interessante se aumentar cobertura mantendo controle explícito de risco e abstention.
