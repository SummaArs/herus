# Evidência — Rodada de fechamento do MVP crítico

## Escopo

Esta rodada fechou o ciclo do MVP de síntese e verificação de máquinas de estado finitas: auditoria da branch publicada, verificador independente, síntese enumerativa, benchmark, perfil C11 sem heap, testes adversariais, proveniência e gate global.

## Defeitos encontrados

A auditoria encontrou dois defeitos de contrato. O verificador executava `max_steps + 1` camadas de transição, violando o orçamento declarado. A correção passou a executar exatamente `max_steps` transições e adicionou regressão para `max_steps=0`. Também era possível aprovar vacuamente uma política incompleta quando o orçamento era zero; agora toda combinação de estado e entrada precisa ter uma regra antes da exploração, ou o resultado é `UNKNOWN`.

O perfil C11 encontrou anteriormente uma segunda classe de erro: a implementação tratava caminhos duplicados como estados distintos e ultrapassava a capacidade fixa. A deduplicação de estados alcançáveis foi mantida e o perfil retorna `verified=1` com `static_bytes=20`.

## Resultado

A suíte direcionada final tem 13 testes para verificador, síntese e benchmark. O gate de pesquisa completo deve incluir esses testes juntamente com o restante do repositório. O benchmark válido retorna `VERIFIED`; o benchmark com modelo incompleto retorna `UNKNOWN`. Nenhum candidato é promovido com base apenas em ajuste, pontuação ou exploração parcial.

## Limites

A prova é relativa ao modelo finito, à política declarada e ao limite de passos. Ela não prova que sensores, especificação, modelo causal, compilador ou ambiente físico estejam corretos. O perfil C11 é uma proxy de memória fixa no host, não uma medição de ESP32-S3.

## Critério de encerramento da rodada

A rodada só é aceita quando proveniência estrita, 142 ou mais testes de pesquisa, perfil C11, `git diff --check` e `./prove.sh --quiet` passam. A branch deve ser publicada no PR #46 sem merge automático no `main`.
