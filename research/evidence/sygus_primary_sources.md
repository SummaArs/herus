# Fontes primárias SyGuS — 2026-09-01

## Escopo verificado

A especificação oficial descreve SyGuS-IF como uma linguagem próxima de SMT-LIB 2 e apresenta problemas LIA com `set-logic`, `synth-fun`, gramática, variáveis declaradas, restrições e `check-synth`.

A documentação oficial informa que a versão 2.1 adiciona suporte a oráculos, SyGuS ponderado, teoria de tabelas, CHC e simplificações sintáticas. O tradutor do HERUS não deve aceitar essas extensões enquanto elas não tiverem semântica implementada.

O repositório oficial de benchmarks declara que contém os benchmarks públicos da competição, mas também alerta que podem existir erros e duplicatas. Portanto, cada instância usada pelo HERUS precisa de validação local e o número bruto de arquivos não pode ser reportado como número de problemas independentes.

## Fontes

1. SyGuS-Org, “Specification Language”, https://sygus-org.github.io/language/
2. SyGuS-Org, “SyGuS-Org/benchmarks”, https://github.com/SyGuS-Org/benchmarks
3. SyGuS-Org, “Benchmarks & Tools”, https://sygus-org.github.io/artifacts/

## Decisão metodológica

A primeira comparação executável será restrita a uma gramática explícita de inteiros lineares sem `ite`, Booleanos, arrays, bit-vectors, strings, quantificadores, múltiplos `synth-fun`, oráculos e extensões de versão 2.1. O resultado será separado em: parsing aceito, tradução semântica, solução encontrada pelo baseline limitado, validação independente e contraexemplo/UNKNOWN. Compatibilidade textual não será contada como solução.
