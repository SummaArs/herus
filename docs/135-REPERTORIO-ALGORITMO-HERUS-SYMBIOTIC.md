# Repertório open source do HERUS Symbiotic

**Status:** mapa científico e de engenharia. Não é uma alegação de superioridade.

## 1. Objetivo

O HERUS é tratado como um **algoritmo de IA em evolução**, não como um produto físico nem como um slogan de AGI.

A pergunta central é:

> Pode um algoritmo aprender e otimizar uma competência condicionada ao hospedeiro, preservando evidência, risco, custo, autoridade e capacidade de abstenção?

A resposta ainda está em investigação.

## 2. Forma algorítmica

Uma forma de trabalho para o HERUS é:

```text
representação: z_t = Encode(x_t, host_t, constraints_t)
proposta:      a_t = Policy(z_t, memory_t) ou ABSTAIN
observação:    y_t = Observe(a_t, host_t)
utilidade:     J_t = U(a_t, y_t) - λR_t - μC_t - νA_t - ρE_t
atualização:   θ_{t+1} = Update(θ_t, z_t, a_t, y_t, J_t)
```

Onde:

- `x_t`: observação do problema;
- `host_t`: capacidades e limitações do hospedeiro;
- `constraints_t`: contratos de risco, custo e autoridade;
- `memory_t`: evidência episódica versionada;
- `U`: utilidade observada;
- `R`: risco;
- `C`: custo;
- `A`: violação ou ausência de autoridade;
- `E`: insuficiência/conflito da evidência;
- `θ`: estado aprendido do algoritmo.

Essa formulação é um alvo de desenvolvimento. O núcleo v2 implementa apenas uma parte: memória episódica, indução contextual, proposta limitada e abstenção.

## 3. Famílias do repertório

| Família | Sinal de aprendizagem | Implementação/registro | Papel no HERUS |
|---|---|---|---|
| Supervisionado | rótulo | baselines MIntRec/MInDS-14 | controle de precisão e cobertura |
| Não supervisionado | estrutura | agrupamentos e representações | controle de organização sem rótulo |
| Auto-supervisionado | sinal derivado dos dados | encoders/representações | controle de pré-treinamento |
| Reinforcement learning | recompensa e retorno | bandit/RL proxy | controle de adaptação sequencial |
| Bandits | recompensa sob exploração | benchmarks proxy | controle de decisão online |
| Memória episódica | casos observados | `symbiotic_learning.py` | núcleo inicial do HERUS |
| Transformers | atenção e representação contextual | BERT/DistilBERT multilíngue | baseline forte de linguagem |
| Redes tensoriais | fatoração compacta | Tensor-Train | baseline de eficiência/representação |
| Conformal prediction | conjunto sob cobertura | `conformal_mintrec.py` | incerteza; não é garantia operacional |
| Síntese enumerativa | busca em gramática | programação proposal-only | geração bounded de candidatos |
| Causal/ablação | intervenção e contraste | protocolos pré-registrados | identificar mecanismos, não decorar score |
| Assurance | contratos e bloqueios | gates, proveniência, ledger | camada de verificabilidade, não modelo preditivo |

## 4. O que torna a hipótese Symbiotic diferente

A hipótese não é simplesmente “usar símbolos” ou “usar memória”. A proposta é combinar:

1. **estado persistente limitado**;
2. **representação condicionada ao hospedeiro**;
3. **evidência positiva e negativa**;
4. **otimização sob restrições**;
5. **feedback com provenance**;
6. **abstenção explícita**;
7. **separação entre proposta e autoridade**;
8. **rollback do estado aprendido**;
9. **explicação por evidência**;
10. **teste de adaptação entre hosts**.

Se os experimentos mostrarem que essa combinação não oferece vantagem mensurável, o nome Symbiotic Learning deverá ser revisto. A hipótese não é protegida por retórica.

## 5. Camadas do projeto

```text
HERUS Symbiotic — algoritmo em evolução
├── HERUS-core
│   ├── memória episódica
│   ├── indução/atualização
│   ├── política de proposta
│   ├── otimização sob contratos
│   └── abstenção e explicação
├── adaptadores
│   ├── texto/estado
│   ├── programação
│   └── hospedeiro simulado
├── controles
│   ├── Naive Bayes
│   ├── SVM/Random Forest/LogReg/k-NN
│   ├── transformer
│   ├── Tensor-Train
│   └── memória finita/maioria
├── assurance
│   ├── Object Lock
│   ├── provenance
│   ├── paired ledger
│   ├── conformal/selective evaluation
│   └── causal ablation
└── hosts futuros
    ├── computador
    ├── firmware
    ├── pulso
    └── outros hosts — não validados ainda
```

## 6. Regras de atribuição

- Resultado de um baseline não é resultado do HERUS-core.
- Resultado de um adaptador composto deve ser atribuído ao composto.
- Precisão seletiva exige cobertura, custo de abstenção e intervalo.
- Bootstrap não cria novas amostras.
- Seeds são replicações, não unidades independentes.
- Conformal em classificação não vira assurance de ação automaticamente.
- Testes de firmware não provam eficácia do algoritmo.
- A API pública não transforma módulos distintos em uma competência única.

## 7. Estado honesto

O HERUS possui uma proposta algorítmica própria e uma base real de implementação. Ainda não provou:

- vantagem universal;
- SOTA geral;
- transferência entre hospedeiros;
- causalidade;
- aprendizagem por recompensa real;
- adaptação geral;
- AGI;
- segurança operacional.

A meta continua ambiciosa. A diferença é que agora o repertório deixa claro qual experimento pode confirmar ou destruir cada parte da meta.

## 8. Próximo ciclo

1. manter a implementação histórica v2;
2. fechar a função de otimização e a semântica de feedback;
3. implementar positivos e negativos como atualização real;
4. testar o algoritmo primeiro em hosts simulados com interfaces permutadas;
5. comparar com controles fortes e memória trivial;
6. só então avaliar os resultados em datasets reais e em hardware.
