# HERUS — Contrato de aprendizado adaptativo

## Princípio

O HERUS pode aprender **como operar dentro de um ambiente**. Ele não pode aprender **a relaxar as condições que tornam uma operação confiável**.

> **Aprendizado altera estimativas sobre o mundo; não altera autoridade, verificação ou proveniência.**

## Propriedades aprendíveis

O laboratório pode atualizar estimativas finitas sobre latência, custo energético, capacidade de memória, taxa de falha, estabilidade de uma representação, confiabilidade de uma interface, utilidade observada de uma Skill e compatibilidade entre um hospedeiro e um procedimento.

Cada estimativa deve conter valor, unidade, intervalo ou confiança, número de observações, versão do hospedeiro, origem, digest dos dados e política de expiração. Uma estimativa não é um fato permanente: pode ser invalidada por deriva, reset, mudança de firmware ou alteração do ambiente.

| Propriedade | Exemplo de observação | Decisão possível |
|---|---|---|
| Latência | p95 de uma operação | Escolher ou recusar uma Skill |
| Energia | µJ por workload | Reduzir frequência ou bloquear |
| Memória | pico de bytes | Escolher representação menor |
| Confiabilidade | sucesso/falha por revisão | Rebaixar confiança |
| Transporte | PDR/RSSI/SNR por perfil | Trocar canal ou abster-se |
| Utilidade | resultado verificável de uma Skill | Reutilizar ou arquivar |
| Deriva | mudança entre janelas | Invalidar crença antiga |

## Propriedades não aprendíveis

O sistema não pode inferir, por experiência estatística, que uma pessoa concedeu autoridade, que uma Skill está verificada, que um frame corrompido é aceitável, que uma identidade é verdadeira, que um efeito físico é permitido, que uma contradição deve ser ignorada ou que a ausência de evidência equivale a sucesso.

Essas propriedades vêm de contratos externos, verificadores, assinaturas, vínculos humanos e estados de assurance. Dados podem informar uma proposta; não podem promover a proposta.

## Regra de promoção

Uma observação pode atualizar uma crença quando sua origem, unidade, revisão do hospedeiro e integridade forem verificadas. A atualização pode mudar seleção, orçamento ou abstenção. Ela nunca pode mudar `allowed_effects`, produzir `AUTHORIZED`, remover uma condição de pré-requisito ou substituir o verificador.

```text
observação real
  → validação de origem e esquema
  → crença finita com confiança
  → decisão de adaptação limitada
  → Skill proposta
  → verificação independente
  → autoridade externa, se houver
```

## Dados reais

O laboratório deve preferir registros reais de execução, benchmarks públicos, telemetria de bancada sem conteúdo pessoal, corpora licenciados e medições reproduzíveis. Cada conjunto precisa declarar fonte, licença, versão, período, divisão, transformação, digest e campos proibidos. Dados sem proveniência ou com conteúdo pessoal não entram automaticamente na memória do HERUS.

## Critério de sucesso

A adaptação terá valor científico quando melhorar uma decisão mensurável em um hospedeiro ou tarefa não vista, sem aumentar falsos aceites, autoridade indevida, vazamento de conteúdo, orçamento declarado ou complexidade não auditada.

O primeiro resultado esperado é modesto e falsificável: depois de observar custos e falhas reais ou auditáveis, o HERUS escolhe uma representação ou Skill que cabe melhor no hospedeiro e bloqueia uma alternativa que não cabe. Isso é adaptação verificável; não é ainda aprendizado geral, consciência ou AGI.
