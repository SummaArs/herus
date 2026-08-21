# HERUS — Floor semântico e reindexação pós-reboot

**Estado:** host-side verificado; o mapeamento do floor para NVS/flash real ainda depende de bancada.  
**Objetivo:** impedir que uma evidência semântica antiga seja reintroduzida depois de um reboot apenas porque o card durável continua presente.

## Problema enfrentado

O HERUS possui duas camadas diferentes: o cofre durável conserva um card mínimo autenticado, enquanto o índice semântico mantém evidência tipada em RAM para consulta local. Limpar a RAM no reboot elimina contexto residual, mas sozinho não impede que uma camada posterior reinsira uma evidência com geração antiga.

A evolução desta etapa adiciona um `generation_floor` ao índice semântico. A fronteira de reboot instala esse floor depois de limpar o índice e importar o floor de sessão autenticado. A reindexação só pode ocorrer com geração estritamente maior que o floor. Um card durável, portanto, não é automaticamente conhecimento corrente apenas porque abriu com autenticação válida.

## Resultado executável

| Controle | Resultado |
|---|---:|
| Índice semântico existente | **19/19** |
| Matriz de reboot, floor e contexto semântico | **14/14** |
| Redteam de reboot, floor e contexto | **10/10 mutantes mortos** |
| Regressão global | **74 suítes** |
| Invariantes simuladas | **111/111** |
| Proveniência | **válida; entradas locais permanecem não atestadas externamente** |

## Semântica implementada

O floor semântico só pode ser definido em um índice vazio. Essa regra evita instalar um floor novo sobre evidência ainda presente. O floor é monotônico: uma tentativa de diminuí-lo retorna `MSE_E_ROLLBACK`. Quando instalado, qualquer `mse_add` com geração zero, igual ou anterior retorna `MSE_E_ROLLBACK` sem aumentar `evidence_count`.

A ponte `memory_reboot_boundary_bootstrap` executa a seguinte ordem: limpa a evidência semântica volátil, fecha a janela contextual mágica, recupera o floor de sessão por meio do bootstrap já existente, instala o mesmo floor no índice semântico quando ele é não nulo e bloqueia a fronteira se qualquer etapa falhar. O resultado informa o floor semântico importado e confirma o fechamento contextual, mas não retorna card, fato, sessão, nonce, propósito, uso ou autoridade.

Uma nova evidência com geração nove pode suceder um floor oito; uma evidência de geração oito não pode. Isso transforma o reboot em uma fronteira de consistência, não em uma simples reinicialização de structs.

## Ataques adversariais

A campanha removeu a limpeza semântica, o fechamento contextual, a limpeza de argumentos inválidos, a validação de índice nulo, as declarações de scrubbed, a importação do floor, a rejeição de geração stale, a monotonicidade e a regra de índice vazio. Cada remoção fez a suíte falhar.

| Mutante | Controle removido | Resultado esperado pelo oráculo |
|---|---|---|
| `reboot-semantic-scrub` | Limpeza de evidência RAM | Fato antigo permanece observável |
| `reboot-argument-failure-scrub` | Limpeza em erro de entrada | Falha deixa estado residual |
| `reboot-null-index-gate` | Validação da composição | Recuperação parcial é aceita |
| `reboot-scrub-result-claim` | Prova declarada de scrub | Resultado alega uma operação não comprovada |
| `reboot-semantic-floor-import` | Instalação do floor no índice | Reindexação stale volta a ser possível |
| `semantic-floor-stale-rejection` | Gate de geração | Evidência no floor é aceita |
| `semantic-floor-monotonicity` | Anti-diminuição | Floor pode retroceder |
| `semantic-floor-empty-only` | Pré-condição de índice vazio | Floor é aplicado sobre evidência ativa |
| `reboot-contextual-scrub` | Janela mágica permanece ativa | Contexto residual pode sobreviver ao reboot |
| `reboot-contextual-scrub-result` | Flag de fechamento contextual | Resultado alega scrub incompleto |

Durante a campanha, um mutante de limpeza sobreviveu inicialmente porque o bootstrap interno já fazia uma limpeza redundante no caminho específico exercitado. Isso foi tratado como uma lacuna de cobertura, não como mutante morto. O teste foi dividido em caminhos independentes para snapshot ausente e índice ausente. O ataque só passou a ser considerado morto depois que cada limpeza necessária tinha um oráculo observável.

## Limites honestos

Nada nesta etapa comprova a atomicidade de NVS, a durabilidade do floor em flash, a proteção de root material, o comportamento durante brownout ou a recuperação depois de perda de alimentação no ESP32-S3. Também não comprova que uma reindexação autorizada será energeticamente viável ou semanticamente útil.

O resultado comprovado é mais estreito: no host C11, o cofre durável não reintroduz automaticamente contexto semântico; o índice pós-reboot inicia vazio, a janela contextual é fechada, recebe no máximo um floor anti-replay coerente e rejeita evidência stale. Um sucessor posterior precisa atravessar a API de reindexação e respeitar a nova geração.

## Reprodução

```bash
cd /home/ubuntu/herus-semantic-compiler-pr
python3 tools/test_memory_reboot_redteam.py
cd firmware
make memory-semantic-evidence memory-reboot-boundary memory-reboot-redteam
cd ..
python3 tools/provenance_audit.py --strict research/software_provenance_manifest.json
./prove.sh --quiet
```

O veredito global aplica-se aos contratos host-side. O próximo gate continua sendo uma bancada controlada para confirmar ou refutar o vínculo entre floor lógico, persistência real e reboot físico.
