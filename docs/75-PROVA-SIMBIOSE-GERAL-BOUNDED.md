# Prova de simbiose geral — bounded

## Resultado

O H0 agora funciona como primeiro hospedeiro real e produz um bundle de saída governado. Esse bundle foi consumido por uma rotina de migração que vincula a mesma identidade a cinco classes declaradas de hospedeiro: computador, embedded, pulso, robô e veículo.

Em cada migração, a identidade `herus_id` permanece, o digest do hospedeiro muda, o contexto do mundo começa vazio, a autoridade permanece `NONE` e a execução retorna `ABSTAIN`. A migração rejeita alvo igual ao hospedeiro de origem, tipos desconhecidos, bundles com autoridade forjada e bundles que tentam transportar contexto.

## O que isso prova

A prova estabelece uma propriedade de **continuidade contratual multi-hospedeiro** dentro de um universo finito de perfis. Em outras palavras, o núcleo não está estruturalmente preso ao computador: ele pode sair de H0 e reconstruir uma ligação limpa com perfis representados de outros hospedeiros.

## O que isso não prova

Os nomes `embedded`, `wrist`, `robot` e `vehicle` nesta prova são classes de perfil, não dispositivos físicos operando. Ainda não há evidência de ESP32-S3, sensores, LRA, rádio, bateria, robô ou veículo reais. Também não há prova de adaptação a um hospedeiro arbitrário fora do vocabulário e dos contratos finitos.

A matriz completa está em `research/evidence/multi_host_symbiosis_matrix.json`.

## Próximo salto de evidência

A prova seguinte não deve adicionar mais nomes de hospedeiros simulados. Deve executar o mesmo protocolo em uma unidade física real:

```text
H0 computador
→ H1 ESP32-S3
→ H2 pulso com LRA, rádio e bateria
→ H3 retorno ao computador ou migração para outro dispositivo
```

Somente depois dessas medições será legítimo atualizar o estado de `host_only_bounded` para evidência física correspondente.
