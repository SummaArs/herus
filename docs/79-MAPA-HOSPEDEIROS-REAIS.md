# Mapa de hospedeiros reais do HERUS

## Regra de classificação

Um hospedeiro real não é necessariamente um dispositivo físico. Há três categorias distintas: um ambiente físico real, um serviço externo real e um ambiente de software real que reproduz restrições e falhas observáveis. Todos podem produzir evidência, mas cada um prova propriedades diferentes.

O HERUS deve treinar somente propriedades operacionais finitas, como latência, custo, taxa de falha, capacidade, utilidade e deriva. Ele não pode aprender autoridade, identidade, permissão de efeito ou relaxamento de verificação a partir de observação estatística.

## Mapa atual

| Hospedeiro | Estado | Treino útil agora | Efeito permitido | Próximo gate |
|---|---|---|---|---|
| Computador H0 | real e disponível | residência, memória, custo, detach/rebind | nenhum sem autoridade externa | tarefas ocultas e deriva |
| Filesystem local | real e disponível | proveniência, rollback, integridade | escrita não destrutiva | campanha de crash e adulteração |
| Processos sandbox | real e disponível | timeout, RAM, falha, leases | rede e efeitos destrutivos negados | campanha hostil de processos |
| GitHub HERUS | serviço real autorizado | histórico, risco de mudança, testes | revisão e publicação controladas | auditoria de mutação |
| Internet oficial | informação real externa | claims finitos de capacidades | nenhuma execução | conflito e obsolescência |
| Datasets públicos | amostras reais auditadas | mudança de distribuição, parser, tempo | pesquisa local | licença, split e leakage |
| Simulador robótico | software real bounded | estado, zona segura, trajetória, parada | simulação e shadow | mundos ocultos e replay |
| Sandbox financeiro | dados reais sem efeito | leakage temporal e risco | proposta/backtest | conta de papel independente |
| Servidor shadow | alvo externo real quando conectado | canary, rollback e deriva | nenhuma mutação de produção | connector explícito |
| ESP32-S3 H1 | hardware real pendente | RAM, energia, latência, temperatura | bancada B1–B10 | identidade e boot |
| Pulso H2 | hardware físico futuro | LRA, bateria, confirmação humana | interação limitada | H1 mais bring-up físico |
| Robô H3 | sistema físico futuro | sensores, atuadores e recuperação | shadow/canary governado | simulador e operador de segurança |

## O que fazer agora, sem esperar o pulso

A melhor ordem é treinar e testar o HERUS em H0, filesystem, processos sandbox, Internet allowlisted, datasets reais auditados e simuladores. Esses ambientes já permitem produzir evidência sobre residência, adaptação, limites, falhas, deriva e migração computacional.

Não é útil baixar “toda a Internet” nem misturar corpora sem proveniência. O ganho científico vem de tarefas ocultas, mudanças de versão, conteúdo conflitante, orçamento reduzido, reinicializações, dados fora da amostra e tentativas explícitas de escalada de autoridade.

## Sequência recomendada

```text
H0 computador
→ filesystem e recuperação
→ processos sandbox hostis
→ Internet allowlisted e documentos oficiais
→ datasets reais com splits preservados
→ simulador robótico
→ sandbox financeiro
→ servidor shadow autorizado
→ H1 ESP32-S3
→ H2 pulso
→ H3 hospedeiros físicos adicionais
```

## Critério de promoção

Um hospedeiro só sobe de `OBSERVE` para `PROPOSE`, `SIMULATE`, `SHADOW`, `CANARY` ou `HUMAN_BOUND` quando houver evidência específica para esse domínio. A adaptação pode escolher uma representação menor, arquivar uma Skill ou se abster. Ela não promove automaticamente nenhum efeito físico ou financeiro.

O registro operacional completo está em `research/evidence/real_host_training_registry.json`.
