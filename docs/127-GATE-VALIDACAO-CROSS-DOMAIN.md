# Gate de validação cross-domain

## Decisão atual

O benchmark em MInDS-14 é real e útil como diagnóstico de classificação de intenções, mas **não está autorizado como prova de transferência cross-domain do HERUS**.

O gate bloqueou a promoção por dois motivos verificáveis:

1. não há split speaker-independent verificado;
2. os rótulos de MInDS-14 não são eventos HERUS nem possuem contrato de alinhamento semântico independente.

O resultado também não é uma execução do núcleo Symbiotic Learning v2; ele mede baselines de texto e uma política de consenso relacionada. Portanto, não pode ser apresentado como vitória do algoritmo principal.

## Regra

> Um dataset real é necessário, mas não suficiente. Uma alegação cross-domain exige holdout independente, alinhamento de rótulos e avaliação explícita do método HERUS sob protocolo congelado.

## Evidência

O artefato `research/evidence/cross_domain_audit_v1.json` registra a decisão `BLOCK` e seus bloqueadores. Isso preserva o resultado negativo em vez de convertê-lo em marketing.

## Como desbloquear

- fixar um snapshot e uma divisão independente por falante, grupo ou domínio;
- definir antes do teste o mapeamento entre rótulos externos e eventos/efeitos HERUS;
- avaliar o HERUS e os baselines no mesmo holdout;
- arquivar previsões por exemplo, abstenções, custo e versão do código;
- aplicar a mesma calibração sem consultar o holdout;
- manter o gate bloqueado se qualquer contrato não puder ser provado.
