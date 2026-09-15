# Scorecard pré-hardware — rodada de hardening

## Resultado da rodada

O SIM foi ampliado sem abandonar o orçamento finito: além da inferência quantizada e da seleção por representação, agora existe adaptação local por protótipos inteiros. Cada atualização requer uma classe conhecida e proveniência, consome um orçamento de amostras, cria uma nova versão e permite um rollback único. O caminho de execução continua ausente.

Também foram adicionados leases de ação com fencing token para impedir que uma instância antiga continue coordenando uma proposta após expiração, migração ou reconexão. O lease organiza concorrência; não é uma autorização.

## Evidência executada

| Evidência | Resultado |
|---|---|
| Testes Python do repositório | **301 passando, 1 skip esperado** |
| Gate oficial `prove.sh` | **Passou; 44/44 gates host-only** |
| SIM C11 | **Compilou com `-Wall -Wextra -std=c11` e passou** |
| Proveniência | **Manifesto válido; entradas novas declaradas** |
| Projeto paralelo `simbionte-1` | **170 testes passando; auditado seletivamente** |
| Benchmark SIM no host | Inferência ~8,85 µs; atualização ~10,21 µs neste ambiente |

Os tempos do benchmark são apenas regressão do host e não são números do ESP32-S3.

## O que melhorou

A rodada aumentou a confiabilidade do pré-hardware em quatro pontos: aprendizado local deixou de ser apenas estatística e passou a ter versão/rollback; a atualização ficou limitada por orçamento; coordenação entre hospedeiros ganhou fencing; e a contribuição paralela foi registrada com decisão explícita do que foi incorporado e rejeitado.

## O que continua bloqueado

Nenhuma medição de RAM, energia, latência, temperatura, rádio, LRA ou sensor foi promovida como evidência física. O aprendizado de protótipos ainda não foi alimentado por sinais reais. A generalização para um hospedeiro desconhecido real continua uma hipótese a ser testada. O estado do manifesto permanece `pre_hardware` e B1/B2 continuam obrigatórios.

## Critério de saída

O pré-hardware está encerrado quando a placa chegar. A próxima alteração relevante deve incluir uma medição ou falha física, não apenas uma nova abstração. O primeiro experimento deve carregar o SIM C11 em modo `OBSERVE`, medir os recursos e só depois permitir `PROPOSE`.
