# Hardening — CI, sanitização e validação

## Objetivo

Este documento registra uma regra operacional: **uma suíte nominal verde não é suficiente quando um diagnóstico de memória ou análise estática consegue encontrar uma falha na mesma trajetória**.

## Gates obrigatórios

A integração deve manter quatro camadas independentes:

1. `./prove.sh` — contratos e invariantes funcionais.
2. `make -C sim sanitizers` — AddressSanitizer/UndefinedBehaviorSanitizer no simulador.
3. `make -C firmware sanitizers` e `make -C firmware analyzer` — memória e análise estática no firmware host-buildable.
4. CodeQL C/C++ — análise de segurança do código compilado no CI.

Nenhuma camada substitui as outras.

## Regra de evidência

- **Host proof** demonstra propriedades da implementação executada no host.
- **Sanitizer proof** demonstra que as trajetórias exercitadas não produziram os tipos de erro instrumentados.
- **Static/security analysis** procura classes de defeito que os testes não atingem.
- **Hardware evidence** continua obrigatória para RF, energia, armazenamento físico, BLE, GPIO, boot, temperatura, antena e comportamento de campo.

Um resultado host-only nunca deve ser promovido automaticamente a alegação de hardware.

## Correção de 2026-08

A campanha de validação identificou um risco no simulador de estudo: o histograma de veredictos era indexado diretamente por `lex_verdict_t`. O domínio agora é derivado de `LEX_VERDICT_COUNT`, e o valor retornado é validado antes da indexação. Um veredicto fora do domínio é tratado como falha da evidência, não como um índice aceitável.

A correção é deliberadamente pequena: ela não muda o protocolo, a autoridade do modelo ou a semântica do produto.

## Política para novos módulos

Todo novo módulo crítico deve incluir:

- pelo menos um teste de entrada nula/limite;
- um caso de valor não canônico quando o tipo permitir representação inválida;
- uma trajetória de falha que permaneça fail-closed;
- execução sob sanitizer quando compilável no host;
- uma descrição explícita do que ainda depende de hardware real.

## Segurança de CI

Actions de terceiros são fixadas por commit SHA. Permissões devem permanecer mínimas. O workflow pode publicar resultados de CodeQL, mas não recebe permissões de escrita no código-fonte.

## Não alegado

Este gate não constitui auditoria criptográfica, certificação de segurança, fuzzing exaustivo, cobertura completa, prova formal, supply-chain attestation ou validação física. Essas lacunas continuam sendo acompanhadas separadamente.
