# Wide Research 05 — convergência com dados reais

## Veredito

O ciclo encontrou uma rota real para auditar texto e metadados do SLURP sem baixar o archive completo nem o áudio. O texto oficial foi obtido do repositório dos autores, no commit `8eb16545762be97ace75334109d73824217311f1` [1]; o metadata JSON foi obtido como um único membro do archive Zenodo por HTTP Range [2], com validação de cabeçalho ZIP, deflate, tamanho e CRC.

**Convergência áudio–texto–intenção localmente verificada: não. Convergência HERUS: não provada.** O áudio SLURP é CC BY-NC 4.0 [1] e não foi baixado.
O join textual–metadata cobre todas as referências, mas não é unívoco devido a uma filename duplicada e uma referência textual repetida. As intenções externas não são mapeadas para `ARRIVE`, `HELP` ou `CANCEL`.

## Fontes e permissões

| Fonte | Obtido | Licença/termos auditados | Alinhamento que pode ser afirmado | Bloqueio |
|---|---|---|---|---|
| SLURP original | JSONL textual; snapshot `8eb16545762be97ace75334109d73824217311f1` [1] | Texto CC BY 4.0; áudio CC BY-NC 4.0 [1] | Texto com referências de gravação; metadata por filename | Áudio não baixado; duplicata impede join unívoco |
| Zenodo `11106554` | 20.000.000 bytes por Range; metadata.json e LICENSE.txt como membros isolados [2] | Registro declara CC BY 4.0, mas LICENSE interno e repo original declaram áudio CC BY-NC 4.0 [1][2] | Estrutura ZIP e dois membros verificados | Terceiro upload, 141.759 membros, lineage de áudio não validado |
| Fluent Speech Commands | Páginas, paper e licença [3][4][7] | Uso acadêmico/não comercial; não compartilhar material [4] | Alinhamento declarado pela fonte | Endpoint histórico redireciona; pacote não obtido |
| Sonos/Snips | README e licença [5] | Pesquisa acadêmica; acesso exige formulário [5] | Alinhamento declarado pela fonte | Grant necessário; sem download |
| MIntRec | `train.tsv`/`test.tsv` auditados no ciclo 04 [6] | Mídia não licenciada para redistribuição no material auditado | Metadata text/label; mídia não validada | Não é resultado multimodal HERUS |

Nenhum áudio, vídeo, archive completo ou clone externo foi versionado. O `.gitignore` mantém dados externos fora do Git.

## Auditoria textual SLURP

| Medida | Resultado |
|---|---:|
| Linhas train/devel/test | 16.521 |
| `slurp_id` únicos | 16.521 |
| Duplicatas entre splits | 0 |
| Intents externos | 93 |
| Cenários externos | 18 |
| Referências de gravação únicas no texto | 72.395 |
| Nomes de gravação únicos no metadata | 72.395 |
| Linhas com todas as referências presentes | 16.521 |
| Referências presentes | 72.396/72.396 |
| Filenames duplicados no metadata | 1 |
| Referências textuais repetidas | 1 |
| Conflitos de assinatura da duplicata | 0 |
| Áudio carregado | não |
| Mapeamento automático para HERUS | 0 |

A licença interna do archive foi extraída e verificada: texto CC BY 4.0 e áudio CC BY-NC 4.0 [1][2].
O metadata JSON tem 72.396 gravações, todas `.flac`, status `correct`; esses valores são estatísticas do metadata, não prova de qualidade acústica.

## Parser C real

O parser português C do firmware foi executado sobre o campo `sentence` das 16.521 linhas JSONL reais. O wrapper não publicou sentenças nem IDs e não leu áudio.

| Saída do parser | Contagem |
|---|---:|
| Linhas JSONL vistas | 16.521 |
| Linhas enviadas ao C | 16.521 |
| Falhas de extração JSONL | 0 |
| `DRAFT` | 0 |
| `CANCEL_LOCAL` | 0 |
| `UNKNOWN` | 16.426 |
| `REJECTED` | 95 |
| Linhas longas | 0 |
| Texto vazio | 0 |
| Bytes não ASCII | 0 |
| Autoridade de comando HERUS | 0 |

O corpus é inglês e o parser é português controlado. Portanto, `UNKNOWN`/`REJECTED` é comportamento fora do domínio, não acurácia de reconhecimento.

## Falhas encontradas e correções

A primeira auditoria tentou relacionar `slurp_id` textual a `nlub_id` do metadata e produziu apenas 4 coincidências. A inspeção da primeira linha mostrou que o metadata é indexado por outro contador; o join foi corrigido para `recordings[].file`, que é o identificador de arquivo compartilhado pelos dois artefatos.

