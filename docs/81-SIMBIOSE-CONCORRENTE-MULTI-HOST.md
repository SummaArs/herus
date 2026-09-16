# Simbiose concorrente multi-hospedeiro

## A hipótese

Simbiose geral não deve significar apenas sair de um hospedeiro e entrar em outro. A hipótese mais forte é que uma identidade HERUS possa manter múltiplas residências ativas ao mesmo tempo, cada uma com seu contexto e suas restrições, enquanto troca experiências operacionais verificadas.

## Implementação

O runtime cria workers concorrentes para computador, filesystem, sandbox de processos, Internet, datasets, simulador robótico, sandbox financeiro e servidor shadow. Todos usam a mesma identidade `herus-general`, mas cada worker produz observações com seu próprio `host_id` e digest.

Uma barreira garante que todos os hospedeiros publiquem antes da fase de consumo. Assim, a troca não é apenas uma sequência acidental: cada residência observa experiências de todas as outras. O barramento é append-only, deduplicado e limitado.

## O que pode atravessar o barramento

Somente estimativas operacionais bounded:

```text
latency
memory
energy
failure_rate
utility
drift
capability
```

Não atravessam:

```text
authority
identity
private_context
message_content
secret
execution
```

A experiência compartilhada pode mudar seleção de Skill, orçamento ou abstenção. Ela não pode autorizar um efeito, substituir verificação ou transportar memória privada de um hospedeiro para outro.

## Resultado

O ensaio executa oito hospedeiros concorrentes com a mesma identidade. Cada um produz duas experiências, consome as experiências dos outros sete e retorna `PROPOSE` sem execução. O estado compartilhado preserva experiência operacional; a autoridade permanece ausente.

[Evidência reproduzível](../research/evidence/concurrent_multi_host_symbiosis.json)

## Limite da alegação

Isto prova simultaneidade computacional e troca controlada de experiência em workers locais. Não prova oito dispositivos físicos ligados ao mesmo tempo, nem garante generalidade aberta. A próxima evolução é substituir gradualmente os workers por adaptadores reais: Internet live, datasets, simulador, servidor shadow e depois ESP32/pulso.

> **O corpo de cada hospedeiro muda; as experiências úteis podem circular; identidade, autoridade e contexto privado não circulam.**
