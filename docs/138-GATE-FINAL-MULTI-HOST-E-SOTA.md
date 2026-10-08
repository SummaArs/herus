# Gate final: multi-host, generalização e SOTA

## Decisão atual

O HERUS **não está 100% validado**. O protocolo do gate foi implementado, mas a execução científica permanece bloqueada até que exista evidência real suficiente.

Isso é deliberado: um contrato correto não é um resultado experimental e um teste unitário não prova SOTA.

## O que o gate exige

A validação final deve usar um dataset real, com pelo menos três hosts ou domínios previamente identificados. Cada host precisa possuir:

- partição de ajuste e holdout disjuntas;
- feedback próprio do host destino, posterior à migração;
- pelo menos três seeds independentes;
- ledger de evidência retida e evidência em quarentena;
- auditoria de leakage sem violações;
- score do HERUS e de pelo menos um baseline pareado;
- direção da métrica explicitamente registrada;
- contagem de exemplos e artefatos versionados.

O protocolo também exige separar quatro camadas:

```text
HERUS-core
adaptador de modalidade
baseline especializado
resultado completo
```

Apenas o resultado que isola o HERUS-core pode sustentar uma afirmação sobre a contribuição do algoritmo. Um encoder ou uma engenharia específica que vença o baseline não pode ser apresentado como vitória geral do HERUS.

## Critérios de aprovação

O gate só pode avançar para análise quando:

1. os três hosts forem reais e identificáveis;
2. os holdouts não forem usados para ajuste;
3. nenhum feedback futuro entrar na decisão;
4. a migração preservar apenas evidência compatível;
5. evidência incompatível permanecer bloqueada;
6. o host destino aprender com feedback próprio;
7. o resultado superar controles simples em uma métrica previamente registrada;
8. o ganho sobreviver às seeds e aos holdouts;
9. custo, cobertura, abstenção e erro forem publicados junto do score;
10. o resultado puder ser reproduzido a partir dos artefatos versionados.

Mesmo com todos esses critérios, o resultado inicial seria **evidência de ganho adaptativo em domínios definidos**, não SOTA geral. Para declarar SOTA, ainda seria necessário comparar com baselines atuais relevantes da modalidade e com o estado da arte publicado no mesmo protocolo.

## Estado versionado

O artefato `research/evidence/multi_host_gate_v1.json` registra que a execução ainda não foi realizada. Ele não contém números sintéticos e não promove fixtures a dados reais.

O validador `research/multi_host_gate.py` falha fechado quando faltam hosts, seeds, identidade do dataset, split, ledger ou controle de leakage.
