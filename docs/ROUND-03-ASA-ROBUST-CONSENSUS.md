# Rodada 3 — ASA com redundância e consenso robusto

## Conclusão

A Rodada 3 demonstrou uma propriedade adicional do ASA: cinco observações independentes podem recuperar um contrato de efeito sob ruído simétrico limitado, enquanto um viés persistente continua sendo rejeitado. Uma réplica defeituosa não alterou o resultado quando as demais quatro observações permaneceram consistentes.

Nos cenários limpo, com ruído simétrico e com uma réplica ruim, o ASA obteve recall de **100% dos cinco eventos válidos**, precisão seletiva de **100%** e **zero decisões inseguras**. Sob viés persistente positivo ou negativo em todas as réplicas, o ASA teve recall de 0%, cobertura 0% e continuou sem decisões inseguras.

O resultado não demonstra percepção robusta em ambiente físico. Ele demonstra que a regra de consenso implementada é conservadora: recupera sinal compatível com o contrato e se abstém quando a mediana não corresponde ao efeito autorizado.

## Hipótese falsificável

> **H3:** A agregação coordenada por mediana recupera contratos de efeito sob ruído simétrico limitado e uma minoria de réplicas defeituosas, mas não deve autorizar um contrato quando todas as réplicas apresentam o mesmo viés.

A hipótese seria falsificada se o viés persistente produzisse uma proposta, se uma única réplica alterasse o resultado com quatro réplicas honestas, ou se o consenso emitisse eventos para casos que exigem abstention.

## Protocolo

Cada cenário utiliza cinco hosts independentes com a mesma especificação semântica e nomes de ações diferentes. A cada host é aplicado um deslocamento determinístico na observação posterior a cada ação. O ASA não recebe o valor verdadeiro nem o rótulo do cenário.

Os efeitos são agregados por chave de estado. Para cada ação, a implementação calcula a mediana dos valores observados e estima suporte pela fração de réplicas dentro de uma distância de uma unidade do centro mediano. Uma ação só é aceita quando o efeito mediano coincide com exatamente um efeito semântico conhecido e o suporte é de pelo menos 60%.

Foram executadas dez seeds por cenário:

| Cenário | Deslocamentos das cinco réplicas |
|---|---|
| `clean_replicates` | `0, 0, 0, 0, 0` |
| `symmetric_bounded_noise` | `-1, 0, +1, 0, -1` |
| `one_bad_replica` | `0, 0, 0, 0, +2` |
| `persistent_positive_bias` | `+1, +1, +1, +1, +1` |
| `persistent_negative_bias` | `-1, -1, -1, -1, -1` |

A avaliação utiliza os 13 casos do corpus local. Cinco casos têm eventos válidos e oito exigem abstention.

## Resultados

| Cenário | Cobertura média | Recall válido | Precisão seletiva | Abstention segura | Decisões inseguras |
|---|---:|---:|---:|---:|---:|
| Réplicas limpas | 5/13 | **100%** | **100%** | 80/80 | **0** |
| Ruído simétrico limitado | 5/13 | **100%** | **100%** | 80/80 | **0** |
| Uma réplica defeituosa | 5/13 | **100%** | **100%** | 80/80 | **0** |
| Viés positivo persistente | 0/13 | 0% | 100% | 80/80 | **0** |
| Viés negativo persistente | 0/13 | 0% | 100% | 80/80 | **0** |

A precisão seletiva de 100% significa que toda proposta emitida coincidiu com o evento esperado. Nos cenários enviesados, não houve propostas para os casos válidos; por isso a precisão seletiva é uma métrica vacuamente segura e não deve ser interpretada como capacidade de reconhecimento.

## Falha encontrada e corrigida

A primeira versão usava igualdade literal para calcular o suporte das réplicas. Como o ruído simétrico produzia observações `0`, `1` e `2` em torno do efeito verdadeiro `1`, apenas duas réplicas eram literalmente iguais ao centro. O sistema rejeitava o consenso válido.

A correção separou duas ideias: a mediana determina o efeito central, enquanto o suporte mede se as réplicas estão dentro de um intervalo declarado de tolerância. O limiar de 60% mantém a rejeição de consenso insuficiente. O teste da Rodada 3 agora protege esse comportamento.

## Limites

O ruído foi criado artificialmente e é independente entre réplicas. O teste não cobre ruído correlacionado, sensores sistematicamente calibrados de forma errada, ataques que controlam a maioria das réplicas, dependências temporais ou estados ocultos.

A tolerância de uma unidade é específica deste domínio discreto. Ela não deve ser transferida para sensores contínuos sem calibração local. Em uma implementação física, o intervalo precisa vir de uma especificação de sensor, e não de um número escolhido depois de observar os resultados.

A campanha ainda é host-only e proposal-only. Nenhum atuador, rádio, ferramenta externa ou credencial foi utilizado. O ASA continua sem demonstrar aprendizagem aberta, generalização de linguagem ou controle físico.

## Reprodução

```bash
PYTHONPATH=. python3 -m research.asa_round3
PYTHONPATH=. python3 -m unittest research.test_asa_round3
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
./prove.sh --quiet
```

A validação final passou com **129 testes Python**, um teste previamente ignorado e nenhum erro. O gate oficial também passou com todas as invariantes do firmware e do host.

## Próxima hipótese

A Rodada 4 deve testar composição temporal: múltiplos passos, pré-condições, efeitos colaterais e rollback. O foco será verificar se a mediana de observações continua segura quando um efeito isolado parece correto, mas a sequência completa viola uma pré-condição ou produz um estado terminal inválido.
