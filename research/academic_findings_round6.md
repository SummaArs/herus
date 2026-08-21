# Achados acadêmicos — adaptação local e corpus textual

## LoRA

Fonte: Hu et al., *LoRA: Low-Rank Adaptation of Large Language Models*, arXiv:2106.09685, https://arxiv.org/abs/2106.09685.

LoRA congela os pesos do modelo base e injeta matrizes treináveis de baixo posto. A ideia relevante para o HERUS é separar um núcleo estável de uma pequena delta adaptável. Isso reduz o conjunto de parâmetros que precisa ser atualizado e facilita manter uma versão base e uma adaptação pessoal separadas. LoRA não torna um modelo pequeno equivalente a uma LLM geral e não resolve por si só segurança, proveniência, esquecimento ou seleção de memórias.

Aplicação: o HERUS deve treinar uma adaptação compacta de estilo/intenção ou um índice semântico local, enquanto conhecimento factual e autoridade permanecem em estruturas tipadas e revogáveis. A delta de estilo nunca deve conceder ação.

## TinyStories

Fonte: Eldan e Li, *TinyStories: How Small Can Language Models Be and Still Speak Coherent English?*, arXiv:2305.07759, https://arxiv.org/abs/2305.07759.

O estudo apresenta um corpus sintético de histórias curtas com vocabulário limitado. Ele relata que modelos com menos de 10 milhões de parâmetros, ou com arquitetura muito simples, podem produzir histórias coerentes dentro desse domínio restrito. O próprio desenho do corpus é uma condição importante: a demonstração é sobre linguagem simples e consistência local, não sobre conhecimento aberto, diálogo adulto, factualidade geral ou uso em hardware limitado.

Aplicação: um corpus pequeno pode ensinar forma linguística e padrões de resposta do HERUS, mas a avaliação deve separar coerência de conhecimento. A rota inicial deve medir perplexidade ou perda de validação, cópia indevida, cobertura de intenções e abstention; não deve chamar geração de histórias de inteligência geral.

## WikiText-2/WikiText

Fonte: cartão do dataset Salesforce/WikiText, https://huggingface.co/datasets/Salesforce/wikitext.

O cartão informa que WikiText é derivado de artigos Good e Featured da Wikipédia, oferece variantes raw e non-raw e declara CC BY-SA 4.0 no dataset publicado. O corpus serve como base pública para modelagem de linguagem, preservando caixa, pontuação e números. A licença exige que qualquer redistribuição e derivação respeite os termos aplicáveis.

Aplicação: usar somente uma cópia local do subset explicitamente documentado, guardar licença e checksum, e não misturar texto privado do usuário com o corpus público no mesmo artefato redistribuível. O dataset é adequado para um experimento linguístico, não para afirmar que o HERUS aprendeu fatos completos ou que o conteúdo é correto.

## Critérios de soberania

| Critério | Regra para o HERUS |
|---|---|
| Origem | arquivo local, URL e checksum registrados |
| Licença | licença preservada junto do manifesto |
| Privacidade | nenhuma fala, identidade, localização ou memória pessoal no corpus público |
| Transfer learning | base e delta separadas; delta não concede autoridade |
| Avaliação | split de treino/validação/teste e testes adversariais de cópia |
| Implantação | só converter depois de medir tamanho, RAM e latência no alvo |
| Alegação | dados textuais do host não provam WER, energia, alcance ou inteligência geral |
