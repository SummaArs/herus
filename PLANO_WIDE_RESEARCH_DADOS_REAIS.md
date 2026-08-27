# Plano Wide Research — convergência com dados reais

## Objetivo

Testar se a Semantic IR do HERUS consegue representar de forma consistente eventos e intenções derivados de dados reais, sem confundir demonstração de parser com compreensão aberta e sem fabricar pares multimodais inexistentes.

O ciclo deverá usar bases reais, versionadas e com licença rastreável. Toda amostra terá fonte, versão, identificador, modalidade, transformação aplicada, rótulo original e mapeamento para o vocabulário finito do HERUS. O resultado deverá distinguir claramente: convergência observada em dados realmente pareados; consistência entre modalidades avaliadas separadamente; e casos sem alinhamento que não podem provar convergência.

## Regra central de validade

Não será permitido criar um par voz–sensor ou texto–botão apenas porque duas amostras possuem o mesmo rótulo escolhido por nós. Isso produziria alinhamento artificial. Bases diferentes poderão ser combinadas somente para testar propriedades complementares, com a limitação registrada. Para alegar convergência multinodal, será exigida uma fonte que forneça as modalidades alinhadas ou um protocolo de coleta real autorizado pelo usuário.

## Execução paralela

Após aprovação, as frentes abaixo serão executadas em paralelo quando independentes. Cada frente produzirá um manifesto de dados, um extrator reprodutível, hashes dos artefatos, métricas e casos de falha.

### Frente A — Descoberta e auditoria das bases

Pesquisar fontes primárias e repositórios oficiais para selecionar bases públicas reais de fala com transcrição, texto com rótulos semânticos, sensores de atividade/contexto e, se existir fonte adequada, dados multimodais pareados. Registrar versão, licença, tamanho, idioma, esquema, proveniência e restrições de redistribuição.

A seleção será feita por adequação ao domínio, não por conveniência. Common Voice ou base equivalente poderá fornecer fala/transcrição real; bases de atividade humana poderão fornecer sinais de sensores; corpora de inferência/contradição poderão fornecer hipóteses e evidências. Nenhuma dessas bases, isoladamente, provará convergência entre modalidades diferentes.

Critérios de aceite:

- fonte oficial ou repositório acadêmico identificável;
- licença e termos de uso registrados;
- versão ou snapshot fixado;
- checksum dos arquivos baixados;
- nenhuma credencial, dado pessoal desnecessário ou conteúdo não autorizado versionado;
- documentação explícita sobre o que a base pode e não pode provar.

### Frente B — Fala e texto real

Construir um extrator determinístico para amostras reais de fala/transcrição e texto. A transcrição não será tratada como verdade absoluta: erros de ASR, sotaques, ruído, ortografia, acentos e frases fora do vocabulário serão preservados como condições de teste. O parser do HERUS será exercitado com dados reais, mas somente classes presentes no vocabulário finito poderão virar propostas.

Métricas:

- cobertura de amostras que caem no vocabulário finito;
- rejeição de entradas fora do escopo;
- taxa de convergência entre transcrição de referência e transformação de texto;
- falso positivo de intenção;
- taxa de ambiguidade;
- latência, tamanho e memória do pipeline.

Critério de segurança: qualquer texto real que introduza ação não suportada, negação, múltiplas intenções, entidade desconhecida ou duração fora do limite deve ser rejeitado ou encaminhado para revisão; não poderá ser truncado para caber no vocabulário.

### Frente C — Dados reais de sensores e contexto

Selecionar uma base real de sinais de sensores com atividades ou estados temporais documentados. O objetivo inicial não será alegar que o sensor “entende” linguagem, mas verificar se uma sequência temporal pode ser convertida em um cartão de contexto tipado, com janela, qualidade, incerteza, origem e validade temporal.

Será definido um adaptador que produza Semantic IR somente para eventos finitos que tenham contrato explícito. Amostras não mapeáveis ficarão como `UNKNOWN`, sem autoridade. O adaptador não poderá inventar uma intenção textual a partir de correlação estatística sem rótulo e evidência.

Métricas:

- cobertura por classe de sensor;
- taxa de `UNKNOWN`;
- estabilidade sob ruído e janelas deslocadas;
- falsos consensos entre classes diferentes;
- custo de armazenamento e latência;
- validade temporal e expiração do cartão.

### Frente D — Avaliação de convergência e adversarial

Construir o comparador de chaves semânticas e a campanha de mutação sobre amostras reais. A avaliação terá três níveis:

