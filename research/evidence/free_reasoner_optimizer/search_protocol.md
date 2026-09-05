# Protocolo de busca simbólica do HERUS

## Regra de promoção

Um candidato gerado pelo otimizador é sempre uma hipótese não confiável. Ele só pode ser promovido a resultado algébrico quando satisfaz, nesta ordem, três condições independentes:

| Etapa | Entrada | Critério | Falha |
|---|---|---|---|
| Treino | Exemplos apresentados à política | Erro exato calculado pelo kernel | Candidato descartado ou mantido como hipótese |
| Validação | Exemplos ocultos durante a busca | Erro exato zero no conjunto de validação | Candidato não promovido; registrar contraexemplo |
| Prova | Alvo formal ou identidade declarada | `Prover`/kernel confirma igualdade exata | Resultado permanece não provado |

Erro zero no treino nunca substitui validação ou prova. O conjunto de prova não pode ser usado para orientar a política durante a busca. Quando o alvo formal não estiver disponível, o sistema deve reportar apenas `fit` e `holdout`, nunca `proved`.

## Orçamento comum

As estratégias devem receber um limite explícito de avaliações de candidatos. O relatório precisa registrar episódios, passos, candidatos avaliados, tempo de host, tamanho máximo do termo e seed. Enumeração, beam search, MCTS e política estocástica devem ser comparados com o mesmo limite de avaliações e o mesmo vocabulário.

## Métricas obrigatórias

A campanha deve registrar taxa de erro zero no treino, taxa de erro zero na validação, taxa de prova quando há alvo formal, avaliações até o primeiro candidato válido, tamanho/complexidade do candidato, variância por seed e taxa de abstenção. Um candidato que ajusta o treino mas falha na validação conta como **falso ajuste**, não como sucesso.

## Casos mínimos

O protocolo deve conter identidades suficientemente observadas (`x²`, `x²+x`, `x²−x`), um caso subdeterminado (`0 → 0` para alvo oculto `x²`) e pelo menos um conjunto inconsistente. O caso subdeterminado é obrigatório porque verifica a propriedade fail-closed contra extrapolação indevida.

## Limites embarcados

O perfil C11 deve usar memória fixa, sem heap, e o mesmo vocabulário reduzido. Seu resultado é apenas um orçamento preliminar até ser compilado e medido em ESP32-S3 ou hardware equivalente. Nenhum resultado de x86-64 pode ser apresentado como desempenho do alvo.
