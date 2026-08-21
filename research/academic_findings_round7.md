# Achados acadêmicos — vivacidade operacional, memória e abstention

## LoCoMo

Fonte: Maharana et al., *Evaluating Very Long-Term Conversational Memory of LLM Agents*, arXiv:2402.17753, https://arxiv.org/abs/2402.17753.

O trabalho introduz LoCoMo, um benchmark de conversas de longo prazo com aproximadamente 300 turnos e 9 mil tokens em média, distribuídos por até 35 sessões. A avaliação inclui perguntas e respostas, sumarização de eventos e geração de diálogo multimodal, ancorada em personas, grafos temporais e eventos. O resumo relata que modelos ainda enfrentam dificuldades com dependências temporais e causais longas; contexto longo e RAG melhoram os resultados, mas continuam atrás do desempenho humano.

Aplicação ao HERUS: “parecer vivo” deve ser operacionalizado como continuidade verificável, não como consciência. O benchmark local deve testar recuperação de evento, mudança de preferência, conflito entre memórias, validade temporal, causalidade simples, resumo seletivo e coerência de estilo. Deve medir também o custo de uma resposta errada e o comportamento de abstenção.

## Classificação seletiva

Fonte: *Calibrated Selective Classification*, arXiv:2208.12084v2, https://arxiv.org/html/2208.12084v2.

Classificação seletiva permite que um modelo se abstenha quando a confiança é insuficiente, trocando cobertura por menor risco. Essa formulação é mais adequada ao HERUS do que uma métrica de acerto única: o dispositivo deve responder quando a evidência é suficiente e preferir declarar incerteza quando não é.

Aplicação ao HERUS: registrar cobertura, risco condicional e taxa de abstention em consultas de memória. A avaliação deve incluir curvas risco–cobertura e casos de autoridade implícita. Uma resposta “não sei” com origem e evidência pode ser uma vitória; uma resposta fluente e falsa é uma falha grave.

## Requisitos derivados

| Requisito | Métrica local proposta |
|---|---|
| Continuidade temporal | recuperação correta antes/depois de mudança e reboot |
| Memória seletiva | fatos relevantes recuperados sem guardar texto irrelevante |
| Conflito | abstention ou apresentação de alternativas, nunca escolha silenciosa |
| Causalidade simples | remoção de um evento altera a resposta dependente |
| Calibração | risco condicional por faixa de confiança |
| Vivacidade operacional | continuidade + sutileza + incerteza honesta + zero ação implícita |

Esses achados não provam consciência, personalidade humana ou inteligência geral. Eles fornecem tarefas observáveis para avaliar a hipótese de uma inteligência pessoal contínua.