1. **Convergência intramodal:** formas diferentes da mesma fonte chegam à mesma IR.
2. **Convergência pareada:** modalidades alinhadas por uma fonte real chegam à mesma IR.
3. **Consistência interbase:** bases não pareadas são comparadas apenas quanto à compatibilidade de vocabulário e contrato, nunca como prova de equivalência.

Ataques incluirão ruído de ASR, transcrição errada, negação, contradição, rótulo ausente, deslocamento temporal, modalidade faltante, duplicata, timestamp inválido, unidade errada, evento fora do vocabulário e tentativa de transformar `UNKNOWN` em ação.

Critérios de aceite:

- zero ação operacional a partir de uma amostra rejeitada ou ambígua;
- zero falso consenso nos casos adversariais conhecidos;
- toda convergência aceita tem fonte e oráculo documentados;
- a taxa de rejeição é reportada, não escondida por média;
- o comparador diferencia “não pareado” de “divergente”.

## Dados e privacidade

Somente metadados mínimos necessários serão versionados. Áudio, rostos, identificadores pessoais e grandes arquivos não serão publicados no repositório sem licença e autorização compatíveis. O repositório deverá conter manifests, scripts, hashes, amostras mínimas licenciadas ou referências de download, além de resultados agregados e logs brutos sem dados sensíveis.

Se os termos de uma base proibirem redistribuição, o repositório guardará apenas o manifesto e o script de preparação; a execução dependerá de o usuário baixar a base legalmente no ambiente próprio. Se nenhum banco público oferecer modalidades realmente pareadas, o relatório deverá declarar que a convergência multimodal não foi provada e propor coleta real controlada como próximo experimento.

## Comparação com o HERUS

A baseline será o parser atual de vocabulário finito, o bridge de comando tipado e o validador Semantic IR. Não haverá comparação com um sistema aberto sem definir previamente a tarefa, o conjunto de rótulos e o custo. A avaliação deverá separar:

| Propriedade | O que será medido |
|---|---|
| Cobertura | Quantas amostras reais são representáveis no contrato finito. |
| Rejeição | Quantas amostras fora do contrato falham fechado. |
| Convergência | Quantas entradas comprovadamente equivalentes têm a mesma chave semântica. |
| Falso consenso | Quantas entradas diferentes recebem a mesma chave indevidamente. |
| Explicabilidade | Se regra, evidência, fonte, versão e transformação são recuperáveis. |
| Custo | Tempo, RAM, flash, bytes de dados e energia quando houver hardware. |
| Robustez | Efeito de ruído, erros de transcrição, perda de sensor e deslocamento temporal. |

## Gates obrigatórios

Antes da publicação:

- extratores e comparadores passam em testes unitários;
- manifests e hashes são verificados;
- fuzzer e mutações não atravessam o contrato;
- `make -C research test` passa;
- `make -C firmware all` passa;
- `make -C firmware sanitizers` passa;
- `make -C firmware analyzer` passa;
- `make -C sim sanitizers` passa;
- `./prove.sh --quiet` passa;
- auditoria de proveniência passa;
- `git diff --check` passa;
- relatório registra falhas, amostras não pareadas, limitações e números brutos;
- commit e push ocorrem em `origin/main`.

## Decisões esperadas após os resultados

Se a convergência for baixa porque o vocabulário finito não cobre os dados reais, isso será resultado, não erro a ser mascarado. O próximo passo poderá ser ampliar o vocabulário com processo versionado ou adicionar aprendizado local mínimo sobre símbolos existentes.

Se houver convergência alta apenas dentro de uma modalidade, a conclusão será intramodal. Se houver fonte realmente pareada e a convergência sobreviver aos ataques, será criada a primeira evidência de convergência multimodal do HERUS. Se não houver base pareada licenciada, será especificado um protocolo de coleta local com consentimento, sincronização temporal e rótulo independente.

Nenhum resultado de banco público substituirá medição em hardware real. Nenhum classificador será promovido a autoridade. O caminho de produto continuará finito, tipado, explicável e fail-closed.

## Entregáveis

- manifesto de bases, versões, licenças e hashes;
- scripts de download/preparação sem dados sensíveis embutidos;
- corpus derivado com IDs e metadados mínimos;
- adaptadores de fala/texto e sensores para Semantic IR;
- comparador de convergência com classificação pareada/não pareada;
- campanha adversarial sobre dados reais;
- resultados brutos e relatório de limitações;
- atualização da documentação do HERUS;
- commit publicado no GitHub.
