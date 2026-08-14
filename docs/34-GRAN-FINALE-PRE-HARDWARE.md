# Gran Finale pré-hardware: cadeia de memória pós-reboot

**Status:** composição C11 portátil verificada em host. Ela junta, sem conceder autoridade, o bootstrap de sessão, a auditoria M14 da coleção e a classificação TM-04. O único sucesso termina com o gate em **`IDLE`**; ele não contém uma sessão reativada.

## Objetivo final da série pré-hardware

Os passos anteriores provaram fronteiras separadas: uma coleção limitada e transacional, um índice privado que preserva abstenção, um oráculo de recuperação de registros, uma sessão vinculada a propósito, uma classificação de reserva de sessão e uma quarentena que importa somente piso após boot. A lacuna restante era composicional: não havia um ponto verificável que obrigasse essas fronteiras a coexistir e que provasse que o caminho de memória pós-reboot ainda começa sem capacidade ativa.

`memory_prehardware_finale.[ch]` é esse ponto. A função recebe somente referências de configuração de sessão, snapshot de reserva, snapshot M14 e snapshot do modelo de ameaças. Ela executa o bootstrap, confirma a quarentena do gate, audita M14 e exige que TM-04 resulte em `MITIGATED_HOST`. O resultado é diagnóstico: `ready_for_target_validation` nunca autoriza inserir, abrir, consultar, apresentar, apagar, reter ou transmitir.

A NIST SP 800-160 Vol. 1 Rev. 1 descreve princípios e tarefas de engenharia para sistemas seguros confiáveis ao longo do ciclo de vida [1]. A NIST SP 800-160 Vol. 2 Rev. 1 situa a resiliência como capacidade de antecipar, suportar, recuperar e adaptar-se a condições adversas [2]. O HERUS usa essas publicações somente como orientação para manter composições, dependências e falhas explícitas. **Não** reivindica conformidade, sistema confiável, resiliência cibernética ou garantia física por causa deste auditor host.

> A composição final é uma porta de verificação, não uma porta de autorização. Em êxito, o gate continua `IDLE`; uma operação futura ainda requer `begin()` com ID estritamente acima do piso recuperado e nova afirmação canônica de evento do adaptador alvo.

## Contrato e ordem obrigatória

| Etapa | Verificação C11 | Efeito de falha |
|---|---|---|
| 1. Bootstrap | `memory_physical_session_bootstrap()` revalida configuração e classifica a reserva. | Gate é zerado e marcado `BLOCKED`. |
| 2. Quarentena | Gate deve estar `IDLE`; piso recuperado coincide; ID, nonce, propósito, prazo, usos e métricas transitórias são zero. | Gate é zerado e marcado `BLOCKED`. |
| 3. Coleção M14 | `memory_collection_finale_audit()` exige captura/política/revisão, coleção autenticada, sessão vinculada, índice limitado, abstenção, sem auto-open/fallback/modelo. | Gate é zerado e marcado `BLOCKED`. |
| 4. TM-04 | `threat_model_assess(THREAT_MODEL_MEMORY_RETENTION)` deve retornar apenas `MITIGATED_HOST`, sem bit de falha. | Gate é zerado e marcado `BLOCKED`. |
| 5. Resultado | Emite ação e piso recuperados apenas como diagnóstico; `ready_for_target_validation == 1`. | Não existe resultado permissivo. |

A função não recebe cartão, ID de cartão, consulta, texto, áudio, transcrição, embedding, localização, pessoa, evento, nonce, relógio, chave, armazenamento, callback, modelo, rádio ou rede. Ela não chama `memory_physical_session_begin()`, nem realiza acesso à coleção. Assim, a composição não pode transformar uma auditoria em atalho de memória.

## Prova adversarial G1

A suíte `test_memory_prehardware_finale.c` usa um backend RAM de teste, mas executa módulos reais de coleção, recuperação, índice, bootstrap, M14 e TM-04. Antes do auditor final, ela persiste um cartão mínimo autorizado e reabre a coleção autenticada. Depois, verifica o comportamento pós-reboot.

