# Rodada 7 — Drift temporal, memória stale e hysteresis

## Conclusão

A Rodada 7 adicionou dimensão temporal ao detector de aplicabilidade. O ASA passou a manter estados `ACTIVE` e `ABSTAIN`, com hysteresis: duas observações inválidas consecutivas desativam propostas; três observações frescas consecutivas recuperam a operação.

O resultado foi positivo no escopo definido. Uma falha isolada não gerou falso alarme. Drift persistente, memória stale e burst de frames ausentes produziram abstention. Frames reorder foram recusados individualmente sem desativar o contrato porque não houve persistência suficiente. Em nenhum cenário houve proposta quando a observação atual era inválida.

O mecanismo comprova uma política temporal bounded, não detecção geral de drift no mundo aberto.

## Hipótese falsificável

> **H7:** Uma incompatibilidade temporal persistente deve interromper propostas após um limiar fixo, enquanto uma falha isolada não deve produzir falso alarme. A recuperação só deve ocorrer depois de uma sequência independente de observações frescas.

A hipótese seria falsificada se uma única falha desativasse o sistema, se drift persistente mantivesse propostas, se a recuperação ocorresse com evidência insuficiente ou se uma proposta fosse emitida para uma observação stale, reorder ou missing.

## Política

O monitor usa `bad_limit=2` e `good_limit=3`. Uma observação é válida somente quando o contrato é compatível e a qualidade é `fresh`.

No estado `ACTIVE`, uma observação inválida incrementa a sequência de falhas. Ao atingir duas falhas, o estado muda para `ABSTAIN`. No estado `ABSTAIN`, uma observação fresca incrementa a sequência de recuperação. Três observações frescas consecutivas retornam o sistema a `ACTIVE`. Uma observação inválida interrompe a recuperação.

A proposta é permitida somente quando a observação atual é válida e o estado após a avaliação é `ACTIVE`. Isso impede que a primeira observação ruim seja emitida apenas porque o limiar de desativação ainda não foi atingido.

## Cenários

| Cenário | Descrição |
|---|---|
| `clean` | 20 observações frescas compatíveis |
| `single_transient` | uma observação incompatível isolada |
| `persistent_drift` | oito observações incompatíveis consecutivas |
| `stale_memory` | três observações stale consecutivas |
| `reordered_frames` | um frame reorder isolado |
| `dropped_frame_burst` | dois frames missing consecutivos |
| `drift_and_recovery` | três observações incompatíveis seguidas e depois observações frescas |

## Resultados

| Cenário | Propostas ASA | Propostas reuso cego | Início de abstention | Recuperação | Decisões inseguras |
|---|---:|---:|---:|---:|---:|
| Limpo | 20 | 20 | — | — | **0** |
| Falha transitória única | 19 | 19 | — | — | **0** |
| Drift persistente | 10 | 12 | amostra 7 | amostra 16 | **0** |
| Memória stale | 15 | 20 | amostra 6 | amostra 10 | **0** |
| Frame reorder isolado | 19 | 20 | — | — | **0** |
| Burst de frames missing | 16 | 18 | amostra 6 | amostra 9 | **0** |
| Drift e recuperação | 15 | 17 | amostra 6 | amostra 10 | **0** |

O baseline de reuso cego continua propondo em todas as amostras compatíveis com seu parser, ignorando qualidade temporal. O ASA sacrifica cobertura quando necessário e nunca propõe em uma observação inválida.

## Interpretação

O limiar de duas falhas evita que uma anomalia única desative o sistema. Ao mesmo tempo, ele não permite que o contrato antigo continue sendo usado indefinidamente sob drift persistente.

A recuperação é deliberadamente mais difícil que a desativação. Isso cria hysteresis contra oscilações e evita que uma única observação boa reative uma competência ainda não estabilizada.

A perda de cobertura em `stale_memory` e `dropped_frame_burst` é esperada. Ela representa o custo de não usar informação obsoleta ou ausente como se fosse evidência atual.

## Falha metodológica corrigida

A primeira versão do baseline contava somente observações frescas. Isso tornava o baseline artificialmente parecido com o monitor em cenários stale e reorder. A correção fez o baseline cego ignorar qualidade temporal, como deveria: ele conta qualquer observação compatível e, portanto, expõe o risco do reuso sem monitoramento.

## Limitações

O drift é gerado por sequências discretas e rótulos de qualidade controlados. Não há relógio físico, latência de rede, clock skew, jitter, sensores contínuos ou mudança de regime aprendida autonomamente.

Os limiares `2` e `3` são política experimental, não constantes universais. Em produção eles precisariam ser calibrados por risco, custo de falsa desativação, custo de falsa continuidade e características do sensor.

O monitor detecta persistência de incompatibilidade, mas não identifica a causa semântica do drift. Ele sabe que a evidência deixou de ser confiável, não necessariamente por que isso ocorreu.

O resultado não prova AGI, segurança física geral ou robustez em produção. Ele prova somente hysteresis temporal e bloqueio de propostas em cenários sintéticos definidos.

## Reprodução

```bash
PYTHONPATH=. python3 -m research.asa_round7
PYTHONPATH=. python3 -m unittest research.test_asa_round7
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
./prove.sh --quiet
```

A validação passou com **149 testes Python**, um teste previamente ignorado e nenhum erro. O gate oficial também passou com todas as invariantes.

## Próxima etapa

A Rodada 8 deverá separar tipos de evidência — observação, hipótese, regra e preferência — e testar conflitos entre fontes com prioridade explícita. O objetivo será impedir que uma fonte externa ou uma hipótese não confirmada sobrescreva a policy engine.
