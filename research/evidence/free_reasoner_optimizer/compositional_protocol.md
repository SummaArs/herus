# Protocolo composicional do HERUS

## Objetivo

Testar se a busca simbólica consegue compor operações já conhecidas em profundidade maior, em vez de somente reencontrar `x²` em uma árvore rasa. O experimento continua restrito ao anel polinomial univariado e não representa compreensão de linguagem.

## Tarefas

| Tarefa | Alvo formal | Treino | Holdout |
|---|---|---|---|
| `cube_plus_x` | `x*x*x+x` | `{-2,-1,0,1,2}` | `{-3,3,4}` |
| `cube_plus_square` | `x*x*x+x*x` | `{-2,-1,0,1,2}` | `{-3,3,4}` |
| `nested_difference` | `(x*x-x)*(x+x)` | `{-2,-1,0,1,2}` | `{-3,3,4}` |

Os valores esperados são calculados pelo kernel exato a partir do alvo declarado para o benchmark. O alvo não é fornecido à política de busca; ele serve somente para avaliação externa da prova. A tarefa `nested_difference` exige reutilizar uma subárvore (`x+x`) como segundo operando; a gramática linear atual não oferece uma pilha de subtermos e, portanto, essa tarefa é deliberadamente um teste de limite, não uma expectativa de sucesso.

## Critérios

Um método é considerado bem-sucedido somente quando o candidato tem erro exato zero no treino, erro exato zero no holdout e passa pelo `Prover` contra o alvo formal. Caso contrário, o resultado é classificado como ajuste parcial, falha de generalização ou não provado.

## Controles

O limite comum é 768 avaliações de candidatos. As seeds são `0..4` para políticas estocásticas e MCTS. Enumeração e beam têm ordenação determinística. O benchmark também conserva um caso subdeterminado e um caso inconsistente para verificar abstinência e impedir que maior profundidade seja confundida com maior conhecimento.

## Limite de interpretação

Uma tarefa composicional bem-sucedida mostra generalização algébrica dentro da gramática fornecida. Não mostra aquisição de símbolos, grounding, linguagem natural, causalidade, planejamento ou substituição de uma LLM. O resultado precisa ser acompanhado de custo, tamanho do candidato e taxa de abstenção.