| Contraprova | Veredito exigido |
|---|---|
| Reserva `COMMITTED` coerente no piso 100 | Resultado G1 é diagnóstico e gate fica `IDLE` com piso 100, sem sessão ativa. |
| Consulta sem novo `begin()` | Índice devolve `E_ACCESS`; resultado permanece zerado. |
| Tentar `begin()` com ID 100 | Gate recusa: ID no piso não volta a ser sessão. |
| Tentar sucessor com confirmação `0` | Gate recusa: o bootstrap não fabrica evento. |
| Novo sucessor 101 com confirmação canônica | Consulta tipada pode chegar ao índice, mas retorna apenas match mínimo e não abre cartão. |
| Auto-open declarado no snapshot M14 | Auditor final bloqueia e limpa o gate. |
| Ausência somente de `memory_physical_session_bootstrap_quarantined` em TM-04 | Auditor final bloqueia e limpa o gate. |
| Reserva declarada sem autenticação | Bootstrap e composição bloqueiam; nenhuma decisão permissiva é emitida. |
| Entrada obrigatória ausente | Erro de argumento e gate bloqueado quando disponível. |

```sh
cd firmware
make memory-prehardware-finale
cd ..
./prove.sh --quiet
```

O ledger esperado é de **35 suítes**, **79 invariantes de prova** e **74 invariantes de sistema simulado**. Esses números mostram contratos exercitados em host e cenários de simulação existentes; não medem confiabilidade física, taxa de erro de fala, energia, latência, qualidade de LLM ou probabilidade de ataque.

## O que o sucesso permite afirmar

| Afirmação limitada | Base verificável |
|---|---|
| A cadeia host pode compor bootstrap, M14 e TM-04 sem reativar uma sessão. | G1 exige gate `IDLE` e campos ativos zerados antes de emitir sucesso. |
| Uma consulta de coleção não passa sem sessão nova. | G1 chama índice real depois de bootstrap ocioso e recebe `E_ACCESS`. |
| O ID recuperado não pode ser reutilizado. | G1 tenta iniciar exatamente no piso e o gate recusa. |
| Nova sessão ainda não abre cartão automaticamente. | G1 alcança índice com sucessor válido e compara que métricas de `open` não aumentam. |
| Falha de qualquer fronteira composta impede prontidão host. | G1 exercita reserva não autenticada, M14 com auto-open, TM-04 sem quarentena e argumentos ausentes. |

## O que o sucesso não permite afirmar

A composição não prova que o snapshot veio de armazenamento autêntico, que operações sobreviveram a corte de energia, que o reset ocorreu, que RAM física foi apagada, que a cadeia de boot é confiável, que o evento é humano ou novo, que nonce/tempo são fortes, que o piso é inviolável, que replay pós-reboot é resistente no alvo, que flash/NVS/eFuse/secure element têm as propriedades esperadas, que coleta/índice são privados em mídia real, que há RF/energia/latência aceitáveis, que ASR/LLM entendem a pessoa, que a memória é relevante, nem que uma interface é compreendida ou acessível.

Essas lacunas não são falhas documentais. Elas definem a próxima fase: selecionar adaptador e backend, executar uma matriz instrumentada de `PREPARED`/piso/`COMMITTED`/boot, verificar cadeia de boot e proteção de estado, medir energia/RF/UX e somente então decidir se alguma evidência de alvo pode substituir `PENDING_TARGET`.

## Gate de transição para hardware

O Gran Finale não libera fabricação, coleta humana ou alegação de segurança física. Ele fecha o conjunto de contratos portáveis e transforma a fase seguinte em uma campanha falsificável de adaptador.

| Evidência alvo pendente | Critério mínimo antes de alegação correspondente |
|---|---|
| Reset e limpeza | Mostrar em placa que o caminho de boot executa a quarentena antes de toda operação de coleção. |
| Persistência de reserva | Interromper cada escrita de `PREPARED`/piso/`COMMITTED` e registrar classificação/reação. |
| Evento novo | Demonstrar fonte de evento, nonce e política de tempo/reset sem expor dados de produto. |
| Proteção de plataforma | Documentar/medir boot seguro, armazenamento, debug, raiz e isolamento no alvo que for escolhido. |
| Anti-replay pós-reboot | Executar tentativas de repetição contra o backend real sob reinicialização e interrupção. |
| UX e privacidade | Avaliar botão/gesto, haptics/voz, compreensão e schema de telemetria com consentimento. |

## Referências

[1] [NIST SP 800-160 Vol. 1 Rev. 1 — *Engineering Trustworthy Secure Systems*](https://csrc.nist.gov/pubs/sp/800/160/v1/r1/final), DOI: [10.6028/NIST.SP.800-160v1r1](https://doi.org/10.6028/NIST.SP.800-160v1r1).

[2] [NIST SP 800-160 Vol. 2 Rev. 1 — *Developing Cyber-Resilient Systems: A Systems Security Engineering Approach*](https://csrc.nist.gov/pubs/sp/800/160/v2/r1/final), DOI: [10.6028/NIST.SP.800-160v2r1](https://doi.org/10.6028/NIST.SP.800-160v2r1).
