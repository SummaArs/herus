# Função objetivo e treino via Internet

## Função objetivo atual

O SIM não usa uma loss neural aberta nem otimiza autoridade. A seleção de representação é uma função de custo discreta e auditável:

```text
J(representação) =
  4 × bytes
+ 3 × passos
+ 2 × incerteza
− 5 × utilidade
```

A representação é elegível somente se estiver no vocabulário do host, couber no orçamento de bytes e passos e satisfizer os requisitos da tarefa. Entre as opções elegíveis, o HERUS escolhe o menor `J`. Os pesos são explícitos e testáveis; não são ajustados pela Internet nem podem alterar `allowed_effects`, autoridade ou o verificador.

O aprendizado local atual atualiza apenas protótipos inteiros bounded quando há exemplos rotulados, preserva uma versão anterior e permite rollback. Isso é adaptação operacional, não treino irrestrito de um modelo de linguagem.

## Acesso Internet

O HERUS agora pode realizar requisições HTTPS públicas dentro de um cliente bounded. O cliente aplica allowlist de domínios, limite de URLs, limite de bytes, timeout e tipos de conteúdo permitidos. O conteúdo é tratado como bytes/texto de evidência: nunca é importado como código executável.

A consulta real às documentações oficiais LILYGO e Espressif funcionou. O datasheet PDF foi recebido como presença de fonte, sem fingir que seu binário bruto é uma claim sem extração. A extração de claims continua sendo uma etapa separada e revisável.

```text
URL allowlisted
  → HTTPS bounded
  → digest
  → claims explícitos
  → comparação / revisão
  → preferência operacional ou proposta
```

Nenhuma resposta Internet concede autoridade, muda o firmware, executa instruções ou promove uma proposta.

## O que será treinado nos próximos hospedeiros

A Internet serve para treinar seleção e hipóteses finitas: qual fonte é útil, qual representação cabe, quais capacidades são declaradas, quando uma documentação diverge e quando uma hipótese deve expirar. O computador, filesystem, sandbox, datasets, simulador e hardware medirão o comportamento operacional correspondente.

O critério de sucesso é melhoria mensurável em tarefa não vista sem aumento de falsos aceites, autoridade indevida, vazamento, orçamento ou complexidade não auditada.
