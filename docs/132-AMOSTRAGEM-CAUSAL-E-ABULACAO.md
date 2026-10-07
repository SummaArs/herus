# Amostragem causal para ablações do HERUS

**Progresso algorítmico:** 94,0%  
**Status:** desenho pré-registrado; nenhum resultado causal foi alegado.

## Resposta curta

Sim, amostragem causal pode melhorar o experimento, mas não transforma automaticamente um benchmark em estudo causal.

O uso correto no HERUS é um **desenho de ablação pareada**:

- tratamento: implementação completa do Symbiotic Learning v2;
- controle: componente ou baseline congelado antes da avaliação;
- mesma amostra `example_id` avaliada nos dois lados;
- unidade de incerteza: episódio, não linha individual, quando várias falas pertencem ao mesmo episódio;
- bootstrap por cluster, mantendo o pareamento dentro do episódio.

O estimando é:

```text
média(Y_HERUS_completo - Y_controle)
```

na população fixa S06 sob o protocolo definido. Isso pode ser chamado de **efeito da intervenção implementacional no protocolo**, não de efeito causal universal do HERUS.

## Por que agrupar por episódio

Linhas do mesmo episódio podem compartilhar contexto, roteiro e distribuição linguística. Reamostrar cada linha como se fosse independente estreita artificialmente a incerteza. O bootstrap por episódio preserva essa dependência e fornece uma incerteza mais honesta.

## O que permanece obrigatório

A amostragem causal não corrige:

- ledger incompleto;
- IDs incorretos;
- tratamento não definido;
- vazamento entre treino e holdout;
- seleção pós-hoc de limiar;
- ausência de versão/checkpoint;
- falta de um pipeline HERUS completo.

Por isso o validador retorna `PASS_FOR_DESIGN_ONLY`, nunca um resultado de execução.

## Resultado do manifesto atual

O manifesto S06 fornece a base para agrupar por episódio. A execução ainda não ocorreu porque o identificador do adaptador HERUS completo não foi congelado e o ledger pareado ainda não existe.

Claims bloqueados:

- “a amostragem provou causalidade”;
- “o efeito é universal”;
- “o HERUS é seguro em produção”;
- “o HERUS é SOTA geral”.
