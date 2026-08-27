# Wide Research — ciclo 04: convergência com dados reais

**Veredito:** `herus_convergence_proven=false`.

## Escopo executado

O ciclo auditou bases públicas reais e implementou um comparador que não cria pares por rótulo, ordem de arquivo, semelhança textual ou coincidência de classe. Uma taxa multimodal só é calculada quando as duas observações têm a mesma origem, o mesmo `sample_id`, modalidades distintas e alinhamento `PAIRED` declarado. Observações `INTRAMODAL` podem ser comparadas apenas dentro da mesma modalidade. Observações `UNPAIRED` são recusadas.

O código do comparador está em `research/convergence.py`; os ataques a ID, origem, modalidade, classe de alinhamento e tentativa de parear MIntRec com WESAD por um rótulo coincidente estão em `research/test_convergence.py`.

## MIntRec: metadados reais auditados

A fonte oficial descreve MIntRec como um dataset de intenção multimodal com texto, áudio e vídeo alinhados por segmento, mas o repositório de código é MIT e isso não foi tratado como licença de redistribuição da mídia.[1] Foram baixados localmente somente `train.tsv` e `test.tsv` do armazenamento oficial; a árvore de dados está explicitamente ignorada pelo Git.

| Medida | Resultado |
|---|---:|
| `train.tsv` | 1.334 linhas; 75.488 bytes; SHA-256 `348289d7d140b2c0716b35a03e7045b684c4b90f5c30d1281f9a6b7cb3b9903b` |
| `test.tsv` | 445 linhas; 26.287 bytes; SHA-256 `e4394869fc02898e00fad0a1b03e0333760a9b0499da2fa3f8af030a384c8f3a` |
| Total | 1.779 linhas |
| IDs de segmento únicos | 1.779 |
| Duplicatas entre splits | 0 |
| Textos vazios | 0 |
| Rótulos de origem | 20 |
| Rótulos automaticamente mapeados para HERUS | 0 |
| Mídia áudio/vídeo validada localmente | não |

Os 20 rótulos de MIntRec não pertencem ao vocabulário operacional finito `ARRIVE`, `HELP`, `CANCEL`. Nenhum mapeamento foi inferido. Logo, os TSVs demonstram integridade de metadados e permitem um teste fora do domínio, mas não demonstram convergência multimodal HERUS.

O auditador reproduzível está em `research/analyze_mintrec_metadata.py`; o resultado agregado está em `mintrec_metadata_audit.json`.

## Execução do parser C real

O runner `research/real_corpus_voice_runner.c` compilou `firmware/core/voice.c` sem warnings com `-Wall -Wextra -Werror -std=c11` e executou sobre as 1.779 transcrições reais em inglês.

| Saída do parser | Contagem |
|---|---:|
| `DRAFT` | 0 |
| `CANCEL_LOCAL` | 0 |
| `UNKNOWN` | 1.636 |
| `REJECTED` | 143 |
| Texto acima do limite da API vocal | 28 |
| Linha TSV truncada pelo runner | 0 |
| Texto não ASCII | 0 |
| Texto vazio | 0 |
| Mapeamento automático de label | 0 |

A soma `1.636 + 143 = 1.779`. O resultado é rejeição/fora de domínio de um parser português sobre transcrições inglesas; não é taxa de compreensão, não é falha a ser mascarada e não é evidência de convergência semântica. O runner também passou no alvo local ASan/UBSan `make -C research real-corpus-sanitizers`.

## Fontes complementares, sem pareamento inventado

| Fonte | Classe | Estado | Limite |
|---|---|---|---|
| MIntRec | `PAIRED` declarado por segmento | TSVs de texto/rótulo auditados; áudio/vídeo não baixados | Não provar convergência local nem mapear classes para HERUS |
| Common Voice Scripted Speech `pt` | `INTRAMODAL` áudio–transcrição | Nenhum arquivo baixado; release/termo específico ainda precisa ser fixado | Não provar intenção HERUS nem voz–sensor [2] |
| WESAD | `PAIRED` interno entre streams de sensores | Nenhum arquivo baixado | Não contém texto/fala; não provar voz–sensor [3] |
| Fluent Speech Commands | `PAIRED` áudio–transcrição–slots declarado pela fonte | Nenhum arquivo baixado | Licença NC-ND; não mapear intenções externas para HERUS [4] |

## Correções e gates

A normalização do parser C teve seu comentário corrigido para declarar o comportamento efetivo: qualquer byte não ASCII, inclusive acento ou pontuação UTF-8, é rejeitado; não é convertido em separador. O runner ganhou leitura limitada de linhas e descarte explícito de linha longa, evitando análise parcial de registros.

Foram executados os seguintes comandos após as alterações:

```text
make -C research test                         # 36 testes, OK
make -C research real-corpus-audit            # OK, relatório agregado
make -C research real-corpus                  # 1.779 linhas processadas
make -C research real-corpus-sanitizers       # ASan/UBSan, OK
make -C firmware voice semantic-ir analyzer sanitizers  # OK
make -C sim sanitizers                         # OK
./prove.sh --quiet                             # 74 invariantes, OK
python3 tools/provenance_audit.py research/software_provenance_manifest.json --strict  # OK
git diff --check                               # OK
```

O manifesto de proveniência e o manifesto de datasets preservam os hashes e o estado de redistribuição. Nenhum TSV, áudio, vídeo, feature binária ou clone de repositório externo deve entrar no commit.

## Próximo bloqueio real

Para medir convergência áudio–texto–vídeo em MIntRec, ainda é necessário verificar os termos de uso da mídia, baixar uma amostra mínima somente se o uso local for permitido, e conferir presença do mesmo identificador em todas as modalidades. Mesmo que essa verificação passe, as 20 classes MIntRec continuam fora do vocabulário HERUS; a comparação inicial deverá permanecer em um benchmark de IR externo, sem produzir comando `ARRIVE`, `HELP` ou `CANCEL`.

### Referências

[1]: https://github.com/thuiar/MIntRec "MIntRec — repositório oficial"
[2]: https://commonvoice.mozilla.org/en/datasets "Common Voice — datasets oficiais"
[3]: https://archive.ics.uci.edu/dataset/465/wesad+wearable+stress+and+affect+detection "UCI WESAD dataset record"
[4]: https://fluent.ai/fluent-speech-commands-a-dataset-for-spoken-language-understanding-research/ "Fluent Speech Commands — descrição dos autores"
