# Firmware HERUS

## Papel

Este diretório contém o núcleo C, a rede, os ports e os testes do hospedeiro físico pretendido. Ele implementa contratos de semântica, interação, memória, confiança, recuperação e rádio em uma forma verificável por compilação, testes e simulador.

## Ordem de leitura

1. `Makefile` — alvos oficiais e toolchain.
2. `core/nucleus.*`, `core/sbc.*`, `core/hv.*`, `core/lexicon.*` — representação e núcleo.
3. `core/intent_gate.*`, `core/dialogue.*`, `core/interaction.*` — intenção e fronteira humana.
4. `core/memory_*` — captura, política, cofre, consolidação, coleção e recuperação.
5. `core/assurance.*`, `core/threat_model.*` — assurance e ameaças.
6. `net/trust.*`, `net/session.*`, `net/core_link.*`, `net/crypto.*` — confiança e enlace.
7. `net/region.*`, `net/beat.*`, `net/weave.*`, `port/sx1262.*` — rádio e orçamento.
8. `core/test_*.c` e `net/test_*.c` — comportamento esperado e regressões.

## Verificação

```bash
make -C firmware test
make -C firmware sanitizers
make -C firmware analyzer
```

O resultado prova contratos no ambiente de build. A placa real ainda depende dos gates B1–B10 em `research/evidence/`.

## Regra de autoridade

Nenhum módulo de percepção, Semantic IR ou modelo local deve obter autoridade física apenas por produzir uma mensagem, uma intenção ou uma proposta. A confirmação e os limites físicos continuam sendo fronteiras separadas.
