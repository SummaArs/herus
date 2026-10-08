# Terceiro hospedeiro: Banking77

O seletor universal foi testado em um terceiro domínio independente: Banking77, com 77 classes de intenção bancária e divisão pública treino/teste.

## Protocolo

- 6.966 exemplos para ajuste;
- 3.037 exemplos de calibração;
- calibração dividida em 1.518 exemplos de ajuste e 1.519 de validação;
- 3.080 exemplos de teste preservados como holdout;
- 77 classes;
- nenhuma informação do holdout usada para escolher a política.

## Resultado

| Política | Validação | Holdout |
|---|---:|---:|
| Roteador padrão | 72,15% | **75,23%** |
| Score calibrado | **72,55%** | 75,16% |
| Política escolhida | score calibrado | 75,16% |

A diferença na validação foi de apenas 0,40 ponto percentual e não se sustentou no holdout: a política padrão venceu por 0,06 ponto percentual.

## Veredito científico

Este experimento **não confirma** a generalização universal do seletor. Ele mostra que:

1. o procedimento consegue selecionar uma política diferente por hospedeiro;
2. a seleção funcionou em MIntRec;
3. a seleção empatou e foi conservadora em MInDS-14;
4. no Banking77, uma vantagem pequena na validação não foi robusta no teste independente.

Portanto, o seletor não deve ser promovido a uma regra universal final. O próximo avanço deve incluir uma margem mínima de decisão ou uma regra de abstinência da política: quando a diferença entre políticas é pequena, o HERUS deve manter a política padrão ou declarar que não há evidência suficiente para trocar.

Não há claim de SOTA.
