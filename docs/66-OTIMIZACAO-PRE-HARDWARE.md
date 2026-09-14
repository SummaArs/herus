# Otimização pré-hardware do HERUS

## Escopo

Esta rodada otimiza caminhos host-only sem alterar a fronteira de autoridade, o vocabulário finito ou os resultados semânticos. O objetivo é reduzir trabalho repetido antes da validação no ESP32-S3. Não é correto converter uma redução de CPU no host em uma promessa de autonomia ou consumo no hardware físico.

## Alterações

O planejador de descoberta (`research/host_probe_planner.py`) agora mantém um cache limitado de 128 vocabulários finitos. Antes, cada chamada convertia novamente formatos, interfaces e alvos de latência em conjuntos. O cache é determinístico, limitado e não altera a ordem de desempate.

O vínculo simbiótico (`research/symbiotic_models.py`) passou a calcular o digest do `HostProfile` uma vez por vínculo e reutilizar o resultado para o evento de continuidade e o `SelfModel`. Antes, o mesmo digest era recalculado em chamadas separadas.

O benchmark (`research/optimization_benchmark.py`) fixa o workload em 200.000 chamadas e cinco repetições. Ele mede somente latência de parede do planejador no host de desenvolvimento.

## Resultado medido

| Métrica | Antes | Depois | Variação |
|---|---:|---:|---:|
| Mediana para 200.000 chamadas | 4,9545 s | 4,8459 s | **2,19% menor** |
| Vazão | 40.367 chamadas/s | 41.272 chamadas/s | **2,24% maior** |
| Conversões de vocabulário por chamada | 3 | 0 após aquecimento do cache | redução do trabalho repetido |
| Cálculos de digest por vínculo | 3 | 1 | **66,7% menos chamadas de hash** |

A diferença de 2,19% é o resultado observado em uma única máquina sob um workload sintético controlado. Ela não deve ser tratada como ganho universal. O efeito no ESP32-S3 só será conhecido com medição de ciclos, memória, temperatura, rádio e energia.

## Confiabilidade preservada

A suíte direcionada passou com 17 testes, incluindo os testes originais de descoberta e os novos testes simbióticos. O cache não concede capacidades: ele apenas reutiliza conjuntos derivados de argumentos já fornecidos. O digest continua sendo calculado sobre a representação canônica do host. A execução continua bloqueada sem autorização externa.

## Percentual honesto de otimização

Para o caminho efetivamente medido, o HERUS ficou **2,19% mais rápido** em latência mediana e **2,24% mais rápido em vazão**. Em trabalho de hash do vínculo, houve redução de **66,7% nas chamadas redundantes**. Não há ainda um percentual válido de economia de energia, RAM física ou tempo de rádio: essas métricas dependem do alvo embarcado e permanecem pendentes do hardware.

O percentual global do HERUS não é calculado como média simples. Um número único misturaria firmware, Python, compilação, rádio e simulação com pesos arbitrários. A próxima etapa correta é medir cada orçamento no host físico e publicar os resultados por componente.
