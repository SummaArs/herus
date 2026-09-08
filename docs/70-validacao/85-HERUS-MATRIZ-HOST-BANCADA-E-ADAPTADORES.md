# HERUS — Matriz host, bancada e adaptadores

**Estado do release:** `pre_hardware`.  
**Baseline host:** `./prove.sh --quiet`.  
**Regra:** nenhum resultado host-side autoriza alegação física, humana ou de produto.

## 1. Princípio de transição

A arquitetura agora separa o núcleo de decisão dos adaptadores que fornecem eventos físicos. O host pode provar ordem, validade, expiração, conflito, abstention, quarentena, autoridade e scrub. A bancada precisa verificar se o mundo real fornece os eventos com qualidade suficiente e se pessoas entendem o comportamento.

A bancada não deve ser usada para descobrir pela primeira vez se uma perda de contato cria autoridade ou se um reboot revive memória. Esses contratos já são exercitados no host. Ela deve responder perguntas diferentes: o DRV2605L produz o sinal esperado, o PMIC mede o custo, o botão registra o contato, a NVS sobrevive ao brownout, o rádio troca os frames e uma pessoa interpreta a experiência.

## 2. Matriz de evidência

| Capacidade | Prova host-side atual | Adaptador físico futuro | Gate físico correspondente | Estado honesto |
|---|---|---|---|---|
| Presença ambiente | **13/13**; oferta única, hold, cooldown, expiração, contato | haptic/UI local e contato | `haptic-bring-up`, `interaction-io` | Política provada; percepção humana pendente |
| Vida semântica | **16/16**; preferência, objetivo, conflito, contexto e consulta | extrator semântico local | `local-model-profile`, `interaction-io` | Composição tipada provada; relevância real pendente |
| Falhas físicas | **14/14**; clock, haptic, contato, energia, power-loss e semântica | relógio, PMIC, DRV2605L, GPIO/sensor | `energy-instrumentation`, `haptic-bring-up`, `board-pin-map` | Degradação lógica provada; sinais elétricos pendentes |
| Redteam semântico | **8/8**; autoridade, conflito, scrub, floor, expiry e quarentena | nenhum novo adaptador | Reexecutado antes da bancada | Controle não decorativo no host |
| Redteam geral | **7/7** mutantes | nenhum novo adaptador | Reexecutado antes da bancada | Campanha global ativa |
| Memória pós-reboot | Floors, sucessores, stale facts, conflitos e abstention | NVS/flash e método de interrupção | `collection-crash-recovery`, `collection-physical-session` | Semântica portável provada; durabilidade pendente |
| Haptic semântico | Plano tipado e safety stop no host | DRV2605L + atuador + I²C real | `haptic-bring-up` | Segurança de plano provada; reprodução física pendente |
| Energia | Orçamento determinístico e supressão fail-closed | PMIC/instrumento | `energy-instrumentation` | Sem consumo físico alegado |
| Rádio | Contratos, autenticação e bench virtual | SX1262, antena, pin map e ambiente | `radio-bring-up`, `urban-pdr`, `tier05-comparison` | Alcance/PDR físicos pendentes |
| Confiança e revogação | Contratos de sessão, revogação e quarentena | armazenamento, RNG e transporte alvo | `companion-trust-port` | Lógica host provada; portabilidade pendente |
| Modelo local | Gates de agência, memória, latência e orçamento | modelo compilado no alvo | `local-model-profile` | Nenhum WER, acurácia ou energia alegado |
| Mecânica | Nenhuma prova host de montagem física | shell, stack e atuador | `mechanical-volume` | Dimensão física pendente |

## 3. O que deve ser ajustado quando o hardware chegar

O primeiro ajuste deve ocorrer nos adaptadores, não no significado dos estados. O relógio físico deve alimentar a geração monotônica sem permitir regressão; o PMIC deve fornecer medições com unidade declarada; o haptic deve consumir apenas planos que o núcleo marcou como seguros; o contato deve ser reportado como evento distinto de autorização; e o backend de persistência deve executar a matriz de interrupção sem reinterpretar estados preparados como comprometidos.

| Adaptador | Contrato que deve respeitar | Falha que não pode ser mascarada |
|---|---|---|
| Relógio/reset | Geração monotônica ou política explícita de reset | Evento fora de ordem aceito como atual |
| DRV2605L | Plano HAP-SEM validado, safety stop e erro I²C observável | Falha elétrica apresentada como vibração entregue |
| PMIC | Energia medida, unidade e workload declarados | Estimativa publicada como medição |
| Contato | Evento físico separado de confirmação/ação | Debounce ou perda convertidos em consentimento |
| NVS/flash | Transições PREPARED/floor/COMMITTED comprovadas | Registro parcial promovido após brownout |
| Percepção local | Evidência tipada, confiança e privacidade | Texto/áudio/identidade vazando para log ou Core |
| Rádio/Core | Transporte autenticado sem autoridade de execução | Core externo promovido a cérebro ou executor |

## 4. Gate de entrada da bancada

A bancada só deve começar após `./prove.sh --quiet` passar e o manifesto continuar em `pre_hardware`. O primeiro ensaio físico deve ser isolado e reversível. Ele não deve misturar simultaneamente haptic, rádio, modelo, NVS e interação humana, porque um resultado combinado não permite atribuir causalidade.

A ordem recomendada é: verificar pin map e selftest; medir haptic em fixture isolado; medir energia com workload declarado; validar relógio e interrupção; validar NVS sob power-loss; validar rádio a um metro; executar interação com protocolo congelado; e somente depois combinar adaptadores. Nenhuma dessas etapas deve alterar o estado de confiança do manifesto sem a evidência correspondente.

## 5. Veredito desta etapa

O host-side chegou a uma fronteira útil: a lógica pessoal e adversarial pode avançar sem o hardware, enquanto as incertezas restantes estão nomeadas como interfaces mensuráveis. Isso não significa que o HERUS já funciona no pulso. Significa que a bancada terá perguntas mais precisas e menos espaço para confundir um mock com uma prova.

> A melhor simulação não elimina a bancada. Ela impede que a bancada seja usada para esconder uma arquitetura ainda indefinida.

## Reprodução

```bash
cd /home/ubuntu/herus-semantic-compiler-pr
python3 tools/provenance_audit.py --strict research/software_provenance_manifest.json
./prove.sh --quiet
```
