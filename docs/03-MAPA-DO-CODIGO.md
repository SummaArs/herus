# HERUS — Mapa do código

## Regra de organização

Os diretórios executáveis permanecem onde estão para preservar imports, Makefiles, commits históricos e proveniência. A organização nova é semântica: cada diretório recebe um papel explícito e cada papel aponta para uma entrada executável.

## Visão de diretórios

| Diretório | Papel | Entrada principal | O que não afirmar |
|---|---|---|---|
| `firmware/core/` | Núcleo C, memória, interação, ameaça e invariantes | `make -C firmware` | Não é prova de hardware real. |
| `firmware/net/` | Rádio, enlace, sessão, confiança e criptografia | `make -C firmware` | Não é alcance físico medido. |
| `firmware/port/` | Abstração de plataforma e SX1262/ESP32-S3 | `firmware/port/esp32s3/` | Não é bring-up concluído. |
| `sim/` | Mundo C controlado com distância e adversários | `make -C sim` | Não substitui a bancada. |
| `research/` | Experimentos Python, contratos, dados e auditorias | `make -C research test` | Não é produto distribuível. |
| `research/symbiont_v2/` | Runtime host-independent de Skill finita | `python3 -m research.symbiont_v2.benchmark` | Não é AGI nem controlador geral. |
| `research/stage4/` | Contratos e harnesses de execução sintética | testes de pesquisa | Não é executor externo real. |
| `research/social/` | Protocolo de futura prova humana | `python3 -m research.social.runner` | Não contém resultados humanos. |
| `research/evidence/` | Resultados, manifests, holdouts e auditorias | comandos documentados em cada artefato | Proveniência local não é assinatura externa. |
| `tools/` | Auditores, geradores, budgets e validadores | scripts individuais | Um auditor só garante seus próprios critérios. |
| `docs/` | Design, contratos, história, limites e decisões | este mapa | Documento histórico não é estado atual automaticamente. |

## Firmware C

### `firmware/core/`

`nucleus.c`, `sbc.c`, `hv.c`, `lexicon.c` e `text.c` formam a base semântica. `intent_gate.c`, `dialogue.c`, `interaction.c` e `interaction_rig.c` tratam proposta, diálogo e interação física. Os módulos `memory_*` implementam captura, cofre, consolidação, coleção, recuperação, sessões e finais compostos. `assurance.c` e `threat_model.c` fazem a composição de assurance e ameaças.

Os arquivos `test_*.c` são testes de contrato. Eles devem ser lidos junto ao módulo correspondente: primeiro o comportamento esperado, depois a implementação, depois o registro no proof.

### `firmware/net/`

`crypto.c`, `trust.c`, `session.c` e `core_link.c` formam a cadeia de confiança e enlace. `link.c`, `beat.c`, `region.c`, `weave.c` e `sx1262.c` tratam o caminho de rádio e suas restrições. O resultado atual é uma verificação estática/simulada; a realidade RF depende da bancada.

## Pesquisa Python

`semantic_ir.py` trata a representação intermediária semântica. `finite_reasoner.py` experimenta raciocínio finito. `symbiosis_utility.py` classifica contratos de utilidade. `holdout_adversarial.py` preserva a campanha pública negativa. `extended_holdout.py` mantém famílias reservadas.

O pacote `symbiont_v2` é o centro da tese ASA. `core.py` possui a fronteira `HostAdapter`, o `WorldModel`, as Skills e a proposta. `stage5.py` possui estados checked e ledger de orçamento. `stage4/` possui contratos de execução sintética. `bridge_*.py` compõe a rodada causal e o produto local.

## Entradas canônicas

```bash
./prove.sh --quiet
make -C research test
make -C sim
PYTHONPATH=research python3 -m research.bridge_causal
PYTHONPATH=research python3 -m research.bridge_product
python3 tools/provenance_audit.py --strict research/software_provenance_manifest.json
```

## Convenção para mudanças futuras

Uma mudança deve começar por um contrato ou teste que declare o comportamento. Depois deve alterar o menor módulo necessário. Em seguida deve atualizar a documentação do papel, executar o gate local e atualizar o manifesto de proveniência. Um novo número no README sem um comando que o reproduza é uma falha de documentação.

## Referências

[1]: ../firmware/README.md "Guia do firmware"
[2]: ../research/README.md "Guia da pesquisa"
[3]: ../sim/README.md "Guia do simulador"
[4]: ../tools/README.md "Guia das ferramentas"
