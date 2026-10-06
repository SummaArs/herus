# Síntese enumerativa bounded v1

## O que foi implementado

O HERUS agora possui uma faixa pequena de síntese de programas sobre uma gramática finita de expressões inteiras binárias, incluindo composição de profundidade 2. Ela enumera candidatos a partir de exemplos públicos e só retorna `PROPOSE` quando exatamente um comportamento candidato os satisfaz e também passa os exemplos ocultos fornecidos pela tarefa. Formas sintáticas equivalentes são deduplicadas por probes canônicos independentes.

Resultados possíveis:

- `PROPOSE`: solução única passou público e oculto;
- `REJECT_HIDDEN`: solução ajusta o público, mas falha no oculto;
- `ABSTAIN`: tarefa inválida ou exemplos públicos ambíguos.

## Evidência v1

| Tarefa | Público | Oculto | Resultado |
|---|---:|---:|---|
| `add.v1` | 3/3 | passou | `PROPOSE`, `x + y` |
| `composed.v1` | 4/4 | passou | `PROPOSE`, `(x + y) * (x - y)` |
| `contradictory.v1` | 2/2 | falhou | `REJECT_HIDDEN`, `x - y` |
| `ambiguous.v1` | 2 exemplos insuficientes | não usado | `ABSTAIN` |

A evidência bruta está em `research/evidence/program_synthesis_v1.json`.

## O que isso prova

Prova que o HERUS consegue realizar síntese enumerativa pequena, compor expressões, detectar subespecificação e rejeitar uma hipótese que se ajusta aos exemplos públicos mas falha na validação oculta.

## O que não prova

Não prova programação geral, compreensão de linguagem aberta, escalabilidade, superioridade sobre transformers ou utilidade em repositórios grandes. A gramática tem apenas operações inteiras finitas e foi criada para ser auditável.

O próximo passo é ampliar tarefas sem ampliar silenciosamente o oráculo: usar conjuntos versionados, casos de treino públicos e holdouts independentes, comparando contra baselines simples e modelos existentes.
