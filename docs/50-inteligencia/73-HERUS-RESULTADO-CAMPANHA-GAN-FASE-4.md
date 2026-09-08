# HERUS — Resultado da campanha GAN adversarial da fase 4

**Estado:** concluído no host, publicado na branch `feat/herus-semantic-compiler`.  
**Commit:** `f8f3aca` — `test: attack Core absence and combined failures`.  
**Escopo:** feed de conhecimento do Core, journal persistente de cursor, ausência do Core, reboot, revogação de consentimento, exaustão de memória e composição de falhas.

## 1. Resultado executivo

A fase 4 transformou os ataques isolados contra autonomia, magia e memória em uma campanha de resiliência que atravessa fronteiras de módulo. A frente sabotadora não recebeu uma API privilegiada: ela adulterou entradas, removeu controles por mutação de fonte, interrompeu a persistência modelada, revogou consentimento durante uma janela pendente e tentou usar a ausência do Core como atalho de autoridade.

A execução final de `./prove.sh --quiet` terminou com o veredito `ALL INVARIANTS HOLD`. O resultado confirmou **66 suítes**, **111 invariantes de sistema simulado**, **24/24 casos combinados**, **7/7 mutantes de autonomia**, **5/5 mutantes de magia/memória** e **7/7 mutantes do Core**. A proveniência local também passou em estado `local_unattested`; isso significa que os digests locais conferem com o manifesto, não que exista uma atestação externa assinada.

| Gate | Resultado | O que foi demonstrado |
|---|---:|---|
| Regressão global | **66 suítes** | Ledger, módulos C11, ferramentas, simulação e checks agregados sem falha |
| Invariantes do simulador | **111/111** | Cenários sistêmicos host-side preservam os contratos declarados |
| Autonomia redteam | **7/7** | Contexto, consentimento, one-shot, escopo, canonicalidade e bindings mortos quando removidos |
| Magia/memória redteam | **5/5** | Privacidade, consentimento, orçamento, estrutura e conflito mortos quando removidos |
| Core redteam | **7/7** | Digest, namespace/source, rollback, ausência, HMAC e readback mortos quando removidos |
| Falhas combinadas | **24/24** | Abstenção, quarentena, revogação e limites preservados sob composição |
| Proveniência | **válida, local não atestada** | Entradas declaradas conferem; nenhuma confiança externa foi inventada |

## 2. Ataques combinados executados

A matriz `firmware/core/test_core_resilience_matrix.c`, executada por `tools/test_combined_failures.py`, cobre cinco famílias de composição. Ela verifica o efeito observável e também impede efeitos laterais como fallback remoto, apresentação não autorizada, avanço de cursor sem confirmação ou sobrescrita de evidência.

| Combinação adversarial | Resultado observado |
|---|---|
| Core ausente + memória local conflitante | `MRB_CONTRADICTED`, sem seleção de cartão e sem consulta remota implícita |
| Reboot + slot mais novo adulterado | `KFC_E_AUTH`; o slot antigo não é promovido silenciosamente |
| Consentimento revogado + proposta contextual pendente | `MAGIC_TRIGGER_SILENT`; nenhuma apresentação ou consumo de orçamento |
| Feed válido + Core indisponível + ausência de confirmação | `KF_PROPOSED`; diálogo e cursor permanecem sem promoção |
| Memória cheia + nova evidência | `MSE_E_FULL`; a contagem existente permanece e não ocorre sobrescrita |

Esses casos não afirmam que o dispositivo consegue detectar todos os modos físicos de falha. Eles demonstram somente que, quando esses eventos são representados pelos adaptadores host-side definidos no contrato, o caminho seguro prevalece sobre a conveniência de recuperar ou aceitar estado.

## 3. Mutantes do Core mortos

A campanha `tools/test_core_redteam.py` recompila a suíte real depois de remover exatamente um controle crítico por vez. Um mutante é considerado morto somente quando a suíte deixa de passar; uma compilação que falha ou um mutante não alcançado não seria contado como vitória.

| Mutante | Controle removido | Suíte que o detecta |
|---|---|---|
| `feed-digest-gate` | Comparação do digest canônico do payload | `knowledge-feed` |
| `feed-namespace-gate` | Validação de source/namespace | `knowledge-feed` |
| `feed-rollback-gate` | Rejeição de sequência repetida ou antiga | `knowledge-feed` |
| `core-absence-gate` | Conversão indevida de ausência em disponibilidade | `knowledge-feed` |
| `cursor-hmac-gate` | Autenticação do registro persistente | `knowledge-feed-cursor` |
| `cursor-rollback-gate` | Piso monotônico no commit | `knowledge-feed-cursor` |
| `cursor-readback-gate` | Verificação autenticada após escrita | `knowledge-feed-cursor` |

A suíte do feed também foi ampliada para **17/17** ao adicionar um caso independente de `source_kind` desconhecido com namespace e fatos coerentes. Isso evita que uma validação posterior de handles mascare a remoção do gate de origem.

## 4. O que a execução permite afirmar

É válido afirmar que os contratos determinísticos implementados na árvore local resistem aos cenários enumerados, que a ausência do Core não concede autoridade, que uma confirmação não atravessa escopo e que a composição de conflito, reboot, revogação e exaustão termina em estado seguro observável. Também é válido afirmar que os mutantes críticos selecionados foram alcançados e mortos pelas suítes correspondentes.

> A vitória desta campanha é uma propriedade do contrato exercitado no host. Ela não é uma demonstração de que o HERUS já funciona como produto, nem uma prova de inteligência geral, conversação aberta, reconhecimento de voz, autonomia de bateria, alcance de rádio ou segurança física.

## 5. Limites que permanecem abertos

Nenhum resultado físico foi inventado. A campanha ainda não mede brownout, atomicidade elétrica, desgaste de flash, persistência real em NVS, secure element, ruído de I²C, comportamento do SX1262, antena, distância, interferência, latência, consumo, temperatura, conforto, qualidade háptica, WER, acurácia, interação humana ou desempenho do ESP32-S3 sob carga real. O `BOARD_HAS_HAPTIC_I2C=0` permanece desabilitado até a verificação física do pin map.

Também não há uma LLM hospedada ou um modelo local geral incorporado por esta fase. O resultado preserva o princípio soberano do HERUS: o Core pode alimentar conhecimento autorizado, mas não é o cérebro, não escolhe entre memórias conflitantes e não fabrica confirmação. A próxima campanha deve tratar os limites de transporte físico, persistent storage real e carga computacional antes de qualquer conclusão sobre uso cotidiano.

## 6. Reprodução

A reprodução exige um checkout da branch publicada, C11 estrito e Python 3 da biblioteca padrão:

```bash
cd /home/ubuntu/herus-semantic-compiler-pr
python3 tools/test_combined_failures.py
python3 tools/test_core_redteam.py
./prove.sh --quiet
python3 tools/provenance_audit.py --strict research/software_provenance_manifest.json
```

O comando global só deve ser usado como autorização para a próxima etapa controlada de bancada, nunca como autorização para apagar os gates físicos ou para tratar simulação como observação do mundo real.
