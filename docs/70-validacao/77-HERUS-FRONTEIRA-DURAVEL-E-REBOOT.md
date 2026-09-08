# HERUS — Fronteira durável, reboot e memória semântica

**Estado:** host-side verificado; persistência física, brownout e comportamento do alvo continuam pendentes de bancada.  
**Objetivo:** garantir que o reboot restaure apenas o estado mínimo anti-replay necessário e não reintroduza sessão física, contexto semântico ou autoridade de uma execução anterior.

## Resultado executável

A etapa adicionou `memory_reboot_boundary`, uma composição explícita entre o bootstrap de sessão física e o índice semântico em RAM. A ponte usa as APIs existentes de recuperação, não cria uma segunda política de armazenamento e não abre uma via automática para reindexar cards duráveis.

| Controle | Resultado |
|---|---:|
| Matriz de fronteira de reboot | **8/8** |
| Redteam de reboot | **4/4 mutantes mortos** |
| Regressão global | **74 suítes** |
| Invariantes da simulação | **111/111** |
| Proveniência | **válida; entradas locais unsigned permanecem não atestadas externamente** |

## Contrato de consistência

O cofre durável e o índice semântico não têm o mesmo papel. O cofre pode conservar um card mínimo autenticado, mas o índice semântico é volátil e contém evidência derivada com geração local. Depois de um reboot, a ponte limpa o índice e restaura apenas o `session_floor` obtido pela recuperação autenticada. Um card durável não volta automaticamente a ser evidência consultável; ele precisa atravessar novamente uma operação explícita e autorizada de reindexação.

A sessão física segue a mesma regra. Um registro `COMMITTED` coerente com o floor pode atualizar o piso anti-replay, mas não pode restaurar `active_session_id`, nonce, propósito, orçamento de usos ou qualquer evento físico. O estado final é `IDLE`. Uma nova sessão precisa de ID estritamente acima do floor e de uma nova asserção física fornecida pelo adaptador.

Snapshots não autenticados ou contraditórios bloqueiam a fronteira inteira. Nesse caminho, o gate fica `BLOCKED`, o índice semântico é limpo e o resultado é zerado. Não existe recuperação parcial que possa parecer bem-sucedida para uma camada superior.

## Cenários provados

A suíte inicia com uma sessão ativa e evidência semântica presente. Após o bootstrap, o floor é preservado, mas os campos ativos da sessão e todos os registros do índice são zerados. O ID igual ao floor é rejeitado; um novo ID acima do floor só é aceito com evento físico confirmado.

A matriz também testa snapshot não autenticado, snapshot de recuperação ausente e índice semântico ausente. Cada caso produz erro apropriado, bloqueia o gate quando necessário e não retorna flags de recuperação positiva. Esse detalhe impede que uma camada de interface trate uma recuperação incompleta como uma nova permissão.

## Mutantes adversariais

A frente sabotadora removeu a limpeza do índice antes do bootstrap, a limpeza do caminho de argumentos inválidos, a validação de índice nulo e a declaração explícita de `semantic_index_scrubbed`. Todos os mutantes foram detectados.

| Mutante | Controle removido | Oráculo que o mata |
|---|---|---|
| `reboot-semantic-scrub` | Limpeza de evidência volátil | Índice não pode conservar fatos após reboot |
| `reboot-argument-failure-scrub` | Limpeza ao receber snapshot ausente | Erro não pode deixar estado residual |
| `reboot-null-index-gate` | Validação da fronteira sem índice | Recuperação parcial é rejeitada |
| `reboot-scrub-result-claim` | Declaração do índice limpo | Resultado não pode alegar limpeza que não demonstrou |

Durante o desenvolvimento, uma primeira versão do redteam apontou um mutante sobrevivente. A análise mostrou que ele removia uma limpeza externa redundante porque o bootstrap interno já limpava o gate e o resultado naquele caminho. Isso não foi contado como vitória. O oráculo foi separado em dois cenários independentes — snapshot ausente e índice ausente — e somente então a campanha atingiu 4/4. A lacuna foi corrigida no teste, não escondida no relatório.

## Limites honestos

Esta etapa não prova que um ESP32-S3 sobreviverá a brownout, que NVS ou flash respeitarão atomicidade, que o root material estará protegido, que o relógio monotônico resistirá a perda de energia ou que um adaptador físico fornecerá evento legítimo. Também não prova que uma reindexação posterior será útil, rápida ou energeticamente viável.

A afirmação sustentada é mais restrita: no modelo C11 host-side, a fronteira de reboot permite atravessar apenas um floor anti-replay coerente. Sessão ativa, nonce, propósito, uso restante e evidência semântica não atravessam. Falhas de entrada bloqueiam e limpam, e uma nova autoridade exige uma nova evidência física.

## Reprodução

```bash
cd /home/ubuntu/herus-semantic-compiler-pr
python3 tools/test_memory_reboot_redteam.py
cd firmware
make memory-reboot-boundary memory-reboot-redteam
cd ..
python3 tools/provenance_audit.py --strict research/software_provenance_manifest.json
./prove.sh --quiet
```

O veredito `ALL INVARIANTS HOLD` refere-se aos contratos host-side. O próximo gate continua sendo a bancada controlada, com alimentação interrompida, flash/NVS real, reboot, pin map, interfaces e instrumentação para confirmar ou refutar as hipóteses físicas.
