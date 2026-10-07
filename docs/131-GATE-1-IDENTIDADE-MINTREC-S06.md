# Gate 1 — identidade do holdout MIntRec S06

**Status:** identidade materializada; snapshot imutável e pipeline HERUS completo ainda pendentes.  
**Progresso algorítmico:** 94,0%.

## Resultado real

O loader oficial retornou:

- dataset: `THU-IAR/MIntRec`;
- 2.224 linhas totais;
- 386 linhas S06;
- 20 classes no universo MIntRec;
- chave de origem: `season + episode + clip`;
- `example_id`: SHA-256 do JSON canônico contendo `season`, `episode`, `clip`, `label` e `text`.

Checksums produzidos:

```text
source_rows_sha256:      13327559f5b42014a18c2f76d6322aa4b183833932b90b776bc344b3faccacd2
holdout_manifest_sha256: 7034771f34ee2a9e02c7ecba6a4a62e8585f032f3cbb63609ce6e03fa45b8083
```

Não foram detectadas colisões de identidade ou perdas no manifesto materializado.

## O que foi corrigido

O carregador anterior descartava `clip`, o campo real que diferencia amostras dentro do mesmo episódio. O loader agora preserva esse campo, permitindo que o ledger futuro faça união por identidade em vez de posição.

## Limites mantidos

Este é um manifesto de identidade derivada, não um ID imutável fornecido pelo provedor. A API de linhas não expõe nesta resposta uma revisão imutável do conteúdo. Portanto, o artefato melhora a reprodutibilidade, mas não substitui:

- snapshot do dataset;
- checksum do pacote de origem;
- ledger completo de todos os modelos;
- definição congelada do adaptador HERUS completo.

O Gate 1 de identidade está **parcialmente atendido**: já há uma chave canônica e 386 registros auditáveis, mas o Gate 1 completo só passa quando o manifesto for usado no ledger pareado e o snapshot/proveniência do dataset for fixado.
