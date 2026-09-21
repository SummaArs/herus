# Rodada 9 — Regressão real-local, holdout e proveniência

## Conclusão

A Rodada 9 foi desenhada após duas auditorias metodológicas independentes em paralelo: uma auditoria de leakage/proveniência e uma auditoria estatística. As duas chegaram à mesma conclusão central: o corpus de 13 casos é uma fixture de regressão do próprio repositório, não telemetria de produção e não um conjunto externo independente.

O benchmark foi implementado com uma fronteira explícita: `infer_public` recebe somente `mode` e `input`. O avaliador lê `expected` ou `oracle` somente depois que a inferência retorna. IDs, proveniência e labels não entram no payload de inferência.

O resultado final foi:

- regressão local: **13/13 corretos**;
- holdout local novo: **10/10 corretos**;
- precisão seletiva: **100%** nos dois conjuntos;
- recall de casos válidos: **100%** nos dois conjuntos;
- zero decisões inseguras;
- generalização externa: **não estimável**.

A conclusão correta não é “o ASA generaliza para o mundo real”. A conclusão é: **o adaptador passou a ter um benchmark de regressão sem oracle na inferência e um holdout local novo, mas ainda sem validade externa mensurável**.

## Auditorias Wide Research

### Leakage e proveniência

A primeira auditoria identificou que os campos `expected.status`, `expected.eventKind`, `expected.minutes`, `expected.requiresConfirmation` e `expected.publicSos` são labels de avaliação, não evidência. Também identificou que IDs como `arrival`, `cancel`, `unknown`, `negated` e `conflicting` podem revelar a classe e devem permanecer fora da inferência.

A auditoria também confirmou que os testes antigos carregavam o oracle completo, mas não executavam o parser nem faziam avaliação black-box. Portanto, passar aqueles testes não demonstrava isolamento contra leakage.

### Desenho estatístico

A segunda auditoria encontrou apenas 13 casos, sem split train/dev/test, sem unidade independente formal, sem anotação interavaliador e sem métricas de desempenho do sistema. Também encontrou confusão entre família e modalidade: os casos V1/V2 são TEXT e V3 é TYPED_COMMAND.

A recomendação foi tratar o corpus original como regressão e criar um holdout local novo, sem chamá-lo de produção. A auditoria recomendou reportar métricas com denominadores e declarar validade externa como não estimável.

## Protocolo

O corpus original foi usado inteiro como regressão, com sua proveniência preservada:

```text
sourceFile: firmware/core/test_voice.c
purpose: fixtures de regressão
notProductionTelemetry: true
```

O holdout foi escrito após a Rodada 8, com dez casos que não aparecem literalmente no corpus original. Ele contém variantes lexicais novas, bordas de duração, comandos tipados, negação, conflito e domínio desconhecido. O oracle permanece no arquivo do avaliador e não é passado a `infer_public`.

O benchmark registra SHA-256 do corpus de regressão e do holdout. Os IDs opacos (`q01` etc.) são usados somente para auditoria do avaliador.

## Métricas finais

| Conjunto | Casos | Corretos | Acurácia | Cobertura | Recall válido | Precisão seletiva | Abstention segura | Decisões inseguras |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| Regressão local | 13 | **13** | 100% | 38,46% | 100% | 100% | 8 | **0** |
| Holdout local novo | 10 | **10** | 100% | 70% | 100% | 100% | 3 | **0** |

A cobertura menor no corpus de regressão não é uma falha: oito casos são rejeitados ou desconhecidos e não devem virar propostas. A métrica de risco inseguro permanece zero.

O baseline sempre-abstain tem zero propostas. O controle de cópia do oracle prevê sete propostas no holdout, mas é explicitamente marcado como controle metodológico e não é usado na inferência.

## Falha encontrada no primeiro passe

O primeiro passe produziu 9/10 no holdout. O caso que falhou foi:

```text
vou chegar em sessenta minutos
```

O adaptador reconhecia `chego`, `chegando` e `cheguei`, mas não a variante legítima `chegar`. O holdout não foi alterado. A implementação foi corrigida para incluir `chegar`, e os testes passaram novamente com 10/10.

Essa falha é evidência útil: o holdout realmente exerceu pressão sobre o adaptador e não foi apenas um espelho dos casos de construção.

## Contrato de inferência

`infer_public(mode, input)` não consulta:

- `expected`;
- `oracle`;
- `opaque`;
- IDs de caso;
- proveniência;
- labels de família.

Para propostas, o adaptador ainda compila Semantic IR com `authority=PROPOSAL_ONLY`, evidência positiva estruturada e validação de comando tipado. A etapa de firmware continua sendo a autoridade de admissão e confirmação.

## Limitações

O holdout é local e foi criado no mesmo repositório. Apesar de não repetir literalmente as strings de construção, não é um conjunto independente de usuários, sessões, dispositivos ou ambiente operacional.

Não há telemetria de produção, áudio, ASR, sotaques, ruído, linguagem espontânea, variação temporal, anotadores independentes ou avaliação prospectiva. A declaração `notProductionTelemetry=true` permanece verdadeira para ambos os conjuntos.

Com apenas 13 casos de regressão e 10 casos de holdout, não há base para intervalos estatísticos fortes nem para afirmar validade externa. Famílias e modos continuam parcialmente confundidos.

O parser é heurístico e cobre um vocabulário estreito. A cobertura de linguagem não equivale a compreensão geral. O resultado não prova AGI, autonomia geral, produção ou segurança física geral.

## Reprodução

```bash
PYTHONPATH=. python3 -m research.asa_round9
PYTHONPATH=. python3 -m unittest research.test_asa_round9
PYTHONPATH=research python3 -m unittest discover -s research -p 'test_*.py'
./prove.sh --quiet
```

A validação passou com **161 testes Python**, um teste previamente ignorado e nenhum erro. O gate oficial também passou com todas as invariantes.

## Classificação do claim

| Claim | Status |
|---|---|
| Inferência não lê oracle durante a execução | **PROVADO NO ESCOPO** |
| Regressão local sem decisões inseguras | **PROVADO NO ESCOPO** |
| Holdout local novo sem decisões inseguras | **EVIDÊNCIA PARCIAL** |
| Generalização para usuários e dados externos | **NÃO ESTIMADO** |
| Telemetria de produção | **NÃO DISPONÍVEL** |
| Inteligência geral | **NÃO TESTADO / NÃO CLAIMADO** |
| Segurança física geral | **NÃO TESTADO / NÃO CLAIMADO** |

A Rodada 10 deverá separar o executor independente da policy engine e testar schema, autorização, orçamento, replay, timeout, reentrância e falhas parciais.
