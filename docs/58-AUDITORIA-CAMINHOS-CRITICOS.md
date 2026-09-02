# Auditoria estrutural de caminhos críticos

## Decisão

O HERUS recebeu um primeiro auditor estrutural de chamadas para reduzir a lacuna entre a auditoria lexical de sinks e uma prova interprocedural completa. O componente está em `research/critical_call_path_audit.py` e é executado somente como pesquisa host-side.

A finalidade é verificar se chamadas diretas a uma operação protegida partem exclusivamente de wrappers declarados no perfil. No estado atual, o perfil declara que `interaction_take_send` só pode ser chamado diretamente por `interaction_take_send_assured`.

## Resultado observado

```text
COVERED interaction-send firmware/core/interaction.c:interaction_take_send_assured
CALL_PATH_AUDIT=PASS
```

A API bruta do adaptador `interaction_rig_take_send` foi adicionalmente tornada fail-closed no firmware: sem snapshot de assurance, retorna `INTERACTION_E_UNTRUSTED`. O caminho explícito `interaction_rig_take_send_assured` fornece o snapshot e delega ao wrapper de produção.

## O que o auditor prova

Dentro do subconjunto suportado, o auditor demonstra que:

1. a função protegida existe em uma fonte existente;
2. o wrapper declarado existe;
3. chamadas diretas observáveis à função protegida aparecem somente nos callers permitidos;
4. uma chamada direta fora do wrapper produz `UNCOVERED`;
5. fonte, função ou chamada ausente produz `UNKNOWN` quando não é possível concluir.

## O que ele não prova

O componente não é parser C11 completo e não prova dominância de controle. Ainda não cobre macros expandidas, ponteiros de função, aliasing, chamadas geradas, análise de pré-processador, caminhos condicionais complexos, unidades de tradução fora do arquivo declarado ou comportamento em assembly/link-time.

Consequentemente, `COVERED` significa apenas **restrição estrutural de callers diretos no escopo declarado**. Não significa que todos os caminhos de execução do firmware tenham sido provados seguros.

## Campanha adversarial

A suíte inclui seis casos: wrapper válido, caller direto proibido, fonte ausente, wrapper ausente, sink sem chamada observável e perfil serializável. A última regra foi adicionada para evitar que ausência de evidência fosse interpretada como cobertura.

A mutação mais importante é substituir o wrapper permitido por um caller `bypass`. O veredicto esperado é `UNCOVERED`, com o nome do caller no resultado. A mutação de remover todas as chamadas produz `UNKNOWN`, não `COVERED`.

## Relação com o produto

Este backend permanece na vertente de pesquisa da branch de integração. Ele não altera o significado do comunicador em `main`, não cria uma autoridade paralela e não autoriza transporte. O snapshot continua sendo uma condição não secreta para solicitar um handoff local de uso único; autenticação, sequência, TTL, prioridade, envio e confirmação física continuam em seus contratos próprios.

## Próxima evolução

A próxima extensão justificável é substituir a extração textual de corpos por uma análise sintática limitada e gerar um grafo de chamadas para múltiplos arquivos. A regra de aceitação permanece conservadora: se o backend não puder representar uma construção sem ambiguidade, retorna `UNKNOWN`.
