# HERUS — Transfer learning local e corpus comportamental

**Estado:** experimento reproduzível em host, pré-hardware.  
**Modelo:** Transformer de caracteres compacto com base congelada e adapter de baixo posto.  
**Princípio:** comportamento textual não concede autoridade física.

## Objetivo

O objetivo desta etapa não é alegar que o HERUS se tornou uma LLM geral. O objetivo é testar uma rota local de adaptação: um modelo linguístico pequeno aprende regularidades gerais de texto em um corpus público e depois recebe uma delta compacta treinada somente em material público do projeto e exemplos comportamentais curados.

A separação é necessária. O corpus público ensina forma linguística. Os documentos do HERUS ensinam vocabulário e contexto técnico. O corpus comportamental ensina padrões de abstenção, privacidade, memória seletiva, soberania on-wrist e confirmação física. Nenhum desses textos é convertido em autorização executável. A autoridade continua no AGSC, no Attribution Guard e nos gates físicos.

A inspiração metodológica vem de LoRA, que congela o modelo base e injeta parâmetros treináveis de baixo posto [1]. A implementação do HERUS não é LoRA canônico: ela usa uma transformação de baixo posto na cabeça de saída de um Transformer de caracteres compacto. Isso é intencionalmente declarado para não transformar uma analogia em reivindicação de equivalência.

## Dados e licenças

O pré-treinamento usa o treino raw do WikiText-2, obtido do dataset público Salesforce/WikiText. O cartão declara CC BY-SA 4.0 e fornece variantes raw e não raw [2]. O arquivo baixado é preservado por checksum no `corpus_manifest.json`, mas o Parquet bruto e os splits derivados ficam fora do Git. O downloader reproduzível verifica URL, formato Parquet e SHA-256 antes de aceitar o arquivo.

A adaptação usa documentos técnicos públicos do HERUS e um corpus curado de exemplos de diálogo comportamental. O corpus comportamental não contém fala real, identidade, localização, áudio, chaves, logs de produto ou memória pessoal. Ele usa exemplos sintéticos do contrato do sistema, como “não sei”, “não armazeno áudio bruto”, “o Core alimenta conhecimento, mas não executa” e “uma sugestão não é uma ação”.

| Camada | Dados | Papel | Pode conceder autoridade? |
|---|---|---|---:|
| Base | WikiText-2 raw train | regularidades gerais de texto | Não |
| Técnico | documentos públicos HERUS | vocabulário e contexto do projeto | Não |
| Comportamental | exemplos sintéticos curados | estilo, abstenção e limites | Não |
| Pessoal futuro | memória local explicitamente autorizada | contexto do usuário | Não diretamente |
| Guard soberano | AGSC, Attribution Guard e firmware | autoridade e ação | Sim, sob gates físicos |

TinyStories oferece uma evidência relacionada: sob um corpus simples e restrito, modelos abaixo de 10 milhões de parâmetros podem produzir texto coerente dentro de seu domínio [3]. Isso não significa que o mesmo modelo tenha conhecimento aberto, diálogo adulto, factualidade ou raciocínio geral. O experimento do HERUS mantém essa distinção.

## Arquitetura treinada

O modelo base possui vocabulário de 834 caracteres, contexto de 128 posições, dimensão 64, quatro cabeças de atenção, dois blocos Transformer e feed-forward de dimensão 128. Ele foi pré-treinado localmente por 700 passos no WikiText-2. Depois, todos os pesos da base foram congelados e apenas uma delta de saída de rank 4 foi atualizada por 350 passos nos documentos HERUS e no corpus comportamental.

A delta possui 3.592 parâmetros, contra 182.016 parâmetros congelados da base. O artefato é salvo em `base.pt`, `herus_adapter.pt` e `vocab.json`. O modelo pode ser carregado sem a etapa de treinamento; a execução de treino requer PyTorch local, mas o gate do artefato no `prove.sh` usa apenas Python stdlib e valida os resultados versionados.

> O adapter é uma especialização textual. Ele não é uma carteira de permissões, uma lista de ações nem uma fonte de soberania.

## Avaliação held-out

Os dados do HERUS são divididos em treino, tuning e teste. O corpus público também possui tuning e teste separados. A força da delta é selecionada no tuning com uma restrição: o aumento de perda no corpus público deve ser menor ou igual a 0,25 quando houver uma configuração segura.

A configuração escolhida foi força **0,75**. No teste, os resultados foram:

| Métrica de perda NLL | Base | Adaptado | Delta |
|---|---:|---:|---:|
| WikiText-2 público | 2,310280 | 2,517608 | +0,207329 |
| Documentos técnicos HERUS | 3,273316 | 2,658104 | **−0,615211** |
| Comportamento HERUS | 3,283090 | 2,603807 | **−0,679283** |

A leitura defensável é que houve **transferência mensurável no host**: o adapter reduziu a perda nos dois conjuntos held-out do HERUS. Também houve esquecimento controlado dentro do limite pré-registrado: o aumento público foi 0,207329, abaixo de 0,25.

Isso não mede qualidade de resposta humana, factualidade, WER, intenção, latência, energia, RAM no ESP32-S3 ou segurança física. Perda de próximo caractere é uma métrica de modelagem textual, não uma prova de inteligência geral.

A amostra gerada ainda apresenta erros ortográficos e cadeias pouco naturais. Isso é uma evidência importante contra exagero: o treinamento funcionou como adaptação estatística, mas o modelo compacto ainda não se comporta como uma LLM conversacional madura. A próxima melhoria deve atacar tokenização, objetivo de diálogo, avaliação semântica e exportação eficiente, não apenas aumentar invariantes ou chamar a amostra de “raciocínio”.

## Gates de segurança

O gate `tools/test_text_transfer.py` verifica a existência e integridade estrutural dos artefatos, a melhora nos dois conjuntos HERUS, o limite de esquecimento público, o tamanho menor do adapter, a ausência de marcadores de dados pessoais no corpus comportamental e a declaração explícita de que autoridade não está nos pesos.

O gate também rejeita metadados que nomeiem LLMs hospedadas de terceiros. O corpus e os artefatos são locais. A decisão de ação permanece fora do modelo textual e passa pelos contratos soberanos já testados.

## Limitações e próximos passos

O modelo atual é pequeno demais para ser considerado uma LLM geral. A amostra demonstra essa limitação. O próximo ciclo deve avaliar um tokenizer de subpalavras ou byte-pair local, objetivos de instrução e abstenção, recuperação semântica tipada e uma forma de quantização que possa ser carregada pelo firmware. Nenhuma dessas melhorias deve remover a distinção entre sugestão, memória, política e ação.

No hardware, ainda será necessário medir tamanho real, RAM, latência, consumo, reset e comportamento sob baixa energia. A presença dos artefatos no repositório não constitui prova de implantação no pulso.

## Referências

[1]: https://arxiv.org/abs/2106.09685 "Hu et al., LoRA: Low-Rank Adaptation of Large Language Models"

[2]: https://huggingface.co/datasets/Salesforce/wikitext "Salesforce/WikiText dataset card and license declaration"

[3]: https://arxiv.org/abs/2305.07759 "Eldan and Li, TinyStories: How Small Can Language Models Be and Still Speak Coherent English?"