O primeiro relatório do metadata listava 88.925 nomes de chaves que incluíam IDs numéricos e filenames. Isso foi tratado como vazamento de identificadores desnecessários: o relatório final publica somente contagens agregadas e hashes.

O probe ZIP de 4 MiB e 16 MiB não continha o diretório central completo. O probe foi corrigido para usar uma cauda limitada de 20 MiB, verificar `206 Partial Content` e ler o diretório central de 19.893.275 bytes. O archive contém 141.759 membros. Um fetch isolado de 1.538.361 bytes verificou o `metadata.json`; outro fetch de 65.781 bytes verificou o `LICENSE.txt`.

O teste unitário inicialmente marcava como válido um mesmo filename repetido em três splits. O fixture foi corrigido para usar nomes distintos; a duplicata passou a ser explicitamente bloqueada.

## Artefatos e reprodução

Os scripts `probe_remote_zip.py`, `fetch_zip_member.py`, `analyze_slurp_metadata.py`, `analyze_slurp_text.py`, `run_slurp_voice.py`, `benchmark_ir.py`, `paired_audio_audit.py` e `real_jsonl_voice_runner.c` são host-only. O `Benchmark IR` representa slots externos em vocabulário finito e não possui caminho de autoridade para HERUS.

Comandos locais usados:

```text
make -C research test
make -C research slurp-text-audit SLURP_ROOT=/tmp/slurp-original SLURP_METADATA=/tmp/slurp_metadata.json
make -C research slurp-text-sanitizers SLURP_ROOT=/tmp/slurp-original
```

O alvo pareado Fluent falha fechado sem paths explícitos:

```text
make -C research paired-audio-audit
# exit 2: missing FSC_ROOT; no Fluent package was downloaded
```

## Limitações restantes

A rota atual não prova convergência multimodal porque nenhum byte de áudio foi decodificado. Não é permitido contornar a licença CC BY-NC 4.0 do áudio com o terceiro archive Zenodo. A duplicata de filename/reference permanece bloqueada, mesmo com assinaturas idênticas, até existir um protocolo de identidade autorizado.

As 93 intenções SLURP e os 18 cenários não foram convertidos para a Semantic IR operacional. `ARRIVE`, `HELP` e `CANCEL` continuam sem cobertura anotada por dados reais neste ciclo. O parser C é pré-hardware: nenhum resultado deste relatório mede ESP32-S3, SX1262, energia, RF, latência física ou robustez de reset.

## References

[1]: https://github.com/pswietojanski/slurp "SLURP official repository and modality-specific LICENSE.txt"
[2]: https://zenodo.org/records/11106554 "SLURP-flac-files Zenodo record"
[3]: https://fluent.ai/fluent-speech-commands-a-dataset-for-spoken-language-understanding-research/ "Fluent.ai Fluent Speech Commands dataset page"
[4]: https://fluent.ai/wp-content/uploads/2021/04/Fluent_Speech_Commands_Public_License.pdf "Fluent Speech Commands Public License"
[5]: https://github.com/sonos/spoken-language-understanding-research-datasets "Sonos spoken-language-understanding research datasets"
[6]: https://github.com/thuiar/MIntRec "MIntRec official repository"
[7]: https://arxiv.org/html/1904.03670 "Speech Model Pre-training for End-to-End Spoken Language Understanding"

## Gates finais

A bateria final passou em todos os comandos abaixo:

```text
make -C research test                         # 59 testes, OK
make -C research real-corpus                  # MIntRec, OK
make -C research real-corpus-sanitizers      # MIntRec ASan/UBSan, OK
make -C research slurp-text-audit ...        # SLURP, OK
make -C research slurp-text-sanitizers ...   # SLURP ASan/UBSan, OK
make -C firmware all                          # OK
make -C firmware analyzer                      # GCC analyzer, OK
make -C firmware sanitizers                   # ASan/UBSan, OK
make -C sim sanitizers                        # 74 invariantes, OK
./prove.sh --quiet                            # invariantes globais, OK
provenance_audit --strict                     # 1 active, 3 pending; válido
git diff --check                               # OK
```

Houve uma falha intermediária legítima: proteger a árvore inteira de evidências por digest fez o log de proveniência alterar o próprio input protegido. A árvore mutável foi removida de `protected_inputs`; os arquivos de código, scripts, testes, manifestos e Makefile continuam protegidos individualmente. A prova foi então repetida e passou.
