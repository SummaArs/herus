# Portabilidade do gate no macOS

## Falha observada

A execução `prove (macos-latest)` falhava no passo `Firmware static analyzer`. O workflow usava `cc` com `-fanalyzer`, mas no runner macOS `cc` é Clang, que rejeita essa opção:

```text
clang: error: unknown argument '-fanalyzer'; did you mean '-Xanalyzer'?
```

O problema era de seleção de backend, não de um defeito detectado no firmware. O erro foi reproduzido a partir dos logs da execução do GitHub Actions `33678366256`, cujo SHA era `edbd33e`.

## Correção

O alvo `make -C firmware analyzer` agora identifica o compilador por `cc --version`:

| Compilador | Backend usado |
|---|---|
| GCC | `-fanalyzer` |
| Clang | `--analyze` |

Ambos os caminhos mantêm `-O0`, warnings estritos, C11 e `-Werror`. Qualquer diagnóstico continua interrompendo o job. A correção não ignora o analisador, não transforma falha em sucesso e não usa uma ferramenta específica do sistema operacional.

## Validação local

No ambiente Linux com GCC, o alvo foi executado com sucesso e o firmware completo continuou passando. A execução do branch após o push será a validação autoritativa do backend Clang no `macos-latest`; o ambiente local não contém Clang e, portanto, não deve fingir essa validação.

## Regra de manutenção

Novos flags de análise estática devem ser selecionados por capacidade do compilador, não pelo sistema operacional. O CI deve continuar executando o mesmo contrato semântico em GCC e Clang, aceitando apenas diferenças de interface inevitáveis entre os backends.
