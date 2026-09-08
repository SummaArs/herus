# HERUS — Gate de entrada em hardware v0

**Estado:** `HARDWARE_REQUIRED_FOR_NEXT_PHYSICAL_CLAIMS`

**Baseline:** commit `7c73933`, branch `integration/herus-meaning-hardening-2026-09`, PR #48 empilhado sobre `feat/herus-camada-do-significado`.

## Decisão

O software host-only atingiu o ponto em que continuar falando sobre rádio, reset, energia, latência, persistência física e operação em dois dispositivos sem medir o alvo produziria apenas especulação. O hardware agora é **necessário para o próximo conjunto de afirmações físicas**.

Isso não significa que o HERUS esteja pronto para operação crítica. Significa que a próxima falsificação de maior valor não pode ser resolvida somente por Python, C11 host, simulação ou AST.

## Pré-condições host-only satisfeitas

Antes deste gate, as seguintes propriedades foram demonstradas:

| Pré-condição | Resultado |
|---|---|
| Workflow propaga falhas através de `tee` | `pipefail` ativo |
| `prove.sh` global | `ALL INVARIANTS HOLD` |
| CI remoto | Ubuntu e macOS verdes no PR #48 |
| Contrato mínimo de intenção | Schema v0 validado |
| Transporte bruto do console | Fail-closed e coberto por contraprova |
| Cadeia C11 | Babel → HIR → Aether → HIR → Babel passa |
| Interoperabilidade mínima | C e Python concordam em frame, HIR e render em 3 vetores |
| Demonstração host-only | Mensagem íntegra aceita; cinco bytes adulterados rejeitados |
| Proveniência | Manifesto estrito válido |
| Loom | Orçamento de 21.581 B medido e envelope subdimensionado rejeitado |

## O que o hardware deve tentar falsificar

O primeiro alvo físico não é “provar que o produto funciona”. É tentar quebrar o contrato em uma sequência pequena, observável e repetível.

### Inicialização e identidade do alvo

O firmware deve compilar no toolchain real, inicializar, reiniciar sob comando e alcançar um estado conhecido sem reviver autoridade de uma sessão anterior. A versão do firmware, o perfil Aether e o digest do pacote devem ser registrados antes de cada ensaio.

### Transporte físico

Dois dispositivos devem trocar um frame produzido pelo pipeline de significado pelo canal físico escolhido. O resultado permitido é binário: o receptor reconstrói exatamente o HIR esperado ou recusa. Um frame corrompido, truncado, fora de perfil, fora de versão ou sem preâmbulo não pode produzir significado aceito.

### Falha de alimentação

A alimentação deve ser interrompida em cada fronteira relevante: antes da preparação, durante o transporte, antes da confirmação, durante a persistência e depois do commit. Após o reinício, o sistema deve descartar autoridade transitória, não promover uma transação incompleta e não afirmar sucesso sem evidência durável.

### Autoridade e confirmação

O dispositivo não pode transformar a recepção de um significado em autoridade de execução. Ações com sink físico devem exigir a autoridade definida pelo contrato, confirmação correspondente, sessão válida e propósito correto. Um pacote transportado, mesmo íntegro, não pode autorizar por si só.

### Recursos reais

Devem ser medidos no alvo o tamanho final de flash, RAM livre, stack watermark, tempo de compilação/decodificação, latência de confirmação, energia por tentativa, comportamento do watchdog e temperatura operacional da bancada. Valores de datasheet ou host não substituem medições.

## Sequência mínima de bancada

| Ensaio | Entrada | Resultado obrigatório |
|---|---|---|
| B1 | Boot limpo | Estado inicial conhecido, sem autoridade herdada |
| B2 | Frame íntegro | HIR esperado e nenhuma autoridade implícita |
| B3 | Corrupção acima do RS/CRC | Recusa, nunca HIR alternativo |
| B4 | Perda de canal | Estado pendente ou recusa; nunca sucesso falso |
| B5 | Reset antes da confirmação | Confirmação consumida ou descartada segundo contrato |
| B6 | Corte durante persistência | Estado recuperado fail-closed e sem rollback promovido |
| B7 | Replay de frame/confirmacão | Recusa idempotente |
| B8 | Sessão expirada | Recusa antes do sink |
| B9 | Propósito incompatível | Recusa antes do sink |
| B10 | Duas implementações/duas unidades | Mesmo envelope, mesmo veredicto e mesma razão de recusa |

## Condições imediatas de aborto

O ensaio deve parar e o certificado físico deve permanecer `UNPROVEN` se ocorrer qualquer um dos eventos abaixo: frame adulterado aceito; HIR diferente aceito como equivalente sem contrato; autoridade reaparecendo depois de reset; confirmação reutilizada; persistência declarada como concluída sem evidência; rollback durável promovido; sink executado com propósito incompatível; watchdog ou corrupção de memória sem classificação; ou divergência entre o veredicto do alvo e o veredicto host sem explicação versionada.

Um resultado negativo não será corrigido relaxando a política. A correção deve ser feita no contrato, no firmware ou na instrumentação, seguida por um novo ensaio com digest novo.

## Resultado que pode ser declarado depois da bancada

Se todos os ensaios passarem, a declaração permitida será limitada a: **“o perfil físico v0 foi demonstrado no hardware-alvo sob as condições documentadas”**. Isso ainda não significará secure boot completo, segurança de produção, resistência a ataques de laboratório, operação prolongada, segurança médica, segurança aeronáutica ou uso crítico.

Se qualquer ensaio falhar, o resultado correto será uma evidência de falha física reproduzível e uma atualização do contrato. Falha é resultado científico; não é motivo para transformar o gate em aviso.

## Fronteira atual

O HERUS está pronto para **ser confrontado pelo hardware**. O hardware agora é demandado para avançar nas propriedades físicas, mas a compra continua sendo uma decisão de bancada experimental, não uma autorização de implantação. O próximo artefato deve ser um protocolo de ensaio com captura de logs, alimentação, reset, canal, timestamps e hashes, preservando o baseline `7c73933` como controle.
