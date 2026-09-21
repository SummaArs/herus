# Rodada 2 — ASA sob observabilidade parcial e ruído

## Conclusão

A Rodada 2 suporta a hipótese de que o ASA pode trocar cobertura por abstention quando a evidência observável fica incompleta ou corrompida, sem transformar incerteza em uma decisão insegura.

O teste usa os 13 casos locais da Rodada 1 e hospedeiros sintéticos com especificação independente da projeção observacional. O ASA recebe apenas observações projetadas. O avaliador conhece o estado completo do hospedeiro, mas esse gabarito nunca é entregue ao ASA.

Com todos os campos visíveis e sem ruído, o ASA alcançou **recall de 100% nos cinco eventos válidos**, com precisão seletiva de 100% e zero decisões inseguras. Com apenas um campo visível, o recall caiu para 60%. Com dois campos, caiu para 80%. Com ruído que altera cada observação de efeito, a cobertura caiu a zero. Em todos os cenários, a precisão seletiva permaneceu em 100% e o número de decisões não seguras foi zero.

A conclusão é limitada: o ASA demonstrou uma política de **abster quando o contrato não pode ser identificado com segurança**. Isto não prova robustez de percepção, aprendizagem probabilística ou operação física.

## Hipótese falsificável

> **H2:** Sob observabilidade parcial ou observações corrompidas, a transferência estrita por contrato de efeito deve reduzir a cobertura, mas preservar precisão seletiva e manter zero decisões não seguras.

A hipótese seria falsificada se o ASA emitisse um evento que não estivesse sustentado pelo contrato observável, se o ruído produzisse um mapeamento incorreto, ou se ele usasse campos ocultos do estado completo.

## Protocolo

O hospedeiro contém três efeitos independentes: `ARRIVE`, `HELP` e `CANCEL`. Cada hospedeiro-alvo recebe nomes diferentes para suas primitivas. A especificação dos efeitos é definida no gerador do host, enquanto a projeção parcial é aplicada por um wrapper separado.

O wrapper pode ocultar chaves do estado e adicionar um deslocamento determinístico aos valores observados. O ASA nunca acessa o hospedeiro interno nem o estado completo. Ele somente recebe a observação projetada, descobre ações e constrói um mapa quando existe exatamente uma correspondência de efeito.

Foram executadas dez seeds em cada cenário:

| Cenário | Visibilidade | Ruído |
|---|---|---|
| `full_clean` | `arrive`, `help`, `cancel` | ausente |
| `partial_one_key` | somente `arrive` | ausente |
| `partial_two_keys` | `arrive`, `help` | ausente |
| `noisy_all_keys` | todas as chaves | deslocamento de 1 em toda observação de efeito |

O corpus local contém cinco casos com evento válido e oito casos que exigem ausência de evento: desconhecido, conflito, negação ou entrada fora do domínio.

## Métricas

**Recall de eventos válidos** mede a fração dos cinco eventos válidos que o ASA recupera. **Cobertura** mede a fração de todos os 13 casos que recebe uma proposta, portanto inclui os oito casos que devem permanecer em abstention. **Precisão seletiva** mede a correção entre as propostas que o ASA decidiu emitir. **Abstention segura** conta casos sem evento esperado nos quais o ASA não emitiu evento. **Decisão insegura** conta qualquer evento emitido para um caso sem evento esperado.

## Resultados

| Cenário | Cobertura média | Recall válido | Precisão seletiva | Abstention segura | Decisões inseguras |
|---|---:|---:|---:|---:|---:|
| Observação completa e limpa | 5/13 | 100% | 100% | 80/80 | **0** |
| Apenas `arrive` visível | 3/13 | 60% | 100% | 80/80 | **0** |
| `arrive` e `help` visíveis | 4/13 | 80% | 100% | 80/80 | **0** |
| Todas as chaves com ruído | 0/13 | 0% | 100% | 80/80 | **0** |

A cobertura inclui dez repetições por cenário. O total de 80 abstentions seguras por cenário corresponde aos oito negativos em cada uma das dez seeds.

O resultado mais importante não é o recall limpo, mas a combinação entre queda previsível de cobertura e risco inseguro igual a zero. Em um sistema de proposta, esse comportamento é preferível a manter cobertura por meio de adivinhação.

## Limitações e falha encontrada

A primeira implementação da Rodada 2 produziu cobertura zero inclusive no cenário limpo. A investigação mostrou um erro de orientação de variáveis no loop que construía o mapa de efeitos. O problema foi corrigido antes da validação final. Esse episódio foi preservado como evidência de que o teste detectou uma falha real do harness.

A versão atual usa ruído determinístico e estados discretos. Ela não modela sensores contínuos, ruído correlacionado, atrasos, oclusão temporal, estados ocultos ou efeitos colaterais. O parser de linguagem também permanece finito e baseado no contrato de regressão existente.

O cenário ruidoso atual é deliberadamente severo: o deslocamento altera todos os efeitos observados e elimina a possibilidade de mapeamento estrito. A próxima rodada deve testar ruído graduado, múltiplas amostras, intervalos de incerteza e calibração sem transformar uma média observada em autorização automática.

## Reprodução

```bash
PYTHONPATH=. python3 -m research.asa_round2
PYTHONPATH=. python3 -m unittest research.test_asa_round2
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
./prove.sh --quiet
```

A validação final passou com **124 testes Python**, um teste previamente ignorado e nenhum erro. O gate oficial de firmware também passou com todas as invariantes do host. A implementação permanece em modo `proposal-only`; nenhuma ação externa é executada.

## Próxima hipótese

A Rodada 3 deve testar se o ASA consegue recuperar cobertura sob ruído moderado usando múltiplas observações independentes e intervalos de confiança, mantendo uma política de risco explícita. O critério de aprovação deve exigir que a cobertura aumente sem aumentar decisões inseguras, e deve incluir calibração, casos adversariais e um baseline sempre-abstain.
