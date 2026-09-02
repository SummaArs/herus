# Certificado composto de assurance

## Objetivo

O HERUS agora possui um orquestrador de evidências finitas em `research/critical_assurance_certificate.py`. Ele não substitui os verificadores existentes. Sua função é impedir que uma cadeia incompleta seja descrita como assurance completo.

O certificado compõe cinco obrigações:

```text
verificação abstrata
        + verificação concreta
        + refinamento da máquina
        + refinamento da política
        + cobertura do caminho crítico
        = certificado composto
```

A saída positiva é `ASSURED` somente quando todas as obrigações retornam seus estados positivos. Uma ausência de evidência não é convertida em sucesso.

## Veredictos

| Veredicto | Condição |
|---|---|
| `ASSURED` | As duas máquinas e políticas são verificadas, ambas as relações de refinamento são satisfeitas e todos os caminhos declarados estão `COVERED` |
| `COUNTEREXAMPLE` | Existe violação na política, na máquina ou no refinamento |
| `INVALID` | Há especificação, mapa ou política estruturalmente inválida |
| `BLOCKED` | As verificações finitas passaram, mas algum caminho crítico não está coberto |
| `UNKNOWN` | Uma obrigação de verificação não pôde ser concluída |

A prioridade dos veredictos é deliberada. Contraexemplos e invalidade não são escondidos por uma camada posterior. `BLOCKED` representa ausência de cobertura de caminho, não sucesso parcial.

## Evidência

Cada certificado contém os certificados individuais, os resultados dos caminhos críticos e um digest SHA-256 canônico dos modelos, políticas, mapas e resultados estruturais usados na composição. O digest identifica a evidência da rodada; não é uma assinatura nem uma prova de autenticidade do mundo físico.

## Limites

O certificado composto continua relativo a modelos finitos. Ele não demonstra que o modelo representa adequadamente sensores ou ambiente, não substitui validação física, não prova C11 completo e não autoriza transporte, persistência ou atuação. O componente é host-only e permanece na branch de pesquisa.

A composição tem valor porque transforma várias garantias locais em uma condição de integração verificável. Ela também torna explícita a falha mais perigosa: todas as partes simbólicas podem passar e, ainda assim, a cadeia deve permanecer `BLOCKED` se o caminho crítico não estiver coberto.
